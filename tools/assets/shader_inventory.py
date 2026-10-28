#!/usr/bin/env python3
"""Summarize Direct3D shader profiles and instructions in a local PCK."""
import argparse
from collections import Counter, defaultdict
import json
from pathlib import Path
import re

PROFILE = re.compile(r'^(vs|ps)[._](\d+)[._](\d+)$', re.IGNORECASE)
INSTRUCTION = re.compile(r'^\+?([a-z][a-z0-9_]*)\b', re.IGNORECASE)


def analyze(data):
    """Return a profile and opcode counts for assembly, or None for other assets."""
    text = data.decode('latin-1')
    profile = None
    counts = Counter()
    for raw in text.splitlines():
        line = raw.split(';', 1)[0].split('//', 1)[0].strip()
        if not line:
            continue
        match = PROFILE.fullmatch(line)
        if not profile:
            if match:
                profile = f'{match.group(1).lower()}.{match.group(2)}.{match.group(3)}'
            continue
        match = INSTRUCTION.match(line)
        if match:
            counts[match.group(1).lower()] += 1
    return (profile, dict(counts)) if profile else None


def main():
    from pck153 import Package

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('package', type=Path)
    args = parser.parse_args()
    profiles = Counter()
    instructions = defaultdict(Counter)
    with Package(args.package) as package:
        for entry in package.entries:
            if not entry.path.lower().endswith(('.txt', '.vsh', '.psh')):
                continue
            result = analyze(package.read(entry))
            if result is None:
                continue
            profile, opcodes = result
            profiles[profile] += 1
            instructions[profile].update(opcodes)
    print(json.dumps({'profiles': dict(sorted(profiles.items())),
                      'instructions': {p: dict(sorted(v.items()))
                                       for p, v in sorted(instructions.items())}}, indent=2))


if __name__ == '__main__':
    main()
