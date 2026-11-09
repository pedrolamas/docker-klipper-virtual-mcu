#!/bin/sh
# Install the virtual mcu board target into a Klipper source tree.
#
# Usage: install.sh <path-to-klipper>
#
# Copies src/virtual into the tree and registers MACH_VIRTUAL in
# src/Kconfig. Safe to run more than once. Exits non-zero if src/Kconfig
# no longer has the expected layout, so upstream changes fail the build
# loudly instead of producing a broken image.
set -e

KLIPPER_DIR="${1:?usage: $0 <path-to-klipper>}"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
KCONFIG="$KLIPPER_DIR/src/Kconfig"

[ -f "$KCONFIG" ] || { echo "install.sh: $KCONFIG not found" >&2; exit 1; }

rm -rf "$KLIPPER_DIR/src/virtual"
cp -r "$SCRIPT_DIR/src/virtual" "$KLIPPER_DIR/src/virtual"

if grep -q 'MACH_VIRTUAL' "$KCONFIG"; then
    echo "install.sh: MACH_VIRTUAL already registered"
    exit 0
fi

grep -q '^endchoice' "$KCONFIG" \
    || { echo "install.sh: no 'endchoice' in $KCONFIG" >&2; exit 1; }
grep -q '^source "src/linux/Kconfig"' "$KCONFIG" \
    || { echo "install.sh: no linux Kconfig source line in $KCONFIG" >&2; exit 1; }

awk '
    /^endchoice/ && !choice_done {
        print "    config MACH_VIRTUAL"
        print "        bool \"Virtual MCU (simulation)\""
        choice_done = 1
    }
    { print }
    /^source "src\/linux\/Kconfig"/ && !source_done {
        print "source \"src/virtual/Kconfig\""
        source_done = 1
    }
' "$KCONFIG" > "$KCONFIG.tmp"
mv "$KCONFIG.tmp" "$KCONFIG"

grep -q 'config MACH_VIRTUAL' "$KCONFIG" && grep -q 'src/virtual/Kconfig' "$KCONFIG" \
    || { echo "install.sh: failed to patch $KCONFIG" >&2; exit 1; }
echo "install.sh: virtual mcu installed into $KLIPPER_DIR"
