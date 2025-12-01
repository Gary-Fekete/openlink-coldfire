# OpenLink ColdFire

**Author:** Gary Fekete

Open source GDB server and debugging tools for Freescale/NXP ColdFire V2 microcontrollers.

## Features

- **GDB Remote Debugging** - Full GDB RSP protocol support
- **Flash Programming** - Program and verify flash memory via GDB `load` command
- **Hardware Breakpoints** - 4 hardware breakpoints (PBR0-PBR3)
- **Software Breakpoints** - Up to 32 software breakpoints
- **Watchpoints** - 1 data watchpoint (read/write/access)
- **Single Stepping** - Step through code instruction by instruction
- **Register Access** - Read/write all CPU registers
- **Memory Access** - Read/write Flash, SRAM, and peripheral registers

## Supported Hardware

### Debug Probes
- USB-ML-12 Multilink (USB VID: 0x1357, PID: 0x0503)

### Target Boards
- M52233DEMO (MCF52233 ColdFire V2)
- Other MCF5223x boards (256KB Flash, 32KB SRAM)

## Requirements

- Linux (tested on Arch Linux, should work on Debian/Ubuntu/Fedora)
- libusb-1.0
- m68k-elf-gcc toolchain (for building target code)
- m68k-elf-gdb (for debugging)

### Installing Dependencies

**Arch Linux:**
```bash
sudo pacman -S libusb base-devel

# For building the m68k-elf toolchain, you also need 32-bit libraries:
# Enable multilib repository in /etc/pacman.conf, then:
sudo pacman -S lib32-glibc lib32-libusb
```

**Debian/Ubuntu:**
```bash
sudo apt install libusb-1.0-0-dev build-essential

# For 32-bit toolchain support:
sudo dpkg --add-architecture i386
sudo apt update
sudo apt install libusb-1.0-0:i386 libc6:i386
```

### Installing the m68k-elf Toolchain

The m68k-elf toolchain (gcc, binutils, gdb, newlib) is required for building ColdFire programs. Pre-built ColdFire-enabled PKGBUILDs are included in the `toolchain/` directory.

**Arch Linux (recommended):**
```bash
# Install in order: binutils -> gcc-bootstrap -> newlib -> gcc -> gdb
cd toolchain/m68k-elf-binutils && makepkg -si
cd ../m68k-elf-gcc-bootstrap && makepkg -si
cd ../m68k-elf-newlib && makepkg -si
cd ../m68k-elf-gcc && makepkg -si
cd ../m68k-elf-gdb && makepkg -si
```

Note: The full multilib build takes 30-60 minutes but supports all ColdFire variants (mcf52235, mcf5307, etc).

**Other Linux distributions:**
Build the toolchain from source using the standard GNU toolchain build process, or adapt the PKGBUILDs to your package manager.

## Building

```bash
git clone https://github.com/Gary-Fekete/openlink-coldfire.git
cd openlink-coldfire
make
```

## Installation

```bash
# Install binary and udev rules (requires root)
sudo make install

# Reconnect USB device after installing udev rules
```

## Quick Start

1. **Connect your debug probe** to the target board and USB

2. **Start the GDB server:**
   ```bash
   m68k-gdbserver -p 3333
   ```

3. **Connect with GDB:**
   ```bash
   m68k-elf-gdb your_program.elf \
     -ex "set architecture m68k:521x" \
     -ex "target remote localhost:3333" \
     -ex "load"
   ```

4. **Debug!**
   ```gdb
   (gdb) break main
   (gdb) continue
   (gdb) step
   (gdb) info registers
   ```

## IDE Integration

### Eclipse CDT
See `templates/eclipse/` for a ready-to-use Eclipse project template with:
- Pre-configured m68k-elf toolchain settings
- GDB Hardware Debugging launch configuration
- Flash programming external tool

### VSCode
See `templates/vscode/` for a VSCode project template with:
- C/C++ extension configuration for m68k-elf-gcc
- Debug launch configuration
- Build and flash tasks

## Memory Map (MCF52233)

| Region | Address Range | Size |
|--------|---------------|------|
| Flash  | 0x00000000 - 0x0003FFFF | 256KB |
| SRAM   | 0x20000000 - 0x20007FFF | 32KB |
| IPSBAR | 0x40000000 - 0x401FFFFF | Peripherals |

## GDB Commands Reference

```gdb
# Connect
target remote localhost:3333
set architecture m68k:521x

# Flash programming
load                    # Program flash with current ELF
compare-sections        # Verify flash contents

# Breakpoints
break main              # Software breakpoint
hbreak *0x400          # Hardware breakpoint at address
watch variable          # Data watchpoint

# Execution
continue               # Run
step                   # Step one source line
stepi                  # Step one instruction
next                   # Step over function calls

# Inspection
info registers         # Show all registers
print $pc              # Show program counter
x/10i $pc              # Disassemble 10 instructions
x/10x 0x20000000       # Examine memory
```

## Troubleshooting

### "Permission denied" or "LIBUSB_ERROR_ACCESS"
Install the udev rules:
```bash
sudo make install-udev
# Then reconnect USB device
```

### "LIBUSB_ERROR_BUSY"
Another process is using the USB device:
```bash
pkill m68k-gdbserver
```

### GDB hangs on connect
Make sure the target board is powered and the debug probe LEDs indicate connection.

## Project Structure

```
openlink-coldfire/
├── src/
│   ├── m68k-gdbserver.c      # GDB server implementation
│   ├── openlink_protocol.c   # USB/BDM protocol
│   ├── openlink_protocol.h
│   ├── elf_loader.c/h        # ELF file parser and flashloader interface
│   ├── flash_gpl.c/h         # GPL flash operations interface
│   └── file_loader.c/h       # Multi-format file loader (ELF, S19, BIN)
├── flashloader/              # Clean flashloader source
│   ├── flashloader.c         # Readable C implementation (GPL v3)
│   ├── flashloader.ld        # Linker script
│   ├── Makefile
│   └── README.md
├── udev/
│   └── 58-openlink.rules     # USB permissions
├── templates/
│   ├── eclipse/              # Eclipse CDT project template
│   ├── vscode/               # VSCode project template
│   └── README.md             # IDE integration guide
├── toolchain/                # Arch Linux PKGBUILDs
│   ├── m68k-elf-binutils/
│   ├── m68k-elf-gcc-bootstrap/
│   ├── m68k-elf-gcc/
│   ├── m68k-elf-gdb/
│   └── m68k-elf-newlib/
├── scripts/
│   ├── install.sh
│   └── uninstall.sh
├── Makefile
├── LICENSE
├── COPYING                   # Full GPL v3 license text
└── README.md
```

## Contributing

Contributions are welcome! Please submit pull requests or open issues on GitHub.

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

Copyright (C) 2025 Gary Fekete

## Acknowledgments

This is a clean-room implementation based on USB protocol analysis. No proprietary code is included.
