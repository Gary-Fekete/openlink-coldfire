# MCF52233 ColdFire Makefile for VSCode
# Target: M52233DEMO board

# Toolchain
PREFIX = m68k-elf-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc
LD = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
SIZE = $(PREFIX)size

# Project
TARGET = firmware
BUILD_DIR = build

# CPU flags
CPU = -mcpu=52235
FPU = -msoft-float

# Compiler flags
# Use -O0 for debugging (variables visible), -O2 for release
CFLAGS = $(CPU) $(FPU) -Wall -Wextra -O0 -g3
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -I./include

# Assembler flags
ASFLAGS = $(CPU) $(FPU) -g3 -x assembler-with-cpp

# Linker flags
LDFLAGS = $(CPU) $(FPU) -T ldscripts/mcf52235.ld
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(BUILD_DIR)/$(TARGET).map
LDFLAGS += -nostartfiles

# Source files
C_SOURCES = $(wildcard src/*.c)
S_SOURCES = $(wildcard startup/*.S)

# Object files - keep directory prefix to avoid name collisions
C_OBJECTS = $(patsubst src/%.c,$(BUILD_DIR)/src_%.o,$(C_SOURCES))
S_OBJECTS = $(patsubst startup/%.S,$(BUILD_DIR)/startup_%.o,$(S_SOURCES))
OBJECTS = $(C_OBJECTS) $(S_OBJECTS)

# Default target
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).hex size

# Create build directory
$(BUILD_DIR):
	mkdir -p $@

# Compile C files
$(BUILD_DIR)/src_%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble S files
$(BUILD_DIR)/startup_%.o: startup/%.S | $(BUILD_DIR)
	$(AS) $(ASFLAGS) -c $< -o $@

# Link
$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

# Create binary
$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

# Create hex file
$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

# Print size
size: $(BUILD_DIR)/$(TARGET).elf
	$(SIZE) $<

# Disassembly
disasm: $(BUILD_DIR)/$(TARGET).elf
	$(OBJDUMP) -d -S $< > $(BUILD_DIR)/$(TARGET).lst

# Clean
clean:
	rm -rf $(BUILD_DIR)

# Flash via GDB
flash: $(BUILD_DIR)/$(TARGET).elf
	m68k-elf-gdb -batch \
		-ex "target remote localhost:3333" \
		-ex "set architecture m68k:521x" \
		-ex "load" \
		-ex "quit" \
		$<

.PHONY: all clean size disasm flash
