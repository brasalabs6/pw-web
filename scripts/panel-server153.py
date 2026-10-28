#!/usr/bin/env python3
"""pw-web server153 — painel simples de contas (dev-auth).

Serve a small web panel on 127.0.0.1:8080:
  GET  /         — server status + account creation form.
  POST /create  — validates/generates a numeric account (multiple of 16,
                  as the gauthd dev-auth requires) and shows the credentials.

The dev-auth scheme (src/server153/cnet/gauthd/gquerypasswd.hrp and
matrixpasswd[2].hrp): accounts are numeric multiples of 16 and the stored
hash is MD5(account||account) — the password equals the account name.
A real account source (MySQL authd) is a separate milestone.
"""
import http.server
import json
import socket
import urllib.parse

HOST, PORT = "127.0.0.1", 8080
SERVER_HOST, SERVER_PORT = "127.0.0.1", 29000

PAGE = """<!doctype html>
<html lang="pt-BR"><head><meta charset="utf-8">
<title>pw-web — servidor 1.5.3</title>
<style>
 body {{ font-family: system-ui, sans-serif; margin: 3rem auto; max-width: 34rem;
        background: #10141a; color: #d7dde6; }}
 h1 {{ font-size: 1.4rem; }} .ok {{ color: #5cd985; }} .bad {{ color: #ff7b72; }}
 form {{ background: #171c24; padding: 1.2rem 1.4rem; border-radius: 8px; }}
 input, button {{ font-size: 1rem; padding: .45rem .7rem; border-radius: 6px;
   border: 1px solid #2c3644; background: #10141a; color: #d7dde6; }}
 button {{ background: #1f6feb; border-color: #1f6feb; cursor: pointer; }}
 .cred {{ background: #171c24; padding: 1rem 1.4rem; border-radius: 8px; margin-top: 1rem; }}
 code {{ background: #0b0e13; padding: .15rem .4rem; border-radius: 4px; }}
</style></head><body>
<h1>Perfect World 1.5.3 — servidor local</h1>
<p>Status do servidor (glinkd 127.0.0.1:29000):
 <span class="{status_cls}">{status}</span></p>
<form method="post" action="/create">
 <label>Conta desejada (número múltiplo de 16 — ex.: 1000000):
  <input name="account" inputmode="numeric" placeholder="1000000" size="14"></label>
 <button type="submit">Criar / validar conta</button>
 <button type="submit" name="auto" value="1">Gerar conta para mim</button>
</form>
{result}
<p style="color:#8b98a5;font-size:.85rem">Esquema dev-auth desta árvore: contas
numéricas múltiplas de 16; a senha é o próprio número da conta. Um backend real
de contas (authd MySQL) é milestone separado.</p>
</body></html>"""


def server_up():
    try:
        with socket.create_connection((SERVER_HOST, SERVER_PORT), timeout=2):
            return True
    except OSError:
        return False


def next_free_account():
    """First multiple of 16 >= 1000000 whose name is not obviously taken.

    The dev-auth has no persistent account store, so any multiple of 16
    works; we simply avoid the couple of well-known test accounts."""
    n = 1000000
    while n in (1000000,):
        n += 16
    return n


class Panel(http.server.BaseHTTPRequestHandler):
    def _send(self, html):
        body = html.encode()
        self.send_response(200)
        self.send_header("Content-Type", "text/html; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        up = server_up()
        self._send(PAGE.format(
            status="online ✓" if up else "offline ✗",
            status_cls="ok" if up else "bad",
            result=""))

    def do_POST(self):
        length = int(self.headers.get("Content-Length", 0))
        form = urllib.parse.parse_qs(self.rfile.read(length).decode())
        up = server_up()
        result_html = ""
        if form.get("auto", [""])[0] == "1":
            account = next_free_account()
        else:
            raw = form.get("account", [""])[0].strip()
            try:
                account = int(raw)
            except ValueError:
                account = None
        if account is None:
            result_html = ('<div class="cred"><span class="bad">Conta inválida:'
                           ' use apenas números múltiplos de 16.</span></div>')
        elif account <= 0 or account % 16 != 0:
            result_html = ('<div class="cred"><span class="bad">Conta inválida:'
                           ' precisa ser múltiplo de 16 (ex.: 1000000).</span></div>')
        else:
            result_html = (
                '<div class="cred"><p><span class="ok">Conta pronta:</span></p>'
                f'<p>Conta: <code>{account}</code><br>Senha: <code>{account}</code></p>'
                '<p style="font-size:.9rem;color:#8b98a5">Abra o cliente, selecione '
                '"Local Test (dev)" e faça login com essas credenciais.</p></div>')
        self._send(PAGE.format(
            status="online ✓" if up else "offline ✗",
            status_cls="ok" if up else "bad",
            result=result_html))

    def log_message(self, *args):
        pass


if __name__ == "__main__":
    print(f"pw-web server153 panel: http://{HOST}:{PORT}")
    http.server.HTTPServer((HOST, PORT), Panel).serve_forever()
