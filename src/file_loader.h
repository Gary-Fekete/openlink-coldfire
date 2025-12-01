/*
 * File Loader for OpenLink ColdFire
 *
 * Supports loading firmware files in various formats:
 * - .bin  - Raw binary (loaded at base address, default 0x00000000)
 * - .elf  - ELF executable (uses embedded load addresses)
 * - .s19, .srec - Motorola S-Record
 *
 * License: GPL v3
 */

#ifndef FILE_LOADER_H
#define FILE_LOADER_H

#include <stdint.h>
#include <stddef.h>

/* File format types */
typedef enum {
    FILE_FORMAT_UNKNOWN,
    FILE_FORMAT_BIN,
    FILE_FORMAT_ELF,
    FILE_FORMAT_SREC
} file_format_t;

/* Memory segment for loaded data */
typedef struct {
    uint32_t addr;      /* Load address */
    uint8_t *data;      /* Segment data */
    uint32_t size;      /* Size in bytes */
} load_segment_t;

/* Loaded file information */
typedef struct {
    file_format_t format;       /* Detected file format */
    load_segment_t *segments;   /* Array of segments */
    int num_segments;           /* Number of segments */
    uint32_t entry_point;       /* Entry point (from ELF/S-Record, 0 for bin) */
    uint32_t total_size;        /* Total size of all segments */
    uint32_t min_addr;          /* Lowest load address */
    uint32_t max_addr;          /* Highest load address + size */
} loaded_file_t;

/*
 * Detect file format from filename extension
 *
 * @param filename  Path to file
 * @return          Detected format, or FILE_FORMAT_UNKNOWN
 */
file_format_t file_detect_format(const char *filename);

/*
 * Load a firmware file
 *
 * @param filename      Path to file
 * @param base_addr     Base address for binary files (ignored for ELF/S-Record)
 * @param file_out      Output: loaded file info (caller must call file_free())
 * @return              0 on success, -1 on error
 */
int file_load(const char *filename, uint32_t base_addr, loaded_file_t *file_out);

/*
 * Load a raw binary file
 *
 * @param filename      Path to file
 * @param base_addr     Load address
 * @param file_out      Output: loaded file info
 * @return              0 on success, -1 on error
 */
int file_load_bin(const char *filename, uint32_t base_addr, loaded_file_t *file_out);

/*
 * Load an ELF file
 *
 * @param filename      Path to file
 * @param file_out      Output: loaded file info
 * @return              0 on success, -1 on error
 */
int file_load_elf(const char *filename, loaded_file_t *file_out);

/*
 * Load a Motorola S-Record file
 *
 * @param filename      Path to file
 * @param file_out      Output: loaded file info
 * @return              0 on success, -1 on error
 */
int file_load_srec(const char *filename, loaded_file_t *file_out);

/*
 * Free loaded file resources
 *
 * @param file          Loaded file to free
 */
void file_free(loaded_file_t *file);

/*
 * Print loaded file information
 *
 * @param file          Loaded file info
 */
void file_print_info(const loaded_file_t *file);

/*
 * Get contiguous data from loaded file
 * Merges all segments into a single buffer, filling gaps with 0xFF
 *
 * @param file          Loaded file info
 * @param base_addr     Output: base address of merged data
 * @param data_out      Output: merged data (caller must free())
 * @param size_out      Output: size of merged data
 * @return              0 on success, -1 on error
 */
int file_get_contiguous(const loaded_file_t *file, uint32_t *base_addr,
                        uint8_t **data_out, uint32_t *size_out);

#endif /* FILE_LOADER_H */
