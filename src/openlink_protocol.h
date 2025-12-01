#ifndef OPENLINK_PROTOCOL_H
#define OPENLINK_PROTOCOL_H

#include <libusb-1.0/libusb.h>
#include <stdint.h>

// Endpoints
#define ENDPOINT_IN  0x81
#define ENDPOINT_OUT 0x02

// Global verbosity flag - set to 0 for quiet operation, 1 for verbose debug output
// Default is 0 (quiet). Call openlink_set_verbose(1) to enable debug output.
extern int g_openlink_verbose;
void openlink_set_verbose(int level);

// Function Prototypes
void print_hex(unsigned char* data, int size);
void print_as_ascii(unsigned char* data, int size);

int usb_reset(libusb_device_handle *dev);
int send_bb_command(libusb_device_handle *handle, unsigned char *cmd_data, int cmd_len, const char *cmd_name);
int send_aa_command(libusb_device_handle *handle, unsigned char *cmd_data, int cmd_len, const char *cmd_name);

// Memory Access Functions
int cmd_write_memory_byte_addr(libusb_device_handle *handle, uint32_t addr, uint8_t data);
int cmd_read_memory_byte_addr(libusb_device_handle *handle, uint32_t addr, uint8_t *data);
int cmd_write_memory_word_addr(libusb_device_handle *handle, uint32_t addr, uint16_t data);
int cmd_read_memory_word_addr(libusb_device_handle *handle, uint32_t addr, uint16_t *data);
int cmd_write_memory_short_addr(libusb_device_handle *handle, uint16_t addr, uint32_t data); // 16-bit address, 32-bit data
int cmd_write_memory_long_addr(libusb_device_handle *handle, uint32_t addr, uint32_t data);
int cmd_read_memory_long_addr(libusb_device_handle *handle, uint32_t addr, uint32_t *data);

// Memory Read Command (cmd_0717) - Universal Flash/RAM reader
// Reads from Flash (0x00000000+) or SRAM (0x20000000+) using 32-bit addressing
// Returns data in buffer. Response format is 88 a5 (not 99 66).
int cmd_0717_read_memory(libusb_device_handle *handle, uint32_t addr, uint16_t length,
                         uint8_t *buffer, int buffer_size);

// Memory Read/Verify Command (cmd_071b) - Used in SRAM validation sequence
// Similar to cmd_0717 but used for verification operations (82 times in SRAM validation)
// Returns data in buffer. Response format is 99 66 (standard format).
int cmd_071b(libusb_device_handle *handle, uint32_t addr, uint16_t length,
             uint8_t *buffer, int buffer_size);

// SRAM-specific read function - extracts longword from interspersed response format
// CRITICAL: cmd_071b returns SRAM data at offsets 0,7,9,11 (not contiguous)
// Use this for reading 32-bit values from SRAM (0x20000000-0x20007FFF)
int cmd_071b_read_sram_longword(libusb_device_handle *handle, uint32_t addr, uint32_t *value);

// Memory Window Control - Critical for accessing Flash Module 1 (upper 128KB)
int cmd_set_memory_window(libusb_device_handle *handle, uint32_t window_addr);

// Flashloader Functions
int cmd_download_block(libusb_device_handle *handle, uint32_t address, unsigned char *data, int length);
int cmd_download_block_single(libusb_device_handle *handle, uint32_t address, unsigned char *data, int length);  // Single large transfer (matches CodeWarrior)
int cmd_download_block_chunk(libusb_device_handle *handle, uint32_t address, unsigned char *data, int length);  // Single chunk upload
int cmd_bdm_resume(libusb_device_handle *handle);
int cmd_07_02_bdm_go(libusb_device_handle *handle);  // BDM GO command to execute flashloader
int cmd_07_14_write_bdm_reg(libusb_device_handle *handle, uint16_t reg, uint32_t value);  // Write BDM register (e.g., PC)
int cmd_bdm_freeze(libusb_device_handle *handle, uint8_t *is_frozen);
int cmd_bdm_halt(libusb_device_handle *handle);
int cmd_bdm_reinit_after_execution(libusb_device_handle *handle);
int cmd_07_95(libusb_device_handle *handle);
int cmd_bdm_cmd_00_02(libusb_device_handle *handle);

// FLASH Read Functions
int cmd_enter_mode(libusb_device_handle *handle, uint8_t mode);
int cmd_enable_memory_access(libusb_device_handle *handle, uint8_t param);
int cmd_read_memory_block(libusb_device_handle *handle, uint32_t addr, uint8_t *buffer, int buffer_size);
int cmd_07_17_setup_window(libusb_device_handle *handle, uint32_t addr);
int cmd_07_1b(libusb_device_handle *handle, uint32_t addr, uint16_t size);

// Device Information
int cmd_01_0b(libusb_device_handle *handle);  // Get Device Info (CodeWarrior sends this TWICE first)

// BDM Configuration Functions
int cmd_07_12(libusb_device_handle *handle, uint16_t param);
int cmd_07_1e(libusb_device_handle *handle, uint16_t p1, uint32_t p2, uint8_t p3);
int cmd_071e_write_sram(libusb_device_handle *handle, uint32_t addr, uint32_t data);
int cmd_read_bdm_reg(libusb_device_handle *handle, uint16_t reg, uint16_t *value);
int cmd_write_bdm_reg(libusb_device_handle *handle, uint16_t reg, uint32_t data);
int cmd_07_11(libusb_device_handle *handle, uint16_t reg, uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4);
int cmd_07_11_read_bdm_reg(libusb_device_handle *handle, uint16_t window, uint16_t reg, uint32_t *value);
int cmd_read_pc(libusb_device_handle *handle, uint32_t *pc_value);
int cmd_read_sr(libusb_device_handle *handle, uint32_t *sr_value);
int cmd_write_pc(libusb_device_handle *handle, uint32_t pc_value);
int cmd_07_16_write_pc(libusb_device_handle *handle, uint32_t pc_value);  // CodeWarrior style PC write before BDM GO
int cmd_07_15(libusb_device_handle *handle, uint16_t reg, uint8_t *params, int param_count);
int cmd_07_a2(libusb_device_handle *handle, uint8_t param);
int cmd_07_13(libusb_device_handle *handle, uint16_t reg, uint32_t *value);

// BDM Command 04 Family (Initialization)
int cmd_04_40_58_04(libusb_device_handle *handle);
int cmd_04_7f_fe_02(libusb_device_handle *handle);
int cmd_04_40_00_02(libusb_device_handle *handle);

// Comprehensive Initialization
int target_init_full(libusb_device_handle *handle, uint32_t *flash_size_kb);

// Memory Window Setup - Full sequence (required for SRAM parameter writes)
int cmd_setup_memory_windows_full(libusb_device_handle *handle);

// SRAM Pre-Initialization - Lines 1-138 from RAM_INIT_SEQUENCE.txt
int sram_pre_init(libusb_device_handle *handle);

// SRAM Validation Sequence - Lines 139-454 from RAM_INIT_SEQUENCE.txt
int sram_validation_sequence(libusb_device_handle *handle);

// Complete SRAM Initialization - Runs pre-init + validation
int sram_init_full(libusb_device_handle *handle);

// Flash Programming Commands
int cmd_07_14(libusb_device_handle *handle, uint16_t reg, uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4, uint32_t addr);
int cmd_07_19(libusb_device_handle *handle, uint32_t addr, uint32_t data);
int cmd_07_10(libusb_device_handle *handle, uint16_t reg);
int cmd_07_17(libusb_device_handle *handle);  // RAM test command

void generated_commands(libusb_device_handle *handle);

#endif // OPENLINK_PROTOCOL_H