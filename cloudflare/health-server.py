#!/usr/bin/env python3
"""Health endpoint for the Cloudflare container: reports the PW daemons' state.

GET /health  -> 200 {"ok": true, "daemons": [...]} when all 8 daemons are alive,
                 503 with the surviving subset otherwise.
GET /probe   -> opens a TCP connection to glinkd (29000) inside the container
                 and reports whether the GNET listener accepts connections.
"""
import http.server
import json
import os
import socket

DAEMONS = ["gauthd", "gdeliveryd", "gamedbd", "uniquenamed",
           "gfactiond", "gacd", "logservice", "glinkd"]


def daemons_alive():
    comms = set()
    for pid in os.listdir("/proc"):
        if not pid.isdigit():
            continue
        try:
            with open("/proc/%s/comm" % pid) as fh:
                comms.add(fh.read().strip())
        except OSError:
            pass
    return [d for d in DAEMONS if d in comms]


def glinkd_accepts():
    try:
        with socket.create_connection(("127.0.0.1", 29000), timeout=3):
            return True
    except OSError:
        return False


class Handler(http.server.BaseHTTPRequestHandler):
    def _send(self, code, payload):
        body = json.dumps(payload).encode()
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        if self.path == "/health":
            alive = daemons_alive()
            ok = len(alive) == len(DAEMONS)
            self._send(200 if ok else 503,
                       {"ok": ok, "daemons": alive, "expected": len(DAEMONS)})
        elif self.path == "/probe":
            self._send(200, {"glinkd_29000_accepts": glinkd_accepts()})
        else:
            self._send(404, {"error": "not found"})

    def log_message(self, *args):
        pass


if __name__ == "__main__":
    http.server.HTTPServer(("0.0.0.0", 8080), Handler).serve_forever()
