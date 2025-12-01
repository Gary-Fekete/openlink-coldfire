/*
 * GPL Flashloader Interface for OpenLink ColdFire
 *
 * Clean interface for flash operations using the GPL flashloader
 * instead of the proprietary CodeWarrior flashloader chunks.
 *
 * License: GPL v3
 */

#ifndef FLASH_GPL_H
#define FLASH_GPL_H

#include <stdint.h>
#include <libusb-1.0/libusb.h>

/* Flash configuration */
#define FLASH_BASE           0x00000000
#define FLASH_SIZE           0x40000    /* 256KB */
#define FLASH_SECTOR_SIZE    0x800      /* 2KB erase/program chunk size */
#define FLASH_NUM_SECTORS    128        /* 256KB / 2KB */

/* Default flashloader path (relative to executable or working directory) */
#define DEFAULT_FLASHLOADER_PATH "flashloader/flashloader.elf"
/* Installed flashloader path (system-wide installation) */
#define INSTALLED_FLASHLOADER_PATH "/usr/local/share/openlink-coldfire/flashloader/flashloader.elf"

/* Include elf_loader for simple_flash_state_t */
#include "elf_loader.h"

/*
 * Flash state structure
 * Manages the GPL flashloader lifecycle
 */
typedef struct {
    libusb_device_handle *usb_handle;
    simple_flash_state_t *loader_state;
    char *flashloader_path;
    int initialized;
    int erased_sectors[FLASH_NUM_SECTORS];  /* Track which sectors are erased */
} gpl_flash_state_t;

/*
 * Initialize flash subsystem
 *
 * @param state          Flash state structure (caller allocated)
 * @param handle         USB device handle
 * @param flashloader    Path to flashloader.elf (NULL for default)
 * @return               0 on success, -1 on error
 */
int gpl_flash_init(gpl_flash_state_t *state, libusb_device_handle *handle,
                   const char *flashloader);

/*
 * Cleanup flash subsystem
 *
 * @param state          Flash state structure
 */
void gpl_flash_cleanup(gpl_flash_state_t *state);

/*
 * Mass erase entire flash (256KB)
 * Uses flashloader operation 1
 *
 * @param state          Flash state structure
 * @return               0 on success, -1 on error
 */
int gpl_flash_mass_erase(gpl_flash_state_t *state);

/*
 * Erase a single sector (2KB chunk)
 * Uses flashloader operation 2
 *
 * @param state          Flash state structure
 * @param sector_num     Sector number (0-127)
 * @return               0 on success, -1 on error
 */
int gpl_flash_erase_sector(gpl_flash_state_t *state, int sector_num);

/*
 * Erase sectors covering an address range
 *
 * @param state          Flash state structure
 * @param start_addr     Start address
 * @param length         Length in bytes
 * @return               0 on success, -1 on error
 */
int gpl_flash_erase_range(gpl_flash_state_t *state, uint32_t start_addr, uint32_t length);

/*
 * Program data to flash
 * Uses flashloader operation 3
 * Handles chunking internally for large data blocks
 *
 * @param state          Flash state structure
 * @param addr           Flash address (must be 4-byte aligned)
 * @param data           Data to program
 * @param length         Length in bytes
 * @return               0 on success, -1 on error
 */
int gpl_flash_program(gpl_flash_state_t *state, uint32_t addr,
                      const uint8_t *data, uint32_t length);

/*
 * Blank check flash region
 * Uses flashloader operation 4
 *
 * @param state          Flash state structure
 * @param addr           Start address
 * @param length         Length in bytes
 * @return               0 if blank, 1 if not blank, -1 on error
 */
int gpl_flash_blank_check(gpl_flash_state_t *state, uint32_t addr, uint32_t length);

/*
 * Verify flash contents against buffer
 * Uses flashloader operation 5
 *
 * @param state          Flash state structure
 * @param addr           Start address
 * @param data           Expected data
 * @param length         Length in bytes
 * @return               0 if match, 1 if mismatch, -1 on error
 */
int gpl_flash_verify(gpl_flash_state_t *state, uint32_t addr,
                     const uint8_t *data, uint32_t length);

/*
 * High-level: Erase and program a binary file to flash
 *
 * @param state          Flash state structure
 * @param data           Binary data
 * @param length         Length in bytes
 * @param base_addr      Base address in flash (usually 0x00000000)
 * @param verify         If non-zero, verify after programming
 * @return               0 on success, -1 on error
 */
int gpl_flash_program_binary(gpl_flash_state_t *state, const uint8_t *data,
                             uint32_t length, uint32_t base_addr, int verify);

/*
 * Get flashloader entry point
 *
 * @param state          Flash state structure
 * @return               Entry point address, or 0 if not loaded
 */
uint32_t gpl_flash_get_entry_point(gpl_flash_state_t *state);

/*
 * Check if flash is ready for operations
 *
 * @param state          Flash state structure
 * @return               1 if ready, 0 if not
 */
int gpl_flash_is_ready(gpl_flash_state_t *state);

#endif /* FLASH_GPL_H */
