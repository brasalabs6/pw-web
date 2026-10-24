#!/usr/bin/env bash
# Grok project MCP wrapper: launch pinned Chrome DevTools against isolated localhost demo.
set -euo pipefail
if ! command -v chrome-devtools-mcp >/dev/null 2>&1; then
  echo "chrome-devtools-mcp is missing (install pinned npm package before running Grok)" >&2
  exit 1
fi
if [ -z "${CHROME_BIN:-}" ] || [ ! -x "$CHROME_BIN" ]; then
  echo "CHROME_BIN must point to a Chrome for Testing executable" >&2
  exit 1
fi
PORT="${PW_WEB_PORT:-8480}"
if ! [[ "$PORT" =~ ^[0-9]+$ ]] || (( PORT < 1024 || PORT > 65535 )); then echo "Invalid MCP browser port" >&2; exit 2; fi
export CHROME_DEVTOOLS_MCP_NO_USAGE_STATISTICS=1
export CHROME_DEVTOOLS_MCP_NO_UPDATE_CHECKS=1
export CHROME_DEVTOOLS_MCP_NO_CONFIG_DISCOVERY=1
exec chrome-devtools-mcp \
  --headless --isolated --no-usage-statistics --no-performance-crux \
  --executablePath="$CHROME_BIN" \
  --allowedUrlPattern="http://127.0.0.1:$PORT/*" \
  --allowedUrlPattern="http://localhost:$PORT/*"
