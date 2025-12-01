# OpenLink ColdFire Flashloader Makefile
#
# Builds the flashloader binary and generates a C header file
# containing the binary data for embedding in the GDB server.

PREFIX = m68k-elf-
CC = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
SIZE = $(PREFIX)size

# ColdFire V2 flags
CPU = -mcpu=52235
FPU = -msoft-float

# Compiler flags - optimize for size
CFLAGS = $(CPU) $(FPU) -Os -g
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -fno-builtin -nostdlib -nostartfiles
CFLAGS += -Wall -Wextra

# Linker flags
LDFLAGS = $(CPU) $(FPU)
LDFLAGS += -Tflashloader.ld
LDFLAGS += -nostdlib -nostartfiles
LDFLAGS += -Wl,--gc-sections

TARGET = flashloader
HEADER = ../src/flashloader_binary.h

all: $(TARGET).bin $(TARGET).lst $(HEADER) size

$(TARGET).elf: flashloader.c flashloader.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ flashloader.c

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).lst: $(TARGET).elf
	$(OBJDUMP) -d -S $< > $@

# Generate C header from binary
$(HEADER): $(TARGET).bin
	@echo "Generating $(HEADER)..."
	@echo "/*" > $(HEADER)
	@echo " * OpenLink ColdFire Flashloader Binary" >> $(HEADER)
	@echo " *" >> $(HEADER)
	@echo " * Auto-generated from flashloader.c - DO NOT EDIT" >> $(HEADER)
	@echo " * Rebuild with: cd flashloader && make" >> $(HEADER)
	@echo " *" >> $(HEADER)
	@echo " * Load address: 0x20000500" >> $(HEADER)
	@echo " * Entry point:  0x20000500 (_start)" >> $(HEADER)
	@echo " */" >> $(HEADER)
	@echo "" >> $(HEADER)
	@echo "#ifndef FLASHLOADER_BINARY_H" >> $(HEADER)
	@echo "#define FLASHLOADER_BINARY_H" >> $(HEADER)
	@echo "" >> $(HEADER)
	@echo "#include <stdint.h>" >> $(HEADER)
	@echo "" >> $(HEADER)
	@echo "#define FLASHLOADER_LOAD_ADDR    0x20000500" >> $(HEADER)
	@echo "#define FLASHLOADER_ENTRY_POINT  0x20000500" >> $(HEADER)
	@echo "#define FLASHLOADER_PARAMS_ADDR  0x20000000" >> $(HEADER)
	@echo "#define FLASHLOADER_BUFFER_ADDR  0x20000100" >> $(HEADER)
	@echo "#define FLASHLOADER_BUFFER_SIZE  0x400  /* 1KB data buffer */" >> $(HEADER)
	@echo "" >> $(HEADER)
	@echo "/* Flashloader operations */" >> $(HEADER)
	@echo "#define FLASH_OP_INIT         0" >> $(HEADER)
	@echo "#define FLASH_OP_MASS_ERASE   1" >> $(HEADER)
	@echo "#define FLASH_OP_SECTOR_ERASE 2" >> $(HEADER)
	@echo "#define FLASH_OP_PROGRAM      3" >> $(HEADER)
	@echo "#define FLASH_OP_BLANK_CHECK  4" >> $(HEADER)
	@echo "#define FLASH_OP_VERIFY       5" >> $(HEADER)
	@echo "" >> $(HEADER)
	@echo "/* Result codes */" >> $(HEADER)
	@echo "#define FLASH_RESULT_SUCCESS      0x00000000" >> $(HEADER)
	@echo "#define FLASH_RESULT_ACCERR       0x00000001" >> $(HEADER)
	@echo "#define FLASH_RESULT_PVIOL        0x00000002" >> $(HEADER)
	@echo "#define FLASH_RESULT_NOT_BLANK    0x00000003" >> $(HEADER)
	@echo "#define FLASH_RESULT_VERIFY_FAIL  0x00000004" >> $(HEADER)
	@echo "#define FLASH_RESULT_TIMEOUT      0x00000005" >> $(HEADER)
	@echo "#define FLASH_RESULT_UNKNOWN_OP   0x000000FF" >> $(HEADER)
	@echo "" >> $(HEADER)
	@printf "#define FLASHLOADER_SIZE %d\n" $$(wc -c < $(TARGET).bin) >> $(HEADER)
	@echo "" >> $(HEADER)
	@echo "static const uint8_t flashloader_binary[] = {" >> $(HEADER)
	@xxd -i < $(TARGET).bin | sed 's/^/    /' >> $(HEADER)
	@echo "};" >> $(HEADER)
	@echo "" >> $(HEADER)
	@echo "#endif /* FLASHLOADER_BINARY_H */" >> $(HEADER)
	@echo "Generated $(HEADER) ($$(wc -c < $(TARGET).bin) bytes)"

size: $(TARGET).elf
	$(SIZE) $<

clean:
	rm -f $(TARGET).elf $(TARGET).bin $(TARGET).lst $(HEADER)

.PHONY: all clean size
