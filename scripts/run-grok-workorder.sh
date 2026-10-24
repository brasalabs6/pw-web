#!/usr/bin/env bash
# Grok Build worker: model/proxy auth from isolated user-level config, no GitHub write credential.
set -euo pipefail
if [ "$#" -ne 2 ]; then echo "Usage: $0 <prompt.md> <result.json>" >&2; exit 2; fi
prompt="$1"; output="$2"
if [ ! -s "$prompt" ]; then echo "Prompt missing or empty" >&2; exit 2; fi
if ! command -v grok >/dev/null 2>&1; then echo "Official grok CLI not installed" >&2; exit 1; fi
if [ -z "${CF_PROXY_API_KEY:-}" ]; then echo "CF_PROXY_API_KEY Actions secret is required" >&2; exit 1; fi
: "${GROK_HOME:?GROK_HOME must point to isolated user config containing all custom models}"
if [ ! -s "$GROK_HOME/config.toml" ]; then echo "Grok model catalog not installed at GROK_HOME" >&2; exit 1; fi
model="${GROK_MODEL:-cf-glm-5.3}"
profile="${GROK_AGENT_PROFILE:-.grok/agents/coder.md}"
sandbox="${GROK_SANDBOX:-strict}"
turns="${GROK_MAX_TURNS:-35}"
case "$model" in
  cf-glm-5.2|cf-glm-5.3|cf-glm-5.3-flash|cf-deepseek-v4-flash|cf-glm-5.3-cyber|cf-glm-5.2-cyber|cf-glm-5.3-flash-cyber|cf-qwen3.8-27b) ;;
  *) echo "Unknown model: $model" >&2; exit 2;;
esac
case "$sandbox" in strict|read-only) ;; *) echo "Unsafe sandbox: $sandbox" >&2; exit 2;; esac
if ! [[ "$turns" =~ ^[0-9]+$ ]] || (( turns<1 || turns>45 )); then echo "Invalid max turns" >&2; exit 2; fi
if [ ! -s "$profile" ]; then echo "Missing agent profile: $profile" >&2; exit 2; fi
mkdir -p "$(dirname "$output")"
# Profile contains system instructions. No output to CI logs from model output.
grok --no-auto-update --sandbox "$sandbox" --always-approve \
  --no-memory --disable-web-search --max-turns "$turns" \
  --agent-profile "$profile" -m "$model" \
  --prompt-file "$prompt" --output-format json > "$output"
python3 - "$output" <<'PY'
import json, pathlib, sys
p = pathlib.Path(sys.argv[1])
if p.stat().st_size > 200000:
    raise SystemExit("Model result is too large")
data = json.loads(p.read_text(encoding="utf-8"))
if not isinstance(data, dict) or not isinstance(data.get("text"), str):
    raise SystemExit("Expected headless Grok Build JSON object containing text")
print("Grok Build completed; structured result recorded for subsequent validation.")
PY
