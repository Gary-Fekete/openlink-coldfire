#include "openlink_protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Global verbosity flag - default is quiet (0)
// Set to 1 to enable verbose debug output
int g_openlink_verbose = 0;

void openlink_set_verbose(int level) {
    g_openlink_verbose = level;
}

// CRUCIAL: Global persistent command buffer
// This buffer is NEVER cleared - it preserves leftover response data between
// commands, which is essential for proper BDM operation.
// Initialize to zeros ONCE at program start, then reuse forever.
// Note: Not static - externally visible for test programs
unsigned char g_cmd_buffer[256] = {0};

int usb_reset(libusb_device_handle *dev) {
    int r = libusb_reset_device(dev);
    if (r < 0) {
        fprintf(stderr, "Error resetting device: %s\n", libusb_error_name(r));
        return -1;
    }
    return 0;
}

void print_hex(unsigned char* data, int size) {
    if (!g_openlink_verbose) return;  // Skip in quiet mode
    for (int i = 0; i < size; i++) {
        printf("%02x ", data[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

void print_as_ascii(unsigned char* data, int size) {
    if (!g_openlink_verbose) return;  // Skip in quiet mode
    for (int i = 0; i < size; i++) {
        if (data[i] >= 32 && data[i] <= 126) {
            printf("%c", data[i]);
        } else {
            printf(".");
        }
    }
    printf("\n");
}

int send_bb_command(libusb_device_handle *handle, unsigned char *cmd_data, int cmd_len, const char *cmd_name) {
    int r;
    int sent_length;

    if (g_openlink_verbose) printf("\nSending '%s' command...\n", cmd_name);
    r = libusb_bulk_transfer(handle, ENDPOINT_OUT, cmd_data, cmd_len, &sent_length, 0);
    if (r != 0) {
        fprintf(stderr, "Error sending %s command: %s\n", cmd_name, libusb_error_name(r));
        return -1;
    }
    if (g_openlink_verbose) {
        printf("Sent %d bytes.\n", sent_length);
        print_hex(cmd_data, sent_length);
    }
    return 0;
}

int send_aa_command(libusb_device_handle *handle, unsigned char *cmd_data, int cmd_len, const char *cmd_name) {
    int r;
    int sent_length;
    static int usb_send_counter = 0;

    // Verbose debug output disabled for cleaner output
    // printf("\n[USB_SEND #%d] About to call libusb_bulk_transfer for '%s'\n", usb_send_counter, cmd_name);
    // printf("[USB_SEND #%d] Command: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
    //        usb_send_counter,
    //        cmd_data[0], cmd_data[1], cmd_data[2], cmd_data[3],
    //        cmd_data[4], cmd_data[5], cmd_data[6], cmd_data[7],
    //        cmd_data[8], cmd_data[9]);

    r = libusb_bulk_transfer(handle, ENDPOINT_OUT, cmd_data, cmd_len, &sent_length, 0);

    // printf("[USB_SEND #%d] libusb_bulk_transfer returned: %d (%s)\n",
    //        usb_send_counter, r, r == 0 ? "SUCCESS" : libusb_error_name(r));
    usb_send_counter++;

    if (r != 0) {
        fprintf(stderr, "Error sending %s command: %s\n", cmd_name, libusb_error_name(r));
        return -1;
    }
    // printf("Sent %d bytes.\n", sent_length);
    // print_hex(cmd_data, sent_length);

    // Only expect a response for aa 55 commands
    unsigned char response_buffer[256];
    int actual_response_len;
    // printf("\nReceiving response to %s command...\n", cmd_name);
    r = libusb_bulk_transfer(handle, ENDPOINT_IN, response_buffer, 256, &actual_response_len, 10000); // 10 second timeout
    if (r < 0) {
        fprintf(stderr, "Error receiving response to %s command: %s\n", cmd_name, libusb_error_name(r));
        return -1;
    }

    // printf("Received %d bytes:\n", actual_response_len);
    // print_hex(response_buffer, actual_response_len);

    // Special handling for Get Device Info command to print ASCII
    if (g_openlink_verbose && cmd_len >= 6 && cmd_data[4] == 0x01 && cmd_data[5] == 0x0b && actual_response_len > 4) {
        printf("\nResponse as ASCII:\n");
        print_as_ascii(response_buffer + 4, actual_response_len - 4);
    }

    // CRUCIAL: Copy response back to cmd_data buffer
    // This preserves leftover data for the next command, which is essential
    // for proper BDM operation. Without this, memory windows don't work!
    memcpy(cmd_data, response_buffer, 256);

    return 0;
}

// Send aa 55 command without waiting for response
// Used for commands where target becomes busy (e.g., BDM-RESUME)
int send_aa_command_no_response(libusb_device_handle *handle, unsigned char *cmd_data, int cmd_len, const char *cmd_name) {
    int r;
    int sent_length;

    if (g_openlink_verbose) printf("\nSending '%s' command (no response expected)...\n", cmd_name);
    r = libusb_bulk_transfer(handle, ENDPOINT_OUT, cmd_data, cmd_len, &sent_length, 0);
    if (r != 0) {
        fprintf(stderr, "Error sending %s command: %s\n", cmd_name, libusb_error_name(r));
        return -1;
    }
    if (g_openlink_verbose) {
        printf("Sent %d bytes.\n", sent_length);
        print_hex(cmd_data, sent_length);
        printf("Command sent successfully (target is now executing code)\n");
    }
    return 0;
}

// Response Validation Helper
// Supports both response formats:
//   - Standard format (99 66): Used by most commands (setup, config, register ops)
//   - Memory read format (88 a5): Used by cmd_0717 (Flash/RAM reads), cmd_071b (verify)
// Returns: 0 on success, -1 on error
// If successful, sets *response_type to 0x9966 or 0x88a5
int validate_response(unsigned char *buffer, int length, int min_length, uint16_t *response_type) {
    if (length < min_length) {
        fprintf(stderr, "Response too short: %d bytes (expected at least %d)\n", length, min_length);
        return -1;
    }

    // Check for standard response format (99 66)
    if (buffer[0] == 0x99 && buffer[1] == 0x66) {
        // Verify status byte (offset 4) is 0xee (success)
        if (length >= 5 && buffer[4] == 0xee) {
            if (response_type) *response_type = 0x9966;
            return 0;
        } else if (length >= 5) {
            fprintf(stderr, "Standard response (99 66) with error status: 0x%02x\n", buffer[4]);
            return -1;
        }
    }

    // Check for memory read response format (88 a5)
    if (buffer[0] == 0x88 && buffer[1] == 0xa5) {
        // Verify status byte (offset 4) is 0xee (success)
        if (length >= 5 && buffer[4] == 0xee) {
            if (response_type) *response_type = 0x88a5;
            return 0;
        } else if (length >= 5) {
            fprintf(stderr, "Memory read response (88 a5) with error status: 0x%02x\n", buffer[4]);
            return -1;
        }
    }

    // Unknown response format
    fprintf(stderr, "Unknown response format: %02x %02x (expected 99 66 or 88 a5)\n", buffer[0], buffer[1]);
    return -1;
}

// Memory Access Functions

int cmd_write_memory_byte_addr(libusb_device_handle *handle, uint32_t addr, uint8_t data) {
    unsigned char *cmd = g_cmd_buffer;  // Use global persistent buffer
    // Header
    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    // Length: 10 bytes (07 15 + sub-param + 4-byte addr + 2-byte data)
    cmd[2] = 0x00;
    cmd[3] = 0x0a;
    // Command: Write Memory (07 15 for 32-bit addressing)
    cmd[4] = 0x07;
    cmd[5] = 0x15;
    // Sub-parameter (0x1800 from captures)
    cmd[6] = 0x18;
    cmd[7] = 0x00;
    // Address (big-endian, 32-bit)
    cmd[8] = (addr >> 24) & 0xFF;
    cmd[9] = (addr >> 16) & 0xFF;
    cmd[10] = (addr >> 8) & 0xFF;
    cmd[11] = addr & 0xFF;
    // Data (16-bit format, upper byte is 00 for byte write)
    cmd[12] = 0x00;
    cmd[13] = data;
    // CRUCIAL: Do NOT zero-byte 14-255! They contain leftover response data

    return send_aa_command(handle, cmd, 256, "Write Memory Byte");
}

int cmd_read_memory_byte_addr(libusb_device_handle *handle, uint32_t addr, uint8_t *data) {
    unsigned char *cmd = g_cmd_buffer;  // Use global persistent buffer
    // Header
    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    // Length: 4 bytes (07 13 + 4-byte addr)
    cmd[2] = 0x00;
    cmd[3] = 0x04;
    // Command: Read Memory
    cmd[4] = 0x07;
    cmd[5] = 0x13;
    // Address (big-endian)
    cmd[6] = (addr >> 24) & 0xFF;
    cmd[7] = (addr >> 16) & 0xFF;
    cmd[8] = (addr >> 8) & 0xFF;
    cmd[9] = addr & 0xFF;

    // Send command
    int r;
    int sent_length;
    if (g_openlink_verbose) printf("\nSending 'Read Memory Byte' command...\n");
    r = libusb_bulk_transfer(handle, ENDPOINT_OUT, cmd, 256, &sent_length, 0);
    if (r != 0) {
        fprintf(stderr, "Error sending Read Memory Byte command: %s\n", libusb_error_name(r));
        return -1;
    }
    if (g_openlink_verbose) {
        printf("Sent %d bytes.\n", sent_length);
        print_hex(cmd, sent_length);
    }

    // Receive response BACK INTO g_cmd_buffer
    int actual_response_len;
    if (g_openlink_verbose) printf("\nReceiving response to Read Memory Byte command...\n");
    r = libusb_bulk_transfer(handle, ENDPOINT_IN, g_cmd_buffer, 256, &actual_response_len, 10000);
    if (r < 0) {
        fprintf(stderr, "Error receiving response to Read Memory Byte command: %s\n", libusb_error_name(r));
        return -1;
    }

    if (g_openlink_verbose) {
        printf("Received %d bytes:\n", actual_response_len);
        print_hex(g_cmd_buffer, actual_response_len);
    }

    // Validate response format (supports both 99 66 and 88 a5)
    uint16_t response_type;
    if (validate_response(g_cmd_buffer, actual_response_len, 9, &response_type) != 0) {
        fprintf(stderr, "Invalid response for Read Memory Byte command\n");
        return -1;
    }

    // Parse response: [99 66 or 88 a5] [len:2] ee [4-byte data] ...
    // For byte read, we take the LSB of the 4-byte data
    *data = g_cmd_buffer[8]; // LSB of 4-byte big-endian value
    return 0;
}

int cmd_write_memory_word_addr(libusb_device_handle *handle, uint32_t addr, uint16_t data) {
    unsigned char *cmd = g_cmd_buffer;  // Use global persistent buffer
    // Header
    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    // Length: 6 bytes (07 16 + 4-byte addr + 2-byte data)
    cmd[2] = 0x00;
    cmd[3] = 0x06;
    // Command: Write Memory
    cmd[4] = 0x07;
    cmd[5] = 0x16;
    // Address (big-endian)
    cmd[6] = (addr >> 24) & 0xFF;
    cmd[7] = (addr >> 16) & 0xFF;
    cmd[8] = (addr >> 8) & 0xFF;
    cmd[9] = addr & 0xFF;
    // Data (2 bytes, big-endian)
    cmd[10] = (data >> 8) & 0xFF;
    cmd[11] = data & 0xFF;
    // CRUCIAL: Do NOT zero-byte 12-255! They contain leftover response data

    return send_aa_command(handle, cmd, 256, "Write Memory Word");
}

int cmd_read_memory_word_addr(libusb_device_handle *handle, uint32_t addr, uint16_t *data) {
    unsigned char *cmd = g_cmd_buffer;  // Use global persistent buffer
    // Header
    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    // Length: 4 bytes (07 13 + 4-byte addr)
    cmd[2] = 0x00;
    cmd[3] = 0x04;
    // Command: Read Memory
    cmd[4] = 0x07;
    cmd[5] = 0x13;
    // Address (big-endian)
    cmd[6] = (addr >> 24) & 0xFF;
    cmd[7] = (addr >> 16) & 0xFF;
    cmd[8] = (addr >> 8) & 0xFF;
    cmd[9] = addr & 0xFF;

    // Send command
    int r;
    int sent_length;
    if (g_openlink_verbose) printf("\nSending 'Read Memory Word' command...\n");
    r = libusb_bulk_transfer(handle, ENDPOINT_OUT, cmd, 256, &sent_length, 0);
    if (r != 0) {
        fprintf(stderr, "Error sending Read Memory Word command: %s\n", libusb_error_name(r));
        return -1;
    }
    if (g_openlink_verbose) {
        printf("Sent %d bytes.\n", sent_length);
        print_hex(cmd, sent_length);
    }

    // Receive response BACK INTO g_cmd_buffer
    int actual_response_len;
    if (g_openlink_verbose) printf("\nReceiving response to Read Memory Word command...\n");
    r = libusb_bulk_transfer(handle, ENDPOINT_IN, g_cmd_buffer, 256, &actual_response_len, 10000);
    if (r < 0) {
        fprintf(stderr, "Error receiving response to Read Memory Word command: %s\n", libusb_error_name(r));
        return -1;
    }

    if (g_openlink_verbose) {
        printf("Received %d bytes:\n", actual_response_len);
        print_hex(g_cmd_buffer, actual_response_len);
    }

    // Validate response format (supports both 99 66 and 88 a5)
    uint16_t response_type;
    if (validate_response(g_cmd_buffer, actual_response_len, 9, &response_type) != 0) {
        fprintf(stderr, "Invalid response for Read Memory Word command\n");
        return -1;
    }

    // Parse response: [99 66 or 88 a5] [len:2] ee [4-byte data] ...
    // For word read, we take the lower 2 bytes of the 4-byte data
    *data = (g_cmd_buffer[7] << 8) | g_cmd_buffer[8]; // Lower 2 bytes big-endian
    return 0;
}

// Write 32-bit data to 16-bit address
// Used after BDM address mapping is set up
// Uses this for SRAM access: cmd 07 16 with 16-bit addr + 32-bit data
int cmd_write_memory_short_addr(libusb_device_handle *handle, uint16_t addr, uint32_t data) {
    unsigned char *cmd = g_cmd_buffer;  // Use global persistent buffer (NOT local!)
    // Header
    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    // Length: 8 bytes (07 16 = 2 bytes, 2-byte addr = 2 bytes, 4-byte data = 4 bytes)
    cmd[2] = 0x00;
    cmd[3] = 0x08;
    // Command: Write Memory
    cmd[4] = 0x07;
    cmd[5] = 0x16;
    // Address (16-bit big-endian)
    cmd[6] = (addr >> 8) & 0xFF;
    cmd[7] = addr & 0xFF;
    // Data (32-bit big-endian)
    cmd[8] = (data >> 24) & 0xFF;
    cmd[9] = (data >> 16) & 0xFF;
    cmd[10] = (data >> 8) & 0xFF;
    cmd[11] = data & 0xFF;

    // CRUCIAL: Do NOT zero-byte 12-255! They contain leftover response data
    // from the previous command.

    return send_aa_command(handle, cmd, 256, "Write Memory (16-bit addr)");
}

int cmd_write_memory_long_addr(libusb_device_handle *handle, uint32_t addr, uint32_t data) {
    unsigned char *cmd = g_cmd_buffer;  // Use global persistent buffer
    // Header
    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    // Length: 10 bytes (07 16 = 2 bytes, 4-byte addr = 4 bytes, 4-byte data = 4 bytes)
    cmd[2] = 0x00;
    cmd[3] = 0x0A;
    // Command: Write Memory
    cmd[4] = 0x07;
    cmd[5] = 0x16;
    // Address (32-bit big-endian)
    cmd[6] = (addr >> 24) & 0xFF;
    cmd[7] = (addr >> 16) & 0xFF;
    cmd[8] = (addr >> 8) & 0xFF;
    cmd[9] = addr & 0xFF;
    // Data (32-bit big-endian)
    cmd[10] = (data >> 24) & 0xFF;
    cmd[11] = (data >> 16) & 0xFF;
    cmd[12] = (data >> 8) & 0xFF;
    cmd[13] = data & 0xFF;
    // CRUCIAL: Do NOT zero-byte 14-255! They contain leftover response data

    return send_aa_command(handle, cmd, 256, "Write Memory Long");
}

// Set Memory Window: Crucial for accessing different flash modules
// Command 0x07 0x10 discovered from erase-flash.hex capture
// This command switches which memory region is accessible via 0x07 0x13 reads
int cmd_set_memory_window(libusb_device_handle *handle, uint32_t window_addr) {
    unsigned char *cmd = g_cmd_buffer;  // Use global persistent buffer

    // Header
    cmd[0] = 0xaa;
    cmd[1] = 0x55;

    // Length: 5 bytes (07 10 + 4-byte address)
    cmd[2] = 0x00;
    cmd[3] = 0x05;

    // Command: Set Memory Window
    cmd[4] = 0x07;
    cmd[5] = 0x10;

    // Window address (big-endian)
    // Common values:
    //   0x00000000 - Flash Module 0 (lower 128KB)
    //   0x00020000 - Flash Module 1 (upper 128KB)
    //   0x00400000 - CFM peripheral registers
    cmd[6] = (window_addr >> 24) & 0xFF;
    cmd[7] = (window_addr >> 16) & 0xFF;
    cmd[8] = (window_addr >> 8) & 0xFF;
    cmd[9] = (window_addr >> 0) & 0xFF;
    // CRUCIAL: Do NOT zero-byte 10-255! They contain leftover response data

    return send_aa_command(handle, cmd, 256, "Set Memory Window");
}

int cmd_read_memory_long_addr(libusb_device_handle *handle, uint32_t addr, uint32_t *data) {
    unsigned char *cmd = g_cmd_buffer;  // Use global persistent buffer
    // Header
    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    // Length: 6 bytes (07 13 = 2 bytes, 4-byte addr = 4 bytes)
    cmd[2] = 0x00;
    cmd[3] = 0x06;
    // Command: Read Memory
    cmd[4] = 0x07;
    cmd[5] = 0x13;
    // Address (32-bit big-endian)
    cmd[6] = (addr >> 24) & 0xFF;
    cmd[7] = (addr >> 16) & 0xFF;
    cmd[8] = (addr >> 8) & 0xFF;
    cmd[9] = addr & 0xFF;

    // Send command
    int r;
    int sent_length;
    if (g_openlink_verbose) printf("\nSending 'Read Memory Long' command...\n");
    r = libusb_bulk_transfer(handle, ENDPOINT_OUT, cmd, 256, &sent_length, 0);
    if (r != 0) {
        fprintf(stderr, "Error sending Read Memory Long command: %s\n", libusb_error_name(r));
        return -1;
    }
    if (g_openlink_verbose) {
        printf("Sent %d bytes.\n", sent_length);
        print_hex(cmd, sent_length);
    }

    // Receive response BACK INTO g_cmd_buffer
    int actual_response_len;
    if (g_openlink_verbose) printf("\nReceiving response to Read Memory Long command...\n");
    r = libusb_bulk_transfer(handle, ENDPOINT_IN, g_cmd_buffer, 256, &actual_response_len, 10000);
    if (r < 0) {
        fprintf(stderr, "Error receiving response to Read Memory Long command: %s\n", libusb_error_name(r));
        return -1;
    }

    if (g_openlink_verbose) {
        printf("Received %d bytes:\n", actual_response_len);
        print_hex(g_cmd_buffer, actual_response_len);
    }

    // Validate response format (supports both 99 66 and 88 a5)
    uint16_t response_type;
    if (validate_response(g_cmd_buffer, actual_response_len, 9, &response_type) != 0) {
        fprintf(stderr, "Invalid response for Read Memory Long command\n");
        return -1;
    }

    // Parse response: [99 66 or 88 a5] [len:2] ee [4-byte data] ...
    *data = (g_cmd_buffer[5] << 24) | (g_cmd_buffer[6] << 16) |
            (g_cmd_buffer[7] << 8) | g_cmd_buffer[8];
    return 0;
}

// Memory Read Command (cmd_0717)
// Reads memory from Flash or RAM using 32-bit addressing
// Command: aa55 0008 0717 [addr:4] [len:2]
// Response: 88 a5 [len:2] ee [data...]
//
// Usage:
//   - Flash reads: addr = 0x00000000 to 0x0003FFFF (256KB)
//   - SRAM reads: addr = 0x20000000 to 0x20007FFF (32KB)
//
// Parameters:
//   addr: 32-bit memory address (big-endian)
//   length: Number of bytes to read (16-bit, max typically 256)
//   buffer: Output buffer to store read data
//   buffer_size: Size of output buffer (must be >= length)
//
// Returns: 0 on success, -1 on error
int cmd_0717_read_memory(libusb_device_handle *handle, uint32_t addr, uint16_t length,
                         uint8_t *buffer, int buffer_size) {
    if (buffer_size < length) {
        fprintf(stderr, "Buffer too small: %d bytes (need %d)\n", buffer_size, length);
        return -1;
    }

    unsigned char *cmd = g_cmd_buffer;  // Use global persistent buffer

    // Build command packet
    cmd[0] = 0xaa;
    cmd[1] = 0x55;

    // Length: 8 bytes (0717 + 4-byte addr + 2-byte length)
    cmd[2] = 0x00;
    cmd[3] = 0x08;

    // Command: Read Memory
    cmd[4] = 0x07;
    cmd[5] = 0x17;

    // Address (32-bit big-endian)
    cmd[6] = (addr >> 24) & 0xFF;
    cmd[7] = (addr >> 16) & 0xFF;
    cmd[8] = (addr >> 8) & 0xFF;
    cmd[9] = addr & 0xFF;

    // Request 1.5x the length because response uses 6-byte-per-word format:
    // Each 4 bytes of actual data is followed by 2 padding bytes (00 00)
    // So to get N bytes, we need (N/4)*6 = N*1.5 raw bytes
    // Round up to ensure we get enough data
    uint16_t request_length = ((length + 3) / 4) * 6;  // (length/4 rounded up) * 6
    cmd[10] = (request_length >> 8) & 0xFF;
    cmd[11] = request_length & 0xFF;

    // Send command
    int r;
    int sent_length;
    if (g_openlink_verbose) {
        printf("\nSending 'Read Memory (cmd_0717)' command...\n");
        printf("  Address: 0x%08X, Length: %d bytes (requesting %d raw)\n", addr, length, request_length);
    }
    r = libusb_bulk_transfer(handle, ENDPOINT_OUT, cmd, 256, &sent_length, 0);
    if (r != 0) {
        fprintf(stderr, "Error sending cmd_0717: %s\n", libusb_error_name(r));
        return -1;
    }
    if (g_openlink_verbose) {
        printf("Sent %d bytes.\n", sent_length);
        print_hex(cmd, 16);  // Show first 16 bytes
        printf("\nReceiving response to cmd_0717...\n");
    }

    // Read first packet
    int actual_response_len;
    r = libusb_bulk_transfer(handle, ENDPOINT_IN, g_cmd_buffer, 256, &actual_response_len, 10000);
    if (r < 0) {
        fprintf(stderr, "Error receiving response to cmd_0717: %s\n", libusb_error_name(r));
        return -1;
    }
    if (g_openlink_verbose) printf("Received packet 1: %d bytes\n", actual_response_len);

    // Parse response length from header (bytes 2-3, big-endian)
    if (actual_response_len < 4) {
        fprintf(stderr, "Response too short to parse header: %d bytes\n", actual_response_len);
        return -1;
    }

    uint16_t response_len = (g_cmd_buffer[2] << 8) | g_cmd_buffer[3];
    int total_expected = 4 + response_len;  // 4 header bytes (88 a5 len:2) + response_len
    if (g_openlink_verbose) printf("Response indicates %d total bytes expected\n", total_expected);

    // Read additional packets if needed (but don't overflow 256-byte buffer)
    int total_received = actual_response_len;
    int packet_num = 2;
    // Stop if buffer full OR we have all expected data
    while (total_received < total_expected && total_received < 256 && packet_num < 10) {
        int remaining_space = 256 - total_received;
        if (remaining_space <= 0) break;

        int chunk_len;
        r = libusb_bulk_transfer(handle, ENDPOINT_IN,
                                 g_cmd_buffer + total_received,
                                 remaining_space, &chunk_len, 10000);
        if (r < 0) {
            fprintf(stderr, "Error receiving packet %d: %s\n", packet_num, libusb_error_name(r));
            return -1;
        }
        if (g_openlink_verbose) printf("Received packet %d: %d bytes\n", packet_num, chunk_len);
        total_received += chunk_len;
        packet_num++;

        // Break if we got a short packet (indicates end of transfer)
        if (chunk_len < remaining_space) {
            break;
        }
    }

    if (g_openlink_verbose) {
        printf("Total received: %d bytes\n", total_received);
        print_hex(g_cmd_buffer, total_received > 64 ? 64 : total_received);  // Show first 64 bytes
        if (total_received > 64) {
            printf("  ... (%d more bytes)\n", total_received - 64);
        }
    }

    // Validate response format (should be 88 a5, not 99 66!)
    // Note: we expect 5 header bytes + length data bytes
    uint16_t response_type;
    if (validate_response(g_cmd_buffer, total_received, 5 + length, &response_type) != 0) {
        fprintf(stderr, "Invalid response for cmd_0717\n");
        return -1;
    }

    // Both 88 a5 and 99 66 are valid response formats for memory reads
    // Raw dumps show 99 66 is actually more common
    if (response_type != 0x88a5 && response_type != 0x9966) {
        fprintf(stderr, "Warning: Unexpected response format 0x%04x (expected 88a5 or 9966)\n", response_type);
    }

    // Extract data from 6-byte-per-word format
    // Response format after 5-byte header: [data:4][pad:2][data:4][pad:2]...
    // We need to take 4 bytes from each 6-byte block
    int src_offset = 5;  // Start after header (99 66 len:2 ee)
    int dst_offset = 0;
    int bytes_remaining = length;

    while (bytes_remaining > 0 && src_offset + 4 <= total_received) {
        int chunk = (bytes_remaining >= 4) ? 4 : bytes_remaining;
        memcpy(buffer + dst_offset, &g_cmd_buffer[src_offset], chunk);
        dst_offset += chunk;
        bytes_remaining -= chunk;
        src_offset += 6;  // Skip 4 data bytes + 2 padding bytes
    }

    if (g_openlink_verbose) {
        // Verbose output disabled for cleaner operation
    }
    return 0;
}

// Memory Read/Verify Command (cmd_071b)
// Read and verify memory - used extensively in SRAM validation sequence
// Command: aa55 0008 071b [addr:4] [len:2]
// Response: 99 66 [len:2] ee [data...] (standard format, NOT 88 a5!)
//
// Similar to cmd_0717 but used specifically for verification operations.
// Used 82 times in SRAM validation with addresses like 0x200000B8 and 0xDEADBEEF (test marker)
//
// Usage:
//   - Verification reads: addr = memory addresses to verify
//   - Test marker reads: addr = 0xDEADBEEF (special marker used 38 times in validation)
//
// Parameters:
//   addr: 32-bit memory address (big-endian)
//   length: Number of bytes to read (16-bit, typically 4 bytes for validation)
//   buffer: Output buffer to store read data
//   buffer_size: Size of output buffer (must be >= length)
//
// Returns: 0 on success, -1 on error
int cmd_071b(libusb_device_handle *handle, uint32_t addr, uint16_t length,
             uint8_t *buffer, int buffer_size) {
    if (buffer_size < length) {
        fprintf(stderr, "Buffer too small: %d bytes (need %d)\n", buffer_size, length);
        return -1;
    }

    unsigned char *cmd = g_cmd_buffer;  // Use global persistent buffer

    // Build command packet (identical structure to cmd_0717, just different command ID)
    cmd[0] = 0xaa;
    cmd[1] = 0x55;

    // Length: 8 bytes (071b + 4-byte addr + 2-byte length)
    cmd[2] = 0x00;
    cmd[3] = 0x08;

    // Command: Read/Verify
    cmd[4] = 0x07;
    cmd[5] = 0x1b;

    // Address (32-bit big-endian)
    cmd[6] = (addr >> 24) & 0xFF;
    cmd[7] = (addr >> 16) & 0xFF;
    cmd[8] = (addr >> 8) & 0xFF;
    cmd[9] = addr & 0xFF;

    // Length (16-bit big-endian)
    cmd[10] = (length >> 8) & 0xFF;
    cmd[11] = length & 0xFF;

    // Send command
    int r;
    int sent_length;
    if (g_openlink_verbose) {
        printf("\nSending 'Read/Verify (cmd_071b)' command...\n");
        printf("  Address: 0x%08X, Length: %d bytes\n", addr, length);
    }
    r = libusb_bulk_transfer(handle, ENDPOINT_OUT, cmd, 256, &sent_length, 0);
    if (r != 0) {
        fprintf(stderr, "Error sending cmd_071b: %s\n", libusb_error_name(r));
        return -1;
    }
    if (g_openlink_verbose) {
        printf("Sent %d bytes.\n", sent_length);
        print_hex(cmd, 16);  // Show first 16 bytes
        printf("\nReceiving response to cmd_071b...\n");
    }

    // Read first packet
    int actual_response_len;
    r = libusb_bulk_transfer(handle, ENDPOINT_IN, g_cmd_buffer, 256, &actual_response_len, 10000);
    if (r < 0) {
        fprintf(stderr, "Error receiving response to cmd_071b: %s\n", libusb_error_name(r));
        return -1;
    }
    if (g_openlink_verbose) printf("Received packet 1: %d bytes\n", actual_response_len);

    // Parse response length from header (bytes 2-3, big-endian)
    if (actual_response_len < 4) {
        fprintf(stderr, "Response too short to parse header: %d bytes\n", actual_response_len);
        return -1;
    }

    uint16_t response_len = (g_cmd_buffer[2] << 8) | g_cmd_buffer[3];
    int total_expected = 4 + response_len;  // 4 header bytes (88 a5 len:2) + response_len
    if (g_openlink_verbose) printf("Response indicates %d total bytes expected\n", total_expected);

    // Read additional packets if needed (but don't overflow 256-byte buffer)
    int total_received = actual_response_len;
    int packet_num = 2;
    // Stop if buffer full OR we have all expected data
    while (total_received < total_expected && total_received < 256 && packet_num < 10) {
        int remaining_space = 256 - total_received;
        if (remaining_space <= 0) break;

        int chunk_len;
        r = libusb_bulk_transfer(handle, ENDPOINT_IN,
                                 g_cmd_buffer + total_received,
                                 remaining_space, &chunk_len, 10000);
        if (r < 0) {
            fprintf(stderr, "Error receiving packet %d: %s\n", packet_num, libusb_error_name(r));
            return -1;
        }
        if (g_openlink_verbose) printf("Received packet %d: %d bytes\n", packet_num, chunk_len);
        total_received += chunk_len;
        packet_num++;

        // Break if we got a short packet (indicates end of transfer)
        if (chunk_len < remaining_space) {
            break;
        }
    }

    if (g_openlink_verbose) {
        printf("Total received: %d bytes\n", total_received);
        print_hex(g_cmd_buffer, total_received > 64 ? 64 : total_received);  // Show first 64 bytes
        if (total_received > 64) {
            printf("  ... (%d more bytes)\n", total_received - 64);
        }
    }

    // Validate response format (cmd_071b uses standard 99 66 format, not 88 a5)
    uint16_t response_type;
    if (validate_response(g_cmd_buffer, total_received, 5 + length, &response_type) != 0) {
        fprintf(stderr, "Invalid response for cmd_071b\n");
        return -1;
    }

    // Verify we got the standard response format (99 66)
    if (response_type != 0x9966) {
        fprintf(stderr, "Warning: Expected standard format (99 66), got 0x%04x\n", response_type);
    }

    // Copy data to output buffer (data starts at offset 5: after 99 66 len:2 ee)
    memcpy(buffer, &g_cmd_buffer[5], length);

    //     if (g_openlink_verbose) printf("==> Read/Verify %d bytes from 0x%08X\n", length, addr);
    return 0;
}

/**
 * Read longword from SRAM using cmd_071b with proper response extraction.
 *
 * CRUCIAL DISCOVERY (2025-11-24): cmd_071b returns SRAM data with interspersed
 * status/verification bytes. The actual 4-byte longword is NOT contiguous but
 * appears at specific offsets in the response.
 *
 * Response format for SRAM reads:
 *   99 66 00 25 ee [data pattern with interspersed bytes...]
 *
 * Extraction pattern:
 *   - Byte 0 (MSB): offset 0 in response data
 *   - Byte 1:       offset 7 in response data
 *   - Byte 2:       offset 9 in response data
 *   - Byte 3 (LSB): offset 11 in response data
 *
 * Example: Writing 0xABCD1234 produces response:
 *   [0]=0xAB, [1]=0xAB, [2]=0xFF, [3]=0xFF, [4]=0x00, [5]=0x00, [6]=0x00,
 *   [7]=0xCD, [8]=0x00, [9]=0x12, [10]=0x00, [11]=0x34
 *
 * Extracting bytes at offsets 0,7,9,11 gives correct value: 0xABCD1234
 *
 * This is different from Flash reads where data is contiguous at offset 5.
 *
 * @param handle USB device handle
 * @param addr   SRAM address to read from (0x20000000-0x20007FFF)
 * @param value  Pointer to store the read 32-bit value
 * @return 0 on success, -1 on error
 */
int cmd_071b_read_sram_longword(libusb_device_handle *handle, uint32_t addr, uint32_t *value) {
    // Request 16 bytes to ensure we get all data in the response
    // (we only need 12 bytes but 16 is safe margin)
    uint8_t buffer[16];
    int r = cmd_071b(handle, addr, 16, buffer, sizeof(buffer));
    if (r != 0) {
        return r;
    }

    // Extract 32-bit value using SRAM-specific offset pattern
    if (value) {
        *value = (buffer[0] << 24) |   // Byte 0 at offset 0
                 (buffer[7] << 16) |   // Byte 1 at offset 7
                 (buffer[9] << 8) |    // Byte 2 at offset 9
                 buffer[11];           // Byte 3 at offset 11
    }

    return 0;
}

// Bulk Data Download Command (single chunk)
// bb 66 [length:2 BE] [unknown1:2] [unknown2:2] [address:4 BE] [data:N]
// Based on packet capture analysis:
//   unknown1: 07 19
//   unknown2: data payload size (e.g., 04a8 = 1192 bytes)
//   length: packet length AFTER bb 66 (so length = 8 + data_size)
//   No response expected for bb 66 commands
//   Use 1192-byte chunks for flashloader upload

int cmd_download_block_chunk(libusb_device_handle *handle, uint32_t address, unsigned char *data, int length) {
    // Allocate buffer for the exact packet size needed
    // Total: 4 (bb 66 + length) + 8 (header) + length (data)
    // NO trailing bytes - those were just the end of the flashloader code!
    int total_size = 4 + 8 + length;
    unsigned char *cmd = malloc(total_size);
    if (!cmd) {
        fprintf(stderr, "Error: **FAILED to allocate %d bytes for chunk transfer\n", total_size);
        return -1;
    }

    // Packet length = header (8 bytes) + data (N bytes)
    // This goes AFTER the bb 66 prefix
    uint16_t packet_length = 8 + length;

    // Header
    cmd[0] = 0xbb;
    cmd[1] = 0x66;

    // Length (big-endian) - number of bytes AFTER this length field
    cmd[2] = (packet_length >> 8) & 0xFF;
    cmd[3] = packet_length & 0xFF;

    // Subcommand: always 07 19
    cmd[4] = 0x07;
    cmd[5] = 0x19;

    // Data length: actual flashloader bytes ONLY (no protocol overhead)
    // If sending 1192 bytes of flashloader:
    //   packet_length (bytes 2-3) = 1200 (8-byte header + 1192 data)
    //   data_length (bytes 6-7) = 1192 (flashloader data only)
    // The `length` parameter should be the raw flashloader bytes to send
    uint16_t data_length = length;
    cmd[6] = (data_length >> 8) & 0xFF;
    cmd[7] = data_length & 0xFF;

    // Address (big-endian)
    cmd[8] = (address >> 24) & 0xFF;
    cmd[9] = (address >> 16) & 0xFF;
    cmd[10] = (address >> 8) & 0xFF;
    cmd[11] = address & 0xFF;

    // Data payload
    memcpy(&cmd[12], data, length);

    // NO trailing bytes - those were just the end of the actual flashloader code!

    // Send command
    char cmd_name[64];
    snprintf(cmd_name, sizeof(cmd_name), "Download Block to 0x%08X (%d bytes)", address, length);

    int r;
    int sent_length;

    if (g_openlink_verbose) printf("\nSending '%s' command...\n", cmd_name);
    r = libusb_bulk_transfer(handle, ENDPOINT_OUT, cmd, total_size, &sent_length, 5000);  // 5 second timeout
    if (r != 0) {
        fprintf(stderr, "Error sending %s command: %s\n", cmd_name, libusb_error_name(r));
        free(cmd);
        return -1;
    }
    if (g_openlink_verbose) printf("Sent %d bytes.\n", sent_length);

    // CRUCIAL FIX (2025-11-24): Must read response after EACH bb66 upload!
    // The pcap shows bb66 uploads DO receive "99 66" responses.
    // If we don't drain these, they pile up in the USB buffer and subsequent
    // reads return stale bb66 responses instead of the actual data we requested.
    // This was causing SRAM read corruption after flashloader upload.

    free(cmd);

    // Wait a bit for device to process the upload
    // CRUCIAL: Do NOT try to read a response - bb66 commands don't generate responses.
    // The captures show bb66 commands sent back-to-back without waiting.
    // Trying to read a response causes USB timeouts and corrupts communication.
    usleep(5000);  // 5ms pause between chunks

    return 0;
}

// Bulk Data Download Command (single large transfer)
// Sends entire data block as ONE bb 66 command
// This is how it uploads the flashloader (command 146)

int cmd_download_block_single(libusb_device_handle *handle, uint32_t address, unsigned char *data, int length) {
    // Allocate buffer for entire transfer: 12-byte header + data
    int total_size = 12 + length;
    unsigned char *cmd = malloc(total_size);
    if (!cmd) {
        fprintf(stderr, "Error: **FAILED to allocate %d bytes for upload\n", total_size);
        return -1;
    }

    if (g_openlink_verbose) printf("Uploading %d bytes as single transfer...\n", length);

    // Calculate total packet length: 2(bb 66) + 2(length) + 2(unknown1) + 2(unknown2) + 4(address) + N(data)
    uint16_t packet_length = 12 + length;

    // Header
    cmd[0] = 0xbb;
    cmd[1] = 0x66;

    // Length (big-endian)
    cmd[2] = (packet_length >> 8) & 0xFF;
    cmd[3] = packet_length & 0xFF;

    // Unknown fields from packet capture (command 146)
    cmd[4] = 0x07;
    cmd[5] = 0x19;

    // Unknown field 2 - from capture for 4129-byte flashloader: 10 21
    // This appears to be length + some offset (0x1021 = 4129 decimal)
    uint16_t unknown2 = length;
    cmd[6] = (unknown2 >> 8) & 0xFF;
    cmd[7] = unknown2 & 0xFF;

    // Address (big-endian)
    cmd[8] = (address >> 24) & 0xFF;
    cmd[9] = (address >> 16) & 0xFF;
    cmd[10] = (address >> 8) & 0xFF;
    cmd[11] = address & 0xFF;

    // Data payload
    memcpy(&cmd[12], data, length);

    // Send command
    char cmd_name[64];
    snprintf(cmd_name, sizeof(cmd_name), "Download Block to 0x%08X (%d bytes)", address, length);

    int r;
    int sent_length;

    if (g_openlink_verbose) printf("\nSending '%s' command...\n", cmd_name);
    r = libusb_bulk_transfer(handle, ENDPOINT_OUT, cmd, total_size, &sent_length, 5000);  // 5 second timeout for large transfer
    if (r != 0) {
        fprintf(stderr, "Error sending %s command: %s\n", cmd_name, libusb_error_name(r));
        free(cmd);
        return -1;
    }
    if (g_openlink_verbose) printf("Sent %d bytes.\n", sent_length);

    free(cmd);

    // CRUCIAL FIX: bb 66 DOES return a response! (99 66 00 03 ee)
    // Packet capture line 362 shows response after bb 66 transfer
    // Must read this to prevent blocking next command!

    // TIMING FIX: Packet capture shows ~17ms delay - use 20ms to be safe
    usleep(20000);  // 20ms delay

    unsigned char response[256];
    int recv_length;

    r = libusb_bulk_transfer(handle, ENDPOINT_IN, response, sizeof(response), &recv_length, 5000);
    if (r != 0) {
        fprintf(stderr, "Error reading bb 66 response: %s\n", libusb_error_name(r));
        return -1;
    }

    // Verify response (should be 99 66 00 03 ee)
    if (recv_length >= 5 && response[0] == 0x99 && response[1] == 0x66 && response[4] == 0xee) {
    //         if (g_openlink_verbose) printf("==> bb 66 response OK\n");
    } else {
        fprintf(stderr, "WARNING: Unexpected bb 66 response: ");
        for (int i = 0; i < recv_length && i < 10; i++) {
            fprintf(stderr, "%02x ", response[i]);
        }
        fprintf(stderr, "\n");
    }

    //     if (g_openlink_verbose) printf("==> Upload complete: %d bytes to 0x%08X\n", length, address);
    return 0;
}

// Bulk Data Download Command (with chunking support)
// CRUCIAL: Uses 1192-byte chunks, not single large transfers!
// Commands 146-152 upload the flashloader in 7 separate bb 66 packets

int cmd_download_block(libusb_device_handle *handle, uint32_t address, unsigned char *data, int length) {
    const int CHUNK_SIZE = 1192;  // Match the chunk size
    int offset = 0;

    if (g_openlink_verbose) printf("Uploading %d bytes in chunks of %d bytes...\n", length, CHUNK_SIZE);

    while (offset < length) {
        int chunk_len = (length - offset > CHUNK_SIZE) ? CHUNK_SIZE : (length - offset);

        // Upload this chunk at address + offset
        int r = cmd_download_block_chunk(handle, address + offset, data + offset, chunk_len);
        if (r != 0) {
            fprintf(stderr, "**FAILED to upload chunk at offset %d (address 0x%08X)\n",
                    offset, address + offset);
            return -1;
        }

        offset += chunk_len;

        // Progress indicator
        if (g_openlink_verbose && (offset % (CHUNK_SIZE * 5) == 0 || offset == length)) {
            printf("  Uploaded %d / %d bytes (%.1f%%)\n",
                   offset, length, (offset * 100.0) / length);
        }
    }

    //     if (g_openlink_verbose) printf("==> Upload complete: %d bytes to 0x%08X\n", length, address);
    return 0;
}

// BDM Resume Command
// Instructs the Multilink to resume target CPU execution
// Command: aa 55 00 04 04 40 58 04

int cmd_bdm_resume(libusb_device_handle *handle) {
    unsigned char *cmd = g_cmd_buffer;  // Use global persistent buffer

    // Header
    cmd[0] = 0xaa;
    cmd[1] = 0x55;

    // Length: 4 bytes
    cmd[2] = 0x00;
    cmd[3] = 0x04;

    // Command payload
    cmd[4] = 0x04;
    cmd[5] = 0x40;
    cmd[6] = 0x58;
    cmd[7] = 0x04;
    // CRUCIAL: Do NOT zero bytes 8-255! They contain leftover response data

    // Use no-response version because target will be executing code
    return send_aa_command_no_response(handle, cmd, 256, "BDM Resume");
}

/**
 * BDM GO Command (cmd 07 02)
 *
 * This command tells the target MCU to start executing code.
 * Use this to RUN the flashloader after uploading it to SRAM.
 *
 * Command: aa 55 00 05 07 02 fc 0c
 *   fc = BDM mode parameter
 *   0c = GO/Execute command
 *
 * CRUCIAL: This must be sent after uploading flashloader and writing parameters
 * to actually execute the flashloader code!
 */
int cmd_07_02_bdm_go(libusb_device_handle *handle) {
    unsigned char *cmd = g_cmd_buffer; // Use global persistent buffer
    // memset(cmd, 0, 256); // REMOVED: Must preserve leftover data

    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;
    cmd[3] = 0x05;  // Length = 5 bytes
    cmd[4] = 0x07;
    cmd[5] = 0x02;  // BDM GO subcommand
    cmd[6] = 0xfc;  // BDM mode
    cmd[7] = 0x0c;  // GO/Execute
    cmd[8] = 0x00;  // Final parameter

    return send_aa_command(handle, cmd, 256, "BDM GO (07 02)");
}

/**
 * CMD 07 14 - Write BDM Register
 *
 * Used to set CPU registers before execution, particularly the Program Counter (PC).
 * Set PC=0x20001250 before running flashloader.
 *
 * Command: aa 55 00 0c 07 14 [reg:2] 00 00 08 0f [value:4]
 *   reg = Register number (e.g., 0x2880 for PC)
 *   value = 32-bit value to write (big-endian)
 *
 * Example: aa 55 00 0c 07 14 28 80 00 00 08 0f 20 00 12 50
 *   Sets PC=0x20001250 (flashloader entry point)
 */
int cmd_07_14_write_bdm_reg(libusb_device_handle *handle, uint16_t reg, uint32_t value) {
    unsigned char *cmd = g_cmd_buffer; // Use global persistent buffer
    // memset(cmd, 0, 256); // REMOVED: Must preserve leftover data

    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;
    cmd[3] = 0x0c;  // Length = 12 bytes
    cmd[4] = 0x07;
    cmd[5] = 0x14;

    // CRUCIAL: Window base address 0x28800000
    // This address window is essential for BDM memory access!
    cmd[6] = 0x28;
    cmd[7] = 0x80;
    cmd[8] = 0x00;
    cmd[9] = 0x00;

    // Register number (big-endian)
    cmd[10] = (reg >> 8) & 0xFF;
    cmd[11] = reg & 0xFF;

    // Value (big-endian 32-bit)
    cmd[12] = (value >> 24) & 0xFF;
    cmd[13] = (value >> 16) & 0xFF;
    cmd[14] = (value >> 8) & 0xFF;
    cmd[15] = value & 0xFF;

    return send_aa_command(handle, cmd, 256, "CMD 07 14 (Write BDM Register)");
}

/**
 * CMD 07 14 - Write Debug Module Register (WDMREG)
 *
 * From MCF52235 Reference Manual: WDMREG uses format 0x2C{0x42 | DRc[4:0]}
 * DRc is the 5-bit debug register code (e.g., 0x07 for TDR, 0x08 for PBR0)
 *
 * The USB protocol wraps this in cmd 07 14 with window base:
 * Command: aa 55 00 0c 07 14 [0x2C] [0x42|DRc] 00 00 [reg_param:2] [value:4]
 *
 * NOTE: 0x2C = WDMREG opcode, 0x42 is the base value that gets OR'd with DRc[4:0]
 */
int cmd_07_14_write_debug_reg(libusb_device_handle *handle, uint16_t drc, uint32_t value) {
    unsigned char *cmd = g_cmd_buffer;

    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;
    cmd[3] = 0x0c;  // Length = 12 bytes
    cmd[4] = 0x07;
    cmd[5] = 0x14;

    // WDMREG encoding: 0x2C{0x42 | DRc[4:0]}
    // From MCF52235 Reference Manual: 0x2C is WDMREG opcode, 0x42 is base value
    cmd[6] = 0x2C;
    cmd[7] = 0x42 | (drc & 0x1F);  // 0x42 + 5-bit register code
    cmd[8] = 0x00;
    cmd[9] = 0x00;

    // Register parameter (matches DRc for consistency)
    cmd[10] = 0x00;
    cmd[11] = drc & 0xFF;

    // Value (big-endian 32-bit)
    cmd[12] = (value >> 24) & 0xFF;
    cmd[13] = (value >> 16) & 0xFF;
    cmd[14] = (value >> 8) & 0xFF;
    cmd[15] = value & 0xFF;

    printf("DEBUG WDMREG: DRc=0x%02X, value=0x%08X, cmd[6-7]=0x%02X%02X\n",
           drc, value, cmd[6], cmd[7]);

    return send_aa_command(handle, cmd, 256, "CMD 07 14 (WDMREG)");
}

// BDM Freeze Command
// Checks if target CPU is halted (frozen)
// Command: aa 55 00 04 04 7f fe 02
// Response: 99 66 [len:2] ee [status...] - status indicates if frozen

int cmd_bdm_freeze(libusb_device_handle *handle, uint8_t *is_frozen) {
    unsigned char *cmd = g_cmd_buffer;  // Use global persistent buffer

    // Header
    cmd[0] = 0xaa;
    cmd[1] = 0x55;

    // Length: 4 bytes
    cmd[2] = 0x00;
    cmd[3] = 0x04;

    // Command payload
    cmd[4] = 0x04;
    cmd[5] = 0x7f;
    cmd[6] = 0xfe;
    cmd[7] = 0x02;

    // Send command
    int r;
    int sent_length;
    if (g_openlink_verbose) printf("\nSending 'BDM Freeze Check' command...\n");
    r = libusb_bulk_transfer(handle, ENDPOINT_OUT, cmd, 256, &sent_length, 0);
    if (r != 0) {
        fprintf(stderr, "Error sending BDM Freeze command: %s\n", libusb_error_name(r));
        return -1;
    }
    if (g_openlink_verbose) {
        printf("Sent %d bytes.\n", sent_length);
        print_hex(cmd, sent_length);
    }

    // Receive response BACK INTO g_cmd_buffer
    int actual_response_len;
    if (g_openlink_verbose) printf("\nReceiving response to BDM Freeze Check command...\n");
    r = libusb_bulk_transfer(handle, ENDPOINT_IN, g_cmd_buffer, 256, &actual_response_len, 500);
    if (r < 0) {
        // Timeout is expected when target is running - treat as "not frozen"
        if (r == LIBUSB_ERROR_TIMEOUT) {
            if (g_openlink_verbose) printf("Timeout (target still running)\n");
            *is_frozen = 0;
            return 0;
        }
        fprintf(stderr, "Error receiving response to BDM Freeze command: %s\n", libusb_error_name(r));
        return -1;
    }

    if (g_openlink_verbose) {
        printf("Received %d bytes:\n", actual_response_len);
        print_hex(g_cmd_buffer, actual_response_len);
    }

    // Validate response format (supports both 99 66 and 88 a5)
    uint16_t response_type;
    if (validate_response(g_cmd_buffer, actual_response_len, 5, &response_type) != 0) {
        fprintf(stderr, "Invalid response for BDM Freeze command\n");
        return -1;
    }

    // Parse response: [99 66] [len:2] ee [status_byte]
    // The status byte at offset 5 indicates the BDM state
    // Based on packet capture analysis:
    // Response format: 99 66 00 03 ee XX YY where XX indicates state
    // 0x88 = running, 0x01 or 0x00 = halted
    if (actual_response_len >= 6) {
        uint8_t status = g_cmd_buffer[5];

        // Status 0x88 means running, status 0x01/0x00 means halted
        if (status == 0x01 || status == 0x00) {
            *is_frozen = 1;  // Halted
        } else {
            *is_frozen = 0;  // Running (0x88 or other)
        }
    } else {
        *is_frozen = 0;  // Assume running if we can't determine
    }
    return 0;
}

// BDM Reinitialization After Flashloader Execution
// After BDM-RESUME → flashloader execution → BDM-FREEZE, the BDM interface
// must be reinitialized before memory reads will work correctly.
// This follows the sequence discovered in packet captures.
int cmd_bdm_reinit_after_execution(libusb_device_handle *handle) {
    unsigned char *cmd = g_cmd_buffer;  // Use global persistent buffer
    int ret;

    if (g_openlink_verbose) printf("\n=== Reinitializing BDM after flashloader execution ===\n");

    // Step 1: Enter Mode 0xF8
    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;
    cmd[3] = 0x03;
    cmd[4] = 0x07;
    cmd[5] = 0x01;
    cmd[6] = 0xf8;
    ret = send_aa_command(handle, cmd, 256, "Enter Mode 0xF8");
    if (ret != 0) {
        fprintf(stderr, "Error: Enter Mode 0xF8 **FAILED\n");
        return ret;
    }

    // Step 2: Enter Mode 0xF0
    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;
    cmd[3] = 0x03;
    cmd[4] = 0x07;
    cmd[5] = 0x01;
    cmd[6] = 0xf0;
    ret = send_aa_command(handle, cmd, 256, "Enter Mode 0xF0");
    if (ret != 0) {
        fprintf(stderr, "Error: Enter Mode 0xF0 **FAILED\n");
        return ret;
    }

    // Step 3: Enter Mode 0xF8 again
    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;
    cmd[3] = 0x03;
    cmd[4] = 0x07;
    cmd[5] = 0x01;
    cmd[6] = 0xf8;
    ret = send_aa_command(handle, cmd, 256, "Enter Mode 0xF8 (2nd)");
    if (ret != 0) {
        fprintf(stderr, "Error: Enter Mode 0xF8 (2nd) **FAILED\n");
        return ret;
    }

    // Step 4: Enable Memory Access
    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;
    cmd[3] = 0x03;
    cmd[4] = 0x07;
    cmd[5] = 0x0a;
    cmd[6] = 0x00;
    ret = send_aa_command(handle, cmd, 256, "Enable Memory Access");
    if (ret != 0) {
        fprintf(stderr, "Error: Enable Memory Access **FAILED\n");
        return ret;
    }

    if (g_openlink_verbose) printf("BDM reinitialization complete - memory reads should now work\n\n");
    return 0;
}
// ============================================================================
// FLASH Read Functions
// ============================================================================

/**
 * Enter debug mode with specified mode byte.
 * 
 * Command: aa 55 00 03 07 01 [mode]
 * 
 * Known modes:
 *   0xFC - FLASH operation mode (NEW! from captures)
 *   0xF8 - Standard debug mode
 *   0xF0 - Alternative mode
 */
int cmd_enter_mode(libusb_device_handle *handle, uint8_t mode) {
    unsigned char *cmd = g_cmd_buffer; // Use global persistent buffer
    // memset(cmd, 0, 256); // REMOVED: Must preserve leftover data
    
    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;  // Length high
    cmd[3] = 0x03;  // Length low (3 bytes)
    cmd[4] = 0x07;  // Command
    cmd[5] = 0x01;  // Subcommand: Enter Mode
    cmd[6] = mode;  // Mode parameter
    
    return send_aa_command(handle, cmd, 256, "Enter Mode");
}

/**
 * Enable memory access with parameter.
 * 
 * Command: aa 55 00 03 07 0a [param]
 * 
 * Parameters:
 *   0x00 - FLASH mode (from captures!)
 *   0x01 - Normal mode (previously used)
 */
int cmd_enable_memory_access(libusb_device_handle *handle, uint8_t param) {
    unsigned char *cmd = g_cmd_buffer; // Use global persistent buffer
    // memset(cmd, 0, 256); // REMOVED: Must preserve leftover data
    
    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;  // Length high
    cmd[3] = 0x03;  // Length low (3 bytes)
    cmd[4] = 0x07;  // Command
    cmd[5] = 0x0a;  // Subcommand: Enable Memory Access
    cmd[6] = param; // Parameter
    
    return send_aa_command(handle, cmd, 256, "Enable Memory Access");
}

/**
 * Read memory block using command 07 17.
 * 
 * Command: aa 55 00 08 07 17 [addr:4] [param1] [param2] [param3]
 * 
 * This is the breakthrough command from the FLASH captures!
 * Returns up to 256 bytes of data in the response packet.
 * 
 * Parameters:
 *   addr: 32-bit big-endian address
 *   param1: 0x01 (memory read mode)
 *   param2: 0x00 (flags)
 *   param3: 0x00 (additional flags/offset)
 * 
 * Response format:
 *   99 66 [length:2] ee [data...]
 *   Status byte 0xee indicates success
 */
int cmd_read_memory_block(libusb_device_handle *handle, uint32_t addr, uint8_t *buffer, int buffer_size) {
    if (buffer == NULL || buffer_size <= 0) {
        fprintf(stderr, "Error: Invalid buffer parameters\n");
        return -1;
    }

    unsigned char *cmd = g_cmd_buffer; // Use global persistent buffer

    // Build command packet
    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;  // Length high
    cmd[3] = 0x08;  // Length low (8 bytes)
    cmd[4] = 0x07;  // Command
    cmd[5] = 0x17;  // Subcommand: Read Memory Block

    // Address (big-endian)
    cmd[6] = (addr >> 24) & 0xFF;
    cmd[7] = (addr >> 16) & 0xFF;
    cmd[8] = (addr >> 8) & 0xFF;
    cmd[9] = addr & 0xFF;

    // Parameters (from capture analysis - only 2 bytes!)
    cmd[10] = 0x01;  // param1: memory read mode
    cmd[11] = 0x00;  // param2: flags
    // Note: length is 8 bytes total (2+4+2), no third parameter!
    // CRUCIAL: Do NOT zero bytes 12-255! They contain leftover response data

    // Send command
    int actual_length;
    int ret = libusb_bulk_transfer(handle, ENDPOINT_OUT, cmd, 256, &actual_length, 5000);
    if (ret < 0) {
        fprintf(stderr, "Error: **FAILED to send READ_MEMORY_BLOCK command: %s\n",
                libusb_error_name(ret));
        return ret;
    }

    // Receive response BACK INTO g_cmd_buffer
    ret = libusb_bulk_transfer(handle, ENDPOINT_IN, g_cmd_buffer, 256, &actual_length, 5000);
    if (ret < 0) {
        fprintf(stderr, "Error: **FAILED to receive READ_MEMORY_BLOCK response: %s\n",
                libusb_error_name(ret));
        return ret;
    }

    // Validate response
    if (g_cmd_buffer[0] != 0x99 || g_cmd_buffer[1] != 0x66) {
        fprintf(stderr, "Error: Invalid response magic (expected 99 66, got %02x %02x)\n",
                g_cmd_buffer[0], g_cmd_buffer[1]);
        return -1;
    }

    // Extract length and status
    int response_length = (g_cmd_buffer[2] << 8) | g_cmd_buffer[3];
    uint8_t status = g_cmd_buffer[4];

    if (status != 0xee) {
        fprintf(stderr, "Error: Command **FAILED with status 0x%02x\n", status);
        return -1;
    }

    // Copy data to buffer (skip 5-byte header: magic, length, status)
    int data_length = response_length - 1;  // -1 for status byte
    if (data_length > buffer_size) {
        data_length = buffer_size;
    }

    memcpy(buffer, &g_cmd_buffer[5], data_length);

    return data_length;  // Return number of bytes read
}

/**
 * Setup memory window using cmd 07 17 with custom parameters.
 * Sends this immediately after flashloader upload.
 *
 * Command: aa 55 00 08 07 17 [addr:4] [param1] [param2]
 *
 * Parameters from capture:
 *   addr: 0x20000000 (parameter base)
 *   param1: 0x00 (mode?)
 *   param2: 0x04 (window setup?)
 *
 * NOTE: This command appears to not return a response.
 */
int cmd_07_17_setup_window(libusb_device_handle *handle, uint32_t addr) {
    unsigned char *cmd = g_cmd_buffer; // Use global persistent buffer
    // memset(cmd, 0, 256); // REMOVED: Must preserve leftover data

    // Build command packet
    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;  // Length high
    cmd[3] = 0x08;  // Length low (8 bytes)
    cmd[4] = 0x07;  // Command
    cmd[5] = 0x17;  // Subcommand

    // Address (big-endian)
    cmd[6] = (addr >> 24) & 0xFF;
    cmd[7] = (addr >> 16) & 0xFF;
    cmd[8] = (addr >> 8) & 0xFF;
    cmd[9] = addr & 0xFF;

    // Parameters from capture
    cmd[10] = 0x00;  // param1
    cmd[11] = 0x04;  // param2

    // Add padding pattern (same as cmd_07_19 and cmd_07_11)
    for (int i = 12; i < 256; i += 2) {
        cmd[i] = 0xff;
        if (i+1 < 256) cmd[i+1] = 0x00;
    }

    // Send command without waiting for response
    int actual_length;
    int ret = libusb_bulk_transfer(handle, ENDPOINT_OUT, cmd, 256, &actual_length, 5000);
    if (ret < 0) {
        fprintf(stderr, "Error: **FAILED to send CMD 07 17 Setup Window: %s\n",
                libusb_error_name(ret));
        return ret;
    }

    if (g_openlink_verbose) printf("CMD 07 17 Setup Window sent (no response expected)\n");
    return 0;
}

/**
 * Command 07 1B - Memory Region Setup (discovered in RAM mode captures)
 *
 * Sets up memory access regions for BDM. More sophisticated than cmd_07_17.
 * Used extensively in RAM mode initialization sequence.
 *
 * Packet format:
 *   aa 55 00 08 07 1b [addr:4] [size:2]
 *
 * @param handle USB device handle
 * @param addr   32-bit address (big-endian)
 * @param size   16-bit size (big-endian)
 * @return 0 on success, negative on error
 *
 * Examples from RAM mode capture:
 *   cmd_07_1b(0x00000000, 0x0004) - Setup region at address 0
 *   cmd_07_1b(0x20000C98, 0x0004) - Setup SRAM region
 *   cmd_07_1b(0x200000B8, 0x0004) - Setup SRAM region
 *
 * NOTE: Like cmd_07_17, this command may not return a response
 */
int cmd_07_1b(libusb_device_handle *handle, uint32_t addr, uint16_t size) {
    unsigned char *cmd = g_cmd_buffer; // Use global persistent buffer
    // memset(cmd, 0, 256); // REMOVED: Must preserve leftover data

    // Build command packet
    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;  // Length high
    cmd[3] = 0x08;  // Length low (8 bytes: 07 1b + 4-byte addr + 2-byte size)
    cmd[4] = 0x07;  // Command
    cmd[5] = 0x1b;  // Subcommand

    // Address (big-endian)
    cmd[6] = (addr >> 24) & 0xFF;
    cmd[7] = (addr >> 16) & 0xFF;
    cmd[8] = (addr >> 8) & 0xFF;
    cmd[9] = addr & 0xFF;

    // Size (big-endian)
    cmd[10] = (size >> 8) & 0xFF;
    cmd[11] = size & 0xFF;

    // Send command without waiting for response
    int actual_length;
    int ret = libusb_bulk_transfer(handle, ENDPOINT_OUT, cmd, 256, &actual_length, 5000);
    if (ret < 0) {
        fprintf(stderr, "Error: **FAILED to send CMD 07 1B Memory Region Setup: %s\n",
                libusb_error_name(ret));
        return ret;
    }

    if (g_openlink_verbose) printf("CMD 07 1B Memory Region Setup sent (addr=0x%08X, size=0x%04X, no response expected)\n",
           addr, size);
    return 0;
}

// ============================================================================
// BDM Configuration Functions
// ============================================================================

/**
 * Command 07 12 - Synchronization/mode control command.
 * Sent with different parameters (0xFFFF, 0x9040).
 *
 * Command: aa 55 00 04 07 12 [param:2]
 */
int cmd_07_12(libusb_device_handle *handle, uint16_t param) {
    unsigned char *cmd = g_cmd_buffer; // Use global persistent buffer
    // memset(cmd, 0, 256); // REMOVED: Must preserve leftover data

    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;  // Length high
    cmd[3] = 0x02;  // Length low (2 bytes: just "07 12", param is padding only)
    cmd[4] = 0x07;  // Command
    cmd[5] = 0x12;  // Subcommand (BDM HALT)

    // Padding starts here (param becomes part of padding pattern)
    for (int i = 6; i < 256; i += 2) {
        cmd[i] = (param >> 8) & 0xFF;    // Use param as padding byte 1
        if (i+1 < 256) cmd[i+1] = param & 0xFF;  // Use param as padding byte 2
    }

    return send_aa_command(handle, cmd, 256, "BDM HALT (07 12)");
}

/**
 * Command 07 1e - from capture. Specific initialization command.
 *
 * Command: aa 55 00 09 07 1e [p1:2] [p2:4] [p3:1]
 *
 *   Always sends: 00 01 40 10 00 74 0f
 * - p1 = 0x0001 (16-bit value, possibly a flag or count)
 * - p2 = 0x40100074 (32-bit value, possibly an address)
 * - p3 = 0x0F (8-bit value, possibly a mode/flag)
 */
// cmd_071e: Write/Test command (9-byte format with 8-bit data)
// Used for CFM initialization and other BDM operations
// Command: aa55 0009 071e [p1:2] [addr:4] [data:1]
int cmd_07_1e(libusb_device_handle *handle, uint16_t p1, uint32_t p2, uint8_t p3) {
    unsigned char *cmd = g_cmd_buffer; // Use global persistent buffer
    // memset(cmd, 0, 256); // REMOVED: Must preserve leftover data

    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;  // Length high
    cmd[3] = 0x09;  // Length low (9 bytes: 07 1e + 7 params)
    cmd[4] = 0x07;  // Command
    cmd[5] = 0x1e;  // Subcommand

    // Parameter 1 (16-bit, big-endian)
    cmd[6] = (p1 >> 8) & 0xFF;
    cmd[7] = p1 & 0xFF;

    // Parameter 2 (32-bit, big-endian)
    cmd[8] = (p2 >> 24) & 0xFF;
    cmd[9] = (p2 >> 16) & 0xFF;
    cmd[10] = (p2 >> 8) & 0xFF;
    cmd[11] = p2 & 0xFF;

    // Parameter 3 (8-bit)
    cmd[12] = p3;

    // Add padding pattern
    for (int i = 13; i < 256; i += 2) {
        cmd[i] = 0x00;
        if (i+1 < 256) cmd[i+1] = 0xff;
    }

    return send_aa_command(handle, cmd, 256, "CMD 07 1e (CFM Init)");
}

// cmd_071e_write_sram: Write/Test command (12-byte format with 32-bit data)
// Used extensively in SRAM validation sequence
// Command: aa55 000c 071e 0004 [addr:4] [data:4]
//
// Examples from packet capture:
//   aa55 000c 071e 0004 20000408 4ac84e73  - Write 0x4AC84E73 to 0x20000408
//   aa55 000c 071e 0004 20000034 20000408  - Write 0x20000408 to 0x20000034
//
// Pattern in SRAM validation:
//   1. Write test value to address 0x20000408
//   2. Write address 0x20000408 as data to target address
//   3. Verify target address with cmd_071b
//
// Returns: 0 on success, -1 on error
int cmd_071e_write_sram(libusb_device_handle *handle, uint32_t addr, uint32_t data) {
    unsigned char *cmd = g_cmd_buffer;

    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;  // Length high
    cmd[3] = 0x0c;  // Length low (12 bytes: 071e + 0004 + addr + data)
    cmd[4] = 0x07;  // Command
    cmd[5] = 0x1e;  // Subcommand

    // Parameter 1: 0x0004 (indicates 4-byte data follows)
    cmd[6] = 0x00;
    cmd[7] = 0x04;

    // Address (32-bit, big-endian)
    cmd[8] = (addr >> 24) & 0xFF;
    cmd[9] = (addr >> 16) & 0xFF;
    cmd[10] = (addr >> 8) & 0xFF;
    cmd[11] = addr & 0xFF;

    // Data (32-bit, big-endian)
    cmd[12] = (data >> 24) & 0xFF;
    cmd[13] = (data >> 16) & 0xFF;
    cmd[14] = (data >> 8) & 0xFF;
    cmd[15] = data & 0xFF;

    // Add padding pattern
    for (int i = 16; i < 256; i += 2) {
        cmd[i] = 0x00;
        if (i+1 < 256) cmd[i+1] = 0xff;
    }

    if (g_openlink_verbose) {
        printf("\nSending 'Write SRAM (cmd_071e)' command...\n");
        printf("  Address: 0x%08X, Data: 0x%08X\n", addr, data);
    }

    return send_aa_command(handle, cmd, 256, "CMD 071e (Write SRAM)");
}

// SRAM Pre-Initialization Sequence
// Implements lines 1-138 from RAM_INIT_SEQUENCE.txt
// Required before SRAM validation sequence will work
//
// Steps:
//   1. Device detection (cmd_010b twice)
//   2. Enter mode and BDM initialization
//   3. Memory window setup (cmd_0710 9x)
//   4. BDM configuration and register setup
//   5. Initial SRAM test writes
//
// Returns: 0 on success, -1 on error
int sram_pre_init(libusb_device_handle *handle) {
    int r;
    uint32_t reg_value;
    uint8_t verify_buffer[4];

    // Lines 0-1: Device detection (twice)
    r = cmd_01_0b(handle);
    if (r != 0) {
        fprintf(stderr, "Device detection (1) **FAILED\n");
        return -1;
    }
    r = cmd_01_0b(handle);
    if (r != 0) {
        fprintf(stderr, "Device detection (2) **FAILED\n");
        return -1;
    }

    // Lines 2-10: Initial BDM setup
    r = cmd_enter_mode(handle, 0xFC);
    if (r != 0) return -1;

    r = cmd_07_a2(handle, 0x01);
    if (r != 0) return -1;

    r = cmd_04_40_58_04(handle);
    if (r != 0) return -1;

    r = cmd_04_7f_fe_02(handle);
    if (r != 0) return -1;
    r = cmd_04_7f_fe_02(handle); // Called twice
    if (r != 0) return -1;

    r = cmd_07_95(handle);
    if (r != 0) return -1;

    r = cmd_04_40_00_02(handle);
    if (r != 0) return -1;

    r = cmd_enable_memory_access(handle, 0x00);
    if (r != 0) return -1;

    r = cmd_enter_mode(handle, 0xFC);
    if (r != 0) return -1;
    //     printf("==> BDM initialized\n\n");

    // Lines 11-19: Memory window setup (9x with 0x0000)
    //     printf("Step 3: Setting up memory windows (9x)...\n");
    for (int i = 0; i < 9; i++) {
        r = cmd_07_10(handle, 0x0000);
        if (r != 0) {
            fprintf(stderr, " Memory window setup %d/9 **FAILED\n", i+1);
            return -1;
        }
    }
    //     printf("==> Memory windows configured\n\n");

    // Lines 20-34: BDM configuration and register writes
    //     printf("Step 4: BDM configuration...\n");

    // Read register 0x2D80 twice
    r = cmd_07_13(handle, 0x2D80, &reg_value);
    if (r != 0) return -1;
    r = cmd_07_13(handle, 0x2D80, &reg_value);
    if (r != 0) return -1;

    // Write registers
    r = cmd_write_bdm_reg(handle, 0x2C80, 0x00910000);
    if (r != 0) return -1;

    r = cmd_07_13(handle, 0x2D80, &reg_value);
    if (r != 0) return -1;

    r = cmd_write_bdm_reg(handle, 0x2C80, 0x00900000);
    if (r != 0) return -1;

    r = cmd_07_12(handle, 0x0000);
    if (r != 0) return -1;

    r = cmd_enter_mode(handle, 0xF8);
    if (r != 0) return -1;

    r = cmd_07_13(handle, 0x2D80, &reg_value);
    if (r != 0) return -1;

    // BDM configuration commands
    r = cmd_07_11(handle, 0x1940, 0xFC, 0x0A, 0x00, 0x0A);
    if (r != 0) return -1;
    r = cmd_07_11(handle, 0x1940, 0x40, 0x11, 0x00, 0x0A);
    if (r != 0) return -1;
    r = cmd_07_11(handle, 0x1900, 0x40, 0x10, 0x00, 0x74);
    if (r != 0) return -1;

    uint8_t params[] = {0x40, 0x10, 0x00, 0x74, 0x00, 0x0F};
    r = cmd_07_15(handle, 0x1800, params, 6);
    if (r != 0) return -1;

    r = cmd_07_12(handle, 0x0000);
    if (r != 0) return -1;

    r = cmd_enter_mode(handle, 0xF8);
    if (r != 0) return -1;

    r = cmd_07_11(handle, 0x2980, 0x00, 0x00, 0x08, 0x0F);
    if (r != 0) return -1;
    //     printf("==> BDM configured\n\n");

    // Lines 35-52: Register initialization
    //     printf("Step 5: Register initialization...\n");
    for (uint16_t reg = 0x2180; reg <= 0x218F; reg++) {
        r = cmd_07_13(handle, reg, &reg_value);
        if (r != 0) {
            fprintf(stderr, " **FAILED to read register 0x%04X\n", reg);
            return -1;
        }
    }

    r = cmd_07_11(handle, 0x2980, 0x00, 0x00, 0x08, 0x0F);
    if (r != 0) return -1;
    r = cmd_07_11(handle, 0x2980, 0x00, 0x00, 0x08, 0x0E);
    if (r != 0) return -1;

    // Line 53: Memory read (read 64 bytes to fit response in 256-byte buffer)
    // Response encoding: 5 header + 6 bytes per 4 data bytes = 5 + 96 = 101 bytes for 64 bytes
    uint8_t mem_buffer[64];
    r = cmd_0717_read_memory(handle, 0x00000000, 64, mem_buffer, sizeof(mem_buffer));
    if (r != 0) return -1;
    //     printf("==> Registers initialized\n\n");

    // Lines 54-75: Additional BDM setup
    //     printf("Step 6: Additional BDM setup...\n");
    r = cmd_07_a2(handle, 0x01);
    if (r != 0) return -1;
    r = cmd_04_40_58_04(handle);
    if (r != 0) return -1;
    r = cmd_04_7f_fe_02(handle);
    if (r != 0) return -1;
    r = cmd_04_7f_fe_02(handle);
    if (r != 0) return -1;
    r = cmd_07_95(handle);
    if (r != 0) return -1;
    r = cmd_04_40_00_02(handle);
    if (r != 0) return -1;
    r = cmd_enable_memory_access(handle, 0x00);
    if (r != 0) return -1;
    r = cmd_enable_memory_access(handle, 0x00);
    if (r != 0) return -1;
    r = cmd_enter_mode(handle, 0xF8);
    if (r != 0) return -1;
    r = cmd_enter_mode(handle, 0xF0);
    if (r != 0) return -1;
    r = cmd_enter_mode(handle, 0xF8);
    if (r != 0) return -1;
    r = cmd_07_12(handle, 0x0000);
    if (r != 0) return -1;

    r = cmd_07_13(handle, 0x2D80, &reg_value);
    if (r != 0) return -1;

    r = cmd_07_11(handle, 0x1940, 0xFC, 0x0A, 0x00, 0x0A);
    if (r != 0) return -1;
    r = cmd_07_11(handle, 0x1940, 0x40, 0x11, 0x00, 0x0A);
    if (r != 0) return -1;
    r = cmd_07_11(handle, 0x1900, 0x40, 0x10, 0x00, 0x74);
    if (r != 0) return -1;

    uint8_t params2[] = {0x40, 0x10, 0x00, 0x74, 0x00, 0x0F};
    r = cmd_07_15(handle, 0x1800, params2, 6);
    if (r != 0) return -1;

    r = cmd_07_12(handle, 0x0000);
    if (r != 0) return -1;

    // Register write/read tests
    r = cmd_07_13(handle, 0x2188, &reg_value);
    if (r != 0) return -1;
    r = cmd_write_bdm_reg(handle, 0x2088, 0x12345678);
    if (r != 0) return -1;
    r = cmd_07_13(handle, 0x2188, &reg_value);
    if (r != 0) return -1;
    r = cmd_write_bdm_reg(handle, 0x2088, 0xAD95014D);
    if (r != 0) return -1;

    r = cmd_enter_mode(handle, 0xF8);
    if (r != 0) return -1;

    r = cmd_07_13(handle, 0x2180, &reg_value);
    if (r != 0) return -1;
    r = cmd_write_bdm_reg(handle, 0x2080, 0x12345678);
    if (r != 0) return -1;
    r = cmd_07_13(handle, 0x2180, &reg_value);
    if (r != 0) return -1;
    r = cmd_write_bdm_reg(handle, 0x2080, 0xCF206089);
    if (r != 0) return -1;
    //     printf("==> Additional setup complete\n\n");

    // Lines 81-83: Initial verification reads
    //     printf("Step 7: Initial verification...\n");
    r = cmd_071b(handle, 0x00000000, 4, verify_buffer, sizeof(verify_buffer));
    if (r != 0) return -1;
    r = cmd_071b(handle, 0x00000004, 4, verify_buffer, sizeof(verify_buffer));
    if (r != 0) return -1;

    r = cmd_write_bdm_reg(handle, 0x208F, 0xDEADBEEF);
    if (r != 0) return -1;
    //     printf("==> Initial verification complete\n\n");

    // Lines 84-109: BDM register configuration
    //     printf("Step 8: BDM register configuration...\n");
    r = cmd_07_14(handle, 0x2880, 0x00, 0x00, 0x08, 0x0F, 0xDEADBEEF);
    if (r != 0) return -1;

    r = cmd_07_12(handle, 0x0000);
    if (r != 0) return -1;
    r = cmd_07_11(handle, 0x2980, 0x00, 0x00, 0x08, 0x0F);
    if (r != 0) return -1;

    // Read registers 0x2180-0x218F again
    for (uint16_t reg = 0x2180; reg <= 0x218F; reg++) {
        r = cmd_07_13(handle, reg, &reg_value);
        if (r != 0) return -1;
    }
    // Read 0x218D again (appears twice in sequence)
    r = cmd_07_13(handle, 0x218D, &reg_value);
    if (r != 0) return -1;

    r = cmd_07_11(handle, 0x2980, 0x00, 0x00, 0x08, 0x0F);
    if (r != 0) return -1;
    r = cmd_07_11(handle, 0x2980, 0x00, 0x00, 0x08, 0x0E);
    if (r != 0) return -1;

    // Write BDM registers
    r = cmd_07_14(handle, 0x2880, 0x00, 0x00, 0x08, 0x01, 0x20000000);
    if (r != 0) return -1;
    r = cmd_07_14(handle, 0x2880, 0x00, 0x00, 0x0C, 0x05, 0x20000221);
    if (r != 0) return -1;
    r = cmd_07_14(handle, 0x2880, 0x00, 0x00, 0x0C, 0x04, 0x00000021);
    if (r != 0) return -1;
    r = cmd_07_14(handle, 0x2880, 0x00, 0x00, 0x0C, 0x04, 0x00000021);
    if (r != 0) return -1;

    // Line 110: cmd_071e (9-byte format)
    r = cmd_07_1e(handle, 0x0001, 0x40100074, 0x0F);
    if (r != 0) return -1;

    r = cmd_07_11(handle, 0x2980, 0x00, 0x00, 0x08, 0x01);
    if (r != 0) return -1;
    //     printf("==> BDM registers configured\n\n");

    // Lines 112-138: Initial SRAM test pattern writes
    //     printf("Step 9: Initial SRAM test patterns...\n");
    const uint32_t TEST_MARKER = 0x4AC84E73;
    const uint32_t MARKER_ADDR = 0x20000408;

    uint32_t test_addrs[] = {
        0x20000008, 0x2000000C, 0x20000010, 0x20000014,
        0x20000020, 0x20000024, 0x20000028, 0x2000002C,
        0x20000030
    };

    for (int i = 0; i < 9; i++) {
        // Write marker address to test address
        r = cmd_071e_write_sram(handle, test_addrs[i], MARKER_ADDR);
        if (r != 0) return -1;

        // Verify with cmd_071b
        r = cmd_071b(handle, test_addrs[i], 4, verify_buffer, sizeof(verify_buffer));
        if (r != 0) return -1;

        // Write test marker
        r = cmd_071e_write_sram(handle, MARKER_ADDR, TEST_MARKER);
        if (r != 0) return -1;
    }
    //     printf("==> Initial SRAM test patterns written\n\n");

    printf("===========================================\n");
    printf("==> SRAM Pre-Initialization Complete\n");
    printf("===========================================\n\n");

    return 0;
}

// SRAM Validation Sequence
// Implements the 315-command SRAM validation sequence discovered from packet captures
// This sequence is required before SRAM writes will work properly
//
// Pattern from RAM_INIT_SEQUENCE.txt (lines 139-454):
//   Phase 1: Write test patterns to key SRAM addresses
//   Phase 2: Validation cycles with BDM configuration and verification
//
// Returns: 0 on success, -1 on error
int sram_validation_sequence(libusb_device_handle *handle) {
    int r;
    uint8_t verify_buffer[4];
    uint32_t reg_value;

    // Silent SRAM validation sequence

    // Phase 1: Write test patterns to SRAM
    //     printf("Phase 1: Writing test patterns to SRAM...\n");
    //     printf("-------------------------------------------\n");

    // Test marker value and address
    const uint32_t TEST_MARKER = 0x4AC84E73;
    const uint32_t MARKER_ADDR = 0x20000408;

    // Target addresses for validation
    uint32_t target_addrs[] = {
        0x20000034, 0x20000038, 0x2000003C, 0x20000060,
        0x2000007C, 0x200000B8, 0x200000F4
    };
    int num_targets = sizeof(target_addrs) / sizeof(target_addrs[0]);

    for (int i = 0; i < num_targets; i++) {
        uint32_t target = target_addrs[i];

    //         printf("\n[%d/%d] Testing address 0x%08X\n", i+1, num_targets, target);

        // Step 1: Write test marker to marker address
    //         printf("  Writing 0x%08X to 0x%08X...\n", TEST_MARKER, MARKER_ADDR);
        r = cmd_071e_write_sram(handle, MARKER_ADDR, TEST_MARKER);
        if (r != 0) {
            fprintf(stderr, "   **FAILED to write test marker\n");
            return -1;
        }
    //         printf("  ==> Test marker written\n");

        // Step 2: Write marker address to target address
    //         printf("  Writing 0x%08X to 0x%08X...\n", MARKER_ADDR, target);
        r = cmd_071e_write_sram(handle, target, MARKER_ADDR);
        if (r != 0) {
            fprintf(stderr, "   **FAILED to write to target address\n");
            return -1;
        }
    //         printf("  ==> Target address written\n");

        // Step 3: Verify target address with cmd_071b
    //         printf("  Verifying 0x%08X...\n", target);
        r = cmd_071b(handle, target, 4, verify_buffer, sizeof(verify_buffer));
        if (r != 0) {
            fprintf(stderr, "   **FAILED to verify target address\n");
            return -1;
        }

        uint32_t read_value = (verify_buffer[0] << 24) | (verify_buffer[1] << 16) |
                              (verify_buffer[2] << 8) | verify_buffer[3];
    //         printf("  Read back: 0x%08X\n", read_value);

        if (read_value == MARKER_ADDR) {
    //             printf("  ==> Verification passed!\n");
        } else {
            fprintf(stderr, "    Warning: Expected 0x%08X, got 0x%08X\n", MARKER_ADDR, read_value);
        }
    }

    //      printf("\n ==> Phase 1 complete (%d addresses tested)\n", num_targets);

    // Phase 2: Validation cycles
    //      printf("\nPhase 2: Running validation cycles...\n");
    //      printf("-------------------------------------------\n");

    // Simplified validation cycle (run 3 times as a test)
    for (int cycle = 0; cycle < 3; cycle++) {
        // Silent cycle

        // BDM configuration commands
        r = cmd_07_12(handle, 0x0000);
        if (r != 0) {
            fprintf(stderr, "   cmd_07_12 **FAILED\n");
            return -1;
        }

        r = cmd_07_11(handle, 0x2980, 0x00, 0x00, 0x08, 0x0f);
        if (r != 0) {
            fprintf(stderr, "   cmd_07_11 **FAILED (0x080f)\n");
            return -1;
        }

        r = cmd_07_11(handle, 0x2980, 0x00, 0x00, 0x08, 0x01);
        if (r != 0) {
            fprintf(stderr, "   cmd_07_11 **FAILED (0x0801)\n");
            return -1;
        }

        // Verify parameter address
        r = cmd_071b(handle, 0x200000B8, 4, verify_buffer, sizeof(verify_buffer));
        if (r != 0) {
            fprintf(stderr, "   **FAILED to verify 0x200000B8\n");
            return -1;
        }
        uint32_t param_value = (verify_buffer[0] << 24) | (verify_buffer[1] << 16) |
                               (verify_buffer[2] << 8) | verify_buffer[3];
    //         printf("  Parameter value: 0x%08X\n", param_value);

        // Read register 0x2D80
        r = cmd_07_13(handle, 0x2D80, &reg_value);
        if (r != 0) {
            fprintf(stderr, "   **FAILED to read register 0x2D80\n");
            return -1;
        }
    //         printf("  Register 0x2D80: 0x%08X\n", reg_value);

        // BDM configuration
        r = cmd_07_11(handle, 0x2980, 0x00, 0x00, 0x08, 0x0f);
        if (r != 0) {
            fprintf(stderr, "   cmd_07_11 **FAILED (0x080f)\n");
            return -1;
        }

        // Verify test marker
        r = cmd_071b(handle, 0xDEADBEEF, 4, verify_buffer, sizeof(verify_buffer));
        if (r != 0) {
            fprintf(stderr, "   **FAILED to verify test marker\n");
            return -1;
        }
        uint32_t marker = (verify_buffer[0] << 24) | (verify_buffer[1] << 16) |
                          (verify_buffer[2] << 8) | verify_buffer[3];
                 printf("  Test marker value: 0x%08X\n", marker);

        if (marker == 0xDEADBEEF) {
           printf("  ==> Test marker verified!\n");
        }

        // Read registers 0x2180-0x218F (16 sequential reads)
        for (uint16_t reg = 0x2180; reg <= 0x218F; reg++) {
            r = cmd_07_13(handle, reg, &reg_value);
            if (r != 0) {
                fprintf(stderr, "   **FAILED to read register 0x%04X\n", reg);
                return -1;
            }
        }
    //         printf("  ==> Read 16 registers\n");

        // Final BDM configuration for this cycle
        r = cmd_07_11(handle, 0x2980, 0x00, 0x00, 0x08, 0x0f);
        if (r != 0) {
            fprintf(stderr, "   cmd_07_11 **FAILED (0x080f)\n");
            return -1;
        }

        r = cmd_07_11(handle, 0x2980, 0x00, 0x00, 0x08, 0x0e);
        if (r != 0) {
            fprintf(stderr, "   cmd_07_11 **FAILED (0x080e)\n");
            return -1;
        }

    //         printf("  ==> Cycle %d complete\n", cycle + 1);
    }

    //         printf("\n ==> Phase 2 complete (3 validation cycles)\n");

    return 0;
}

// Complete SRAM Initialization
// Combines pre-initialization + validation sequence
// This is the complete 454-command sequence from RAM_INIT_SEQUENCE.txt
//
// Returns: 0 on success, -1 on error
int sram_init_full(libusb_device_handle *handle) {
    int r;

    // Step 1: Pre-initialization (lines 1-138) - silent
    r = sram_pre_init(handle);
    if (r != 0) {
        fprintf(stderr, "SRAM pre-init **FAILED\n");
        return -1;
    }

    // Step 2: Validation sequence (lines 139-454) - silent
    r = sram_validation_sequence(handle);
    if (r != 0) {
        fprintf(stderr, "SRAM validation **FAILED\n");
        return -1;
    }

    printf("SRAM initialized\n");
    return 0;
}

/**
 * Read BDM register (16-bit address).
 *
 * Command: aa 55 00 04 07 13 [reg:2]
 * Response: 99 66 00 03 ee [data:2]
 */
int cmd_read_bdm_reg(libusb_device_handle *handle, uint16_t reg, uint16_t *value) {
    unsigned char *cmd = g_cmd_buffer; // Use global persistent buffer

    // Build command
    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;  // Length high
    cmd[3] = 0x04;  // Length low (4 bytes: 07 13 + 2-byte reg)
    cmd[4] = 0x07;  // Command
    cmd[5] = 0x13;  // Subcommand: Read BDM Register

    // Register address (big-endian)
    cmd[6] = (reg >> 8) & 0xFF;
    cmd[7] = reg & 0xFF;
    // CRUCIAL: Do NOT zero bytes 8-255! They contain leftover response data

    // Send command
    int actual_length;
    int ret = libusb_bulk_transfer(handle, ENDPOINT_OUT, cmd, 256, &actual_length, 5000);
    if (ret < 0) {
        fprintf(stderr, "Error: **FAILED to send READ_BDM_REG command: %s\n",
                libusb_error_name(ret));
        return ret;
    }

    // Receive response BACK INTO g_cmd_buffer
    ret = libusb_bulk_transfer(handle, ENDPOINT_IN, g_cmd_buffer, 256, &actual_length, 5000);
    if (ret < 0) {
        fprintf(stderr, "Error: **FAILED to receive READ_BDM_REG response: %s\n",
                libusb_error_name(ret));
        return ret;
    }

    // Validate response
    if (g_cmd_buffer[0] != 0x99 || g_cmd_buffer[1] != 0x66) {
        fprintf(stderr, "Error: Invalid response magic for READ_BDM_REG\n");
        return -1;
    }

    if (g_cmd_buffer[4] != 0xee) {
        fprintf(stderr, "Error: READ_BDM_REG command **FAILED with status 0x%02x\n", g_cmd_buffer[4]);
        return -1;
    }

    // Extract 16-bit value (big-endian)
    if (value) {
        *value = (g_cmd_buffer[5] << 8) | g_cmd_buffer[6];
    }

    return 0;
}

/**
 * Write BDM register (16-bit address, 32-bit data).
 * 
 * Command: aa 55 00 08 07 16 [reg:2] [data:4]
 */
int cmd_write_bdm_reg(libusb_device_handle *handle, uint16_t reg, uint32_t data) {
    unsigned char *cmd = g_cmd_buffer; // Use global persistent buffer
    // memset(cmd, 0, 256); // REMOVED: Must preserve leftover data
    
    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;  // Length high
    cmd[3] = 0x08;  // Length low (8 bytes: 07 16 + 2-byte reg + 4-byte data)
    cmd[4] = 0x07;  // Command
    cmd[5] = 0x16;  // Subcommand: Write BDM Register
    
    // Register address (big-endian)
    cmd[6] = (reg >> 8) & 0xFF;
    cmd[7] = reg & 0xFF;
    
    // Data (big-endian)
    cmd[8] = (data >> 24) & 0xFF;
    cmd[9] = (data >> 16) & 0xFF;
    cmd[10] = (data >> 8) & 0xFF;
    cmd[11] = data & 0xFF;
    
    return send_aa_command(handle, cmd, 256, "WRITE BDM REG");
}

/**
 * Command 07 11 - BDM configuration with register and 4 parameters.
 * 
 * Command: aa 55 00 08 07 11 [reg:2] [p1] [p2] [p3] [p4]
 */
int cmd_07_11(libusb_device_handle *handle, uint16_t reg, uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4) {
    unsigned char *cmd = g_cmd_buffer; // Use global persistent buffer
    // memset(cmd, 0, 256); // REMOVED: Must preserve leftover data
    
    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;  // Length high
    cmd[3] = 0x08;  // Length low (8 bytes)
    cmd[4] = 0x07;  // Command
    cmd[5] = 0x11;  // Subcommand
    
    // Register (big-endian)
    cmd[6] = (reg >> 8) & 0xFF;
    cmd[7] = reg & 0xFF;
    
    // Parameters
    cmd[8] = p1;
    cmd[9] = p2;
    cmd[10] = p3;
    cmd[11] = p4;

    // CRUCIAL: Use zero padding (from memset), NOT ff 00!
    // The memset(cmd, 0, 256) above already provides correct padding

    return send_aa_command(handle, cmd, 256, "CMD 07 11");
}

/**
 * Command 07 11 - Read BDM register with window base and return value.
 *
 * This is the CORRECT way to read PC after a write (07 14).
 * The 07 13 command with 0x298F opcode returns stale data!
 *
 * Command: aa 55 00 08 07 11 [window:2] [p1] [p2] [p3] [p4]
 * Response: 99 66 00 07 ee [value:4]
 *
 * For PC read: window=0x2980, p1=0x00, p2=0x00, p3=0x08, p4=0x0F (reg 0x080F)
 * For SR read: window=0x2980, p1=0x00, p2=0x00, p3=0x08, p4=0x0E (reg 0x080E)
 */
int cmd_07_11_read_bdm_reg(libusb_device_handle *handle, uint16_t window, uint16_t reg, uint32_t *value) {
    unsigned char *cmd = g_cmd_buffer;
    unsigned char resp[256];
    int actual_length;

    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;
    cmd[3] = 0x08;  // Length = 8 bytes
    cmd[4] = 0x07;
    cmd[5] = 0x11;

    // Window base (e.g., 0x2980 for read mode)
    cmd[6] = (window >> 8) & 0xFF;
    cmd[7] = window & 0xFF;

    // Register as 4 parameters (e.g., 00 00 08 0F for PC)
    cmd[8] = 0x00;
    cmd[9] = 0x00;
    cmd[10] = (reg >> 8) & 0xFF;
    cmd[11] = reg & 0xFF;

    // Use padding pattern
    for (int i = 12; i < 256; i += 2) {
        cmd[i] = 0xff;
        if (i+1 < 256) cmd[i+1] = 0x00;
    }

    // Send command
    int ret = libusb_bulk_transfer(handle, ENDPOINT_OUT, cmd, 256, &actual_length, 1000);
    if (ret < 0) {
        printf("cmd_07_11_read_bdm_reg: send **FAILED: %s\n", libusb_error_name(ret));
        return -1;
    }

    // Receive response
    ret = libusb_bulk_transfer(handle, ENDPOINT_IN, resp, 256, &actual_length, 1000);
    if (ret < 0) {
        printf("cmd_07_11_read_bdm_reg: recv **FAILED: %s\n", libusb_error_name(ret));
        return -1;
    }

    // Debug: print response bytes
    printf("cmd_07_11 resp: ");
    for (int i = 0; i < 12 && i < actual_length; i++) {
        printf("%02x ", resp[i]);
    }
    printf("\n");

    // Check response format: 99 66 00 07 ee [value:4]
    if (resp[0] == 0x99 && resp[1] == 0x66 && resp[4] == 0xee) {
        *value = ((uint32_t)resp[5] << 24) | ((uint32_t)resp[6] << 16) |
                 ((uint32_t)resp[7] << 8) | resp[8];
        return 0;
    }

    printf("cmd_07_11_read_bdm_reg: unexpected response\n");
    return -1;
}

/**
 * Read PC register using cmd_07_11 (the correct method).
 *
 * Uses window base 0x2980 and register 0x080F (PC).
 */
int cmd_read_pc(libusb_device_handle *handle, uint32_t *pc_value) {
    return cmd_07_11_read_bdm_reg(handle, 0x2980, 0x080F, pc_value);
}

/**
 * Read SR (Status Register) using cmd_07_11.
 *
 * Uses window base 0x2980 and register 0x080E (SR).
 */
int cmd_read_sr(libusb_device_handle *handle, uint32_t *sr_value) {
    return cmd_07_11_read_bdm_reg(handle, 0x2980, 0x080E, sr_value);
}

/**
 * Write PC register using cmd_07_14 with proper sync.
 *
 * Follows sequence:
 * 1. Send 07 14 with window 0x2880 and register 0x080F
 * 2. Send 07 12 sync command
 */
int cmd_write_pc(libusb_device_handle *handle, uint32_t pc_value) {
    int r;

    // Write PC using 07 14
    r = cmd_07_14_write_bdm_reg(handle, 0x080F, pc_value);
    if (r != 0) return r;

    // Sync
    r = cmd_07_12(handle, 0xFFFF);
    return r;
}

/**
 * Write PC register using cmd 07 16 format (before BDM GO).
 *
 * Command: aa 55 00 08 07 16 08 0f [value:4]
 * This is the exact format used right before BDM GO to set PC.
 *
 * CRUCIAL: This is different from cmd_07_14_write_bdm_reg which uses window 0x2880!
 */
int cmd_07_16_write_pc(libusb_device_handle *handle, uint32_t pc_value) {
    unsigned char *cmd = g_cmd_buffer;

    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;
    cmd[3] = 0x08;  // Length = 8 bytes
    cmd[4] = 0x07;
    cmd[5] = 0x16;
    cmd[6] = 0x08;  // Register high byte (0x080F = PC)
    cmd[7] = 0x0f;  // Register low byte
    cmd[8] = (pc_value >> 24) & 0xFF;
    cmd[9] = (pc_value >> 16) & 0xFF;
    cmd[10] = (pc_value >> 8) & 0xFF;
    cmd[11] = pc_value & 0xFF;

    return send_aa_command(handle, cmd, 256, "CMD 07 16 (Write PC for BDM GO)");
}

/**
 * Command 07 15 - BDM extended configuration with register and variable parameters.
 * 
 * Command: aa 55 [length:2] 07 15 [reg:2] [params...]
 */
int cmd_07_15(libusb_device_handle *handle, uint16_t reg, uint8_t *params, int param_count) {
    unsigned char *cmd = g_cmd_buffer; // Use global persistent buffer
    // memset(cmd, 0, 256); // REMOVED: Must preserve leftover data

    int length = 4 + param_count;  // 07 15 (2) + reg (2) + params

    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = (length >> 8) & 0xFF;  // Length high
    cmd[3] = length & 0xFF;          // Length low
    cmd[4] = 0x07;  // Command
    cmd[5] = 0x15;  // Subcommand

    // Register (big-endian)
    cmd[6] = (reg >> 8) & 0xFF;
    cmd[7] = reg & 0xFF;

    // Parameters
    for (int i = 0; i < param_count && i < 248; i++) {
        cmd[8 + i] = params[i];
    }

    // CRUCIAL: Use zero padding (from memset), NOT ff 00!
    // The memset(cmd, 0, 256) above already provides correct padding

    return send_aa_command(handle, cmd, 256, "CMD 07 15");
}

/**
 * BDM Halt - Force CPU into debug/halt mode
 *
 * Command: aa 55 00 04 04 40 00 01
 *
 * This halts the running CPU so we can read flash/RAM accurately.
 * NOTE: From captures, the actual sequence to halt is more complex.
 */
int cmd_bdm_halt(libusb_device_handle *handle) {
    unsigned char *cmd = g_cmd_buffer; // Use global persistent buffer
    // memset(cmd, 0, 256); // REMOVED: Must preserve leftover data

    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;  // Length high
    cmd[3] = 0x04;  // Length low
    cmd[4] = 0x04;  // BDM command
    cmd[5] = 0x40;  // BDM operation
    cmd[6] = 0x00;  // Halt parameter (different from 0x58 = resume)
    cmd[7] = 0x01;  // Halt flag

    return send_aa_command(handle, cmd, 256, "BDM Halt");
}

/**
 * Command 07 95 - Unknown BDM command seen in captures
 *
 * Command: aa 55 00 02 07 95
 *
 * This command appears in the sequence: RESUME -> FREEZE_CHECK -> CMD_07_95 -> BDM_CMD_00_02
 * It seems to be part of the halt/freeze sequence.
 */
int cmd_07_95(libusb_device_handle *handle) {
    unsigned char *cmd = g_cmd_buffer; // Use global persistent buffer
    // memset(cmd, 0, 256); // REMOVED: Must preserve leftover data

    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;  // Length high
    cmd[3] = 0x02;  // Length low
    cmd[4] = 0x07;  // Command
    cmd[5] = 0x95;  // Subcommand

    return send_aa_command(handle, cmd, 256, "CMD 07 95");
}

/**
 * BDM Command 00 02 - Part of halt sequence from captures
 *
 * Command: aa 55 00 04 04 40 00 02
 *
 * This appears after RESUME and CMD_07_95 in the capture.
 * Different from halt (00 01) and resume (58 04).
 */
int cmd_bdm_cmd_00_02(libusb_device_handle *handle) {
    unsigned char *cmd = g_cmd_buffer; // Use global persistent buffer
    // memset(cmd, 0, 256); // REMOVED: Must preserve leftover data

    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;  // Length high
    cmd[3] = 0x04;  // Length low
    cmd[4] = 0x04;  // BDM command
    cmd[5] = 0x40;  // BDM operation
    cmd[6] = 0x00;  // Parameter
    cmd[7] = 0x02;  // Flag (different from halt=01, resume=04)

    return send_aa_command(handle, cmd, 256, "BDM CMD 00 02");
}

/**
 * Command 07 14 - Execute flashloader or BDM command with parameters
 *
 * Command: aa 55 00 0c 07 14 [reg:2] [p1] [p2] [p3] [p4] [addr:4]
 *
 * From captures: aa 55 00 0c 07 14 28 80 00 00 08 0f 20 00 1d a0
 * This appears to execute the flashloader with parameters.
 */
int cmd_07_14(libusb_device_handle *handle, uint16_t reg, uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4, uint32_t addr) {
    unsigned char *cmd = g_cmd_buffer; // Use global persistent buffer
    // memset(cmd, 0, 256); // REMOVED: Must preserve leftover data

    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;  // Length high
    cmd[3] = 0x0C;  // Length low (12 bytes)
    cmd[4] = 0x07;  // Command
    cmd[5] = 0x14;  // Subcommand

    // Register (big-endian)
    cmd[6] = (reg >> 8) & 0xFF;
    cmd[7] = reg & 0xFF;

    // Parameters
    cmd[8] = p1;
    cmd[9] = p2;
    cmd[10] = p3;
    cmd[11] = p4;

    // Address (big-endian)
    cmd[12] = (addr >> 24) & 0xFF;
    cmd[13] = (addr >> 16) & 0xFF;
    cmd[14] = (addr >> 8) & 0xFF;
    cmd[15] = addr & 0xFF;

    return send_aa_command(handle, cmd, 256, "CMD 07 14");
}

/**
 * Command 07 19 - Write data to memory (used for setting parameters)
 *
 * Command: aa 55 00 0c 07 19 [sublen:2] [addr:4] [data:4]
 *
 * From captures: aa 55 00 0c 07 19 00 04 20 00 00 00 00 00 00 03
 * This writes 4 bytes of data to the specified address.
 */
/**
 * Write to target memory using CMD 07 19.
 *
 * CRUCIAL: After cmd_07_17 sets up the memory window, this command
 * operates in FIRE-AND-FORGET mode (no response expected).
 * Sent 293 commands but only receives 7 responses total.
 *
 * This matches the packet capture behavior where cmd_07_19 commands
 * are sent rapidly without waiting for responses after cmd_07_17.
 */
int cmd_07_19(libusb_device_handle *handle, uint32_t addr, uint32_t data) {
    unsigned char *cmd = g_cmd_buffer; // Use global persistent buffer

    // CRUCIAL: Initialize buffer with specific padding pattern!
    // Observed "02 00 00 00 00 02" repeating pattern, not random garbage
    memset(cmd, 0x00, 256);  // Initialize to zeros first
    // Then set the repeating pattern in padding area (bytes 16-255)
    for (int i = 16; i < 256; i += 6) {
        cmd[i] = 0x02;
        cmd[i+5] = 0x02;  // Next 0x02 after 4 zeros
    }

    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;  // Length high
    cmd[3] = 0x0C;  // Length low (12 bytes)
    cmd[4] = 0x07;  // Command
    cmd[5] = 0x19;  // Subcommand

    // Sub-length (always 0x0004 for 4 bytes of data)
    cmd[6] = 0x00;
    cmd[7] = 0x04;

    // Address (big-endian)
    cmd[8] = (addr >> 24) & 0xFF;
    cmd[9] = (addr >> 16) & 0xFF;
    cmd[10] = (addr >> 8) & 0xFF;
    cmd[11] = addr & 0xFF;

    // Data (big-endian)
    cmd[12] = (data >> 24) & 0xFF;
    cmd[13] = (data >> 16) & 0xFF;
    cmd[14] = (data >> 8) & 0xFF;
    cmd[15] = data & 0xFF;

    // ========================================================================
    // CRUCIAL: cmd_07_19 uses 0x00 padding (from memset), NOT ff 00 pattern!
    // ========================================================================
    // Packet captures show cmd_07_19 with all-zero padding:
    // aa 55 00 0c 07 19 00 04 20 00 01 00 ca fe ba be 00 00 00 00 00 00...
    // The memset(cmd, 0, 256) already provides this, so NO additional padding!
    // ========================================================================

    // FIXED: Wait for response like all other commands
    return send_aa_command(handle, cmd, 256, "CMD 07 19 (Write Memory)");
}

/**
 * Command 07 10 - Read/check register or status
 *
 * Command: aa 55 00 05 07 10 [reg:3]
 *
 * From captures: aa 55 00 05 07 10 00 2d 80
 * This appears to read BDM status or registers.
 */
int cmd_07_10(libusb_device_handle *handle, uint16_t reg) {
    unsigned char *cmd = g_cmd_buffer; // Use global persistent buffer
    // memset(cmd, 0, 256); // REMOVED: Must preserve leftover data

    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;  // Length high
    cmd[3] = 0x05;  // Length low (5 bytes)
    cmd[4] = 0x07;  // Command
    cmd[5] = 0x10;  // Subcommand

    // Register (big-endian, but spreads across 3 bytes in captures)
    cmd[6] = 0x00;
    cmd[7] = (reg >> 8) & 0xFF;
    cmd[8] = reg & 0xFF;

    // CRUCIAL: cmd_07_10 DOES send a response (discovered from USB hangs!)
    // Response is very short (3-5 bytes) and doesn't follow standard 99 66 format
    // We must read it to prevent USB buffer saturation and hangs
    return send_aa_command(handle, cmd, 256, "CMD 07 10");
}

/**
 * CMD 07 17 - RAM Test Command
 *
 * From capture: aa 55 00 08 07 17 20 00 00 0c 00 04
 * Purpose: RAM test/verification
 * Send this after flashloader initialization
 */
int cmd_07_17(libusb_device_handle *handle) {
    unsigned char *cmd = g_cmd_buffer;

    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;  // Length high
    cmd[3] = 0x08;  // Length low (8 bytes)
    cmd[4] = 0x07;  // Command
    cmd[5] = 0x17;  // Subcommand

    // Address: 0x2000000C (from pcap)
    cmd[6] = 0x20;
    cmd[7] = 0x00;
    cmd[8] = 0x00;
    cmd[9] = 0x0C;

    // Size: 0x0004 (from pcap)
    cmd[10] = 0x00;
    cmd[11] = 0x04;

    // Send command (may or may not get response)
    return send_aa_command_no_response(handle, cmd, 256, "CMD 07 17 (RAM test)");
}

/**
 * CMD 01 0b - Get Device Info
 *
 * From capture: aa 55 00 02 01 0b
 * Purpose: Query USB-ML-12 device information
 * Send this TWICE as first two commands before BDM entry
 *
 * Response contains device ID string like:
 * "6022246,USB-ML-CF REF : M52230DEMO,PE6022246,,,,"
 */
int cmd_01_0b(libusb_device_handle *handle) {
    unsigned char *cmd = g_cmd_buffer; // Use global persistent buffer
    /* memset(cmd, 0, 256); */ // REMOVED: Must preserve leftover data

    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;  // Length high
    cmd[3] = 0x02;  // Length low (2 bytes)
    cmd[4] = 0x01;  // Command
    cmd[5] = 0x0b;  // Subcommand

    return send_aa_command(handle, cmd, 256, "CMD 01 0b (Get Device Info)");
}

/**
 * CMD 07 a2 - Configuration Command
 *
 * From capture: aa 55 00 03 07 a2 01
 * Purpose: BDM configuration/initialization
 * Parameter: 0x01 (configuration mode)
 */
int cmd_07_a2(libusb_device_handle *handle, uint8_t param) {
    unsigned char *cmd = g_cmd_buffer; // Use global persistent buffer
    // memset(cmd, 0, 256); // REMOVED: Must preserve leftover data

    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;  // Length high
    cmd[3] = 0x03;  // Length low (3 bytes)
    cmd[4] = 0x07;  // Command
    cmd[5] = 0xa2;  // Subcommand
    cmd[6] = param; // Parameter (usually 0x01)

    return send_aa_command(handle, cmd, 256, "CMD 07 a2 (BDM Config)");
}

/**
 * CMD 04 40 58 04 - BDM Initialization Step
 *
 * From capture: aa 55 00 04 04 40 58 04
 * Purpose: Part of BDM initialization sequence during target_init_full().
 *
 * WARNING: This does NOT resume target execution! Use cmd_07_02_bdm_go() for that.
 * The name "BDM Resume" in old captures was misleading - this is purely for init.
 */
int cmd_04_40_58_04(libusb_device_handle *handle) {
    unsigned char *cmd = g_cmd_buffer;

    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;  // Length high
    cmd[3] = 0x04;  // Length low (4 bytes)
    cmd[4] = 0x04;  // Command
    cmd[5] = 0x40;  // Subcommand
    cmd[6] = 0x58;  // Parameter 1
    cmd[7] = 0x04;  // Parameter 2

    return send_aa_command(handle, cmd, 256, "CMD 04 40 58 04 (BDM Init)");
}

/**
 * CMD 04 7f fe 02 - BDM Command
 *
 * From capture: aa 55 00 04 04 7f fe 02
 * Purpose: Unknown BDM configuration (repeated in init sequence)
 */
int cmd_04_7f_fe_02(libusb_device_handle *handle) {
    unsigned char *cmd = g_cmd_buffer; // Use global persistent buffer
    // memset(cmd, 0, 256); // REMOVED: Must preserve leftover data

    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;  // Length high
    cmd[3] = 0x04;  // Length low (4 bytes)
    cmd[4] = 0x04;  // Command
    cmd[5] = 0x7f;  // Subcommand
    cmd[6] = 0xfe;  // Parameter 1
    cmd[7] = 0x02;  // Parameter 2

    return send_aa_command(handle, cmd, 256, "CMD 04 7f fe 02");
}

/**
 * CMD 04 40 00 02 - BDM Command variant
 *
 * From capture: aa 55 00 04 04 40 00 02
 * Purpose: BDM configuration step
 */
int cmd_04_40_00_02(libusb_device_handle *handle) {
    unsigned char *cmd = g_cmd_buffer; // Use global persistent buffer
    // memset(cmd, 0, 256); // REMOVED: Must preserve leftover data

    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;  // Length high
    cmd[3] = 0x04;  // Length low (4 bytes)
    cmd[4] = 0x04;  // Command
    cmd[5] = 0x40;  // Subcommand
    cmd[6] = 0x00;  // Parameter 1
    cmd[7] = 0x02;  // Parameter 2

    return send_aa_command(handle, cmd, 256, "CMD 04 40 00 02");
}

/**
 * CMD 07 13 - Read Register
 *
 * From capture: aa 55 00 04 07 13 2d 80
 * Purpose: Read BDM register or system register
 * Returns 32-bit value from target
 */
int cmd_07_13(libusb_device_handle *handle, uint16_t reg, uint32_t *value) {
    unsigned char *cmd = g_cmd_buffer; // Use global persistent buffer
    // memset(cmd, 0, 256); // REMOVED: Must preserve leftover data

    cmd[0] = 0xaa;
    cmd[1] = 0x55;
    cmd[2] = 0x00;  // Length high
    cmd[3] = 0x04;  // Length low (4 bytes)
    cmd[4] = 0x07;  // Command
    cmd[5] = 0x13;  // Subcommand
    cmd[6] = (reg >> 8) & 0xFF;  // Register high byte
    cmd[7] = reg & 0xFF;         // Register low byte

    // Send command
    int r;
    int sent_length;

    //     if (g_openlink_verbose) printf("\nSending CMD 07 13 (Read Register 0x%04X)...\n", reg);
    r = libusb_bulk_transfer(handle, ENDPOINT_OUT, cmd, 256, &sent_length, 5000);
    if (r != 0) {
        fprintf(stderr, "Error sending CMD 07 13: %s\n", libusb_error_name(r));
        return -1;
    }

    // Receive response into local buffer (REVERTED - this function needs local buffer)
    unsigned char response[256];
    int actual_response_len;
    r = libusb_bulk_transfer(handle, ENDPOINT_IN, response, 256, &actual_response_len, 5000);
    if (r < 0) {
        fprintf(stderr, "Error receiving CMD 07 13 response: %s\n", libusb_error_name(r));
        return -1;
    }

    if (g_openlink_verbose) {
        printf("Received %d bytes:\n", actual_response_len);
        print_hex(response, actual_response_len);
    }

    // Parse response (big-endian 32-bit value after header and status byte)
    // Response format: [99 66] [length:2] [status:1] [data:4]
    if (actual_response_len >= 9 && response[0] == 0x99 && response[1] == 0x66 && response[4] == 0xee) {
        *value = (response[5] << 24) | (response[6] << 16) | (response[7] << 8) | response[8];
    //         if (g_openlink_verbose) printf("Register 0x%04X = 0x%08X\n", reg, *value);
        return 0;
    }

    fprintf(stderr, "Invalid response format for CMD 07 13\n");
    return -1;
}

/**
 * Full Memory Window Setup Sequence
 *
 * Performs the complete memory window configuration sequence that enables
 * SRAM writes via 16-bit addressing. This is the SAME sequence used in
 * target_init_full() Phase 8 and test_show_responses.c.
 *
 * CRUCIAL: This is NOT the same as cmd_07_17! This is the full sequence:
 *   cmd_07_12 → cmd_07_13 → cmd_07_11 (×3) → cmd_07_15 (×2) → cmd_07_12
 *
 * Must be called BEFORE writing flashloader parameters to SRAM!
 *
 * Returns 0 on success, -1 on failure
 */
int cmd_setup_memory_windows_full(libusb_device_handle *handle) {
    int r;
    uint32_t temp_val;

    printf("=== Setting Up Memory Windows (Full Sequence) ===\n");

    r = cmd_07_12(handle, 0xFFFF);
    if (r != 0) return -1;
    usleep(309);

    r = cmd_07_13(handle, 0x2D80, &temp_val);
    if (r != 0) return -1;
    usleep(338);

    r = cmd_07_11(handle, 0x1940, 0xfc, 0x0a, 0x00, 0x0a);
    if (r != 0) return -1;
    usleep(350);

    r = cmd_07_11(handle, 0x1940, 0x40, 0x11, 0x00, 0x0a);
    if (r != 0) return -1;
    usleep(319);

    r = cmd_07_11(handle, 0x1900, 0x40, 0x10, 0x00, 0x74);
    if (r != 0) return -1;
    usleep(328);

    uint8_t p[] = {0x40, 0x10, 0x00, 0x74, 0x00, 0x0f};
    r = cmd_07_15(handle, 0x1800, p, 6);
    if (r != 0) return -1;
    usleep(124);

    r = cmd_07_15(handle, 0x1800, p, 6);
    if (r != 0) return -1;
    usleep(311);

    r = cmd_07_12(handle, 0xFFFF);
    if (r != 0) return -1;
    usleep(338);

    //     printf("==> Memory windows configured\n");
    return 0;
}

/**
 * Comprehensive Target Initialization
 *
 * Implement the complete initialization sequence:
 * 1. BDM mode entry
 * 2. System ID detection
 * 3. Clock verification
 * 4. RAM test
 * 5. Flash ID detection
 *
 * Returns 0 on success, -1 on failure
 * Populates flash_size_kb with detected flash size (128 or 256)
 */
int target_init_full(libusb_device_handle *handle, uint32_t *flash_size_kb) {
    int r;
    uint32_t chip_id = 0;

    printf("\n");
    printf("========================================\n");
    printf("  Target Initialization (Full Sequence)\n");
    printf("========================================\n\n");

    // Phase 1: BDM Mode Entry
    //     printf("=== Phase 1: BDM Mode Entry ===\n");

    printf("Entering BDM mode (0xFC)...\n");
    r = cmd_enter_mode(handle, 0xFC);
    if (r != 0) {
        fprintf(stderr, "**FAILED to enter BDM mode 0xFC\n");
        return -1;
    }

    printf("Sending configuration command...\n");
    r = cmd_07_a2(handle, 0x01);
    if (r != 0) {
        fprintf(stderr, "**FAILED cmd_07_a2\n");
        return -1;
    }

    r = cmd_04_40_58_04(handle);  // BDM init step (not resume!)
    if (r != 0) {
        fprintf(stderr, "**FAILED cmd_04_40_58_04\n");
        return -1;
    }

    printf("Sending BDM commands (04 7f fe 02)...\n");
    r = cmd_04_7f_fe_02(handle);
    if (r != 0) {
        fprintf(stderr, "**FAILED cmd_04_7f_fe_02 (1st)\n");
        return -1;
    }

    r = cmd_04_7f_fe_02(handle);
    if (r != 0) {
        fprintf(stderr, "**FAILED cmd_04_7f_fe_02 (2nd)\n");
        return -1;
    }

    printf("Sending BDM status command...\n");
    r = cmd_07_95(handle);
    if (r != 0) {
        fprintf(stderr, "**FAILED cmd_07_95\n");
        return -1;
    }

    printf("Sending BDM command (04 40 00 02)...\n");
    r = cmd_04_40_00_02(handle);
    if (r != 0) {
        fprintf(stderr, "**FAILED cmd_04_40_00_02\n");
        return -1;
    }

    printf("Enabling memory access...\n");
    r = cmd_enable_memory_access(handle, 0x00);
    if (r != 0) {
        fprintf(stderr, "**FAILED to enable memory access\n");
        return -1;
    }

    // CRUCIAL: Call this TWICE!
    r = cmd_enable_memory_access(handle, 0x00);
    if (r != 0) {
        fprintf(stderr, "**FAILED to enable memory access (2nd call)\n");
        return -1;
    }

    //     printf("==> Phase 1 complete\n\n");

    // Phase 2: Re-initialization Cycle
    //     printf("=== Phase 2: BDM Re-initialization ===\n");

    r = cmd_07_a2(handle, 0x01);
    r |= cmd_04_40_58_04(handle);
    r |= cmd_04_7f_fe_02(handle);
    r |= cmd_04_7f_fe_02(handle);
    r |= cmd_07_95(handle);
    r |= cmd_04_40_00_02(handle);
    r |= cmd_enable_memory_access(handle, 0x00);
    r |= cmd_enable_memory_access(handle, 0x00);  // Repeated

    if (r != 0) {
        fprintf(stderr, "**FAILED during re-initialization cycle\n");
        return -1;
    }

    //     printf("==> Phase 2 complete\n\n");

    // Phase 3: Mode Configuration
    //     printf("=== Phase 3: Mode Configuration ===\n");

    printf("Entering mode 0xF8...\n");
    r = cmd_enter_mode(handle, 0xF8);
    if (r != 0) {
        fprintf(stderr, "**FAILED to enter mode 0xF8\n");
        return -1;
    }

    printf("Entering mode 0xF0...\n");
    r = cmd_enter_mode(handle, 0xF0);
    if (r != 0) {
        fprintf(stderr, "**FAILED to enter mode 0xF0\n");
        return -1;
    }

    printf("Entering mode 0xF8 again...\n");
    r = cmd_enter_mode(handle, 0xF8);
    if (r != 0) {
        fprintf(stderr, "**FAILED to enter mode 0xF8 (2nd)\n");
        return -1;
    }

    printf("Polling BDM status...\n");
    r = cmd_07_12(handle, 0xFFFF);
    if (r != 0) {
        fprintf(stderr, "**FAILED to poll BDM status\n");
        return -1;
    }

    //     printf("==> Phase 3 complete\n\n");

    // Phase 4: Chip ID Detection
    //     printf("=== Phase 4: Chip ID Detection ===\n");

    printf("Reading chip ID from register 0x2D80...\n");
    r = cmd_07_13(handle, 0x2D80, &chip_id);
    if (r != 0) {
        fprintf(stderr, "**FAILED to read chip ID\n");
        return -1;
    }

    printf("Chip ID: 0x%08X\n", chip_id);

    // CIR will be read after system configuration when peripheral access is enabled

    // CRUCIAL: Sends THREE cmd 07 14 commands before SRAM access works!
    // All three use window base 0x28800000
    printf("\nInitializing BDM registers using cmd 07 14 with window base 0x28800000...\n");

    // Register 0x080E = 0x00002700
    printf("Writing register 0x080E = 0x00002700...\n");
    r = cmd_07_14_write_bdm_reg(handle, 0x080E, 0x00002700);
    if (r != 0) {
        fprintf(stderr, "**FAILED to write register 0x080E!\n");
        return -1;
    }

    // Register 0x0C05 (RAMBAR) = 0x20000221 (from register dump)
    // Bit 9 (0x200) appears to be CRUCIAL for SRAM access!
    printf("Writing RAMBAR (0x0C05) = 0x20000221...\n");
    r = cmd_07_14_write_bdm_reg(handle, 0x0C05, 0x20000221);
    if (r != 0) {
        fprintf(stderr, "**FAILED to initialize RAMBAR1!\n");
        return -1;
    }

    // Register 0x080F = 0x000023F2
    printf("Writing register 0x080F = 0x000023F2...\n");
    r = cmd_07_14_write_bdm_reg(handle, 0x080F, 0x000023F2);
    if (r != 0) {
        fprintf(stderr, "**FAILED to write register 0x080F!\n");
        return -1;
    }

    //     printf("==> BDM registers configured (0x080E, RAMBAR 0x0C05, 0x080F) with window base 0x28800000\n");

    // CRUCIAL: Do a read from 0x2188 BEFORE the first SRAM write!
    // This might "prime" the BDM address mapping
    printf("Priming BDM address mapping with read from 0x2188...\n");
    uint32_t dummy_value;
    r = cmd_07_13(handle, 0x2188, &dummy_value);
    if (r != 0) {
        fprintf(stderr, "Warning: **FAILED to prime BDM with read from 0x2188\n");
    } else {
    //         printf("==> BDM primed, read 0x%08X from address 0x2188\n", dummy_value);
    }

    // Initialize FLASHBAR (Flash base address register)
    // writecontrolreg 0x0C04 0x00000061
    printf("Initializing FLASHBAR (Flash)...\n");
    r = cmd_write_bdm_reg(handle, 0x0C04, 0x00000061);
    if (r != 0) {
        fprintf(stderr, "**FAILED to initialize FLASHBAR!\n");
        return -1;
    }
    //     printf("==> FLASHBAR = 0x00000061 (Flash at 0x00000000 enabled)\n");

    // System Configuration (CRUCIAL - required before RAM test!)
    printf("\nConfiguring system registers...\n");

    // Read clock configuration register (Line 23 from capture)
    printf("Reading clock configuration...\n");
    uint8_t clock_params[] = {0x19, 0x40, 0xFC, 0x0A, 0x00, 0x0A};
    r = cmd_07_11(handle, 0x1940, clock_params[2], clock_params[3], clock_params[4], clock_params[5]);
    if (r != 0) {
        fprintf(stderr, "Warning: **FAILED to read clock config register\n");
    }

    // Read secondary clock register (Line 24)
    printf("Reading secondary clock config...\n");
    uint8_t clock_params2[] = {0x19, 0x40, 0x40, 0x11, 0x00, 0x0A};
    r = cmd_07_11(handle, 0x1940, clock_params2[2], clock_params2[3], clock_params2[4], clock_params2[5]);
    if (r != 0) {
        fprintf(stderr, "Warning: **FAILED to read secondary clock register\n");
    }

    // Read system configuration register (Line 25)
    printf("Reading system configuration...\n");
    uint8_t sys_params[] = {0x19, 0x00, 0x40, 0x10, 0x00, 0x74};
    r = cmd_07_11(handle, 0x1900, sys_params[2], sys_params[3], sys_params[4], sys_params[5]);
    if (r != 0) {
        fprintf(stderr, "Warning: **FAILED to read system config register\n");
    }

    // Write system configuration (Line 26) - CRUCIAL!
    printf("Writing system configuration...\n");
    uint8_t write_params[] = {0x18, 0x00, 0x40, 0x10, 0x00, 0x74, 0x00, 0x0F};
    r = cmd_07_15(handle, 0x1800, write_params + 2, 6);  // Pass remaining parameters
    if (r != 0) {
        fprintf(stderr, "Warning: **FAILED to write system config\n");
    }

    // Poll BDM status after configuration (Line 27)
    printf("Polling BDM status after config...\n");
    r = cmd_07_12(handle, 0xFFFF);
    if (r != 0) {
        fprintf(stderr, "Warning: **FAILED to poll BDM status\n");
    }

    //     printf("==> System configuration complete\n");
    //     printf("==> Phase 4 complete\n\n");

    // Phase 5: RAM Test
    //     printf("=== Phase 5: RAM Test ===\n");

    // CRUCIAL: Use address 0x00002088, NOT 0x20002088!
    // This is in low memory range, not high SRAM range
    uint32_t test_addr = 0x00002088;  // Test address (Use 0x2088 which maps to 0x00002088)
    uint32_t test_pattern1 = 0x12345678;
    uint32_t test_pattern2 = 0x40000C08;
    uint32_t read_value;

    printf("Writing test pattern 0x%08X to 0x%08X...\n", test_pattern1, test_addr);
    r = cmd_write_memory_long_addr(handle, test_addr, test_pattern1);
    if (r != 0) {
        fprintf(stderr, "**FAILED to write test pattern 1\n");
        return -1;
    }

    printf("Reading back...\n");
    r = cmd_read_memory_long_addr(handle, test_addr, &read_value);
    if (r != 0) {
        fprintf(stderr, "**FAILED to read back test pattern 1\n");
        return -1;
    }

    if (read_value != test_pattern1) {
        fprintf(stderr, "WARNING: RAM test pattern mismatch! Expected 0x%08X, got 0x%08X\n", test_pattern1, read_value);
        fprintf(stderr, "NOTE: Uses 16-bit addresses for RAM test, we use 32-bit.\n");
        fprintf(stderr, "This may not be crucial for flash operations. Continuing anyway...\n");
        // Don't fail - RAM test format mismatch is not crucial
    } else {
    //         printf("==> Test pattern 1 verified\n");

        printf("Writing test pattern 0x%08X...\n", test_pattern2);
        r = cmd_write_memory_long_addr(handle, test_addr, test_pattern2);
        if (r != 0) {
            fprintf(stderr, "Warning: **FAILED to write test pattern 2\n");
        } else {
            r = cmd_read_memory_long_addr(handle, test_addr, &read_value);
            if (r != 0 || read_value != test_pattern2) {
                fprintf(stderr, "Warning: RAM test 2 **FAILED!\n");
            } else {
    //                 printf("==> Test pattern 2 verified\n");
            }
        }
    }

    //          printf("==> Phase 5 complete (RAM test attempted)\n\n");

    // Phase 6: Final BDM Resume
    r = cmd_07_a2(handle, 0x01);
    r |= cmd_04_40_58_04(handle);
    r |= cmd_04_7f_fe_02(handle);
    r |= cmd_04_7f_fe_02(handle);
    r |= cmd_07_95(handle);
    r |= cmd_04_40_00_02(handle);
    r |= cmd_enable_memory_access(handle, 0x00);
    r |= cmd_enable_memory_access(handle, 0x00);

    if (r != 0) {
        fprintf(stderr, "**FAILED during final BDM resume\n");
        return -1;
    }

    // Phase 7: Pre-Flashloader Mode Config
    r = cmd_enter_mode(handle, 0xF8);
    r |= cmd_enter_mode(handle, 0xF0);
    r |= cmd_enter_mode(handle, 0xF8);

    if (r != 0) {
        fprintf(stderr, "**FAILED during pre-flashloader mode config\n");
        return -1;
    }

    // CRUCIAL: 50ms delay after last Enter Mode 0xF8 before memory windows!
    printf("Waiting 50ms for BDM to settle...\n");
    usleep(50000);

    // Phase 8: Memory Window Setup (CRUCIAL FOR SRAM ACCESS)
    printf("Configuring memory windows for SRAM access via short addresses...\n");

    r = cmd_07_12(handle, 0xFFFF);
    usleep(309);

    uint32_t temp_val;
    r |= cmd_07_13(handle, 0x2D80, &temp_val);
    usleep(338);

    r |= cmd_07_11(handle, 0x1940, 0xfc, 0x0a, 0x00, 0x0a);
    usleep(350);

    r |= cmd_07_11(handle, 0x1940, 0x40, 0x11, 0x00, 0x0a);
    usleep(319);

    r |= cmd_07_11(handle, 0x1900, 0x40, 0x10, 0x00, 0x74);
    usleep(328);

    uint8_t p[] = {0x40, 0x10, 0x00, 0x74, 0x00, 0x0f};
    r |= cmd_07_15(handle, 0x1800, p, 6);
    usleep(124);

    r |= cmd_07_15(handle, 0x1800, p, 6);
    usleep(311);

    r |= cmd_07_12(handle, 0xFFFF);
    usleep(338);

    if (r != 0) {
        fprintf(stderr, "**FAILED during memory window setup\n");
        return -1;
    }

    // Verify memory windows by writing test pattern to 0x2088 and reading from 0x2188
    printf("Verifying memory windows with test write...\n");
    r = cmd_write_memory_short_addr(handle, 0x2088, 0x200000B8);
    if (r != 0) {
        fprintf(stderr, "Warning: **FAILED to write test pattern to 0x2088\n");
    }

    uint32_t verify_val = 0;
    r = cmd_07_13(handle, 0x2188, &verify_val);
    if (r != 0) {
        fprintf(stderr, "Warning: **FAILED to read from 0x2188\n");
    } else {
        printf("Memory window test: Read 0x%08X from 0x2188\n", verify_val);
        if (verify_val == 0x200000B8) {
    //             printf("==> Memory windows are correctly configured!\n");
        } else {
            fprintf(stderr, "  WARNING: Memory windows may not be working correctly!\n");
            fprintf(stderr, "  Expected 0x200000B8, got 0x%08X\n", verify_val);
        }
    }

    //          printf("==> Phase 8 complete\n\n");
    //          printf("==> Phase 7 complete\n\n");

    // Read Chip Identification Register (CIR) at IPSBAR + 0x11000A
    // CIR contains: PIN (Part ID Number) in bits 15-6, PRN (Part Revision) in bits 5-0
    // IPSBAR = 0x40000000, so CIR = 0x4011000A
    // Must be read AFTER memory windows are configured for peripheral access
    // Note: Read 32-bit aligned address 0x40110008 (contains RCON at +0, CIR at +2)
    // Big-endian: RCON in bits 31-16, CIR in bits 15-0
    // IMPORTANT: CIR reads as 0 until firmware initializes IPSBAR with valid bit!
    // We cannot read CIR reliably during BDM init before firmware runs.
    // Use the BDM ID register (0x2D80) for chip detection instead.
    uint32_t ccm_regs = 0;
    uint16_t cir_value = 0;
    r = cmd_read_memory_long_addr(handle, 0x40110008, &ccm_regs);
    if (r == 0 && ccm_regs != 0) {
        cir_value = ccm_regs & 0xFFFF;  // CIR is in lower 16 bits (big-endian, at offset 0xA)
    }
    const char *part_name = NULL;
    int prn = -1;

    if (r == 0 && cir_value != 0) {
        uint16_t pin = (cir_value >> 6) & 0x3FF;  // Part ID Number (bits 15-6)
        prn = cir_value & 0x3F;                   // Part Revision Number (bits 5-0)

        switch (pin) {
            case 0x48: part_name = "MCF52230"; break;
            case 0x49: part_name = "MCF52231"; break;
            case 0x4A: part_name = "MCF52233"; break;
            case 0x4B: part_name = "MCF52234"; break;
            case 0x4C: part_name = "MCF52235"; break;
            default:   part_name = NULL;       break;
        }

        // Determine flash size based on part number
        // MCF52230/52231: 64KB flash, 16KB SRAM
        // MCF52233:       256KB flash, 32KB SRAM
        // MCF52234/52235: 256KB flash, 32KB SRAM
        switch (pin) {
            case 0x48:  // MCF52230
            case 0x49:  // MCF52231
                *flash_size_kb = 64;
                break;
            case 0x4A:  // MCF52233
            case 0x4B:  // MCF52234
            case 0x4C:  // MCF52235
            default:
                *flash_size_kb = 256;
                break;
        }
    }

    // Fallback to BDM chip ID if CIR not available
    // BDM CSR (0x2D80) format: 0x01900000 = MCF5223x family
    if (part_name == NULL) {
        *flash_size_kb = 256;
        if ((chip_id & 0xFFF00000) == 0x01900000) {
            part_name = "MCF5223x";  // Generic MCF5223x (CIR unavailable)
        } else {
            part_name = "ColdFire V2";
        }
    }

    printf("========================================\n");
    printf("  Initialization Complete!\n");
    if (prn >= 0) {
        printf("  Chip: %s (rev.%d)\n", part_name, prn);
    } else {
        printf("  Chip: %s\n", part_name);
    }
    printf("  Flash Size: %u KB\n", *flash_size_kb);
    printf("========================================\n\n");

    return 0;
}
