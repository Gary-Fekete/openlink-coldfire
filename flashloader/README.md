# OpenLink ColdFire Flashloader

This directory contains a clean, open-source flashloader implementation for the MCF52233/MCF52235 ColdFire microcontroller.

## Overview

The flashloader is a small program that runs on the target MCU to perform flash operations (erase, program, verify). It is loaded into SRAM via BDM and executed by setting the PC register and resuming.

## Files

- `flashloader.c` - Main source code with all flash operations
- `flashloader.ld` - Linker script for SRAM execution
- `Makefile` - Build system
- `flashloader.bin` - Compiled binary (after build)
- `flashloader.lst` - Disassembly listing (after build)

## Building

```bash
make
```

This generates:
- `flashloader.bin` - Raw binary to load to target
- `../src/flashloader_binary.h` - C header for embedding in GDB server

## Memory Layout

```
0x20000000  Parameter block (32 bytes)
            - operation (4 bytes)
            - flash_addr (4 bytes)
            - length (4 bytes)
            - result (4 bytes)
            - status (4 bytes)
            - reserved (12 bytes)

0x20000100  Data buffer (1KB) - for programming data

0x20000500  Flashloader code (< 1KB)

0x20007FF0  Stack pointer (grows down)
```

## Operations

| Code | Operation     | Parameters | Description |
|------|---------------|------------|-------------|
| 0    | Init          | none       | Initialize flash module |
| 1    | Mass Erase    | none       | Erase entire 256KB flash |
| 2    | Sector Erase  | flash_addr | Erase 8KB sector |
| 3    | Program       | flash_addr, length | Write data buffer to flash |
| 4    | Blank Check   | flash_addr, length | Verify flash is erased |
| 5    | Verify        | flash_addr, length | Compare flash with buffer |

## Result Codes

| Code | Meaning |
|------|---------|
| 0x00 | Success |
| 0x01 | Access error (ACCERR) |
| 0x02 | Protection violation (PVIOL) |
| 0x03 | Not blank (blank check failed) |
| 0x04 | Verify failed |
| 0x05 | Timeout |
| 0xFF | Unknown operation |

## Usage from GDB Server

1. Load flashloader binary to 0x20000500
2. Write operation code to 0x20000000
3. Write flash_addr to 0x20000004
4. Write length to 0x20000008
5. For programming: write data to 0x20000100
6. Set PC to 0x20000500
7. Resume (BDM GO)
8. Wait for halt
9. Read result from 0x2000000C

## Current Status

**Note:** The GDB server currently uses the original CodeWarrior flashloader
(`flashloader_chunks.h`) which has a more complex interface. This clean
implementation is provided as:

1. Documentation of how flash programming works
2. A tested alternative for future use
3. Reference for porting to other ColdFire variants

To switch the GDB server to use this flashloader, the flash programming
functions in `m68k-gdbserver.c` would need to be updated to use the
simpler parameter block interface.

## License

GPL v3 - See LICENSE file in parent directory.
