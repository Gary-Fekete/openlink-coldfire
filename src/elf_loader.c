/*
 * ELF Loader for OpenLink ColdFire
 *
 * Minimal ELF parser for loading flashloader.elf to target SRAM.
 * Supports 32-bit big-endian ELF files (M68K).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "elf_loader.h"
#include "openlink_protocol.h"

/* ELF32 Header (big-endian) */
typedef struct {
    uint8_t  e_ident[16];    /* Magic number and other info */
    uint16_t e_type;         /* Object file type */
    uint16_t e_machine;      /* Architecture */
    uint32_t e_version;      /* Object file version */
    uint32_t e_entry;        /* Entry point virtual address */
    uint32_t e_phoff;        /* Program header table file offset */
    uint32_t e_shoff;        /* Section header table file offset */
    uint32_t e_flags;        /* Processor-specific flags */
    uint16_t e_ehsize;       /* ELF header size in bytes */
    uint16_t e_phentsize;    /* Program header table entry size */
    uint16_t e_phnum;        /* Program header table entry count */
    uint16_t e_shentsize;    /* Section header table entry size */
    uint16_t e_shnum;        /* Section header table entry count */
    uint16_t e_shstrndx;     /* Section header string table index */
} __attribute__((packed)) Elf32_Ehdr;

/* ELF32 Program Header (big-endian) */
typedef struct {
    uint32_t p_type;         /* Segment type */
    uint32_t p_offset;       /* Segment file offset */
    uint32_t p_vaddr;        /* Segment virtual address */
    uint32_t p_paddr;        /* Segment physical address */
    uint32_t p_filesz;       /* Segment size in file */
    uint32_t p_memsz;        /* Segment size in memory */
    uint32_t p_flags;        /* Segment flags */
    uint32_t p_align;        /* Segment alignment */
} __attribute__((packed)) Elf32_Phdr;

/* ELF32 Section Header (big-endian) */
typedef struct {
    uint32_t sh_name;        /* Section name (index into string table) */
    uint32_t sh_type;        /* Section type */
    uint32_t sh_flags;       /* Section flags */
    uint32_t sh_addr;        /* Section virtual address */
    uint32_t sh_offset;      /* Section file offset */
    uint32_t sh_size;        /* Section size in bytes */
    uint32_t sh_link;        /* Link to another section */
    uint32_t sh_info;        /* Additional section information */
    uint32_t sh_addralign;   /* Section alignment */
    uint32_t sh_entsize;     /* Entry size if section holds table */
} __attribute__((packed)) Elf32_Shdr;

/* ELF constants */
#define EI_MAG0     0
#define EI_MAG1     1
#define EI_MAG2     2
#define EI_MAG3     3
#define EI_CLASS    4
#define EI_DATA     5

#define ELFMAG0     0x7f
#define ELFMAG1     'E'
#define ELFMAG2     'L'
#define ELFMAG3     'F'

#define ELFCLASS32  1
#define ELFDATA2MSB 2   /* Big endian */

#define ET_EXEC     2   /* Executable file */
#define EM_68K      4   /* MC68000 */

#define PT_LOAD     1   /* Loadable program segment */

#define SHT_PROGBITS  1   /* Program data */
#define SHF_ALLOC     0x2 /* Occupies memory during execution */

/* Convert 16-bit big-endian to host */
static uint16_t be16_to_host(uint16_t val) {
    uint8_t *p = (uint8_t *)&val;
    return (p[0] << 8) | p[1];
}

/* Convert 32-bit big-endian to host */
static uint32_t be32_to_host(uint32_t val) {
    uint8_t *p = (uint8_t *)&val;
    return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

int elf_load_file(const char *filename, elf_info_t *info) {
    FILE *fp = NULL;
    Elf32_Ehdr ehdr;
    Elf32_Shdr shdr;
    int result = -1;

    if (!filename || !info) {
        return -1;
    }

    memset(info, 0, sizeof(*info));

    fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "ELF: Cannot open %s\n", filename);
        return -1;
    }

    /* Read ELF header */
    if (fread(&ehdr, sizeof(ehdr), 1, fp) != 1) {
        fprintf(stderr, "ELF: Failed to read ELF header\n");
        goto cleanup;
    }

    /* Verify ELF magic */
    if (ehdr.e_ident[EI_MAG0] != ELFMAG0 ||
        ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
        ehdr.e_ident[EI_MAG2] != ELFMAG2 ||
        ehdr.e_ident[EI_MAG3] != ELFMAG3) {
        fprintf(stderr, "ELF: Invalid ELF magic\n");
        goto cleanup;
    }

    /* Check ELF class (32-bit) */
    if (ehdr.e_ident[EI_CLASS] != ELFCLASS32) {
        fprintf(stderr, "ELF: Not a 32-bit ELF file\n");
        goto cleanup;
    }

    /* Check endianness (big-endian for M68K) */
    if (ehdr.e_ident[EI_DATA] != ELFDATA2MSB) {
        fprintf(stderr, "ELF: Not a big-endian ELF file\n");
        goto cleanup;
    }

    /* Check file type */
    if (be16_to_host(ehdr.e_type) != ET_EXEC) {
        fprintf(stderr, "ELF: Not an executable file\n");
        goto cleanup;
    }

    /* Check machine type */
    if (be16_to_host(ehdr.e_machine) != EM_68K) {
        fprintf(stderr, "ELF: Not an M68K ELF file\n");
        goto cleanup;
    }

    /* Get entry point */
    info->entry_point = be32_to_host(ehdr.e_entry);

    /* Read section headers to find ALLOC sections with PROGBITS data */
    uint32_t shoff = be32_to_host(ehdr.e_shoff);
    uint16_t shnum = be16_to_host(ehdr.e_shnum);
    uint16_t shentsize = be16_to_host(ehdr.e_shentsize);

    if (shnum == 0) {
        fprintf(stderr, "ELF: No section headers\n");
        goto cleanup;
    }

    /* First pass: find memory range needed */
    uint32_t min_addr = 0xFFFFFFFF;
    uint32_t max_addr = 0;
    int found_sections = 0;

    for (int i = 0; i < shnum; i++) {
        if (fseek(fp, shoff + i * shentsize, SEEK_SET) != 0) {
            fprintf(stderr, "ELF: Failed to seek to section header %d\n", i);
            goto cleanup;
        }

        if (fread(&shdr, sizeof(shdr), 1, fp) != 1) {
            fprintf(stderr, "ELF: Failed to read section header %d\n", i);
            goto cleanup;
        }

        uint32_t sh_type = be32_to_host(shdr.sh_type);
        uint32_t sh_flags = be32_to_host(shdr.sh_flags);
        uint32_t sh_addr = be32_to_host(shdr.sh_addr);
        uint32_t sh_size = be32_to_host(shdr.sh_size);

        /* Only process PROGBITS sections with ALLOC flag */
        if (sh_type == SHT_PROGBITS && (sh_flags & SHF_ALLOC) && sh_size > 0) {
            if (sh_addr < min_addr) min_addr = sh_addr;
            if (sh_addr + sh_size > max_addr) max_addr = sh_addr + sh_size;
            found_sections++;
        }
    }

    if (found_sections == 0) {
        fprintf(stderr, "ELF: No loadable sections found\n");
        goto cleanup;
    }

    /* Allocate buffer for all sections */
    uint32_t total_size = max_addr - min_addr;
    info->data = (uint8_t *)calloc(1, total_size);  /* Zero-fill for gaps */
    if (!info->data) {
        fprintf(stderr, "ELF: Failed to allocate %u bytes\n", total_size);
        goto cleanup;
    }

    info->load_addr = min_addr;
    info->data_size = total_size;

    printf("ELF: Memory range 0x%08X - 0x%08X (%u bytes)\n",
           min_addr, max_addr, total_size);

    /* Second pass: load section data */
    for (int i = 0; i < shnum; i++) {
        if (fseek(fp, shoff + i * shentsize, SEEK_SET) != 0) {
            goto cleanup;
        }

        if (fread(&shdr, sizeof(shdr), 1, fp) != 1) {
            goto cleanup;
        }

        uint32_t sh_type = be32_to_host(shdr.sh_type);
        uint32_t sh_flags = be32_to_host(shdr.sh_flags);
        uint32_t sh_addr = be32_to_host(shdr.sh_addr);
        uint32_t sh_offset = be32_to_host(shdr.sh_offset);
        uint32_t sh_size = be32_to_host(shdr.sh_size);

        if (sh_type == SHT_PROGBITS && (sh_flags & SHF_ALLOC) && sh_size > 0) {
            printf("ELF: Loading section at VMA 0x%08X, file offset 0x%X, size %u\n",
                   sh_addr, sh_offset, sh_size);

            /* Read section data to correct offset in buffer */
            uint32_t buf_offset = sh_addr - min_addr;

            if (fseek(fp, sh_offset, SEEK_SET) != 0) {
                fprintf(stderr, "ELF: Failed to seek to section data\n");
                goto cleanup;
            }

            if (fread(info->data + buf_offset, sh_size, 1, fp) != 1) {
                fprintf(stderr, "ELF: Failed to read section data\n");
                goto cleanup;
            }
        }
    }

    printf("ELF: Loaded %u bytes total, entry point 0x%08X\n",
           info->data_size, info->entry_point);

    result = 0;

cleanup:
    if (fp) {
        fclose(fp);
    }
    if (result != 0 && info->data) {
        free(info->data);
        info->data = NULL;
    }
    return result;
}

int elf_upload_to_target(libusb_device_handle *handle, const elf_info_t *info) {
    if (!handle || !info || !info->data) {
        return -1;
    }

    printf("ELF: Uploading %u bytes to target at 0x%08X using cmd_07_19...\n",
           info->data_size, info->load_addr);

    /* Use cmd_07_19 for flashloader upload - this is the proven working method.
     * We write 4 bytes at a time (longword aligned).
     */
    uint32_t num_words = (info->data_size + 3) / 4;  /* Round up */

    for (uint32_t i = 0; i < num_words; i++) {
        uint32_t addr = info->load_addr + (i * 4);
        uint32_t offset = i * 4;
        uint32_t value;

        /* Build 32-bit big-endian value from data */
        if (offset + 4 <= info->data_size) {
            value = (info->data[offset] << 24) |
                    (info->data[offset + 1] << 16) |
                    (info->data[offset + 2] << 8) |
                    info->data[offset + 3];
        } else {
            /* Handle partial word at end */
            value = 0;
            for (uint32_t j = offset; j < info->data_size; j++) {
                value |= info->data[j] << (24 - 8 * (j - offset));
            }
        }

        int r = cmd_07_19(handle, addr, value);
        if (r != 0) {
            fprintf(stderr, "ELF: Failed to write at 0x%08X\n", addr);
            return -1;
        }
    }

    printf("ELF: Upload complete (%u words)\n", num_words);

    /* Brief pause to let the target process the upload */
    usleep(50000);

    /* Debug: Verify first 4 bytes of uploaded code */
    uint32_t verify_code;
    int verify_ret = cmd_071b_read_sram_longword(handle, info->load_addr, &verify_code);
    uint32_t expected = (info->data[0] << 24) | (info->data[1] << 16) |
                        (info->data[2] << 8) | info->data[3];

    if (verify_ret != 0) {
        fprintf(stderr, "ELF: WARNING - Verification read failed\n");
    } else {
        printf("ELF: Verify first word at 0x%08X: read=0x%08X, expected=0x%08X\n",
               info->load_addr, verify_code, expected);

        if (verify_code != expected) {
            fprintf(stderr, "ELF: WARNING - Upload verification mismatch!\n");
        }
    }

    return 0;
}

void elf_free(elf_info_t *info) {
    if (info && info->data) {
        free(info->data);
        info->data = NULL;
    }
}

/*
 * Simple Flashloader Operations Implementation
 */

/* Wait for flashloader to halt after operation */
static int wait_for_flashloader_halt(libusb_device_handle *handle, int timeout_sec) {
    /* Wait less frequently to avoid interrupting the CPU too often */
    for (int i = 0; i < timeout_sec; i++) {
        sleep(1);  /* 1 second */

        uint32_t csr;
        int r = cmd_07_13(handle, 0x2D80, &csr);
        if (r != 0) {
            fprintf(stderr, "Flash: Failed to read CSR\n");
            return -1;
        }

        printf("Flash: [%d/%d sec] CSR=0x%08X %s\n", i+1, timeout_sec, csr,
               (csr & 0x00004000) ? "HALTED" : "running");
        fflush(stdout);

        /* Check halted bit (bit 14) */
        if (csr & 0x00004000) {
            return 0;
        }
    }

    fprintf(stderr, "Flash: Timeout waiting for halt\n");
    return -1;
}

int simple_flash_init(libusb_device_handle *handle, const char *elf_path,
                      simple_flash_state_t *state) {
    if (!handle || !elf_path || !state) {
        return -1;
    }

    memset(state, 0, sizeof(*state));

    /* Load the ELF file */
    if (elf_load_file(elf_path, &state->elf) != 0) {
        fprintf(stderr, "Flash: Failed to load %s\n", elf_path);
        return -1;
    }

    printf("Flash: Loaded flashloader ELF (entry 0x%08X)\n", state->elf.entry_point);
    return 0;
}

int simple_flash_run_op(libusb_device_handle *handle, simple_flash_state_t *state,
                        uint32_t operation, uint32_t flash_addr, uint32_t length,
                        uint32_t *result) {
    int r;

    if (!handle || !state || !result) {
        return -1;
    }

    /* Upload flashloader if not already done */
    if (!state->loaded) {
        printf("Flash: Uploading flashloader to target...\n");

        /* Setup memory windows (required for SRAM writes) */
        cmd_07_10(handle, 0x2D80);
        cmd_07_10(handle, 0x0000);
        cmd_07_10(handle, 0x0000);
        cmd_07_10(handle, 0x2C80);
        cmd_07_10(handle, 0x0290);
        cmd_07_10(handle, 0x4000);

        if (elf_upload_to_target(handle, &state->elf) != 0) {
            fprintf(stderr, "Flash: Failed to upload flashloader\n");
            return -1;
        }
        state->loaded = 1;
    }

    /* Write operation parameters to parameter block using cmd_07_19
     * Parameter block at 0x20000000:
     *   +0x00: operation (4 bytes)
     *   +0x04: flash_addr (4 bytes)
     *   +0x08: length (4 bytes)
     *   +0x0C: result (4 bytes)
     *   +0x10: status (4 bytes)
     *
     * Using cmd_07_19 which is proven to work (same as test_flash_program.c)
     */
    printf("Flash: Writing params via cmd_07_19: op=%u, addr=0x%08X, len=%u\n", operation, flash_addr, length);
    fflush(stdout);

    cmd_07_19(handle, FLASHLOADER_PARAM_OPERATION, operation);
    cmd_07_19(handle, FLASHLOADER_PARAM_FLASH_ADDR, flash_addr);
    cmd_07_19(handle, FLASHLOADER_PARAM_LENGTH, length);
    cmd_07_19(handle, FLASHLOADER_PARAM_RESULT, 0xFFFFFFFF);  /* Will be set by flashloader */
    cmd_07_19(handle, FLASHLOADER_PARAM_STATUS, 0x00000000);  /* Will be set by flashloader */

    usleep(10000);  /* Small delay to let it settle */

    /* Debug: Read back parameters to verify writes persisted */
    uint32_t verify_op, verify_addr, verify_len;
    cmd_071b_read_sram_longword(handle, FLASHLOADER_PARAM_OPERATION, &verify_op);
    cmd_071b_read_sram_longword(handle, FLASHLOADER_PARAM_FLASH_ADDR, &verify_addr);
    cmd_071b_read_sram_longword(handle, FLASHLOADER_PARAM_LENGTH, &verify_len);
    printf("Flash: Verify readback: op=%u, addr=0x%08X, len=%u\n", verify_op, verify_addr, verify_len);
    fflush(stdout);
    if (verify_op != operation || verify_addr != flash_addr || verify_len != length) {
        printf("Flash: WARNING - Parameters did NOT persist to SRAM!\n");
        fflush(stdout);
    }

    /* Set PC to flashloader entry point */
    cmd_07_14_write_bdm_reg(handle, 0x080F, state->elf.entry_point);

    /* Set SR to supervisor mode, interrupts disabled */
    cmd_07_14_write_bdm_reg(handle, 0x080E, 0x2700);

    /* BDM config (similar to CW flashloader) */
    cmd_07_11(handle, 0x2980, 0x00, 0x00, 0x08, 0x0F);  /* Read PC */
    cmd_07_11(handle, 0x2980, 0x00, 0x00, 0x00, 0x02);  /* Read D2 */

    /* Setup memory windows again before execution */
    cmd_07_10(handle, 0x2D80);
    cmd_07_10(handle, 0x0000);
    cmd_07_10(handle, 0x0000);
    cmd_07_10(handle, 0x2C80);
    cmd_07_10(handle, 0x0290);
    cmd_07_10(handle, 0x4000);

    /* Final BDM config and PC set (like CW) */
    cmd_07_11(handle, 0x2980, 0x00, 0x00, 0x08, 0x0F);  /* Read PC */
    cmd_07_14_write_bdm_reg(handle, 0x080F, state->elf.entry_point);  /* Set PC again */

    /* Execute (BDM GO) */
    r = cmd_07_02_bdm_go(handle);
    if (r != 0) {
        fprintf(stderr, "Flash: BDM GO failed\n");
        return -1;
    }

    /* Wait for flashloader to complete (halt instruction) */
    int timeout = 5;
    if (operation == FLASH_OP_MASS_ERASE) {
        timeout = 30;  /* Mass erase takes longer */
    } else if (operation == FLASH_OP_SECTOR_ERASE) {
        timeout = 10;
    }

    r = wait_for_flashloader_halt(handle, timeout);
    if (r != 0) {
        return -1;
    }

    /* Read result from parameter block */
    r = cmd_071b_read_sram_longword(handle, FLASHLOADER_PARAM_RESULT, result);
    if (r != 0) {
        fprintf(stderr, "Flash: Failed to read result\n");
        return -1;
    }

    /* Debug: Read back status */
    uint32_t dbg_status;
    cmd_071b_read_sram_longword(handle, FLASHLOADER_PARAM_STATUS, &dbg_status);
    printf("Flash: op=%u completed, result=0x%08X, CFMUSTAT=0x%02X\n",
           operation, *result, dbg_status);
    fflush(stdout);

    /* Re-enter debug mode for next operation */
    cmd_enter_mode(handle, 0xf8);
    usleep(50000);

    return 0;
}

int simple_flash_erase_sector(libusb_device_handle *handle, simple_flash_state_t *state,
                              uint32_t sector_addr) {
    uint32_t result;
    int r;

    printf("Flash: Erasing sector at 0x%08X\n", sector_addr);

    /* Initialize flash module if needed */
    if (!state->initialized) {
        r = simple_flash_run_op(handle, state, FLASH_OP_INIT, 0, 0, &result);
        if (r != 0 || result != FLASH_RESULT_SUCCESS) {
            fprintf(stderr, "Flash: Init failed (result=0x%08X)\n", result);
            return -1;
        }
        state->initialized = 1;
        printf("Flash: Module initialized\n");
    }

    /* Sector erase */
    r = simple_flash_run_op(handle, state, FLASH_OP_SECTOR_ERASE, sector_addr, 0, &result);
    if (r != 0) {
        return -1;
    }

    if (result != FLASH_RESULT_SUCCESS) {
        fprintf(stderr, "Flash: Sector erase failed (result=0x%08X)\n", result);
        return -1;
    }

    printf("Flash: Sector erased successfully\n");
    return 0;
}

int simple_flash_program(libusb_device_handle *handle, simple_flash_state_t *state,
                         uint32_t flash_addr, const uint8_t *data, uint32_t length) {
    uint32_t result;
    int r;

    if (length > FLASHLOADER_DATA_BUFFER_SIZE) {
        fprintf(stderr, "Flash: Data too large (%u > %u)\n", length, FLASHLOADER_DATA_BUFFER_SIZE);
        return -1;
    }

    printf("Flash: Programming %u bytes at 0x%08X\n", length, flash_addr);

    /* Initialize flash module if needed */
    if (!state->initialized) {
        r = simple_flash_run_op(handle, state, FLASH_OP_INIT, 0, 0, &result);
        if (r != 0 || result != FLASH_RESULT_SUCCESS) {
            fprintf(stderr, "Flash: Init failed (result=0x%08X)\n", result);
            return -1;
        }
        state->initialized = 1;
    }

    /* Upload flashloader if not done (run_op will do this, but we need it for buffer write) */
    if (!state->loaded) {
        /* Setup memory windows */
        cmd_07_10(handle, 0x2D80);
        cmd_07_10(handle, 0x0000);
        cmd_07_10(handle, 0x0000);
        cmd_07_10(handle, 0x2C80);
        cmd_07_10(handle, 0x0290);
        cmd_07_10(handle, 0x4000);

        if (elf_upload_to_target(handle, &state->elf) != 0) {
            return -1;
        }
        state->loaded = 1;
    }

    /* Write data to flashloader data buffer using cmd_07_19
     * FLASHLOADER_DATA_BUFFER = 0x20000100
     * Using cmd_07_19 which is proven to work.
     */
    uint32_t num_words = (length + 3) / 4;  /* Round up to whole words */

    for (uint32_t i = 0; i < num_words; i++) {
        uint32_t addr = FLASHLOADER_DATA_BUFFER + (i * 4);
        uint32_t offset = i * 4;
        uint32_t value;

        /* Build 32-bit big-endian value from data */
        if (offset + 4 <= length) {
            value = (data[offset] << 24) |
                    (data[offset + 1] << 16) |
                    (data[offset + 2] << 8) |
                    data[offset + 3];
        } else {
            /* Handle partial word at end - pad with 0xFF */
            value = 0xFFFFFFFF;
            for (uint32_t j = offset; j < length; j++) {
                uint8_t byte_val = data[j];
                value &= ~(0xFF << (24 - 8 * (j - offset)));
                value |= byte_val << (24 - 8 * (j - offset));
            }
        }

        r = cmd_07_19(handle, addr, value);
        if (r != 0) {
            fprintf(stderr, "Flash: Failed to write data at 0x%08X\n", addr);
            return -1;
        }
    }

    usleep(10000);  /* Small delay to let it settle */

    /* Run program operation */
    r = simple_flash_run_op(handle, state, FLASH_OP_PROGRAM, flash_addr, length, &result);
    if (r != 0) {
        return -1;
    }

    if (result != FLASH_RESULT_SUCCESS) {
        fprintf(stderr, "Flash: Program failed (result=0x%08X)\n", result);
        return -1;
    }

    return 0;
}

int simple_flash_mass_erase(libusb_device_handle *handle, simple_flash_state_t *state) {
    uint32_t result;
    int r;

    printf("Flash: Mass erasing entire flash...\n");

    /* Initialize flash module if needed */
    if (!state->initialized) {
        r = simple_flash_run_op(handle, state, FLASH_OP_INIT, 0, 0, &result);
        if (r != 0 || result != FLASH_RESULT_SUCCESS) {
            fprintf(stderr, "Flash: Init failed (result=0x%08X)\n", result);
            return -1;
        }
        state->initialized = 1;
        printf("Flash: Module initialized\n");
    }

    /* Mass erase */
    r = simple_flash_run_op(handle, state, FLASH_OP_MASS_ERASE, 0, 0, &result);
    if (r != 0) {
        return -1;
    }

    if (result != FLASH_RESULT_SUCCESS) {
        fprintf(stderr, "Flash: Mass erase failed (result=0x%08X)\n", result);
        return -1;
    }

    printf("Flash: Mass erase complete\n");
    return 0;
}

void simple_flash_cleanup(simple_flash_state_t *state) {
    if (state) {
        elf_free(&state->elf);
        state->loaded = 0;
        state->initialized = 0;
    }
}
