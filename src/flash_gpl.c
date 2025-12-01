/*
 * GPL Flashloader Interface for OpenLink ColdFire
 *
 * Implementation of flash operations using the GPL flashloader
 * instead of the proprietary CodeWarrior flashloader chunks.
 *
 * License: GPL v3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "flash_gpl.h"
#include "elf_loader.h"
#include "openlink_protocol.h"

int gpl_flash_init(gpl_flash_state_t *state, libusb_device_handle *handle,
                   const char *flashloader)
{
    if (!state || !handle) {
        return -1;
    }

    memset(state, 0, sizeof(*state));
    state->usb_handle = handle;

    /* Use default path if not specified, with fallback to installed location */
    if (flashloader) {
        state->flashloader_path = strdup(flashloader);
    } else {
        /* Try default path first (relative to current directory) */
        FILE *test = fopen(DEFAULT_FLASHLOADER_PATH, "r");
        if (test) {
            fclose(test);
            state->flashloader_path = strdup(DEFAULT_FLASHLOADER_PATH);
        } else {
            /* Fall back to installed location */
            test = fopen(INSTALLED_FLASHLOADER_PATH, "r");
            if (test) {
                fclose(test);
                state->flashloader_path = strdup(INSTALLED_FLASHLOADER_PATH);
            } else {
                /* Default to original path for error message */
                state->flashloader_path = strdup(DEFAULT_FLASHLOADER_PATH);
            }
        }
    }

    if (!state->flashloader_path) {
        fprintf(stderr, "Flash: Out of memory\n");
        return -1;
    }

    /* Allocate internal state */
    state->loader_state = malloc(sizeof(simple_flash_state_t));
    if (!state->loader_state) {
        fprintf(stderr, "Flash: Out of memory\n");
        free(state->flashloader_path);
        state->flashloader_path = NULL;
        return -1;
    }

    /* Initialize target SRAM */
    printf("Flash: Initializing target SRAM...\n");
    int r = sram_init_full(handle);
    if (r != 0) {
        fprintf(stderr, "Flash: Failed to initialize SRAM\n");
        gpl_flash_cleanup(state);
        return -1;
    }

    /* Load flashloader ELF */
    printf("Flash: Loading %s...\n", state->flashloader_path);
    simple_flash_state_t *sstate = (simple_flash_state_t *)state->loader_state;
    r = simple_flash_init(handle, state->flashloader_path, sstate);
    if (r != 0) {
        fprintf(stderr, "Flash: Failed to load flashloader\n");
        gpl_flash_cleanup(state);
        return -1;
    }

    printf("Flash: Flashloader loaded (entry=0x%08X, size=%u bytes)\n",
           sstate->elf.entry_point, sstate->elf.data_size);

    /* Run init operation (Op 0) */
    printf("Flash: Initializing flash module...\n");
    fflush(stdout);
    uint32_t result;
    r = simple_flash_run_op(handle, sstate, FLASH_OP_INIT, 0, 0, &result);
    if (r != 0 || result != FLASH_RESULT_SUCCESS) {
        fprintf(stderr, "Flash: Init failed (result=0x%08X)\n", result);
        gpl_flash_cleanup(state);
        return -1;
    }

    state->initialized = 1;
    printf("Flash: Initialized successfully\n");
    return 0;
}

void gpl_flash_cleanup(gpl_flash_state_t *state)
{
    if (!state) {
        return;
    }

    if (state->loader_state) {
        simple_flash_cleanup((simple_flash_state_t *)state->loader_state);
        free(state->loader_state);
        state->loader_state = NULL;
    }

    if (state->flashloader_path) {
        free(state->flashloader_path);
        state->flashloader_path = NULL;
    }

    state->initialized = 0;
}

int gpl_flash_mass_erase(gpl_flash_state_t *state)
{
    if (!state || !state->initialized) {
        fprintf(stderr, "Flash: Not initialized\n");
        return -1;
    }

    printf("Flash: Mass erasing entire flash (256KB)...\n");

    simple_flash_state_t *sstate = (simple_flash_state_t *)state->loader_state;
    int r = simple_flash_mass_erase(state->usb_handle, sstate);
    if (r != 0) {
        fprintf(stderr, "Flash: Mass erase failed\n");
        return -1;
    }

    /* Mark all sectors as erased */
    for (int i = 0; i < FLASH_NUM_SECTORS; i++) {
        state->erased_sectors[i] = 1;
    }

    printf("Flash: Mass erase complete\n");
    return 0;
}

int gpl_flash_erase_sector(gpl_flash_state_t *state, int sector_num)
{
    if (!state || !state->initialized) {
        fprintf(stderr, "Flash: Not initialized\n");
        return -1;
    }

    if (sector_num < 0 || sector_num >= FLASH_NUM_SECTORS) {
        fprintf(stderr, "Flash: Invalid sector number %d\n", sector_num);
        return -1;
    }

    uint32_t sector_addr = sector_num * FLASH_SECTOR_SIZE;
    printf("Flash: Erasing sector %d (0x%08X)...\n", sector_num, sector_addr);

    simple_flash_state_t *sstate = (simple_flash_state_t *)state->loader_state;
    int r = simple_flash_erase_sector(state->usb_handle, sstate, sector_addr);
    if (r != 0) {
        fprintf(stderr, "Flash: Sector erase failed\n");
        return -1;
    }

    state->erased_sectors[sector_num] = 1;
    printf("Flash: Sector %d erased\n", sector_num);
    return 0;
}

int gpl_flash_erase_range(gpl_flash_state_t *state, uint32_t start_addr, uint32_t length)
{
    if (!state || !state->initialized) {
        fprintf(stderr, "Flash: Not initialized\n");
        return -1;
    }

    if (start_addr + length > FLASH_SIZE) {
        fprintf(stderr, "Flash: Address range 0x%08X-0x%08X exceeds flash size\n",
                start_addr, start_addr + length);
        return -1;
    }

    /* Calculate sector range */
    int start_sector = start_addr / FLASH_SECTOR_SIZE;
    int end_sector = (start_addr + length + FLASH_SECTOR_SIZE - 1) / FLASH_SECTOR_SIZE;

    printf("Flash: Erasing sectors %d-%d (0x%08X-0x%08X)...\n",
           start_sector, end_sector - 1, start_addr, start_addr + length);

    for (int s = start_sector; s < end_sector; s++) {
        /* Skip already erased sectors */
        if (state->erased_sectors[s]) {
            printf("Flash: Sector %d already erased, skipping\n", s);
            continue;
        }

        int r = gpl_flash_erase_sector(state, s);
        if (r != 0) {
            return -1;
        }
    }

    printf("Flash: Range erase complete\n");
    return 0;
}

int gpl_flash_program(gpl_flash_state_t *state, uint32_t addr,
                      const uint8_t *data, uint32_t length)
{
    if (!state || !state->initialized) {
        fprintf(stderr, "Flash: Not initialized\n");
        return -1;
    }

    if (!data || length == 0) {
        return 0;  /* Nothing to program */
    }

    if (addr + length > FLASH_SIZE) {
        fprintf(stderr, "Flash: Address range 0x%08X-0x%08X exceeds flash size\n",
                addr, addr + length);
        return -1;
    }

    printf("Flash: Programming %u bytes at 0x%08X...\n", length, addr);

    simple_flash_state_t *sstate = (simple_flash_state_t *)state->loader_state;

    /* Program in chunks (data buffer is 1KB) */
    uint32_t offset = 0;
    while (offset < length) {
        uint32_t chunk_size = length - offset;
        if (chunk_size > FLASHLOADER_DATA_BUFFER_SIZE) {
            chunk_size = FLASHLOADER_DATA_BUFFER_SIZE;
        }

        int r = simple_flash_program(state->usb_handle, sstate,
                                     addr + offset, data + offset, chunk_size);
        if (r != 0) {
            fprintf(stderr, "Flash: Programming failed at offset 0x%08X\n", offset);
            return -1;
        }

        offset += chunk_size;
        printf("Flash: Programmed %u/%u bytes\r", offset, length);
        fflush(stdout);
    }

    printf("\nFlash: Programming complete\n");
    return 0;
}

int gpl_flash_blank_check(gpl_flash_state_t *state, uint32_t addr, uint32_t length)
{
    if (!state || !state->initialized) {
        fprintf(stderr, "Flash: Not initialized\n");
        return -1;
    }

    printf("Flash: Blank checking 0x%08X-0x%08X...\n", addr, addr + length);

    simple_flash_state_t *sstate = (simple_flash_state_t *)state->loader_state;
    uint32_t result;

    int r = simple_flash_run_op(state->usb_handle, sstate,
                                FLASH_OP_BLANK_CHECK, addr, length, &result);
    if (r != 0) {
        fprintf(stderr, "Flash: Blank check operation failed\n");
        return -1;
    }

    if (result == FLASH_RESULT_SUCCESS) {
        printf("Flash: Region is blank\n");
        return 0;
    } else if (result == FLASH_RESULT_NOT_BLANK) {
        printf("Flash: Region is NOT blank\n");
        return 1;
    } else {
        fprintf(stderr, "Flash: Blank check error (result=0x%08X)\n", result);
        return -1;
    }
}

int gpl_flash_verify(gpl_flash_state_t *state, uint32_t addr,
                     const uint8_t *data, uint32_t length)
{
    if (!state || !state->initialized) {
        fprintf(stderr, "Flash: Not initialized\n");
        return -1;
    }

    if (!data || length == 0) {
        return 0;  /* Nothing to verify */
    }

    printf("Flash: Verifying %u bytes at 0x%08X...\n", length, addr);

    simple_flash_state_t *sstate = (simple_flash_state_t *)state->loader_state;

    /* Verify in chunks (data buffer is 1KB) */
    uint32_t offset = 0;
    while (offset < length) {
        uint32_t chunk_size = length - offset;
        if (chunk_size > FLASHLOADER_DATA_BUFFER_SIZE) {
            chunk_size = FLASHLOADER_DATA_BUFFER_SIZE;
        }

        /* Upload expected data to data buffer */
        for (uint32_t i = 0; i < chunk_size; i += 4) {
            uint32_t value = 0;
            for (int j = 0; j < 4 && (i + j) < chunk_size; j++) {
                value |= data[offset + i + j] << (24 - j * 8);
            }
            cmd_07_19(state->usb_handle, FLASHLOADER_DATA_BUFFER + i, value);
        }

        /* Run verify operation */
        uint32_t result;
        int r = simple_flash_run_op(state->usb_handle, sstate,
                                    FLASH_OP_VERIFY, addr + offset, chunk_size, &result);
        if (r != 0) {
            fprintf(stderr, "Flash: Verify operation failed at offset 0x%08X\n", offset);
            return -1;
        }

        if (result != FLASH_RESULT_SUCCESS) {
            fprintf(stderr, "Flash: Verification mismatch at offset 0x%08X\n", offset);
            return 1;
        }

        offset += chunk_size;
        printf("Flash: Verified %u/%u bytes\r", offset, length);
        fflush(stdout);
    }

    printf("\nFlash: Verification passed\n");
    return 0;
}

int gpl_flash_program_binary(gpl_flash_state_t *state, const uint8_t *data,
                             uint32_t length, uint32_t base_addr, int verify)
{
    if (!state || !state->initialized) {
        fprintf(stderr, "Flash: Not initialized\n");
        return -1;
    }

    if (!data || length == 0) {
        fprintf(stderr, "Flash: No data to program\n");
        return -1;
    }

    printf("Flash: Programming %u bytes at 0x%08X\n", length, base_addr);

    /* Erase required sectors */
    int r = gpl_flash_erase_range(state, base_addr, length);
    if (r != 0) {
        return -1;
    }

    /* Program the data */
    r = gpl_flash_program(state, base_addr, data, length);
    if (r != 0) {
        return -1;
    }

    /* Verify if requested */
    if (verify) {
        r = gpl_flash_verify(state, base_addr, data, length);
        if (r != 0) {
            return -1;
        }
    }

    printf("Flash: Programming complete\n");
    return 0;
}

uint32_t gpl_flash_get_entry_point(gpl_flash_state_t *state)
{
    if (!state || !state->loader_state) {
        return 0;
    }

    simple_flash_state_t *sstate = (simple_flash_state_t *)state->loader_state;
    return sstate->elf.entry_point;
}

int gpl_flash_is_ready(gpl_flash_state_t *state)
{
    return (state && state->initialized) ? 1 : 0;
}
