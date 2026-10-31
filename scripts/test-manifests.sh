#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
command -v node >/dev/null 2>&1 || { echo "Missing tool: node" >&2; exit 2; }
node -e 'const [major, minor] = process.versions.node.split(".").map(Number); if (major < 22 || (major === 22 && minor < 12)) process.exit(1)' || {
  echo "Manifest contracts require Node.js 22.12 or newer" >&2
  exit 2
}
node --test tests/contracts/manifest-validator.test.mjs
