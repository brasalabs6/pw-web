#!/usr/bin/env python3
"""WebSocket-to-TCP bridge for the browser client's GNET transport.

The browser client's POSIX socket seam runs on Emscripten's SOCKFS, which
bridges TCP sockets to WebSockets (connect() opens ws://host:port/ and the
payload travels as binary frames). The PW 1.5.3 link server (glinkd) speaks
raw TCP on 29000, so the browser needs this bridge: one WebSocket listener
that pipes binary frames to a TCP peer, both directions, one connection at
a time per WebSocket.

Usage: client153-ws-tcp-bridge.py [LISTEN_PORT] [TARGET_HOST] [TARGET_PORT]
Defaults: 29001 127.0.0.1 29000.
"""

import asyncio
import sys

import websockets


async def pipe_ws_to_tcp(ws, reader, writer):
    async def ws_to_tcp():
        try:
            async for message in ws:
                if isinstance(message, str):
                    message = message.encode('utf-8')
                writer.write(message)
                await writer.drain()
        except Exception:
            pass
        finally:
            try:
                writer.close()
            except Exception:
                pass

    async def tcp_to_ws():
        try:
            while True:
                data = await reader.read(65536)
                if not data:
                    break
                await ws.send(data)
        except Exception:
            pass
        finally:
            try:
                await ws.close()
            except Exception:
                pass

    await asyncio.gather(ws_to_tcp(), tcp_to_ws())


async def handle(ws):
    host, port = BRIDGE_TARGET
    try:
        reader, writer = await asyncio.open_connection(host, port)
    except Exception as error:
        print(f'bridge: tcp connect {host}:{port} failed: {error}', flush=True)
        return
    peer = ws.remote_address
    print(f'bridge: ws {peer[0]}:{peer[1]} -> tcp {host}:{port}', flush=True)
    try:
        await pipe_ws_to_tcp(ws, reader, writer)
    finally:
        print(f'bridge: ws {peer[0]}:{peer[1]} closed', flush=True)


BRIDGE_TARGET = ('127.0.0.1', 29000)


async def main():
    listen_port, target_host, target_port = 29001, '127.0.0.1', 29000
    if len(sys.argv) > 1:
        listen_port = int(sys.argv[1])
    if len(sys.argv) > 2:
        target_host = sys.argv[2]
    if len(sys.argv) > 3:
        target_port = int(sys.argv[3])
    BRIDGE_TARGET = (target_host, target_port)
    globals()['BRIDGE_TARGET'] = BRIDGE_TARGET
    # The browser's socket layer (Emscripten SOCKFS) offers the
    # "binary" subprotocol in its handshake; the server must select it
    # or the browser fails the handshake right after connect and the
    # client's first recv returns 0 (the peer-closed read).
    async with websockets.serve(handle, '0.0.0.0', listen_port,
                                subprotocols=['binary'],
                                max_size=None):
        print(f'bridge: ws listen 0.0.0.0:{listen_port} -> tcp '
              f'{target_host}:{target_port}', flush=True)
        await asyncio.Future()


if __name__ == '__main__':
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        pass
