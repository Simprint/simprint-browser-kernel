#!/usr/bin/env bash
# Apply all patches in patches/APPLY_ORDER to Chromium src.
# Run from simprint-browser-kernel root. Chromium path: config/kernel.config or SIMPRINT_CHROMIUM_ROOT.

set -e
KERNEL_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PATCHES_ROOT="$KERNEL_ROOT/patches"
ORDER_FILE="$PATCHES_ROOT/APPLY_ORDER"

# Resolve Chromium root: env > config
if [ -n "$SIMPRINT_CHROMIUM_ROOT" ]; then
  CHROMIUM_SRC="$SIMPRINT_CHROMIUM_ROOT"
elif [ -f "$KERNEL_ROOT/config/kernel.config" ]; then
  CHROMIUM_SRC=$(grep -E '^SIMPRINT_CHROMIUM_ROOT=' "$KERNEL_ROOT/config/kernel.config" 2>/dev/null | cut -d= -f2- | tr -d '"' | tr -d "'")
fi
if [ -z "$CHROMIUM_SRC" ] || [ ! -d "$CHROMIUM_SRC" ]; then
  echo "Error: Chromium root not set or not a directory. Set SIMPRINT_CHROMIUM_ROOT or config/kernel.config"
  exit 1
fi

cd "$CHROMIUM_SRC"
while IFS= read -r line; do
  line="${line%%#*}"
  line="${line// /}"
  [ -z "$line" ] && continue
  patch_file="$PATCHES_ROOT/$line"
  if [ ! -f "$patch_file" ]; then
    echo "Error: patch not found: $patch_file"
    exit 1
  fi
  echo "Applying: $line"
  patch -p1 < "$patch_file"
done < "$ORDER_FILE"
echo "All patches applied."
