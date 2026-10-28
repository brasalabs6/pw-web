// Cloudflare Worker + Durable Object fronting the PW 1.5.3 server container.
//
// Platform model (docs/DECISIONS/ADR-0006 and the Cloudflare Containers
// docs): containers receive traffic ONLY through the Durable Object Container
// API (ctx.container.getTcpPort(port)) — raw TCP from external game clients
// is not the supported inbound path. Endpoints (whitelisted; anything else
// is 404):
//   GET /health            — DYNAMIC health of the daemons + gs (world server)
//                            inside the container, proxied to the busybox
//                            CGI /cgi-bin/health.sh over getTcpPort(8080).
//   GET /cgi-bin/health.sh — same CGI, direct path.
//   GET /tcpprobe          — proves the MEDIATION TCP path the game-client
//                            gateway will use: getTcpPort(29000).connect().
//
// The container boots the 8 daemons + the gs (world load) on start; cold
// start takes a few minutes (image pull + VM boot + data load). Requests
// poll readiness instead of failing on the first attempt.

import { DurableObject } from "cloudflare:workers";

const INACTIVITY_TIMEOUT_MS = 30 * 60 * 1000;
const HEALTH_PORT = 8080;
const GLINKD_PORT = 29000;

// Instance sizing: the gs (world server) needs real RAM. Measured RSS on the
// local runtime: is61 (parallel world, default) ~650MB + daemons ~50-100MB;
// "gs01 is61" (adds the main world) ~1.9GB total. The custom instance form
// is { vcpu, memoryMib, diskMb } (Durable Object Container API docs).
const INSTANCE = { vcpu: 2, memoryMib: 2048, diskMb: 8192 };
// Worlds the gs boots (entrypoint env). "is61" = where new characters spawn.
// For "gs01 is61" raise memoryMib to ~3072.
const GS_INSTANCES = "is61";

const ALLOWED_PATHS = new Set([
  "/health",
  "/cgi-bin/health.sh",
  "/tcpprobe",
  "/worldcheck",
]);

export class Server153 extends DurableObject {
  constructor(ctx, env) {
    super(ctx, env);
    const container = ctx.container;
    if (container?.running) {
      void ctx.blockConcurrencyWhile(() =>
        container.setInactivityTimeout(INACTIVITY_TIMEOUT_MS),
      );
    }
  }

  async ensureRunning() {
    const container = this.ctx.container;
    if (!container) {
      throw new Error("no container configured for this Durable Object");
    }
    if (!container.running) {
      // durable_object scheduling policy: pass the image from the wrangler
      // `images` map (key "default" in wrangler.jsonc). Instance size is a
      // custom object ({vcpu, memoryMib, diskMb}); named sizes are
      // lite/standard-1..4. GS_INSTANCES reaches the entrypoint as a
      // container env var.
      container.start({
        image: container.images.default,
        enableInternet: false,
        instance: INSTANCE,
        env: { GS_INSTANCES },
      });
      await container.setInactivityTimeout(INACTIVITY_TIMEOUT_MS);
    }
    return container;
  }

  async fetch(request) {
    const url = new URL(request.url);
    try {
      const container = await this.ensureRunning();

      if (!ALLOWED_PATHS.has(url.pathname)) {
        return new Response(JSON.stringify({ error: "not found" }), {
          status: 404,
          headers: { "Content-Type": "application/json" },
        });
      }

      if (url.pathname === "/tcpprobe") {
        // Mediation-layer TCP to glinkd: the path a Workers-based game
        // gateway (ADR-0006 phase 1) will relay client sessions through.
        const conn = container.getTcpPort(GLINKD_PORT).connect("127.0.0.1:29000");
        await conn.opened;
        conn.close();
        return new Response(
          JSON.stringify({ glinkd_29000_mediation_tcp: true }),
          { status: 200, headers: { "Content-Type": "application/json" } },
        );
      }

      if (url.pathname === "/worldcheck") {
        // Direct proof that a world is registered: exec a fixed command in
        // the container and return the gdeliveryd provider-announce lines
        // ("gameserver 101 ... worldtag is 161") plus the gs process count.
        // The command is constant — no user input reaches the shell.
        const process = await container.exec([
          "bash",
          "-c",
          "grep -a 'worldtag is' /opt/state/logs/gdeliveryd.log | tail -3; " +
            "n=0; for p in /proc/[0-9]*/comm; do " +
            "read c < \"$p\" 2>/dev/null; " +
            '[ "$c" = gs-ours ] && { st=$(grep -m1 "^State:" "${p%/comm}/status" | awk \'{print $2}\'); [ "$st" != Z ] && n=$((n+1)); }; ' +
            "done; echo gs_alive=$n",
        ]);
        const output = await process.output();
        const text = new TextDecoder().decode(output.stdout);
        const announce = text
          .split("\n")
          .filter((l) => l.includes("worldtag is"));
        const gsAlive = (text.match(/gs_alive=(\d+)/) || [])[1] || "?";
        return new Response(
          JSON.stringify({
            worlds_registered: announce,
            gs_alive: parseInt(gsAlive, 10) || 0,
            world_up: announce.length > 0 && parseInt(gsAlive, 10) > 0,
          }),
          { status: 200, headers: { "Content-Type": "application/json" } },
        );
      }

      // /health is the DYNAMIC daemon+gs count served by the busybox CGI
      // (a dead daemon or a dead gs flips ok:false — the old static
      // {"ok":true} file masked every failure). Cold start (image pull +
      // VM boot + daemons + gs world load) exceeds the edge's ~180s hold
      // limit, so a request that holds until readiness is always cut off.
      // Try a few short fetches; if the container is not ready yet, answer
      // 503 + Retry-After and let the client poll.
      const target =
        url.pathname === "/health" ? "/cgi-bin/health.sh" : url.pathname;
      const port = container.getTcpPort(HEALTH_PORT);
      let lastError;
      for (let attempt = 0; attempt < 3; attempt++) {
        try {
          const response = await port.fetch("http://container" + target);
          if (!response.ok) {
            throw new Error(`container returned ${response.status}`);
          }
          return response;
        } catch (e) {
          lastError = e;
          await scheduler.wait(2000);
        }
      }
      return new Response(
        JSON.stringify({
          starting: true,
          detail: lastError?.message || String(lastError),
        }),
        {
          status: 503,
          headers: {
            "Content-Type": "application/json",
            "Retry-After": "15",
          },
        },
      );
    } catch (e) {
      return new Response(
        JSON.stringify({
          error: e.message || String(e),
          hasCtxContainer: typeof this.ctx.container,
          hasGetTcpPort: typeof this.ctx.container?.getTcpPort,
        }),
        { status: 500, headers: { "Content-Type": "application/json" } },
      );
    }
  }
}

export default {
  async fetch(request, env) {
    const id = env.SERVER153.idFromName("singleton");
    const stub = env.SERVER153.get(id);
    return stub.fetch(request);
  },
};
