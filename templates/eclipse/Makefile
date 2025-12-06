# MCF52235 ColdFire Makefile
# For use with OpenLink GDB Server

# Toolchain
PREFIX = m68k-elf-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc
LD = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
SIZE = $(PREFIX)size

# Project name
TARGET = firmware

# Directories
SRCDIR = src
STARTUPDIR = startup
INCDIR = include
LDDIR = ldscripts
BUILDDIR = Debug

# CPU flags
CPU = -mcpu=52235
FPU = -msoft-float

# Compiler flags
CFLAGS = $(CPU) $(FPU) -Wall -Wextra -O0 -g3
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -DMCF52235 -D__COLDFIRE__
CFLAGS += -I$(INCDIR)

# Assembler flags
ASFLAGS = $(CPU)

# Linker flags
LDFLAGS = $(CPU) $(FPU)
LDFLAGS += -T$(LDDIR)/mcf52235.ld
LDFLAGS += -nostartfiles
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(BUILDDIR)/$(TARGET).map

# Source files
C_SOURCES = $(wildcard $(SRCDIR)/*.c)
S_SOURCES = $(wildcard $(STARTUPDIR)/*.S)

# Object files - prefix with directory name to avoid collisions
C_OBJECTS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/src_%.o,$(C_SOURCES))
S_OBJECTS = $(patsubst $(STARTUPDIR)/%.S,$(BUILDDIR)/startup_%.o,$(S_SOURCES))
OBJECTS = $(C_OBJECTS) $(S_OBJECTS)

# Default target
all: $(BUILDDIR)/$(TARGET).elf size

# Create build directory
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Compile C files
$(BUILDDIR)/src_%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	@echo 'Compiling: $<'
	$(CC) -c $(CFLAGS) -MMD -MP -o $@ $<

# Assemble startup files
$(BUILDDIR)/startup_%.o: $(STARTUPDIR)/%.S | $(BUILDDIR)
	@echo 'Assembling: $<'
	$(AS) -c $(ASFLAGS) -o $@ $<

# Link
$(BUILDDIR)/$(TARGET).elf: $(OBJECTS)
	@echo 'Linking: $@'
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

# Print size
size: $(BUILDDIR)/$(TARGET).elf
	$(SIZE) $<

# Clean
clean:
	rm -rf $(BUILDDIR)/*.o $(BUILDDIR)/*.d $(BUILDDIR)/*.elf $(BUILDDIR)/*.map

# Flash using GDB
flash: $(BUILDDIR)/$(TARGET).elf
	$(PREFIX)gdb $< -batch \
		-ex "target remote localhost:3333" \
		-ex "set architecture m68k:521x" \
		-ex "load" \
		-ex "compare-sections" \
		-ex "quit"

.PHONY: all clean size flash

# Dependencies
-include $(OBJECTS:.o=.d)
