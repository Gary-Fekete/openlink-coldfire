# OpenLink ColdFire - Makefile
# Open source GDB server for ColdFire/M68K debugging

CC = gcc
CFLAGS = -Wall -Wextra -O2 -I/usr/include/libusb-1.0
LDFLAGS = -lusb-1.0

PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin
UDEVDIR = /etc/udev/rules.d
SHAREDIR = $(PREFIX)/share/openlink-coldfire

# Source files
SRCDIR = src
SOURCES = $(SRCDIR)/m68k-gdbserver.c $(SRCDIR)/openlink_protocol.c $(SRCDIR)/elf_loader.c $(SRCDIR)/flash_gpl.c $(SRCDIR)/file_loader.c
HEADERS = $(SRCDIR)/openlink_protocol.h $(SRCDIR)/elf_loader.h $(SRCDIR)/flash_gpl.h $(SRCDIR)/file_loader.h

# Target binary
TARGET = m68k-gdbserver

.PHONY: all clean install uninstall install-udev install-templates flashloader

all: flashloader $(TARGET)

# Build flashloader first
flashloader:
	$(MAKE) -C flashloader

$(TARGET): $(SOURCES) $(HEADERS) flashloader
	$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)
	$(MAKE) -C flashloader clean

install: $(TARGET) install-udev install-templates
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/
	install -d $(DESTDIR)$(SHAREDIR)/flashloader
	install -m 644 flashloader/flashloader.elf $(DESTDIR)$(SHAREDIR)/flashloader/
	@echo ""
	@echo "Installation complete!"
	@echo "  Binary: $(DESTDIR)$(BINDIR)/$(TARGET)"
	@echo "  Flashloader: $(DESTDIR)$(SHAREDIR)/flashloader/"
	@echo "  Templates: $(DESTDIR)$(SHAREDIR)/templates/"
	@echo ""
	@echo "You may need to reconnect your USB device for udev rules to take effect."

install-udev:
	install -d $(DESTDIR)$(UDEVDIR)
	install -m 644 udev/58-openlink.rules $(DESTDIR)$(UDEVDIR)/
	@echo "Reloading udev rules..."
	-udevadm control --reload-rules 2>/dev/null || true
	-udevadm trigger 2>/dev/null || true
	@echo "NOTE: You may need to reconnect your USB device"

install-templates:
	install -d $(DESTDIR)$(SHAREDIR)/templates
	cp -r templates/eclipse $(DESTDIR)$(SHAREDIR)/templates/ 2>/dev/null || true
	cp -r templates/vscode $(DESTDIR)$(SHAREDIR)/templates/ 2>/dev/null || true

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -f $(DESTDIR)$(UDEVDIR)/58-openlink.rules
	rm -rf $(DESTDIR)$(SHAREDIR)
	-udevadm control --reload-rules 2>/dev/null || true

help:
	@echo "OpenLink ColdFire Build System"
	@echo ""
	@echo "Build Targets:"
	@echo "  all              - Build m68k-gdbserver"
	@echo "  flashloader      - Build flashloader only"
	@echo "  clean            - Remove build artifacts"
	@echo ""
	@echo "Installation (requires root):"
	@echo "  sudo make install - Install with udev rules and templates"
	@echo "  install-udev      - Install udev rules only"
	@echo "  install-templates - Install IDE templates (Eclipse, VSCode)"
	@echo "  uninstall         - Remove installation"
	@echo ""
	@echo "Usage after installation:"
	@echo "  m68k-gdbserver --erase              # Erase flash"
	@echo "  m68k-gdbserver --program file.elf   # Program flash"
	@echo "  m68k-gdbserver --gdb                # Start GDB server"
	@echo ""
	@echo "Variables:"
	@echo "  PREFIX           - Installation prefix (default: /usr/local)"
	@echo "  DESTDIR          - Staging directory for packaging"
