"""Bounded read-only port of the observed Angelica 1.5.3 PCK v0x20002 layout.

Derived from AFilePackGame.cpp/AFilePackBase.cpp in client-153 at
 e3d6d0075d0a745c9f9859f2cb25b01d71dec045.
Original: Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
Use authorized by docs/LEGAL.md (2026-10-25). No upstream files are modified.
"""
from __future__ import annotations

import argparse
from dataclasses import dataclass
import hashlib
import json
from pathlib import Path
import struct
import zlib

SPLIT_BYTES = 0x7FFFFF00
MASK = 0xA8937462
CHECK_MASK = 0x59374231
HEADER_BYTES = 272
ENTRY_BYTES = 276
MAX_ASSET_BYTES = 512 * 1024 * 1024


class FormatError(ValueError):
    pass


class LogicalFile:
    def __init__(self, path):
        self.path = Path(path)
        self.first = self.path.open('rb')
        self.second = None
        self.first_size = self.path.stat().st_size
        self.size = self.first_size
        continuation = self.path.with_suffix('.pkx')
        try:
            if continuation.exists():
                if self.first_size != SPLIT_BYTES:
                    raise FormatError('PCK continuation has an invalid split boundary')
                self.second = continuation.open('rb')
                self.size += continuation.stat().st_size
        except BaseException:
            self.close()
            raise

    def read_at(self, offset, size):
        if offset < 0 or size < 0 or offset + size > self.size:
            raise FormatError(f'package range outside input: offset={offset}, bytes={size}')
        result = b''
        if offset < self.first_size:
            self.first.seek(offset)
            count = min(size, self.first_size - offset)
            result = self.first.read(count)
            if len(result) != count:
                raise FormatError('truncated PCK')
            offset += count
            size -= count
        if size:
            if self.second is None:
                raise FormatError('missing PKX continuation')
            self.second.seek(offset - self.first_size)
            tail = self.second.read(size)
            if len(tail) != size:
                raise FormatError('truncated PKX')
            result += tail
        return result

    def close(self):
        self.first.close()
        if self.second:
            self.second.close()

    def __enter__(self):
        return self

    def __exit__(self, *unused):
        self.close()


@dataclass(frozen=True)
class Entry:
    path: str
    name_hex: str
    offset: int
    size: int
    stored_size: int


def inflate(data, expected):
    decoder = zlib.decompressobj()
    try:
        result = decoder.decompress(data, expected + 1)
    except zlib.error as exc:
        raise FormatError(f'invalid zlib stream: {exc}') from exc
    if len(result) != expected or not decoder.eof or decoder.unused_data or decoder.unconsumed_tail:
        raise FormatError('zlib output length or stream boundary mismatch')
    return result


def asset_path(raw):
    try:
        name = raw.decode('cp936').replace('\\', '/')
    except UnicodeDecodeError as exc:
        raise FormatError('invalid CP936 package filename') from exc
    parts = name.split('/')
    if not name or name.startswith('/') or ':' in name or any(p in ('', '.', '..') for p in parts) or '\x00' in name:
        raise FormatError('unsafe package asset path')
    return name


class Package:
    def __init__(self, path):
        self.file = LogicalFile(path)
        try:
            self._load_index()
        except BaseException:
            self.file.close()
            raise

    def _load_index(self):
        tag1, end, tag2 = struct.unpack('<III', self.file.read_at(0, 12))
        if (tag1, tag2) != (0x4DCA23EF, 0x56A089B7) or end < HEADER_BYTES + 20 or end > self.file.size:
            raise FormatError('invalid safe header or truncated package')
        count, self.version = struct.unpack('<II', self.file.read_at(end - 8, 8))
        if self.version != 0x20002:
            raise FormatError(f'unsupported package version: {self.version:#x}')
        header_start = end - HEADER_BYTES - 8
        guard0, version, masked_offset, flags, description, guard1 = struct.unpack(
            '<IIII252sI', self.file.read_at(header_start, HEADER_BYTES))
        if (guard0, guard1) != (0xFDFDFEEE, 0xF00DBEEF) or version != self.version:
            raise FormatError('invalid package guards or header version')
        if b'lica File Package' not in description:
            raise FormatError('invalid package description')
        if flags & 0x80000000:
            raise FormatError('encrypted package not yet supported')
        pos = masked_offset ^ MASK
        if not 12 <= pos <= header_start or count > min(1_000_000, (header_start - pos) // 8):
            raise FormatError('invalid package index bounds')
        self.data_end = pos
        self.entries = []
        for _ in range(count):
            if pos + 8 > header_start:
                raise FormatError('truncated index record')
            masked_size, masked_check = struct.unpack('<II', self.file.read_at(pos, 8))
            size = masked_size ^ MASK
            if size != masked_check ^ MASK ^ CHECK_MASK:
                raise FormatError('index check mismatch')
            pos += 8
            if not 0 < size <= ENTRY_BYTES or pos + size > header_start:
                raise FormatError('invalid index record size')
            stored = self.file.read_at(pos, size)
            pos += size
            record = stored if size == ENTRY_BYTES else inflate(stored, ENTRY_BYTES)
            raw_name, offset, length, stored_size, _ = struct.unpack('<260sIIII', record)
            if b'\x00' not in raw_name:
                raise FormatError('unterminated package filename')
            raw_name = raw_name.split(b'\x00', 1)[0]
            name = asset_path(raw_name)
            actual_stored_size = min(length, stored_size)
            if length > MAX_ASSET_BYTES or offset < 12 or offset + actual_stored_size > self.data_end:
                raise FormatError('asset payload outside data section or exceeds limit')
            self.entries.append(Entry(name, raw_name.hex(), offset, length, stored_size))
        self.index_end = pos

    def read(self, entry):
        if entry not in self.entries:
            raise FormatError('entry does not belong to this package')
        data = self.file.read_at(entry.offset, min(entry.size, entry.stored_size))
        return inflate(data, entry.size) if entry.size > entry.stored_size else data

    def close(self):
        self.file.close()

    def __enter__(self):
        return self

    def __exit__(self, *unused):
        self.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('package', type=Path)
    parser.add_argument('--find', default='')
    parser.add_argument('--extract', help='exact logical path; output must be supplied')
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    with Package(args.package) as package:
        if args.extract:
            matches = [e for e in package.entries if e.path.casefold() == args.extract.replace('\\', '/').casefold()]
            if len(matches) != 1 or args.output is None:
                parser.error('extraction requires one exact entry and --output')
            data = package.read(matches[0])
            args.output.parent.mkdir(parents=True, exist_ok=True)
            with args.output.open('xb') as target:
                target.write(data)
            print(json.dumps({'asset': matches[0].path, 'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()}))
        else:
            for entry in package.entries:
                if args.find.casefold() in entry.path.casefold():
                    print(json.dumps(vars(entry), ensure_ascii=False))


if __name__ == '__main__':
    main()
