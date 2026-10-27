// Cloudflare Worker + Durable Object fronting the PW 1.5.3 server container.
//
// Platform model (docs/DECISIONS/ADR-0006 and the Cloudflare Containers
// docs): containers receive traffic ONLY through the Worker/DO mediation
// layer — raw TCP from external game clients is not the supported inbound
// path. This Worker exposes:
//   GET /health — HTTP health of the daemons inside the container.
//   GET /probe  — proves the container's glinkd:29000 accepts TCP from the
//                 mediation layer (getTcpPort().connect()).

import { DurableObject } from "cloudflare:workers";

export class Server153 extends DurableObject {
  constructor(ctx, env) {
    super(ctx, env);
  }

  async fetch(request) {
    const url = new URL(request.url);
    try {
      if (url.pathname === "/health") {
        const port = this.getTcpPort(8080);
        return await port.fetch("http://container/health");
      }
      if (url.pathname === "/probe") {
        const port = this.getTcpPort(8080);
        return await port.fetch("http://container/probe");
      }
      return new Response("not found", { status: 404 });
    } catch (e) {
      return new Response(
        JSON.stringify({
          error: e.message || String(e),
          hasGetTcpPort: typeof this.getTcpPort,
        }),
        { status: 500, headers: { "Content-Type": "application/json" } }
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
