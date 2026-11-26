#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
SDK=${1:?Usage: bash BUILD-ARM32.sh /path/to/mpc-plugin-builder-sdk}
python3 "$SDK/tools/build.py" . --target arm32 --toolchain zig
