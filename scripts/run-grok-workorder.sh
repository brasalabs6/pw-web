#!/usr/bin/env bash
# Execute the official Grok Build CLI in bounded, noninteractive mode.
# No GitHub write token. XAI_API_KEY supplied only to this process by CI.
set -euo pipefail

if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <prompt.md> <result.json>" >&2
  exit 2
fi
prompt="$1"
output="$2"
if [ ! -s "$prompt" ]; then
  echo "Prompt file is missing or empty" >&2
  exit 2
fi
if ! command -v grok >/dev/null 2>&1; then
  echo "Grok Build CLI is unavailable. Install official @xai-official/grok." >&2
  exit 1
fi
if [ -z "${XAI_API_KEY:-}" ]; then
  echo "XAI_API_KEY is required for headless CI authentication" >&2
  exit 1
fi
mkdir -p "$(dirname "$output")"
# Do NOT echo XAI_API_KEY, prompt or raw session output to the job log.
# --sandbox strict uses Linux sandbox restrictions; subprocess network is blocked.
# --always-approve is required for unattended tool use, NOT PR approval.
grok --no-auto-update \
  --sandbox strict \
  --always-approve \
  --no-memory \
  --disable-web-search \
  --max-turns 35 \
  -p "$(cat "$prompt")" \
  --output-format json > "$output"
python3 - "$output" <<'PY'
import json
import pathlib
import sys

path = pathlib.Path(sys.argv[1])
result = json.loads(path.read_text(encoding="utf-8"))
if not isinstance(result, dict):
    raise SystemExit("Grok returned non-object JSON")
if not result.get("text") and not result.get("stopReason"):
    raise SystemExit("Grok returned no completion metadata")
print("Grok Build finished; JSON result saved for independent review.")
PY
