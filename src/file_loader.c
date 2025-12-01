/*
 * File Loader for OpenLink ColdFire
 *
 * Supports loading firmware files in various formats:
 * - .bin  - Raw binary (loaded at base address)
 * - .elf  - ELF executable (uses embedded load addresses)
 * - .s19, .srec - Motorola S-Record
 *
 * License: GPL v3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "file_loader.h"

/* ELF header structures (32-bit big-endian) */
#define ELF_MAGIC       0x7F454C46  /* "\x7FELF" */
#define ET_EXEC         2           /* Executable file */
#define EM_68K          4           /* Motorola 68K */
#define PT_LOAD         1           /* Loadable segment */

typedef struct {
    uint8_t  e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} __attribute__((packed)) Elf32_Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} __attribute__((packed)) Elf32_Phdr;

/* Big-endian conversion helpers */
static uint16_t be16(uint16_t val) {
    return ((val & 0xFF) << 8) | ((val >> 8) & 0xFF);
}

static uint32_t be32(uint32_t val) {
    return ((val & 0xFF) << 24) |
           ((val & 0xFF00) << 8) |
           ((val >> 8) & 0xFF00) |
           ((val >> 24) & 0xFF);
}

/* Get file extension (lowercase) */
static const char *get_extension(const char *filename) {
    const char *dot = strrchr(filename, '.');
    return dot ? dot + 1 : "";
}

file_format_t file_detect_format(const char *filename) {
    const char *ext = get_extension(filename);

    if (strcasecmp(ext, "bin") == 0) {
        return FILE_FORMAT_BIN;
    } else if (strcasecmp(ext, "elf") == 0) {
        return FILE_FORMAT_ELF;
    } else if (strcasecmp(ext, "s19") == 0 ||
               strcasecmp(ext, "srec") == 0 ||
               strcasecmp(ext, "s") == 0 ||
               strcasecmp(ext, "mot") == 0) {
        return FILE_FORMAT_SREC;
    }

    /* Try to detect by file content */
    FILE *f = fopen(filename, "rb");
    if (!f) {
        return FILE_FORMAT_UNKNOWN;
    }

    uint8_t magic[4];
    if (fread(magic, 1, 4, f) == 4) {
        /* Check ELF magic */
        if (magic[0] == 0x7F && magic[1] == 'E' &&
            magic[2] == 'L' && magic[3] == 'F') {
            fclose(f);
            return FILE_FORMAT_ELF;
        }
        /* Check S-Record (starts with 'S') */
        if (magic[0] == 'S' && isdigit(magic[1])) {
            fclose(f);
            return FILE_FORMAT_SREC;
        }
    }

    fclose(f);
    return FILE_FORMAT_UNKNOWN;
}

int file_load_bin(const char *filename, uint32_t base_addr, loaded_file_t *file_out) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return -1;
    }

    /* Get file size */
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0 || size > 0x100000) {  /* Max 1MB */
        fprintf(stderr, "Error: Invalid file size %ld\n", size);
        fclose(f);
        return -1;
    }

    /* Allocate segment */
    file_out->segments = malloc(sizeof(load_segment_t));
    if (!file_out->segments) {
        fprintf(stderr, "Error: Out of memory\n");
        fclose(f);
        return -1;
    }

    file_out->segments[0].data = malloc(size);
    if (!file_out->segments[0].data) {
        fprintf(stderr, "Error: Out of memory\n");
        free(file_out->segments);
        fclose(f);
        return -1;
    }

    /* Read file */
    if (fread(file_out->segments[0].data, 1, size, f) != (size_t)size) {
        fprintf(stderr, "Error: Failed to read file\n");
        free(file_out->segments[0].data);
        free(file_out->segments);
        fclose(f);
        return -1;
    }

    fclose(f);

    /* Fill in file info */
    file_out->format = FILE_FORMAT_BIN;
    file_out->num_segments = 1;
    file_out->segments[0].addr = base_addr;
    file_out->segments[0].size = size;
    file_out->entry_point = base_addr;
    file_out->total_size = size;
    file_out->min_addr = base_addr;
    file_out->max_addr = base_addr + size;

    return 0;
}

int file_load_elf(const char *filename, loaded_file_t *file_out) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return -1;
    }

    /* Read ELF header */
    Elf32_Ehdr ehdr;
    if (fread(&ehdr, 1, sizeof(ehdr), f) != sizeof(ehdr)) {
        fprintf(stderr, "Error: Failed to read ELF header\n");
        fclose(f);
        return -1;
    }

    /* Verify ELF magic */
    if (ehdr.e_ident[0] != 0x7F || ehdr.e_ident[1] != 'E' ||
        ehdr.e_ident[2] != 'L' || ehdr.e_ident[3] != 'F') {
        fprintf(stderr, "Error: Invalid ELF magic\n");
        fclose(f);
        return -1;
    }

    /* Check big-endian (M68K) */
    int is_be = (ehdr.e_ident[5] == 2);

    /* Convert header fields */
    uint16_t e_type = is_be ? be16(ehdr.e_type) : ehdr.e_type;
    uint16_t e_machine = is_be ? be16(ehdr.e_machine) : ehdr.e_machine;
    uint32_t e_entry = is_be ? be32(ehdr.e_entry) : ehdr.e_entry;
    uint32_t e_phoff = is_be ? be32(ehdr.e_phoff) : ehdr.e_phoff;
    uint16_t e_phentsize = is_be ? be16(ehdr.e_phentsize) : ehdr.e_phentsize;
    uint16_t e_phnum = is_be ? be16(ehdr.e_phnum) : ehdr.e_phnum;

    /* Verify executable and M68K */
    if (e_type != ET_EXEC) {
        fprintf(stderr, "Error: Not an executable ELF (type=%d)\n", e_type);
        fclose(f);
        return -1;
    }

    if (e_machine != EM_68K) {
        fprintf(stderr, "Warning: Not M68K architecture (machine=%d)\n", e_machine);
    }

    /* Count loadable segments */
    int num_load = 0;
    fseek(f, e_phoff, SEEK_SET);
    for (int i = 0; i < e_phnum; i++) {
        Elf32_Phdr phdr;
        if (fread(&phdr, 1, e_phentsize, f) != e_phentsize) {
            break;
        }
        uint32_t p_type = is_be ? be32(phdr.p_type) : phdr.p_type;
        uint32_t p_filesz = is_be ? be32(phdr.p_filesz) : phdr.p_filesz;
        if (p_type == PT_LOAD && p_filesz > 0) {
            num_load++;
        }
    }

    if (num_load == 0) {
        fprintf(stderr, "Error: No loadable segments\n");
        fclose(f);
        return -1;
    }

    /* Allocate segments */
    file_out->segments = calloc(num_load, sizeof(load_segment_t));
    if (!file_out->segments) {
        fprintf(stderr, "Error: Out of memory\n");
        fclose(f);
        return -1;
    }

    /* Load segments */
    file_out->format = FILE_FORMAT_ELF;
    file_out->num_segments = 0;
    file_out->entry_point = e_entry;
    file_out->total_size = 0;
    file_out->min_addr = 0xFFFFFFFF;
    file_out->max_addr = 0;

    fseek(f, e_phoff, SEEK_SET);
    for (int i = 0; i < e_phnum; i++) {
        Elf32_Phdr phdr;
        if (fread(&phdr, 1, e_phentsize, f) != e_phentsize) {
            break;
        }

        uint32_t p_type = is_be ? be32(phdr.p_type) : phdr.p_type;
        uint32_t p_offset = is_be ? be32(phdr.p_offset) : phdr.p_offset;
        uint32_t p_paddr = is_be ? be32(phdr.p_paddr) : phdr.p_paddr;
        uint32_t p_filesz = is_be ? be32(phdr.p_filesz) : phdr.p_filesz;

        if (p_type != PT_LOAD || p_filesz == 0) {
            continue;
        }

        load_segment_t *seg = &file_out->segments[file_out->num_segments];
        seg->addr = p_paddr;
        seg->size = p_filesz;
        seg->data = malloc(p_filesz);
        if (!seg->data) {
            fprintf(stderr, "Error: Out of memory\n");
            file_free(file_out);
            fclose(f);
            return -1;
        }

        /* Read segment data - save position to continue reading program headers */
        long phdr_pos = ftell(f);
        fseek(f, p_offset, SEEK_SET);
        if (fread(seg->data, 1, p_filesz, f) != p_filesz) {
            fprintf(stderr, "Error: Failed to read segment\n");
            file_free(file_out);
            fclose(f);
            return -1;
        }
        fseek(f, phdr_pos, SEEK_SET);

        file_out->num_segments++;
        file_out->total_size += p_filesz;

        if (p_paddr < file_out->min_addr) {
            file_out->min_addr = p_paddr;
        }
        if (p_paddr + p_filesz > file_out->max_addr) {
            file_out->max_addr = p_paddr + p_filesz;
        }
    }

    fclose(f);
    return 0;
}

/* Parse hex character */
static int hex_char(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

/* Parse hex byte from two characters */
static int hex_byte(const char *s) {
    int hi = hex_char(s[0]);
    int lo = hex_char(s[1]);
    if (hi < 0 || lo < 0) return -1;
    return (hi << 4) | lo;
}

int file_load_srec(const char *filename, loaded_file_t *file_out) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return -1;
    }

    /* First pass: count data records */
    char line[1024];
    int num_records = 0;
    uint32_t total_bytes = 0;
    uint32_t min_addr = 0xFFFFFFFF;
    uint32_t max_addr = 0;
    uint32_t entry_point = 0;

    while (fgets(line, sizeof(line), f)) {
        if (line[0] != 'S') continue;

        int type = line[1] - '0';
        if (type < 1 || type > 3) continue;  /* S1, S2, S3 are data records */

        int byte_count = hex_byte(&line[2]);
        if (byte_count < 0) continue;

        int addr_bytes = type + 1;  /* S1=2, S2=3, S3=4 */
        uint32_t addr = 0;
        for (int i = 0; i < addr_bytes; i++) {
            int b = hex_byte(&line[4 + i * 2]);
            if (b < 0) goto next_line;
            addr = (addr << 8) | b;
        }

        int data_bytes = byte_count - addr_bytes - 1;  /* -1 for checksum */
        if (data_bytes <= 0) continue;

        num_records++;
        total_bytes += data_bytes;

        if (addr < min_addr) min_addr = addr;
        if (addr + data_bytes > max_addr) max_addr = addr + data_bytes;

    next_line:
        continue;
    }

    /* Check for S7/S8/S9 entry point */
    fseek(f, 0, SEEK_SET);
    while (fgets(line, sizeof(line), f)) {
        if (line[0] != 'S') continue;

        int type = line[1] - '0';
        if (type < 7 || type > 9) continue;  /* S7, S8, S9 are entry points */

        int addr_bytes = 11 - type;  /* S9=2, S8=3, S7=4 */
        uint32_t addr = 0;
        for (int i = 0; i < addr_bytes; i++) {
            int b = hex_byte(&line[4 + i * 2]);
            if (b < 0) break;
            addr = (addr << 8) | b;
        }
        entry_point = addr;
        break;
    }

    if (num_records == 0) {
        fprintf(stderr, "Error: No data records in S-Record file\n");
        fclose(f);
        return -1;
    }

    /* Allocate one contiguous segment */
    file_out->segments = malloc(sizeof(load_segment_t));
    if (!file_out->segments) {
        fprintf(stderr, "Error: Out of memory\n");
        fclose(f);
        return -1;
    }

    uint32_t span = max_addr - min_addr;
    file_out->segments[0].data = malloc(span);
    if (!file_out->segments[0].data) {
        fprintf(stderr, "Error: Out of memory\n");
        free(file_out->segments);
        fclose(f);
        return -1;
    }

    /* Initialize with 0xFF (erased flash value) */
    memset(file_out->segments[0].data, 0xFF, span);

    /* Second pass: load data */
    fseek(f, 0, SEEK_SET);
    while (fgets(line, sizeof(line), f)) {
        if (line[0] != 'S') continue;

        int type = line[1] - '0';
        if (type < 1 || type > 3) continue;

        int byte_count = hex_byte(&line[2]);
        if (byte_count < 0) continue;

        int addr_bytes = type + 1;
        uint32_t addr = 0;
        for (int i = 0; i < addr_bytes; i++) {
            int b = hex_byte(&line[4 + i * 2]);
            if (b < 0) goto skip_line;
            addr = (addr << 8) | b;
        }

        int data_bytes = byte_count - addr_bytes - 1;
        if (data_bytes <= 0) continue;

        /* Read data bytes */
        int data_start = 4 + addr_bytes * 2;
        for (int i = 0; i < data_bytes; i++) {
            int b = hex_byte(&line[data_start + i * 2]);
            if (b < 0) break;
            file_out->segments[0].data[addr - min_addr + i] = b;
        }

    skip_line:
        continue;
    }

    fclose(f);

    /* Fill in file info */
    file_out->format = FILE_FORMAT_SREC;
    file_out->num_segments = 1;
    file_out->segments[0].addr = min_addr;
    file_out->segments[0].size = span;
    file_out->entry_point = entry_point ? entry_point : min_addr;
    file_out->total_size = span;
    file_out->min_addr = min_addr;
    file_out->max_addr = max_addr;

    return 0;
}

int file_load(const char *filename, uint32_t base_addr, loaded_file_t *file_out) {
    memset(file_out, 0, sizeof(*file_out));

    file_format_t format = file_detect_format(filename);

    switch (format) {
        case FILE_FORMAT_BIN:
            return file_load_bin(filename, base_addr, file_out);

        case FILE_FORMAT_ELF:
            return file_load_elf(filename, file_out);

        case FILE_FORMAT_SREC:
            return file_load_srec(filename, file_out);

        default:
            /* Try binary as fallback */
            fprintf(stderr, "Warning: Unknown format, assuming binary\n");
            return file_load_bin(filename, base_addr, file_out);
    }
}

void file_free(loaded_file_t *file) {
    if (!file) return;

    if (file->segments) {
        for (int i = 0; i < file->num_segments; i++) {
            if (file->segments[i].data) {
                free(file->segments[i].data);
            }
        }
        free(file->segments);
    }

    memset(file, 0, sizeof(*file));
}

void file_print_info(const loaded_file_t *file) {
    const char *format_name;
    switch (file->format) {
        case FILE_FORMAT_BIN:  format_name = "Binary"; break;
        case FILE_FORMAT_ELF:  format_name = "ELF"; break;
        case FILE_FORMAT_SREC: format_name = "S-Record"; break;
        default:               format_name = "Unknown"; break;
    }

    printf("File Format:   %s\n", format_name);
    printf("Segments:      %d\n", file->num_segments);
    printf("Total Size:    %u bytes (0x%X)\n", file->total_size, file->total_size);
    printf("Address Range: 0x%08X - 0x%08X\n", file->min_addr, file->max_addr);
    printf("Entry Point:   0x%08X\n", file->entry_point);

    for (int i = 0; i < file->num_segments; i++) {
        printf("  Segment %d:   0x%08X - 0x%08X (%u bytes)\n",
               i, file->segments[i].addr,
               file->segments[i].addr + file->segments[i].size,
               file->segments[i].size);
    }
}

int file_get_contiguous(const loaded_file_t *file, uint32_t *base_addr,
                        uint8_t **data_out, uint32_t *size_out) {
    if (!file || file->num_segments == 0) {
        return -1;
    }

    uint32_t span = file->max_addr - file->min_addr;
    uint8_t *data = malloc(span);
    if (!data) {
        fprintf(stderr, "Error: Out of memory\n");
        return -1;
    }

    /* Initialize with 0xFF (erased flash value) */
    memset(data, 0xFF, span);

    /* Copy all segments */
    for (int i = 0; i < file->num_segments; i++) {
        uint32_t offset = file->segments[i].addr - file->min_addr;
        memcpy(data + offset, file->segments[i].data, file->segments[i].size);
    }

    *base_addr = file->min_addr;
    *data_out = data;
    *size_out = span;

    return 0;
}
