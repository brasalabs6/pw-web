#!/usr/bin/env bash
# Creates an isolated user-scoped Grok model catalog from the repository template.
# No API key is ever stored in this file. Run in CI with GROK_HOME set.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
: "${GROK_HOME:?`GROK_HOME` must point to an isolated directory}"
if [[ "$GROK_HOME" != /* ]]; then
  echo "GROK_HOME must be absolute" >&2
  exit 2
fi
mkdir -p "$GROK_HOME"
chmod 700 "$GROK_HOME"
if [[ -e "$GROK_HOME/config.toml" ]]; then
  echo "Refusing to overwrite existing $GROK_HOME/config.toml; use a fresh GROK_HOME" >&2
  exit 2
fi
install -m 600 "$ROOT/.grok/models.ci.toml" "$GROK_HOME/config.toml"
echo "Configured 8 Cloudflare proxy models; default cf-glm-5.3 (credentials come from environment)."
