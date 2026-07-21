#!/usr/bin/env bash
# Prism - one-shot environment bootstrap (Linux)
# Checks Python (for build/tooling scripts).
# NOTE: the engine does not build on Linux yet (Windows-only premake paths),
#       so .NET SDK / Vulkan SDK checks are intentionally omitted here.
set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

printf '========================================\n'
printf ' Prism - Environment Setup (Linux)\n'
printf '========================================\n'
echo

echo "[1/1] Python"
bash "$SCRIPT_DIR/Setup-Python.sh"
rc=$?
if [ $rc -ne 0 ]; then
    echo
    echo "[XX] Python setup failed. Aborting."
    exit $rc
fi

echo
printf '========================================\n'
printf ' Prism environment ready.\n'
printf '========================================\n'
