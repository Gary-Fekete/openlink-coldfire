#!/bin/bash
# OpenLink ColdFire Uninstallation Script

set -e

PREFIX="${PREFIX:-/usr/local}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

echo "==================================="
echo "OpenLink ColdFire Uninstaller"
echo "==================================="
echo ""

# Check for root
if [[ $EUID -ne 0 ]]; then
    echo "Error: Uninstallation requires root privileges"
    echo "Run with: sudo $0"
    exit 1
fi

cd "$PROJECT_DIR"
make PREFIX="$PREFIX" uninstall

echo ""
echo "Uninstallation complete!"
echo ""
