/*
 * ELF Loader for OpenLink ColdFire
 *
 * Minimal ELF parser for loading flashloader.elf to target SRAM.
 * Supports 32-bit big-endian ELF files (M68K).
 */

#ifndef ELF_LOADER_H
#define ELF_LOADER_H

#include <stdint.h>
#include <libusb-1.0/libusb.h>

/* Loaded ELF information */
typedef struct {
    uint32_t entry_point;       /* Entry point address */
    uint32_t load_addr;         /* Load address of first segment */
    uint8_t *data;              /* Loaded segment data */
    uint32_t data_size;         /* Size of loaded data */
} elf_info_t;

/*
 * Load an ELF file and extract loadable segments
 *
 * @param filename  Path to ELF file
 * @param info      Output: populated with ELF info (caller must free info->data)
 * @return          0 on success, -1 on error
 */
int elf_load_file(const char *filename, elf_info_t *info);

/*
 * Upload loaded ELF data to target SRAM via BDM
 *
 * @param handle    USB device handle
 * @param info      ELF info from elf_load_file()
 * @return          0 on success, -1 on error
 */
int elf_upload_to_target(libusb_device_handle *handle, const elf_info_t *info);

/*
 * Free ELF info resources
 */
void elf_free(elf_info_t *info);

/* Simple flashloader parameter addresses */
#define FLASHLOADER_PARAM_OPERATION   0x20000000
#define FLASHLOADER_PARAM_FLASH_ADDR  0x20000004
#define FLASHLOADER_PARAM_LENGTH      0x20000008
#define FLASHLOADER_PARAM_RESULT      0x2000000C
#define FLASHLOADER_PARAM_STATUS      0x20000010
#define FLASHLOADER_DATA_BUFFER       0x20000100
#define FLASHLOADER_DATA_BUFFER_SIZE  0x400  /* 1KB */

/* Flashloader operations */
#define FLASH_OP_INIT         0
#define FLASH_OP_MASS_ERASE   1
#define FLASH_OP_SECTOR_ERASE 2
#define FLASH_OP_PROGRAM      3
#define FLASH_OP_BLANK_CHECK  4
#define FLASH_OP_VERIFY       5

/* Result codes */
#define FLASH_RESULT_SUCCESS      0x00000000
#define FLASH_RESULT_ACCERR       0x00000001
#define FLASH_RESULT_PVIOL        0x00000002
#define FLASH_RESULT_NOT_BLANK    0x00000003
#define FLASH_RESULT_VERIFY_FAIL  0x00000004
#define FLASH_RESULT_TIMEOUT      0x00000005
#define FLASH_RESULT_UNKNOWN_OP   0x000000FF

/*
 * Simple Flashloader Operations
 * These use the new C flashloader with simple parameter block interface
 */

/* Flashloader state */
typedef struct {
    elf_info_t elf;             /* Loaded flashloader ELF */
    int loaded;                 /* 1 if flashloader is loaded to target */
    int initialized;            /* 1 if flash module is initialized */
} simple_flash_state_t;

/*
 * Initialize simple flashloader system
 * Loads flashloader.elf and prepares for flash operations
 *
 * @param handle    USB device handle
 * @param elf_path  Path to flashloader.elf
 * @param state     Output: state structure (caller manages lifetime)
 * @return          0 on success, -1 on error
 */
int simple_flash_init(libusb_device_handle *handle, const char *elf_path,
                      simple_flash_state_t *state);

/*
 * Run a flashloader operation
 *
 * @param handle     USB device handle
 * @param state      Flashloader state
 * @param operation  Operation code (FLASH_OP_*)
 * @param flash_addr Flash address for operation
 * @param length     Length in bytes
 * @param result     Output: result code from flashloader
 * @return           0 on success, -1 on error
 */
int simple_flash_run_op(libusb_device_handle *handle, simple_flash_state_t *state,
                        uint32_t operation, uint32_t flash_addr, uint32_t length,
                        uint32_t *result);

/*
 * Erase a flash sector (2KB)
 *
 * @param handle     USB device handle
 * @param state      Flashloader state
 * @param sector_addr Address of sector to erase (must be sector-aligned)
 * @return           0 on success, -1 on error
 */
int simple_flash_erase_sector(libusb_device_handle *handle, simple_flash_state_t *state,
                              uint32_t sector_addr);

/*
 * Program data to flash
 *
 * @param handle     USB device handle
 * @param state      Flashloader state
 * @param flash_addr Destination flash address
 * @param data       Data to program
 * @param length     Length in bytes (max FLASHLOADER_DATA_BUFFER_SIZE per call)
 * @return           0 on success, -1 on error
 */
int simple_flash_program(libusb_device_handle *handle, simple_flash_state_t *state,
                         uint32_t flash_addr, const uint8_t *data, uint32_t length);

/*
 * Mass erase entire flash
 *
 * @param handle     USB device handle
 * @param state      Flashloader state
 * @return           0 on success, -1 on error
 */
int simple_flash_mass_erase(libusb_device_handle *handle, simple_flash_state_t *state);

/*
 * Cleanup flashloader state
 */
void simple_flash_cleanup(simple_flash_state_t *state);

#endif /* ELF_LOADER_H */
