#!/usr/bin/env python3
"""Qualify original AUI map member pointers; dry-run unless --apply is given."""
import re
from pathlib import Path

begin = re.compile(r'^\s*AUI_BEGIN_(COMMAND|EVENT)_MAP(?:_BASE)?\s*\(\s*(\w+)')
end = re.compile(r'^\s*AUI_END_(COMMAND|EVENT)_MAP\s*\(')
entry = re.compile(r'^\s*AUI_ON_(COMMAND|EVENT)\s*\(')
last_arg = re.compile(r',\s*([A-Za-z_]\w*(?:::\w+)?)\s*\)\s*(?://.*)?$')

def code_mask(line, block):
    out = list(line)
    i = 0
    quote = None
    while i < len(line):
        if block:
            out[i] = ' '
            if line[i:i+2] == '*/':
                if i+1 < len(line): out[i+1] = ' '
                i += 2; block = False
            else: i += 1
        elif quote:
            ch = line[i]; out[i] = ' '
            if ch == '\\' and i+1 < len(line): out[i+1] = ' '; i += 2
            elif ch == quote: quote = None; i += 1
            else: i += 1
        elif line[i:i+2] == '//':
            for j in range(i,len(line)): out[j] = ' '
            break
        elif line[i:i+2] == '/*':
            out[i:i+2] = [' ',' ']; i += 2; block = True
        elif line[i] in ('\"', "'"):
            quote = line[i]; out[i] = ' '; i += 1
        else: i += 1
    return ''.join(out), block

def transform(text):
    active = {}; block = False; out = []; changes = 0
    for line in text.splitlines(keepends=True):
        mask, block = code_mask(line, block)
        m = begin.match(mask)
        if m: active[m.group(1)] = m.group(2)
        m = end.match(mask)
        if m: active.pop(m.group(1), None)
        m = entry.match(mask)
        if m:
            kind = m.group(1)
            if kind not in active: raise ValueError('map entry without active map: ' + line.strip())
            arg = last_arg.search(mask.rstrip('\r\n'))
            if not arg: raise ValueError('unrecognized map entry: ' + line.strip())
            name = arg.group(1)
            if '::' not in name:
                i = arg.start(1)
                line = line[:i] + active[kind] + '::' + line[i:]
                changes += 1
        out.append(line)
    return ''.join(out), changes

if __name__ == '__main__':
    import sys
    apply = '--apply' in sys.argv
    changed_files = changed_entries = 0
    for p in Path('src/client153/Element').rglob('*'):
        if p.suffix.lower() not in ('.cpp', '.cxx'): continue
        text = p.read_bytes().decode('utf-8')
        updated, count = transform(text)
        if count:
            changed_files += 1; changed_entries += count
            if apply: p.write_bytes(updated.encode('utf-8'))
    print(f'{changed_entries} entries in {changed_files} files; applied={apply}')
