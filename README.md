# OpenLink ColdFire

**Author:** Gary Fekete

Open source GDB server and debugging tools for Freescale/NXP ColdFire V2 microcontrollers.

## Features

- **GDB Remote Debugging** - Full GDB RSP protocol support with binary escaping
- **Flash Programming** - Program and verify flash memory via GDB `load` command
- **Hardware Breakpoints** - 4 hardware breakpoints (PBR0-PBR3) with TDR accumulation
- **Software Breakpoints** - Up to 32 software breakpoints using HALT opcode injection
- **Watchpoints** - 1 data watchpoint (read/write/access)
- **Single Stepping** - Step through code instruction by instruction
- **Register Access** - Read/write all CPU registers (D0-D7, A0-A7, PC, SR, VBR, etc.)
- **Memory Access** - Read/write Flash, SRAM, and peripheral registers
- **Fast Halt Detection** - ~9ms response time via CSR BKPT bit polling
- **Chip Identification** - Automatic MCF5223x variant detection via CIR/BDM CSR

## Supported Hardware

### Debug Probes
- P&E USB-ML-12 Multilink (USB VID: 0x1357, PID: 0x0503)

### Target MCUs
- MCF52230 (PIN: 0x48)
- MCF52231 (PIN: 0x49)
- MCF52232 (PIN: 0x4A)
- MCF52233 (PIN: 0x4A) - M52233DEMO board
- MCF52234 (PIN: 0x4B)
- MCF52235 (PIN: 0x4C)

All variants: 256KB Flash, 32KB SRAM

## Requirements

- Linux (tested on Arch Linux, should work on Debian/Ubuntu/Fedora)
- libusb-1.0
- m68k-elf-gcc toolchain (for building target code)
- m68k-elf-gdb (for debugging)

### Installing Dependencies

**Arch Linux:**
```bash
sudo pacman -S libusb base-devel

# For 32-bit support (required for toolchain):
# Enable multilib repository in /etc/pacman.conf, then:
sudo pacman -S lib32-glibc lib32-libusb
```

**Debian/Ubuntu:**
```bash
sudo apt install libusb-1.0-0-dev build-essential

# For 32-bit support (required for toolchain):
sudo dpkg --add-architecture i386
sudo apt update
sudo apt install libc6:i386 libusb-1.0-0:i386
```

### Installing the m68k-elf Toolchain

Pre-built ColdFire-enabled PKGBUILDs are included in the `toolchain/` directory.

**Arch Linux (recommended):**
```bash
# Install in order: binutils -> gcc-bootstrap -> newlib -> gcc -> gdb
cd toolchain/m68k-elf-binutils && makepkg -si
cd ../m68k-elf-gcc-bootstrap && makepkg -si
cd ../m68k-elf-newlib && makepkg -si
cd ../m68k-elf-gcc && makepkg -si
cd ../m68k-elf-gdb && makepkg -si
```

Note: The full multilib build takes 30-60 minutes but supports all ColdFire variants.

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
# Install binary to /usr/local/bin and udev rules
sudo make install

# Reconnect USB device after installing udev rules
```

Or install just the udev rules:
```bash
sudo make install-udev
```

## Quick Start

1. **Connect your debug probe** to the target board and USB

2. **Start the GDB server:**
   ```bash
   m68k-gdbserver
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
- LED blink example code
- MCF5223x peripheral headers

### VSCode
See `templates/vscode/` for a VSCode project template with:
- C/C++ extension configuration for m68k-elf-gcc
- Debug launch configuration
- Build and flash tasks
- Comprehensive MCF5223x header library

## Template Library Features

The templates include a complete MCF5223x peripheral library:

| Header | Peripheral |
|--------|------------|
| `mcf5xxx.h` | Common ColdFire CPU definitions |
| `mcf52235.h` | Main MCF52235 header with LED macros |
| `mcf5223_adc.h` | 12-bit ADC (8 channels) |
| `mcf5223_uart.h` | UART serial ports |
| `mcf5223_gpio.h` | GPIO ports |
| `mcf5223_pit.h` | Programmable Interrupt Timers |
| `mcf5223_cfm.h` | ColdFire Flash Module |
| `mcf5223_intc.h` | Interrupt Controller |
| ... | And more |

See `templates/vscode/docs/API_REFERENCE.md` for complete API documentation.

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

# Breakpoints (4 hardware + 32 software)
break main              # Software breakpoint (auto-fallback)
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

See `docs/GDB_COMMANDS.md` for a complete reference.

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

### Flash programming fails
Ensure the target is halted before programming. The GDB server handles this automatically, but if you're having issues:
```gdb
monitor reset
load
```

## Project Structure

```
openlink-coldfire/
├── src/
│   ├── m68k-gdbserver.c      # GDB server implementation
│   ├── openlink_protocol.c   # USB/BDM protocol
│   ├── openlink_protocol.h
│   ├── elf_loader.c/h        # ELF file parser
│   ├── flash_gpl.c/h         # GPL flash operations
│   └── file_loader.c/h       # Multi-format loader (ELF, S19, BIN)
├── flashloader/              # Target-side flashloader
│   ├── flashloader.c         # Clean C implementation (GPL v3)
│   ├── flashloader.ld        # Linker script
│   ├── Makefile
│   └── README.md
├── docs/
│   └── GDB_COMMANDS.md       # GDB command reference
├── udev/
│   └── 58-openlink.rules     # USB permissions
├── templates/
│   ├── eclipse/              # Eclipse CDT project template
│   │   ├── src/              # Source files
│   │   ├── include/          # Header files
│   │   │   └── mcf5223/      # Peripheral headers
│   │   ├── startup/          # Startup code
│   │   ├── ldscripts/        # Linker scripts
│   │   └── docs/             # API documentation
│   ├── vscode/               # VSCode project template
│   │   ├── src/              # Source files
│   │   ├── include/          # Header files
│   │   │   └── mcf5223/      # Peripheral headers
│   │   ├── startup/          # Startup code
│   │   ├── ldscripts/        # Linker scripts
│   │   └── docs/             # API documentation
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
├── LICENSE                   # GPL v3 license
├── COPYING                   # Full GPL v3 text
└── README.md
```

## Contributing

Contributions are welcome! Please submit pull requests or open issues on GitHub.

### Building for Development

```bash
make clean
make DEBUG=1    # Build with debug symbols
```

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

Copyright (C) 2025 Gary Fekete

## Acknowledgments

- This is a clean-room implementation based on USB protocol analysis
- No proprietary code is included
- Flashloader implementation is original GPL v3 code
- MCF5223x register definitions derived from public datasheets
