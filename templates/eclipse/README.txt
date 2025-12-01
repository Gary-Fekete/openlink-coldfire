MCF52233 ColdFire Template Project for Eclipse
==============================================

This is a template project for developing firmware for the M52233DEMO
board using Eclipse CDT with the m68k-elf toolchain.

Project Structure:
------------------
  src/           - Source files (.c)
  include/       - Header files (.h)
  startup/       - Startup code and vector table (.S)
  ldscripts/     - Linker scripts
  Debug/         - Build output (created by make)
  Makefile       - External makefile (not Eclipse managed)

Import into Eclipse:
--------------------
1. File -> Import -> General -> Existing Projects into Workspace
2. Browse to the template folder
3. Select the project and click Finish

Note: This project uses an external Makefile instead of Eclipse's
managed build system for better reliability with cross-compilers.

Build:
------
1. Right-click project -> Build Project
   Or press Ctrl+B

The external Makefile will compile the project and create:
  - Debug/firmware.elf (the executable)
  - Debug/firmware.map (linker map file)

Command line build:
  make        - Build the project
  make clean  - Clean build artifacts
  make flash  - Flash via GDB server

Debug:
------
1. Start the GDB server first:
   m68k-gdbserver -p 3333

2. In Eclipse:
   - Run -> Debug Configurations
   - Select "MCF52233-Debug" under GDB Hardware Debugging
   - Click Debug

Or import the included MCF52233-Debug.launch file:
   - File -> Import -> Run/Debug -> Launch Configurations

Flash (without debugging):
--------------------------
1. Make sure the GDB server is running

2. Import External Tools:
   - Run -> External Tools -> External Tools Configurations
   - Click "Import..." (or manually create a new Program configuration)
   - Browse to .externalToolBuilders/Flash.launch

3. Run the Flash tool:
   - Run -> External Tools -> Flash
   - Or add it to the toolbar favorites

Available external tools:
   - Flash: Programs the firmware and verifies it
   - FlashAndRun: Programs and starts execution (no debugging)

Toolchain Settings:
-------------------
- Compiler: m68k-elf-gcc
- Prefix: m68k-elf-
- Path: /usr/bin
- CPU: -mcpu=52235
- Float: -msoft-float (MCF52235 has no FPU)

Memory Map:
-----------
- Flash: 0x00000000 - 0x0003FFFF (256KB)
- SRAM:  0x20000000 - 0x20007FFF (32KB)

GDB Server:
-----------
The m68k-gdbserver connects to the USB-ML-12 Multilink debug probe
and provides a GDB remote interface on port 3333.

Features:
- Remote debugging (target remote localhost:3333)
- Hardware breakpoints (4 available)
- Software breakpoints (32 available)
- Watchpoints (1 available)
- Memory read/write
- Register read/write
- Single stepping
- vFlash programming (GDB load command)

License: GPL v3
