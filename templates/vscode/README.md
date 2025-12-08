# MCF52233 ColdFire Template Project for VSCode

Template project for developing firmware for the M52233DEMO board using VSCode.

## Project Structure

```
vscode-coldfire-template/
├── .vscode/
│   ├── launch.json          # Debug configurations
│   ├── tasks.json           # Build tasks
│   ├── c_cpp_properties.json # IntelliSense settings
│   └── settings.json        # Editor settings
├── src/                     # Source files (.c)
├── include/                 # Header files (.h)
├── startup/                 # Startup code (.S)
├── ldscripts/               # Linker scripts
├── build/                   # Build output
└── Makefile
```

## Prerequisites

1. **m68k-elf toolchain** installed in `/usr/bin/`
2. **VSCode** with the following extensions:
   - **C/C++** (ms-vscode.cpptools) - Required for debugging
   - **C/C++ Extension Pack** (optional but recommended)

## Setup

1. Open this folder in VSCode:
   ```bash
   code /home/username/M52233DEMO/vscode-coldfire-template
   ```

2. Install the C/C++ extension if prompted

## Build

- Press `Ctrl+Shift+B` to build
- Or use Terminal: `make all`
- Output: `build/firmware.elf`

## Debug

1. Start the GDB server manually (recommended for first time):
   ```bash
   cd /home/username/M52233DEMO
   ./m68k-gdbserver -p 3333
   ```

2. In VSCode:
   - Press `F5` or go to Run > Start Debugging
   - Select "Debug MCF52233 (GDB)"

3. The debugger will:
   - Connect to localhost:3333
   - Load the firmware
   - Stop at main()

## Debug Features

- **Breakpoints**: Click in the gutter to set breakpoints
- **Step Over**: F10
- **Step Into**: F11
- **Step Out**: Shift+F11
- **Continue**: F5
- **Variables**: View in the Debug sidebar
- **Registers**: View CPU registers in Debug sidebar
- **Watch**: Add expressions to watch

## Memory Map

- Flash: 0x00000000 - 0x0003FFFF (256KB)
- SRAM:  0x20000000 - 0x20007FFF (32KB)

## Makefile Targets

- `make` or `make all` - Build the project
- `make clean` - Remove build artifacts
- `make size` - Show memory usage
- `make disasm` - Generate disassembly listing
- `make flash` - Flash via GDB (requires server running)

## Troubleshooting

### "Connection refused" error
The GDB server isn't running. Start it manually:
```bash
/home/username/M52233DEMO/m68k-gdbserver -p 3333
```

### "LIBUSB_ERROR_BUSY"
Another process is using the USB device. Kill old servers:
```bash
pkill -9 m68k-gdbserver
```

### IntelliSense errors
Make sure the C/C++ extension is installed and reload the window.
