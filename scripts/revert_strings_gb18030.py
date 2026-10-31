#!/usr/bin/env python3
"""Selectively convert string literals from UTF-8 back to GB18030.

The port converted all source files from GB18030 to UTF-8, which broke every
runtime comparison against the original game data (package indexes, config
files, network protocol) that still uses GB18030. This script converts the
STRING LITERALS in C/C++ files back to GB18030 while leaving comments and
code structure in UTF-8.

The tokenizer is deliberately simple: it tracks whether it's inside a
comment, a string literal, or a character literal, and converts only the
string literal contents. Escape sequences are preserved byte-for-byte.

Usage: revert_strings_gb18030.py <file> [<file> ...]
       revert_strings_gb18030.py --dry-run <file>  (report only)
"""
import argparse
import pathlib
import sys


def utf8_to_gb18030(data: bytes) -> bytes:
    """Convert UTF-8 bytes to GB18030, passing ASCII through unchanged."""
    try:
        text = data.decode("utf-8")
    except UnicodeDecodeError:
        return data  # Not valid UTF-8; leave as-is.
    # Fast path: pure ASCII needs no conversion.
    if all(ord(c) < 128 for c in text):
        return data
    try:
        return text.encode("gb18030")
    except UnicodeEncodeError:
        return data  # Character not in GB18030; leave as-is.


def convert_string_literals(source: bytes) -> tuple[bytes, int]:
    """Convert string literal contents from UTF-8 to GB18030.

    Returns (converted_bytes, num_literals_changed).
    """
    result = bytearray()
    changed = 0
    i = 0
    n = len(source)
    state = "code"  # code | line_comment | block_comment | string | char

    while i < n:
        byte = source[i]

        if state == "code":
            if byte == ord("/") and i + 1 < n:
                if source[i + 1] == ord("/"):
                    state = "line_comment"
                    result.append(byte)
                    i += 1
                elif source[i + 1] == ord("*"):
                    state = "block_comment"
                    result.append(byte)
                    i += 1
                else:
                    result.append(byte)
                    i += 1
            elif byte == ord('"'):
                state = "string"
                result.append(byte)
                i += 1
            elif byte == ord("'"):
                state = "char"
                result.append(byte)
                i += 1
            else:
                result.append(byte)
                i += 1
        elif state == "line_comment":
            if byte == ord("\n"):
                state = "code"
            result.append(byte)
            i += 1
        elif state == "block_comment":
            if byte == ord("*") and i + 1 < n and source[i + 1] == ord("/"):
                state = "code"
                result.append(byte)
                result.append(source[i + 1])
                i += 2
            else:
                result.append(byte)
                i += 1
        elif state == "string":
            if byte == ord("\\") and i + 1 < n:
                # Escape sequence: copy both bytes verbatim.
                result.append(byte)
                result.append(source[i + 1])
                i += 2
            elif byte == ord('"'):
                state = "code"
                result.append(byte)
                i += 1
            elif byte == ord("\n"):
                # Unterminated string (shouldn't happen in valid C/C++).
                state = "code"
                result.append(byte)
                i += 1
            else:
                # Regular string byte: accumulate for conversion.
                # We need to find the full multi-byte UTF-8 sequence.
                if byte < 0x80:
                    result.append(byte)
                    i += 1
                else:
                    # Multi-byte UTF-8: determine sequence length.
                    if byte & 0xE0 == 0xC0:
                        seq_len = 2
                    elif byte & 0xF0 == 0xE0:
                        seq_len = 3
                    elif byte & 0xF8 == 0xF0:
                        seq_len = 4
                    else:
                        # Invalid UTF-8 lead byte; copy as-is.
                        result.append(byte)
                        i += 1
                        continue
                    seq = source[i:i + seq_len]
                    converted = utf8_to_gb18030(seq)
                    if converted != seq:
                        changed += 1
                    result.extend(converted)
                    i += seq_len
        elif state == "char":
            if byte == ord("\\") and i + 1 < n:
                result.append(byte)
                result.append(source[i + 1])
                i += 2
            elif byte == ord("'"):
                state = "code"
                result.append(byte)
                i += 1
            else:
                result.append(byte)
                i += 1

    return bytes(result), changed


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("files", nargs="+", help="C/C++ files to process")
    parser.add_argument("--dry-run", action="store_true",
                        help="report what would change without writing")
    args = parser.parse_args()

    total_changed = 0
    total_files = 0
    for path_str in args.files:
        path = pathlib.Path(path_str)
        if not path.is_file():
            print(f"skipping (not a file): {path}", file=sys.stderr)
            continue
        source = path.read_bytes()
        converted, changed = convert_string_literals(source)
        if changed > 0:
            total_changed += changed
            total_files += 1
            if args.dry_run:
                print(f"would change {changed} literals: {path}")
            else:
                path.write_bytes(converted)
                print(f"changed {changed} literals: {path}")
        else:
            # Check if there's a difference even without multi-byte changes
            # (could be an edge case).
            if converted != source:
                total_files += 1
                if args.dry_run:
                    print(f"would change (byte-level diff): {path}")
                else:
                    path.write_bytes(converted)
                    print(f"changed (byte-level diff): {path}")

    print(f"\ntotal: {total_files} files, {total_changed} string literals converted")
    return 0


if __name__ == "__main__":
    sys.exit(main())
