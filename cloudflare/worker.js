// Cloudflare Worker + Durable Object fronting the PW 1.5.3 server container.
//
// Platform model (docs/DECISIONS/ADR-0006 and the Cloudflare Containers
// docs): containers receive traffic ONLY through the Durable Object Container
// API (ctx.container.getTcpPort(port)) — raw TCP from external game clients
// is not the supported inbound path. Endpoints:
//   GET /health   — HTTP health of the daemons inside the container
//                    (proxied over getTcpPort(8080).fetch()).
//   GET /probe    — the container's internal glinkd:29000 TCP accept check.
//   GET /tcpprobe — proves the MEDIATION TCP path the game-client gateway
//                    will use: getTcpPort(29000).connect() from this DO.
//
// The container boots the 8 daemons on start (~30-60s); requests poll
// readiness instead of failing on the first attempt.

import { DurableObject } from "cloudflare:workers";

const INACTIVITY_TIMEOUT_MS = 30 * 60 * 1000;
const HEALTH_PORT = 8080;
const GLINKD_PORT = 29000;

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
      // `images` map (key "default" in wrangler.jsonc). The instance size is
      // omitted -> defaults to "lite" (the valid named sizes for start() are
      // lite and standard-1..4; "basic" is a wrangler-config-only name and
      // makes start() throw a TypeError).
      container.start({
        image: container.images.default,
        enableInternet: false,
      });
      await container.setInactivityTimeout(INACTIVITY_TIMEOUT_MS);
    }
    return container;
  }

  async fetch(request) {
    const url = new URL(request.url);
    try {
      const container = await this.ensureRunning();

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

      // /health and /probe: HTTP via the mediation layer to the container's
      // health server. Cold start (image pull + VM boot + 8 daemons) exceeds
      // the edge's ~180s hold limit, so a request that holds until readiness
      // is always cut off. Try a few short fetches; if the container is not
      // ready yet, answer 503 + Retry-After and let the client poll.
      const port = container.getTcpPort(HEALTH_PORT);
      let lastError;
      for (let attempt = 0; attempt < 3; attempt++) {
        try {
          const response = await port.fetch("http://container" + url.pathname);
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
