#!/usr/bin/env python3
"""Compare default RoleInfo bytes from actual client and server definitions.

Returns 1 on a wire mismatch; this does not prove complete protocol compatibility.
The server worktree is read-only and requires its 32-bit toolchain.
"""
import argparse
import json
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference-root', type=Path, required=True)
    parser.add_argument('--server-root', type=Path, required=True)
    parser.add_argument('--emxx', required=True)
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--node', default='node')
    args = parser.parse_args()
    reference = args.reference_root.resolve()
    server = args.server_root.resolve()
    source = ROOT / 'tests/client153/roleinfo_wire.cpp'
    with tempfile.TemporaryDirectory(prefix='pw153-role-wire-') as tmp:
        temp = Path(tmp)
        report = temp / 'probe.json'
        probe = [sys.executable, str(ROOT / 'scripts/probe-client153-project.py'),
                 '--source', str(reference), '--emxx', args.emxx,
                 '--port-root', str(ROOT / 'src/client153/Angelica2'),
                 '--adapted-root', str(ROOT / 'src/client153'), '--entry', 'ElementClient.cpp',
                 '--output', str(report)]
        subprocess.run(probe, cwd=ROOT, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False)
        if not report.is_file():
            raise RuntimeError('client project probe did not produce a report')
        command = json.loads(report.read_text())['probe']['command']
        command = [arg for arg in command if arg not in ('-fsyntax-only', '-ferror-limit=5')]
        matches = [i for i, arg in enumerate(command) if arg.endswith('/ElementClient.cpp')]
        if len(matches) != 1:
            raise RuntimeError('could not identify the client entry in the probe command')
        command[matches[0]] = str(source)
        client_js = temp / 'client.js'
        subprocess.run(command + ['-fexceptions', '-sENVIRONMENT=node', '-o', str(client_js)], cwd=ROOT, check=True)
        server_dirs = ['share', 'share/io', 'share/common', 'share/perf', 'share/rpc', 'cnet/rpcdata', 'cnet/inl']
        server_base = server / 'src/server153'
        server_exe = temp / 'server'
        server_command = [args.cxx, '-w', '-m32', '-std=gnu++11', '-fpermissive',
                          *(f'-I{server_base / directory}' for directory in server_dirs),
                          str(source), str(server_base / 'share/common/octets.cpp'), '-o', str(server_exe)]
        subprocess.run(server_command, cwd=server, check=True)
        client = bytes.fromhex(subprocess.check_output([args.node, str(client_js)], text=True).strip())
        server_bytes = bytes.fromhex(subprocess.check_output([str(server_exe)], text=True).strip())
        offset = next((i for i, (a, b) in enumerate(zip(client, server_bytes)) if a != b),
                      min(len(client), len(server_bytes)))
        matches = client == server_bytes
        print(json.dumps({'match': matches, 'clientBytes': len(client), 'serverBytes': len(server_bytes),
                          'firstDifference': None if matches else offset}))
        return 0 if matches else 1


if __name__ == '__main__':
    sys.exit(main())
