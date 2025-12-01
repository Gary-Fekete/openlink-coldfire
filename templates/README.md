# IDE Integration Templates for OpenLink ColdFire

This directory contains ready-to-use configuration templates for integrating OpenLink ColdFire with popular IDEs.

## MCF5223 Header Files

The templates include comprehensive peripheral header files for MCF52233/MCF52235 development:

### Usage Options

**Option 1: Simple (backward compatible)**
```c
#include "mcf52235.h"

// Direct register access
PORTTC = 0x0F;
UART0_UTB = 'A';
CFMCLKD = 0x64;
```

**Option 2: Full peripheral headers**
```c
#define MCF5223_FULL_HEADERS
#include "mcf52235.h"

// Access via MCF_* prefixed names with bit definitions
MCF_GPIO_PORTTC = MCF_GPIO_PIN0 | MCF_GPIO_PIN1;
MCF_UART0_UTB = 'A';
MCF_CFM_CFMCLKD = MCF_CFM_CFMCLKD_PRDIV8 | MCF_CFM_CFMCLKD_DIV(36);
```

**Option 3: Individual peripheral headers**
```c
#define IPSBAR 0x40000000
#include "mcf5223/mcf5223_uart.h"
#include "mcf5223/mcf5223_gpio.h"

// Only the included peripherals are available
MCF_UART0_UTB = 'A';
MCF_GPIO_SETTC = MCF_GPIO_PIN0;
```

### Available Headers

| Header | Description |
|--------|-------------|
| `mcf52235.h` | Main header with basic register definitions |
| `mcf5xxx.h` | Common ColdFire types and CPU definitions |
| `mcf5223/mcf5223.h` | Master include for all peripherals |
| `mcf5223/mcf5223_gpio.h` | GPIO ports, pin assignment, set/clear |
| `mcf5223/mcf5223_uart.h` | UART0/1/2 with bit definitions |
| `mcf5223/mcf5223_intc.h` | Interrupt controller with source definitions |
| `mcf5223/mcf5223_cfm.h` | Flash module with commands and status bits |
| `mcf5223/mcf5223_pit.h` | Programmable interrupt timers |
| `mcf5223/mcf5223_dtim.h` | DMA timers |
| `mcf5223/mcf5223_i2c.h` | I2C module |
| `mcf5223/mcf5223_qspi.h` | Queued SPI |
| `mcf5223/mcf5223_clock.h` | PLL and clock configuration |
| `mcf5223/mcf5223_scm.h` | System control, watchdog, bus parking |

---

## Prerequisites

1. **Install OpenLink ColdFire:**
   ```bash
   cd /path/to/openlink-coldfire
   make
   sudo make install
   ```

2. **Install m68k-elf toolchain:**
   - GCC cross-compiler: `m68k-elf-gcc`
   - GDB debugger: `m68k-elf-gdb`
   - Binutils: `m68k-elf-objcopy`, `m68k-elf-size`, etc.

3. **USB permissions:** Install the udev rules
   ```bash
   sudo make install-udev
   ```

---

## VSCode Setup

### Quick Start

1. Copy the `.vscode` folder to your project root:
   ```bash
   cp -r /path/to/openlink-coldfire/templates/vscode/.vscode /path/to/your-project/
   ```

2. Open your project in VSCode

3. Install the **C/C++** extension (ms-vscode.cpptools)

### Configuration Files

| File | Purpose |
|------|---------|
| `.vscode/launch.json` | Debug configurations |
| `.vscode/tasks.json` | Build, flash, erase tasks |
| `.vscode/c_cpp_properties.json` | IntelliSense configuration |
| `.vscode/settings.json` | Editor settings |

### Available Tasks (Ctrl+Shift+B)

| Task | Description |
|------|-------------|
| **Build** | Compile the project (default) |
| **Clean** | Remove build artifacts |
| **Rebuild** | Clean + Build |
| **Flash** | Program flash with verification |
| **Erase Flash** | Erase entire 256KB flash |
| **Start GDB Server** | Start server for debugging |
| **Stop GDB Server** | Kill the GDB server |

### Debug Configurations (F5)

| Configuration | Description |
|---------------|-------------|
| **Debug MCF52233** | Flash, start GDB server, and debug |
| **Debug (No Flash)** | Debug without reprogramming |
| **Attach to Running Target** | Connect to already-running GDB server |

### Customization

Edit `.vscode/tasks.json` to change:
- Build directory (default: `build/`)
- Output filename (default: `${workspaceFolderBasename}.elf`)
- GDB server port (default: 3333)

---

## Eclipse CDT Setup

### Quick Start

1. Copy Eclipse configuration files to your project:
   ```bash
   cp /path/to/openlink-coldfire/templates/eclipse/MCF52233-Debug.launch /path/to/your-project/
   cp -r /path/to/openlink-coldfire/templates/eclipse/.externalToolBuilders /path/to/your-project/
   ```

2. Import the launch configurations:
   - **Run** → **Run Configurations** → **Import**
   - Select `MCF52233-Debug.launch`

3. Configure External Tools:
   - **Run** → **External Tools** → **External Tools Configurations**
   - Import from `.externalToolBuilders/`

### Configuration Files

| File | Purpose |
|------|---------|
| `MCF52233-Debug.launch` | GDB JTAG debug configuration |
| `.externalToolBuilders/Flash.launch` | Flash programming tool |
| `.externalToolBuilders/FlashAndRun.launch` | Flash + start GDB server |
| `.externalToolBuilders/Erase.launch` | Erase entire flash |

### Creating a New Project

1. **File** → **New** → **C/C++ Project**
2. Select **Makefile Project** or **Managed Build**
3. Configure for cross-compilation:
   - Prefix: `m68k-elf-`
   - Path: `/usr/bin` or wherever your toolchain is installed

### Debug Setup

1. **Run** → **Debug Configurations**
2. Create new **GDB Hardware Debugging** configuration
3. Set:
   - **Main** tab:
     - Project: Your project name
     - C/C++ Application: `Debug/yourproject.elf`
   - **Debugger** tab:
     - GDB Command: `/usr/bin/m68k-elf-gdb`
     - Connection: TCP, localhost:3333
   - **Startup** tab:
     - Init Commands: `set architecture m68k:521x`
     - Load image: Checked
     - Load symbols: Checked
     - Set breakpoint at: `main`

---

## Project Structure

Your project should have this structure:

```
your-project/
├── src/
│   ├── main.c
│   ├── startup.S
│   └── ...
├── include/
│   ├── mcf52235.h            # Main MCF5223x header
│   ├── mcf5xxx.h             # Common ColdFire definitions
│   └── mcf5223/              # Comprehensive peripheral headers
│       ├── mcf5223.h         # Master include for all peripherals
│       ├── mcf5223_gpio.h    # GPIO definitions
│       ├── mcf5223_uart.h    # UART definitions
│       ├── mcf5223_intc.h    # Interrupt controller
│       ├── mcf5223_cfm.h     # Flash module
│       ├── mcf5223_pit.h     # Programmable interrupt timers
│       ├── mcf5223_dtim.h    # DMA timers
│       ├── mcf5223_i2c.h     # I2C module
│       ├── mcf5223_qspi.h    # Queued SPI
│       ├── mcf5223_clock.h   # Clock module
│       └── mcf5223_scm.h     # System control module
├── build/                    # Build output directory
│   └── yourproject.elf
├── Makefile
├── linker.ld                 # Linker script for MCF52233
├── .vscode/                  # VSCode config (from template)
│   ├── launch.json
│   ├── tasks.json
│   ├── c_cpp_properties.json
│   └── settings.json
└── MCF52233-Debug.launch     # Eclipse debug config
```

---

## Example Makefile

```makefile
# MCF52233 Project Makefile
TARGET = yourproject
BUILD_DIR = build

# Toolchain
PREFIX = m68k-elf-
CC = $(PREFIX)gcc
AS = $(PREFIX)as
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy
SIZE = $(PREFIX)size

# MCF52233 specific flags
CPU_FLAGS = -mcpu=52235 -msoft-float
CFLAGS = $(CPU_FLAGS) -Os -g -Wall -Wextra
CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS = $(CPU_FLAGS) -Tlinker.ld -nostartfiles
LDFLAGS += -Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(TARGET).map

# Sources
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:src/%.c=$(BUILD_DIR)/%.o)

.PHONY: all clean flash erase

all: $(BUILD_DIR)/$(TARGET).elf
	$(SIZE) $<

$(BUILD_DIR)/$(TARGET).elf: $(OBJS) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

flash: all
	m68k-gdbserver --program $(BUILD_DIR)/$(TARGET).elf --verify

erase:
	m68k-gdbserver --erase
```

---

## Troubleshooting

### "Permission denied" when accessing USB device

Install udev rules:
```bash
sudo cp /path/to/openlink-coldfire/udev/58-openlink.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo udevadm trigger
# Reconnect the USB device
```

### GDB cannot connect to server

1. Check if server is running:
   ```bash
   ps aux | grep m68k-gdbserver
   ```

2. Kill any existing servers:
   ```bash
   pkill -f m68k-gdbserver
   ```

3. Start server manually to see errors:
   ```bash
   m68k-gdbserver --gdb -p 3333
   ```

### "Unknown architecture" in GDB

Make sure GDB is configured for m68k:
```bash
m68k-elf-gdb --version
```

Add architecture command to GDB init:
```
set architecture m68k:521x
```

### Flash programming fails

1. **Erase first:** Try erasing before programming
   ```bash
   m68k-gdbserver --erase
   ```

2. **Check connections:** Ensure USB cable is connected and board is powered

3. **Verify file format:** Supported formats are `.elf`, `.bin`, `.s19`

---

## Command Reference

```bash
# Erase entire flash
m68k-gdbserver --erase

# Program ELF file with verification
m68k-gdbserver --program firmware.elf --verify

# Program binary file at specific address
m68k-gdbserver --program firmware.bin --base 0x00000000 --verify

# Start GDB server on port 3333
m68k-gdbserver --gdb -p 3333

# Start GDB server on custom port
m68k-gdbserver --gdb -p 4444
```

---

## License

These templates are provided under the GPL v3 license as part of the OpenLink ColdFire project.
