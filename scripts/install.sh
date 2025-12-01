#!/bin/bash
# OpenLink ColdFire Installation Script

set -e

PREFIX="${PREFIX:-/usr/local}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

echo "==================================="
echo "OpenLink ColdFire Installer"
echo "==================================="
echo ""
echo "Installation prefix: $PREFIX"
echo ""

# Check for root if installing to system directories
if [[ "$PREFIX" == "/usr" || "$PREFIX" == "/usr/local" ]]; then
    if [[ $EUID -ne 0 ]]; then
        echo "Error: Installation to $PREFIX requires root privileges"
        echo "Run with: sudo $0"
        exit 1
    fi
fi

# Build if needed
cd "$PROJECT_DIR"
if [[ ! -f m68k-gdbserver ]]; then
    echo "Building m68k-gdbserver..."
    make
fi

# Install
echo ""
echo "Installing files..."
make PREFIX="$PREFIX" install

echo ""
echo "==================================="
echo "Installation complete!"
echo "==================================="
echo ""
echo "The following files were installed:"
echo "  - $PREFIX/bin/m68k-gdbserver"
echo "  - /etc/udev/rules.d/58-openlink.rules"
echo ""
echo "IMPORTANT: Reconnect your USB debug probe for the"
echo "udev rules to take effect."
echo ""
echo "Quick start:"
echo "  1. Connect your debug probe"
echo "  2. Run: m68k-gdbserver -p 3333"
echo "  3. Connect with: m68k-elf-gdb -ex 'target remote :3333'"
echo ""
