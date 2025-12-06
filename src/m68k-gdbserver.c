/**
 * m68k-gdbserver - GDB Remote Serial Protocol Server for ColdFire MCF52233
 *
 * This implements a GDB server that translates GDB RSP commands to BDM
 * operations via the USB Multilink debugger.
 *
 * Usage:
 *   ./m68k-gdbserver [-p port]
 *
 * Then connect with:
 *   m68k-elf-gdb -ex "set architecture m68k:521x" -ex "target remote :3333" program.elf
 *
 * GDB RSP Protocol Reference:
 *   https://sourceware.org/gdb/current/onlinedocs/gdb/Remote-Protocol.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <libusb-1.0/libusb.h>
#include "openlink_protocol.h"

#include "flash_gpl.h"
#include "file_loader.h"

/* Operation modes */
typedef enum {
    MODE_GDB,       /* GDB server mode (default) */
    MODE_ERASE,     /* Erase only */
    MODE_PROGRAM    /* Program file to flash */
} operation_mode_t;

#define DEFAULT_PORT 3333
#define MAX_PACKET_SIZE 4096

/* Common flash configuration */
#define FLASH_BASE           0x00000000
#define FLASH_SIZE           0x40000    /* 256KB */

/* Flash configuration - uses 2KB sectors */
#define SECTOR_SIZE          FLASH_SECTOR_SIZE  /* 2KB from flash_gpl.h */
#define MAX_SECTORS          FLASH_NUM_SECTORS  /* 128 sectors from flash_gpl.h */
#define GPL_DATA_BUFFER_SIZE FLASHLOADER_DATA_BUFFER_SIZE  /* 1KB from elf_loader.h */

#define USB_VENDOR_ID  0x1357
#define USB_PRODUCT_ID 0x0503

/* ColdFire register indices for GDB
 * GDB expects registers in this order for m68k:
 *   D0-D7, A0-A7, SR, PC
 * Total: 18 registers (17 general + PC)
 */
#define NUM_REGISTERS 18
#define REG_D0  0
#define REG_D7  7
#define REG_A0  8
#define REG_A7  15  /* Also SP */
#define REG_SR  16
#define REG_PC  17

/* BDM register window for CPU registers */
#define BDM_WINDOW_CPU 0x28800000

/* Global state */
static libusb_device_handle *g_usb_dev = NULL;
static int g_server_socket = -1;
static int g_client_socket = -1;
static volatile int g_running = 1;
static int g_target_halted = 1;
static int g_step_count = 0;  /* Track single-steps for BDM reset workaround */

/* Signal handler for clean shutdown */
static void signal_handler(int sig) {
    (void)sig;
    printf("\nShutting down...\n");
    g_running = 0;
}

/* Calculate GDB RSP checksum */
static uint8_t calc_checksum(const char *data, int len) {
    uint8_t sum = 0;
    for (int i = 0; i < len; i++) {
        sum += (uint8_t)data[i];
    }
    return sum;
}

/* Send a GDB RSP packet */
static int send_packet(int sock, const char *data) {
    char packet[MAX_PACKET_SIZE];
    int len = strlen(data);
    uint8_t checksum = calc_checksum(data, len);

    int pkt_len = snprintf(packet, sizeof(packet), "$%s#%02x", data, checksum);

    printf("TX: %s\n", packet);
    fflush(stdout);

    if (send(sock, packet, pkt_len, 0) != pkt_len) {
        perror("send");
        return -1;
    }
    return 0;
}

/* Send an empty/OK response */
static int send_ok(int sock) {
    return send_packet(sock, "OK");
}

/* Send an error response */
static int send_error(int sock, int errno_val) {
    char buf[16];
    snprintf(buf, sizeof(buf), "E%02x", errno_val & 0xff);
    return send_packet(sock, buf);
}

/* Convert hex char to nibble */
static int hex_to_nibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/* Convert hex string to bytes */
static int hex_to_bytes(const char *hex, uint8_t *bytes, int max_bytes) {
    int i = 0;
    while (hex[0] && hex[1] && i < max_bytes) {
        int hi = hex_to_nibble(hex[0]);
        int lo = hex_to_nibble(hex[1]);
        if (hi < 0 || lo < 0) break;
        bytes[i++] = (hi << 4) | lo;
        hex += 2;
    }
    return i;
}

/* Convert bytes to hex string */
static void bytes_to_hex(const uint8_t *bytes, int len, char *hex) {
    for (int i = 0; i < len; i++) {
        sprintf(hex + i * 2, "%02x", bytes[i]);
    }
    hex[len * 2] = '\0';
}

/* GDB-compatible xcrc32 for qCRC command (from libiberty/crc32.c)
 * Uses polynomial 0x04c11db7, MSB-first, no final XOR, init 0xFFFFFFFF
 * See: https://github.com/gcc-mirror/gcc/blob/master/libiberty/crc32.c
 */
static const uint32_t crc32_table[] = {
    0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
    0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
    0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
    0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
    0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
    0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
    0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
    0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
    0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
    0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
    0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
    0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
    0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
    0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
    0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
    0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
    0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
    0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
    0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
    0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
    0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
    0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
    0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
    0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
    0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
    0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
    0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
    0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
    0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
    0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
    0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
    0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
    0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
    0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
    0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
    0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
    0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
    0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
    0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
    0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
    0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
    0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
    0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
    0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
    0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
    0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
    0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
    0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
    0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
    0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
    0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
    0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
    0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
    0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
    0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
    0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
    0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
    0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
    0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
    0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
    0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
    0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
    0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
    0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

static uint32_t xcrc32(const uint8_t *buf, uint32_t len, uint32_t init) {
    uint32_t crc = init;
    while (len--) {
        crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ *buf) & 255];
        buf++;
    }
    return crc;
}

/* Un-escape RSP binary data in-place
 * GDB RSP binary protocol escapes special characters:
 *   $, #, }, * are sent as 0x7d followed by (char XOR 0x20)
 * Returns the new length of the data after un-escaping
 */
static size_t rsp_unescape_binary(uint8_t *data, size_t len) {
    size_t read_pos = 0;
    size_t write_pos = 0;

    while (read_pos < len) {
        if (data[read_pos] == 0x7d && read_pos + 1 < len) {
            /* Escape sequence: next byte XOR 0x20 */
            data[write_pos++] = data[read_pos + 1] ^ 0x20;
            read_pos += 2;
        } else {
            data[write_pos++] = data[read_pos++];
        }
    }

    return write_pos;
}

/* Cached register values - read from vector table at startup */
static uint32_t g_cached_sp = 0;
static uint32_t g_cached_pc = 0;
static int g_registers_initialized = 0;

/* Initialize register cache from flash vector table */
static void init_register_cache(void) {
    if (g_registers_initialized) return;

    uint8_t buffer[8];
    if (cmd_0717_read_memory(g_usb_dev, 0x00000000, 8, buffer, sizeof(buffer)) == 0) {
        g_cached_sp = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
        g_cached_pc = (buffer[4] << 24) | (buffer[5] << 16) | (buffer[6] << 8) | buffer[7];
        printf("Register cache initialized: SP=0x%08X, PC=0x%08X\n", g_cached_sp, g_cached_pc);
        g_registers_initialized = 1;
    }
}

/* Read a ColdFire CPU register via BDM
 * Uses the correct BDM read methods:
 * - PC: cmd_read_pc() (uses 07 11 with window 0x2980)
 * - SR: cmd_read_sr() (uses 07 11 with window 0x2980)
 * - D0-D7, A0-A7: cmd_07_13() (uses 07 13 with opcode 0x218x)
 */
static int read_cpu_register(int reg_num, uint32_t *value) {
    init_register_cache();

    /* For PC, use cmd_read_pc() which properly reads via 07 11
     * Note: cmd_07_13 with 0x298F returns stale data after a PC write!
     */
    if (reg_num == REG_PC) {
        int r = cmd_read_pc(g_usb_dev, value);
        if (r != 0) {
            *value = g_cached_pc;  /* Fall back to cached value on error */
        }
        return 0;
    }

    /* For SR, use cmd_read_sr() */
    if (reg_num == REG_SR) {
        int r = cmd_read_sr(g_usb_dev, value);
        if (r != 0) {
            *value = 0x2700;  /* Default: supervisor mode, interrupts disabled */
        }
        return 0;
    }

    /* For SP (A7), try BDM read first, fall back to cached */
    if (reg_num == REG_A7) {
        int r = cmd_07_13(g_usb_dev, 0x218F, value);
        if (r != 0 || *value == 0) {
            printf("A7 read: r=%d, value=0x%08X, using cached=0x%08X\n", r, *value, g_cached_sp);
            *value = g_cached_sp;  /* Fall back to cached value */
        }
        return 0;
    }

    /* Read D0-D7, A0-A6 via BDM using correct READ addresses
     * From MCF52235 Reference Manual:
     *   Read A/D register: 0x218{A/D,Reg[2:0]}
     *   - D0-D7: 0x2180-0x2187 (READ)
     *   - A0-A7: 0x2188-0x218F (READ)
     */
    uint16_t bdm_reg;
    if (reg_num >= REG_D0 && reg_num <= REG_D7) {
        bdm_reg = 0x2180 + reg_num;  /* D0=0x2180, D7=0x2187 */
    } else if (reg_num >= REG_A0 && reg_num <= REG_A7) {
        bdm_reg = 0x2188 + (reg_num - REG_A0);  /* A0=0x2188, A7=0x218F */
    } else {
        *value = 0;
        return 0;
    }

    int r = cmd_07_13(g_usb_dev, bdm_reg, value);
    if (r != 0) {
        *value = 0;  /* Return 0 on error */
    }
    return 0;
}

/* Write a ColdFire CPU register via BDM
 * Uses cmd_write_pc() for PC (includes sync), cmd_07_14_write_bdm_reg for others
 *
 * BDM Store (write) register addresses:
 *   D0-D7: 0x0180-0x0187
 *   A0-A6: 0x0188-0x018E
 *   A7:    0x018F (active stack pointer)
 *   PC:    0x080F (handled by cmd_write_pc)
 *   SR:    0x080E
 */
static int write_cpu_register(int reg_num, uint32_t value) {
    /* For PC, use cmd_write_pc() which includes proper sync command */
    if (reg_num == REG_PC) {
        return cmd_write_pc(g_usb_dev, value);
    }

    uint16_t bdm_reg;

    if (reg_num >= REG_D0 && reg_num <= REG_D7) {
        /* D0-D7: BDM store registers 0x0180-0x0187 */
        bdm_reg = 0x0180 + reg_num;
    } else if (reg_num >= REG_A0 && reg_num <= REG_A7) {
        /* A0-A7: BDM store registers 0x0188-0x018F */
        bdm_reg = 0x0188 + (reg_num - REG_A0);
    } else if (reg_num == REG_SR) {
        bdm_reg = 0x080E;  /* SR */
    } else {
        return -1;
    }

    return cmd_07_14_write_bdm_reg(g_usb_dev, bdm_reg, value);
}

/* Handle 'g' - read all registers */
static int handle_read_registers(int sock) {
    char response[NUM_REGISTERS * 8 + 1];  /* 8 hex chars per 32-bit reg */
    char *ptr = response;

    for (int i = 0; i < NUM_REGISTERS; i++) {
        uint32_t value = 0;
        if (read_cpu_register(i, &value) != 0) {
            value = 0xDEADBEEF;  /* Indicate read error */
        }
        /* GDB expects big-endian hex for m68k */
        sprintf(ptr, "%08x", value);
        ptr += 8;
    }

    return send_packet(sock, response);
}

/* Handle 'G' - write all registers */
static int handle_write_registers(int sock, const char *data) {
    /* Data is hex string of all register values */
    for (int i = 0; i < NUM_REGISTERS && data[0] && data[1]; i++) {
        uint32_t value = 0;
        for (int j = 0; j < 8 && data[j*2] && data[j*2+1]; j++) {
            int hi = hex_to_nibble(data[j*2]);
            int lo = hex_to_nibble(data[j*2+1]);
            if (hi >= 0 && lo >= 0) {
                value = (value << 8) | ((hi << 4) | lo);
            }
        }
        write_cpu_register(i, value);
        data += 8;
    }

    return send_ok(sock);
}

/* Handle 'p' - read single register */
static int handle_read_register(int sock, const char *data) {
    int reg_num = strtol(data, NULL, 16);
    uint32_t value = 0;

    if (reg_num >= NUM_REGISTERS) {
        /* For registers beyond our set (FP registers, etc.),
         * return zeros instead of error - they don't exist on this chip
         * but GDB may query them. Return appropriate size for FP regs (96 bits).
         */
        if (reg_num >= 18 && reg_num <= 25) {
            /* FP0-FP7: 96-bit extended precision (24 hex chars) */
            return send_packet(sock, "000000000000000000000000");
        } else if (reg_num >= 26 && reg_num <= 28) {
            /* FPCONTROL, FPSTATUS, FPIADDR: 32-bit */
            return send_packet(sock, "00000000");
        }
        /* Unknown register - empty response */
        return send_packet(sock, "");
    }

    if (read_cpu_register(reg_num, &value) != 0) {
        return send_error(sock, 1);
    }

    char response[16];
    sprintf(response, "%08x", value);
    return send_packet(sock, response);
}

/* Handle 'P' - write single register */
static int handle_write_register(int sock, const char *data) {
    char *eq = strchr(data, '=');
    if (!eq) {
        return send_error(sock, 1);
    }

    int reg_num = strtol(data, NULL, 16);
    uint32_t value = strtoul(eq + 1, NULL, 16);

    if (reg_num >= NUM_REGISTERS) {
        return send_error(sock, 0);
    }

    if (write_cpu_register(reg_num, value) != 0) {
        return send_error(sock, 1);
    }

    return send_ok(sock);
}

/* Handle 'm' - read memory */
static int handle_read_memory(int sock, const char *data) {
    char *comma = strchr(data, ',');
    if (!comma) {
        return send_error(sock, 1);
    }

    uint32_t addr = strtoul(data, NULL, 16);
    uint32_t len = strtoul(comma + 1, NULL, 16);

    if (len > MAX_PACKET_SIZE / 2) {
        len = MAX_PACKET_SIZE / 2;
    }

    uint8_t *buffer = malloc(len);
    if (!buffer) {
        return send_error(sock, 12);  /* ENOMEM */
    }

    int result = cmd_0717_read_memory(g_usb_dev, addr, len, buffer, len);
    if (result != 0) {
        free(buffer);
        return send_error(sock, 5);  /* EIO */
    }

    char *response = malloc(len * 2 + 1);
    if (!response) {
        free(buffer);
        return send_error(sock, 12);
    }

    bytes_to_hex(buffer, len, response);
    int ret = send_packet(sock, response);

    free(buffer);
    free(response);
    return ret;
}

/* Handle 'M' - write memory */
static int handle_write_memory(int sock, const char *data) {
    char *comma = strchr(data, ',');
    char *colon = strchr(data, ':');
    if (!comma || !colon) {
        return send_error(sock, 1);
    }

    uint32_t addr = strtoul(data, NULL, 16);
    uint32_t len = strtoul(comma + 1, NULL, 16);
    const char *hex_data = colon + 1;

    /* Write memory using cmd_07_19 (32-bit writes) */
    for (uint32_t i = 0; i < len; i += 4) {
        uint32_t value = 0;
        int bytes_to_write = (len - i >= 4) ? 4 : (len - i);

        for (int j = 0; j < bytes_to_write; j++) {
            int hi = hex_to_nibble(hex_data[(i + j) * 2]);
            int lo = hex_to_nibble(hex_data[(i + j) * 2 + 1]);
            if (hi >= 0 && lo >= 0) {
                value |= ((hi << 4) | lo) << (24 - j * 8);
            }
        }

        if (cmd_07_19(g_usb_dev, addr + i, value) != 0) {
            return send_error(sock, 5);
        }
    }

    return send_ok(sock);
}

/* CSR bit definitions for ColdFire V2 Debug Module */
#define CSR_SSM     (1 << 4)    /* Single Step Mode */
#define CSR_READ    0x2D80      /* Read CSR address */
#define CSR_WRITE   0x2C80      /* Write CSR address */

/* ColdFire V2 Debug Module Registers (DRc[4-0])
 * From MCF52235 Reference Manual Table 31-3
 * These are the 5-bit debug register codes (DRc)
 */
#define DEBUG_REG_CSR   0x00    /* Configuration/Status Register */
#define DEBUG_REG_BAAR  0x05    /* BDM Address Attribute Register */
#define DEBUG_REG_AATR  0x06    /* Address Attribute Trigger Register */
#define DEBUG_REG_TDR   0x07    /* Trigger Definition Register */
#define DEBUG_REG_PBR0  0x08    /* PC Breakpoint Register 0 */
#define DEBUG_REG_PBMR  0x09    /* PC Breakpoint Mask Register */
#define DEBUG_REG_ABHR  0x0C    /* Address Breakpoint High Register */
#define DEBUG_REG_ABLR  0x0D    /* Address Breakpoint Low Register */
#define DEBUG_REG_DBR   0x0E    /* Data Breakpoint Register */
#define DEBUG_REG_DBMR  0x0F    /* Data Breakpoint Mask Register */
#define DEBUG_REG_PBR1  0x18    /* PC Breakpoint Register 1 */
#define DEBUG_REG_PBR2  0x1A    /* PC Breakpoint Register 2 */
#define DEBUG_REG_PBR3  0x1B    /* PC Breakpoint Register 3 */

/* Legacy defines - kept for reference but NOT USED */
#define PBR0_READ   0x2D88      /* WRONG - do not use */
#define PBR0_WRITE  0x2C88      /* WRONG - do not use */
#define PBR1_READ   0x2D89
#define PBR1_WRITE  0x2C89
#define PBR2_READ   0x2D8A
#define PBR2_WRITE  0x2C8A
#define PBR3_READ   0x2D8B
#define PBR3_WRITE  0x2C8B
#define TDR_READ    0x2D87      /* WRONG - do not use */
#define TDR_WRITE   0x2C87      /* WRONG - do not use */
#define ABLR_READ   0x2D8C
#define ABLR_WRITE  0x2C8C
#define ABHR_READ   0x2D8D
#define ABHR_WRITE  0x2C8D
#define DBR_READ    0x2D8E
#define DBR_WRITE   0x2C8E
#define DBMR_READ   0x2D8F
#define DBMR_WRITE  0x2C8F

/* TDR bit definitions */
#define TDR_TRC_HALT    (1 << 30)  /* Halt on trigger */
#define TDR_LPC1_ANY    (3 << 28)  /* PC breakpoint 1 any */
#define TDR_LPC_MASK    0x0F000000 /* PC breakpoint enable mask */
#define TDR_EBL1        (1 << 13)  /* Enable breakpoint level 1 */
#define TDR_EDLW1       (1 << 12)  /* Enable data/longword level 1 */
#define TDR_EAI1        (1 << 11)  /* Enable address/instruction level 1 */
#define TDR_EAR1        (1 << 10)  /* Enable address range level 1 */
#define TDR_EPC1        (1 << 9)   /* Enable PC level 1 */

/* Address comparison mode bits in TDR */
#define TDR_EAL_MASK    (0x3 << 14) /* Address level comparison mode */
#define TDR_EAL_INSIDE  (0x1 << 14) /* Trigger when inside range */
#define TDR_EAL_OUTSIDE (0x2 << 14) /* Trigger when outside range */
#define TDR_EAL_EXACT   (0x0 << 14) /* Trigger on exact address match */

/* Data/RW mode bits */
#define TDR_DRW_READ    (1 << 21)  /* Match on read */
#define TDR_DRW_WRITE   (1 << 20)  /* Match on write */
#define TDR_DRW_RW      (3 << 20)  /* Match on read or write */

/* Breakpoint state tracking */
#define MAX_HW_BREAKPOINTS 4
static uint32_t hw_breakpoints[MAX_HW_BREAKPOINTS] = {0, 0, 0, 0};
static int hw_breakpoint_used[MAX_HW_BREAKPOINTS] = {0, 0, 0, 0};

/* Shadow copy of TDR (Trigger Definition Register)
 * TDR is WRITE-ONLY from the programming model, so we must track its state ourselves.
 * This shadow is updated whenever we modify TDR.
 */
static uint32_t tdr_shadow = 0;

/* Software breakpoint tracking */
#define MAX_SW_BREAKPOINTS 32
#define COLDFIRE_HALT_OPCODE 0x4AC8  /* HALT instruction */
static struct {
    uint32_t addr;
    uint16_t original_insn;
    int active;
} sw_breakpoints[MAX_SW_BREAKPOINTS];

/* Watchpoint (data breakpoint) tracking
 * ColdFire V2 supports ONE address range watchpoint via ABLR/ABHR/TDR
 * Types:
 *   2 = write watchpoint (break on write)
 *   3 = read watchpoint (break on read)
 *   4 = access watchpoint (break on read or write)
 */
#define MAX_WATCHPOINTS 1
typedef enum {
    WP_TYPE_NONE = 0,
    WP_TYPE_WRITE = 2,      /* GDB Z2 */
    WP_TYPE_READ = 3,       /* GDB Z3 */
    WP_TYPE_ACCESS = 4      /* GDB Z4 */
} watchpoint_type_t;

static struct {
    uint32_t addr;          /* Start address */
    uint32_t length;        /* Size of watched region */
    watchpoint_type_t type; /* Type of watchpoint */
    int active;             /* Is this slot in use? */
} watchpoints[MAX_WATCHPOINTS];

/* Flash programming state for vFlash* commands */
static struct {
    int initialized;           /* Flashloader initialized */
    int erased;               /* Sectors erased */
    uint32_t erase_start;     /* Start of erased region */
    uint32_t erase_end;       /* End of erased region */
    uint8_t *write_buffer;    /* Buffer for pending writes */
    uint32_t write_addr;      /* Start address of pending writes */
    uint32_t write_len;       /* Length of data in buffer */
    uint32_t write_capacity;  /* Capacity of write buffer */
    gpl_flash_state_t gpl_state;  /* GPL flashloader state */
} flash_state;

/* Read the Debug Module CSR */
static int read_csr(uint32_t *value) {
    return cmd_07_13(g_usb_dev, CSR_READ, value);
}

/* Write the Debug Module CSR */
static int write_csr(uint32_t value) {
    /* Use cmd_07_14_write_bdm_reg which uses the proper BDM register write protocol
     * with window base 0x28800000 (command 07 14), not memory write (command 07 16)
     */
    return cmd_07_14_write_bdm_reg(g_usb_dev, CSR_WRITE, value);
}

/* Wait for target to halt with timeout */
static int wait_for_halt(int timeout_ms) {
    int elapsed = 0;
    while (elapsed < timeout_ms) {
        uint8_t is_frozen = 0;
        if (cmd_bdm_freeze(g_usb_dev, &is_frozen) == 0 && is_frozen) {
            return 0; /* Halted */
        }
        usleep(1000); /* 1ms */
        elapsed++;
    }
    return -1; /* Timeout */
}

/*
 * Hardware Breakpoint Functions
 * ColdFire V2 Debug Module supports 4 PC breakpoints (PBR0-PBR3)
 */

/* PBR register offsets indexed by slot number
 * From MCF52235 RM Table 31-3: PBR0=0x08, PBR1=0x18, PBR2=0x1A, PBR3=0x1B
 */
static const uint16_t pbr_reg[4] = {
    DEBUG_REG_PBR0, DEBUG_REG_PBR1, DEBUG_REG_PBR2, DEBUG_REG_PBR3
};

/* Write to a PC Breakpoint Register
 * PBR0=0x08, PBR1=0x18, PBR2=0x1A, PBR3=0x1B (from Table 31-3)
 * Uses WDMREG command via cmd_07_14_write_debug_reg
 * NOTE: PBR registers are WRITE-ONLY - cannot read back to verify!
 */
static int write_pbr(int index, uint32_t addr) {
    if (index < 0 || index > 3) return -1;
    int r = cmd_07_14_write_debug_reg(g_usb_dev, pbr_reg[index], addr);
    if (r != 0) return r;
    /* Sync required after debug register writes */
    return cmd_07_12(g_usb_dev, 0xFFFF);
}

/* Read from a PC Breakpoint Register
 * Try using cmd_07_13 with combined window+register format
 */
/* NOTE: PBR registers are WRITE-ONLY from the programming model.
 * This function is kept for reference but will return stale/garbage values.
 * Do NOT rely on this for verification - use the hw_breakpoints[] shadow instead.
 */
static int read_pbr(int index, uint32_t *addr) {
    (void)index;
    (void)addr;
    /* PBR registers are write-only - cannot read back */
    return -1;
}

/* NOTE: TDR is WRITE-ONLY from the programming model.
 * This function is kept for reference but will return stale/garbage values.
 * Do NOT rely on this - use the tdr_shadow global instead.
 */
static int read_tdr(uint32_t *value) {
    (void)value;
    /* TDR is write-only - cannot read back */
    return -1;
}

/* Write the Trigger Definition Register
 * TDR is at DRc=0x07 in debug register space
 * Uses WDMREG command via cmd_07_14_write_debug_reg
 * NOTE: TDR is WRITE-ONLY - cannot read back to verify!
 */
static int write_tdr(uint32_t value) {
    int r = cmd_07_14_write_debug_reg(g_usb_dev, DEBUG_REG_TDR, value);
    if (r != 0) return r;
    /* Sync required after debug register writes */
    return cmd_07_12(g_usb_dev, 0xFFFF);
}

/* Set a hardware breakpoint at the given address
 * Returns: breakpoint index (0-3) on success, -1 on failure
 */
static int set_hw_breakpoint(uint32_t addr) {
    /* Find a free breakpoint slot */
    int slot = -1;
    for (int i = 0; i < MAX_HW_BREAKPOINTS; i++) {
        if (!hw_breakpoint_used[i]) {
            slot = i;
            break;
        }
    }

    if (slot < 0) {
        printf("No free hardware breakpoint slots\n");
        return -1;
    }

    /* Write the breakpoint address to PBRn
     * NOTE: PBR registers are write-only - cannot verify!
     */
    printf("DEBUG: Writing PBR%d = 0x%08X (DRc=0x%02X)\n", slot, addr, pbr_reg[slot]);
    if (write_pbr(slot, addr) != 0) {
        printf("Failed to write PBR%d\n", slot);
        return -1;
    }

    /* Build TDR value to enable PC breakpoint triggering:
     * - TRC_HALT: Halt processor on trigger (bit 30)
     * - EBL1: Enable breakpoint level 1 (bit 13)
     * - EPC1: Enable PC level 1 (bit 9)
     * - Set LPC bits for which PBRs are active (bits 24-27)
     *
     * NOTE: TDR is write-only - we use the global tdr_shadow to track state
     */
    tdr_shadow |= TDR_TRC_HALT | TDR_EBL1 | TDR_EPC1;
    /* Enable the specific PBR slot in bits 24-27 */
    tdr_shadow |= (1 << (24 + slot));

    printf("DEBUG: Writing TDR = 0x%08X (DRc=0x%02X)\n", tdr_shadow, DEBUG_REG_TDR);
    if (write_tdr(tdr_shadow) != 0) {
        printf("Failed to write TDR\n");
        return -1;
    }

    hw_breakpoints[slot] = addr;
    hw_breakpoint_used[slot] = 1;
    printf("Hardware breakpoint %d set at 0x%08X (TDR=0x%08X)\n", slot, addr, tdr_shadow);

    return slot;
}

/* Clear a hardware breakpoint at the given address
 * Returns: 0 on success, -1 on failure
 */
static int clear_hw_breakpoint(uint32_t addr) {
    /* Find the breakpoint slot for this address */
    int slot = -1;
    for (int i = 0; i < MAX_HW_BREAKPOINTS; i++) {
        if (hw_breakpoint_used[i] && hw_breakpoints[i] == addr) {
            slot = i;
            break;
        }
    }

    if (slot < 0) {
        printf("No hardware breakpoint at 0x%08X\n", addr);
        return -1;
    }

    /* Clear the PBR register */
    write_pbr(slot, 0);

    /* Update TDR shadow to disable this breakpoint slot
     * NOTE: TDR is write-only, so we use the shadow copy
     */
    tdr_shadow &= ~(1 << (24 + slot));

    /* If no more breakpoints active, disable PC breakpoint triggering */
    int any_active = 0;
    for (int i = 0; i < MAX_HW_BREAKPOINTS; i++) {
        if (i != slot && hw_breakpoint_used[i]) {
            any_active = 1;
            break;
        }
    }
    if (!any_active) {
        tdr_shadow &= ~(TDR_EBL1 | TDR_EPC1);
    }

    write_tdr(tdr_shadow);

    hw_breakpoints[slot] = 0;
    hw_breakpoint_used[slot] = 0;
    printf("Hardware breakpoint %d cleared (was at 0x%08X)\n", slot, addr);

    return 0;
}

/*
 * Software Breakpoint Functions
 * Insert HALT instruction (0x4AC8) at breakpoint address
 */

/* Initialize software breakpoint table */
static void init_sw_breakpoints(void) {
    for (int i = 0; i < MAX_SW_BREAKPOINTS; i++) {
        sw_breakpoints[i].addr = 0;
        sw_breakpoints[i].original_insn = 0;
        sw_breakpoints[i].active = 0;
    }
}

/* Set a software breakpoint at the given address
 * Returns: 0 on success, -1 on failure
 */
static int set_sw_breakpoint(uint32_t addr) {
    /* Find a free slot */
    int slot = -1;
    for (int i = 0; i < MAX_SW_BREAKPOINTS; i++) {
        if (!sw_breakpoints[i].active) {
            slot = i;
            break;
        }
    }

    if (slot < 0) {
        printf("No free software breakpoint slots\n");
        return -1;
    }

    /* Read original instruction */
    uint8_t insn_bytes[2];
    if (cmd_0717_read_memory(g_usb_dev, addr, 2, insn_bytes, 2) != 0) {
        printf("Failed to read instruction at 0x%08X\n", addr);
        return -1;
    }
    uint16_t original = (insn_bytes[0] << 8) | insn_bytes[1];

    /* Write HALT instruction (big-endian) */
    /* We need to write just the 2-byte HALT instruction */
    /* Using cmd_07_19 writes 4 bytes at a time, so we need a different approach */
    /* For now, write as part of a 4-byte word, preserving the next 2 bytes */
    uint8_t next_bytes[2];
    if (cmd_0717_read_memory(g_usb_dev, addr + 2, 2, next_bytes, 2) != 0) {
        next_bytes[0] = 0xFF;
        next_bytes[1] = 0xFF;
    }
    uint32_t write_val = (COLDFIRE_HALT_OPCODE << 16) | (next_bytes[0] << 8) | next_bytes[1];
    if (cmd_07_19(g_usb_dev, addr, write_val) != 0) {
        printf("Failed to write HALT instruction at 0x%08X\n", addr);
        return -1;
    }

    sw_breakpoints[slot].addr = addr;
    sw_breakpoints[slot].original_insn = original;
    sw_breakpoints[slot].active = 1;
    printf("Software breakpoint set at 0x%08X (original insn: 0x%04X)\n", addr, original);

    return 0;
}

/* Clear a software breakpoint at the given address
 * Returns: 0 on success, -1 on failure
 */
static int clear_sw_breakpoint(uint32_t addr) {
    /* Find the breakpoint */
    int slot = -1;
    for (int i = 0; i < MAX_SW_BREAKPOINTS; i++) {
        if (sw_breakpoints[i].active && sw_breakpoints[i].addr == addr) {
            slot = i;
            break;
        }
    }

    if (slot < 0) {
        printf("No software breakpoint at 0x%08X\n", addr);
        return -1;
    }

    /* Restore original instruction */
    uint16_t original = sw_breakpoints[slot].original_insn;
    uint8_t next_bytes[2];
    if (cmd_0717_read_memory(g_usb_dev, addr + 2, 2, next_bytes, 2) != 0) {
        next_bytes[0] = 0xFF;
        next_bytes[1] = 0xFF;
    }
    uint32_t write_val = (original << 16) | (next_bytes[0] << 8) | next_bytes[1];
    if (cmd_07_19(g_usb_dev, addr, write_val) != 0) {
        printf("Failed to restore instruction at 0x%08X\n", addr);
        return -1;
    }

    sw_breakpoints[slot].active = 0;
    printf("Software breakpoint cleared at 0x%08X (restored insn: 0x%04X)\n", addr, original);

    return 0;
}

/* Check if we stopped at a software breakpoint and adjust PC if needed */
static int check_sw_breakpoint_hit(void) {
    uint32_t pc;
    read_cpu_register(REG_PC, &pc);

    for (int i = 0; i < MAX_SW_BREAKPOINTS; i++) {
        if (sw_breakpoints[i].active && sw_breakpoints[i].addr == pc) {
            printf("Hit software breakpoint at 0x%08X\n", pc);
            return 1;
        }
    }
    return 0;
}

/*
 * Watchpoint (Data Breakpoint) Functions
 * ColdFire V2 Debug Module supports address range triggering via:
 *   - ABLR: Address Bound Low Register
 *   - ABHR: Address Bound High Register
 *   - TDR: Trigger Definition Register (controls R/W mode)
 *
 * Note: Only ONE watchpoint can be active at a time due to hardware limitations.
 */

/* Initialize watchpoint tracking */
static void init_watchpoints(void) {
    for (int i = 0; i < MAX_WATCHPOINTS; i++) {
        watchpoints[i].addr = 0;
        watchpoints[i].length = 0;
        watchpoints[i].type = WP_TYPE_NONE;
        watchpoints[i].active = 0;
    }
}

/* Write Address Bound Low Register */
static int write_ablr(uint32_t addr) {
    return cmd_07_14_write_bdm_reg(g_usb_dev, ABLR_WRITE, addr);
}

/* Write Address Bound High Register */
static int write_abhr(uint32_t addr) {
    return cmd_07_14_write_bdm_reg(g_usb_dev, ABHR_WRITE, addr);
}

/* Read Address Bound Low Register */
static int read_ablr(uint32_t *addr) {
    return cmd_07_13(g_usb_dev, ABLR_READ, addr);
}

/* Read Address Bound High Register */
static int read_abhr(uint32_t *addr) {
    return cmd_07_13(g_usb_dev, ABHR_READ, addr);
}

/* Set a watchpoint (data breakpoint) at the given address range
 * Parameters:
 *   addr   - Start address of memory region to watch
 *   length - Size of memory region (1, 2, 4 bytes typically)
 *   type   - Watchpoint type (WP_TYPE_WRITE, WP_TYPE_READ, WP_TYPE_ACCESS)
 * Returns: 0 on success, -1 on failure
 */
static int set_watchpoint(uint32_t addr, uint32_t length, watchpoint_type_t type) {
    /* Check if watchpoint slot is available */
    if (watchpoints[0].active) {
        printf("Watchpoint already active (only 1 supported)\n");
        return -1;
    }

    /* Calculate address range */
    uint32_t addr_low = addr;
    uint32_t addr_high = addr + length - 1;

    printf("Setting watchpoint: addr=0x%08X-0x%08X, type=%d\n",
           addr_low, addr_high, type);

    /* Program address bound registers */
    if (write_ablr(addr_low) != 0) {
        printf("Failed to write ABLR\n");
        return -1;
    }

    if (write_abhr(addr_high) != 0) {
        printf("Failed to write ABHR\n");
        return -1;
    }

    /* Configure TDR for address range watchpoint
     * NOTE: TDR is write-only, so we use the global shadow copy
     */

    /* Set trigger to halt on address range match */
    tdr_shadow |= TDR_TRC_HALT;    /* Halt processor on trigger */
    tdr_shadow |= TDR_EBL1;        /* Enable breakpoint level 1 */
    tdr_shadow |= TDR_EAR1;        /* Enable address range level 1 */
    tdr_shadow |= TDR_EAL_INSIDE;  /* Trigger when address is inside range */

    /* Set read/write mode based on watchpoint type */
    tdr_shadow &= ~TDR_DRW_RW;     /* Clear R/W bits first */
    switch (type) {
        case WP_TYPE_WRITE:
            tdr_shadow |= TDR_DRW_WRITE;
            break;
        case WP_TYPE_READ:
            tdr_shadow |= TDR_DRW_READ;
            break;
        case WP_TYPE_ACCESS:
            tdr_shadow |= TDR_DRW_RW;
            break;
        default:
            printf("Invalid watchpoint type\n");
            return -1;
    }

    if (write_tdr(tdr_shadow) != 0) {
        printf("Failed to write TDR\n");
        return -1;
    }

    /* Record watchpoint in tracking array */
    watchpoints[0].addr = addr;
    watchpoints[0].length = length;
    watchpoints[0].type = type;
    watchpoints[0].active = 1;

    printf("Watchpoint set: 0x%08X-0x%08X (type=%d, TDR=0x%08X)\n",
           addr_low, addr_high, type, tdr_shadow);

    return 0;
}

/* Clear a watchpoint at the given address
 * Returns: 0 on success, -1 on failure
 */
static int clear_watchpoint(uint32_t addr, uint32_t length, watchpoint_type_t type) {
    /* Find matching watchpoint */
    if (!watchpoints[0].active) {
        printf("No active watchpoint to clear\n");
        return -1;
    }

    if (watchpoints[0].addr != addr ||
        watchpoints[0].length != length ||
        watchpoints[0].type != type) {
        printf("Watchpoint mismatch: requested 0x%08X/%u/%d, active 0x%08X/%u/%d\n",
               addr, length, type,
               watchpoints[0].addr, watchpoints[0].length, watchpoints[0].type);
        return -1;
    }

    /* Clear address bound registers */
    write_ablr(0);
    write_abhr(0);

    /* Update TDR shadow to disable address range triggering
     * NOTE: TDR is write-only, so we use the global shadow copy
     */

    /* Clear address range and R/W bits */
    tdr_shadow &= ~TDR_EAR1;       /* Disable address range */
    tdr_shadow &= ~TDR_EAL_MASK;   /* Clear address level mode */
    tdr_shadow &= ~TDR_DRW_RW;     /* Clear R/W bits */

    /* If no more breakpoints/watchpoints, disable triggering entirely */
    int any_bp_active = 0;
    for (int i = 0; i < MAX_HW_BREAKPOINTS; i++) {
        if (hw_breakpoint_used[i]) {
            any_bp_active = 1;
            break;
        }
    }
    if (!any_bp_active) {
        tdr_shadow &= ~(TDR_TRC_HALT | TDR_EBL1);
    }

    write_tdr(tdr_shadow);

    /* Clear tracking */
    watchpoints[0].addr = 0;
    watchpoints[0].length = 0;
    watchpoints[0].type = WP_TYPE_NONE;
    watchpoints[0].active = 0;

    printf("Watchpoint cleared at 0x%08X\n", addr);
    return 0;
}

/* Check if we hit a watchpoint (returns the watched address, or 0 if no hit) */
static uint32_t check_watchpoint_hit(void) {
    if (!watchpoints[0].active) {
        return 0;
    }

    /* NOTE: TDR is write-only, so we can't read it to check trigger status.
     * Instead, we check our shadow copy to see if a watchpoint was armed.
     * If the target halted and we have an active watchpoint with EAR1 set
     * in our shadow, we assume the watchpoint triggered.
     *
     * In the future, we could potentially read CSR (which IS readable via BDM)
     * to get more precise trigger information.
     */
    if ((tdr_shadow & TDR_EAR1) && (tdr_shadow & TDR_TRC_HALT)) {
        printf("Watchpoint may have triggered at 0x%08X\n", watchpoints[0].addr);
        return watchpoints[0].addr;
    }

    return 0;
}

/*
 * Flash Programming Functions for vFlash* GDB commands
 */

/*
 * GPL Flashloader Implementation
 * Uses the ELF-based flashloader from flashloader/flashloader.elf
 */

/* Initialize flashloader (must be called before erase/program) */
static int flash_init_loader(void) {
    if (flash_state.initialized) {
        return 0;  /* Already initialized */
    }

    printf("Flash: Initializing GPL flashloader...\n");

    int r = gpl_flash_init(&flash_state.gpl_state, g_usb_dev, NULL);
    if (r != 0) {
        printf("Flash: GPL flashloader init failed\n");
        return -1;
    }

    flash_state.initialized = 1;
    printf("Flash: GPL flashloader initialized (entry=0x%08X)\n",
           gpl_flash_get_entry_point(&flash_state.gpl_state));
    return 0;
}

/* Erase flash sectors covering addr to addr+length */
static int flash_erase_region(uint32_t addr, uint32_t length) {
    if (addr + length > FLASH_SIZE) {
        printf("Flash: Erase region 0x%08X-0x%08X exceeds flash size\n",
               addr, addr + length);
        return -1;
    }

    /* Initialize flashloader if needed */
    int r = flash_init_loader();
    if (r != 0) return -1;

    /* Use GPL flashloader for erase */
    r = gpl_flash_erase_range(&flash_state.gpl_state, addr, length);
    if (r != 0) {
        printf("Flash: GPL erase failed\n");
        return -1;
    }

    /* Track erased region */
    if (!flash_state.erased || addr < flash_state.erase_start) {
        flash_state.erase_start = addr;
    }
    if (!flash_state.erased || addr + length > flash_state.erase_end) {
        flash_state.erase_end = addr + length;
    }
    flash_state.erased = 1;

    return 0;
}

/* Program data to flash */
static int flash_program_region(uint32_t addr, const uint8_t *data, uint32_t length) {
    if (addr + length > FLASH_SIZE) {
        printf("Flash: Program region 0x%08X-0x%08X exceeds flash size\n",
               addr, addr + length);
        return -1;
    }

    /* Initialize flashloader if needed */
    int r = flash_init_loader();
    if (r != 0) return -1;

    /* Use GPL flashloader for programming */
    r = gpl_flash_program(&flash_state.gpl_state, addr, data, length);
    if (r != 0) {
        printf("Flash: GPL program failed\n");
        return -1;
    }

    return 0;
}

/* Reset flash state after programming complete */
static void flash_reset_state(void) {
    if (flash_state.initialized) {
        gpl_flash_cleanup(&flash_state.gpl_state);
    }
    flash_state.initialized = 0;
    flash_state.erased = 0;
    flash_state.erase_start = 0;
    flash_state.erase_end = 0;
    if (flash_state.write_buffer) {
        free(flash_state.write_buffer);
        flash_state.write_buffer = NULL;
    }
    flash_state.write_addr = 0;
    flash_state.write_len = 0;
    flash_state.write_capacity = 0;
}


/* Handle 'c' - continue execution */
static int handle_continue(int sock, const char *data) {
    /* Optional: resume from address if specified */
    if (data && *data) {
        uint32_t addr = strtoul(data, NULL, 16);
        write_cpu_register(REG_PC, addr);
    }

    /* Debug: read PC before continue */
    uint32_t pc_before = 0;
    cmd_read_pc(g_usb_dev, &pc_before);
    printf("DEBUG continue: PC before GO = 0x%08X\n", pc_before);

    /* Enter BDM mode 0xF8 and send BDM GO to resume target */
    cmd_enter_mode(g_usb_dev, 0xF8);
    int go_result = cmd_07_02_bdm_go(g_usb_dev);  /* BDM GO - start execution from current PC */
    printf("DEBUG continue: BDM GO returned %d\n", go_result);
    g_target_halted = 0;

    /* Give target time to start executing before polling for halt */
    usleep(100000);  /* 100ms delay */

    /* Wait for target to halt (breakpoint, exception, or user interrupt)
     * Poll for halt status with timeout - target should halt on:
     * - Hardware breakpoint (TDR trigger)
     * - Software breakpoint (HALT instruction)
     * - Exception
     * - Manual halt (Ctrl-C)
     */
    int halted = 0;
    for (int i = 0; i < 5000; i++) {  /* 5 second timeout */
        usleep(1000);  /* 1ms between polls */

        uint8_t is_frozen = 0;
        int poll_result = cmd_bdm_freeze(g_usb_dev, &is_frozen);

        if (poll_result == 0 && is_frozen) {
            printf("Target halted after %d ms (freeze detected)\n", i);
            halted = 1;
            break;
        }

        /* Every 10ms, check CSR for BKPT bit (hardware breakpoint trigger).
         * cmd_bdm_freeze() doesn't detect hardware breakpoint halts reliably,
         * but CSR bit 24 (BKPT) is set when a hardware breakpoint triggers.
         */
        if ((i % 10) == 9) {
            cmd_enter_mode(g_usb_dev, 0xF8);
            uint32_t csr = 0;
            if (read_csr(&csr) == 0) {
                int bkpt_bit = (csr >> 24) & 1;
                if (bkpt_bit) {
                    printf("Target halted after %d ms (BKPT detected, CSR=0x%08X)\n", i, csr);
                    halted = 1;
                    break;
                }
            }
        }
    }

    if (!halted) {
        /* Timeout - force halt */
        printf("Continue timeout, forcing halt\n");
        cmd_bdm_halt(g_usb_dev);

        /* Wait for target to actually halt */
        usleep(10000);  /* 10ms delay */

        /* Re-enter BDM mode after halt to enable register access */
        cmd_enter_mode(g_usb_dev, 0xF8);

        /* Verify target is now halted */
        uint8_t is_frozen = 0;
        for (int i = 0; i < 10; i++) {
            cmd_bdm_freeze(g_usb_dev, &is_frozen);
            if (is_frozen) break;
            usleep(1000);
        }
        printf("DEBUG: After halt, is_frozen=%d\n", is_frozen);

        /* Read CSR to see target state */
        uint32_t csr = 0;
        read_csr(&csr);
        printf("DEBUG: CSR after halt = 0x%08X (HALT=%d, BKPT=%d)\n",
               csr, (csr >> 25) & 1, (csr >> 24) & 1);

        /* Debug: read PC after halt */
        uint32_t pc_after = 0;
        cmd_read_pc(g_usb_dev, &pc_after);
        printf("DEBUG: PC after halt = 0x%08X\n", pc_after);
    }

    g_target_halted = 1;

    /* Check if we hit a watchpoint - report with watch reason */
    uint32_t wp_addr = check_watchpoint_hit();
    if (wp_addr != 0) {
        char response[64];
        /* GDB expects: T05watch:ADDR; for watchpoint hits */
        snprintf(response, sizeof(response), "T05watch:%x;", wp_addr);
        printf("Watchpoint hit at 0x%08X\n", wp_addr);
        return send_packet(sock, response);
    }

    /* Check if we hit a software breakpoint */
    if (check_sw_breakpoint_hit()) {
        /* PC is at the HALT instruction - report breakpoint */
    }

    /* Report stop reason: SIGTRAP (breakpoint/trace trap) */
    return send_packet(sock, "S05");
}

/*
 * Reset BDM single-step capability using mode transition sequence.
 *
 * The Multilink USB-ML-12 firmware has an internal counter that limits single-step
 * operations to 2 before getting stuck. The sequence F8->F0->F8 resets this
 * counter. This is a workaround discovered through reverse engineering.
 *
 * The mode transitions don't affect CPU state (registers/memory) but do
 * affect the BDM controller state. PC is preserved by reading it before
 * and restoring it after the reset sequence.
 */
static void reset_single_step_capability(void) {
    uint32_t saved_pc;

    /* Save current PC (mode transitions may affect it) */
    read_cpu_register(REG_PC, &saved_pc);

    /* Apply the magic reset sequence: F8 -> F0 -> F8 */
    cmd_enter_mode(g_usb_dev, 0xF8);
    cmd_enter_mode(g_usb_dev, 0xF0);
    cmd_enter_mode(g_usb_dev, 0xF8);

    /* Restore PC */
    write_cpu_register(REG_PC, saved_pc);

    /* Reset step counter */
    g_step_count = 0;

    printf("BDM single-step capability reset (PC=0x%08X)\n", saved_pc);
}

/* Handle 's' - single step */
static int handle_step(int sock, const char *data) {
    /* Optional: step from address if specified */
    if (data && *data) {
        uint32_t addr = strtoul(data, NULL, 16);
        write_cpu_register(REG_PC, addr);
    }

    /*
     * BDM single-step workaround: The Multilink USB-ML-12 firmware has a 2-step
     * limit before getting stuck. Reset the BDM state every 2 steps.
     */
    if (g_step_count >= 2) {
        reset_single_step_capability();
    }

    printf("Single stepping... (step %d)\n", g_step_count + 1);

    /* Read PC before step */
    uint32_t pc_before = 0;
    read_cpu_register(REG_PC, &pc_before);
    printf("PC before step: 0x%08X\n", pc_before);

    /* ColdFire single step sequence:
     * 1. Read current CSR
     * 2. Set SSM (Single Step Mode) bit
     * 3. Write CSR
     * 4. Execute GO command
     * 5. Wait for target to halt
     * 6. Clear SSM bit
     */

    /* Step 1: Read current CSR */
    uint32_t csr = 0;
    if (read_csr(&csr) != 0) {
        printf("Failed to read CSR\n");
        return send_packet(sock, "S05"); /* Report halt anyway */
    }
    printf("CSR before step: 0x%08X\n", csr);

    /* Step 2-3: Set SSM bit and write */
    csr |= CSR_SSM;
    if (write_csr(csr) != 0) {
        printf("Failed to write CSR with SSM\n");
        return send_packet(sock, "S05");
    }

    /* Step 4: Execute GO */
    cmd_07_02_bdm_go(g_usb_dev);

    /* Step 5: Wait for auto-halt (single-step mode should halt after one instruction)
     * The ColdFire should automatically halt when SSM is set, but we need to
     * poll to detect when it happens
     */
    int halted = 0;
    for (int i = 0; i < 100; i++) {  /* Up to 100ms timeout */
        usleep(1000);  /* 1ms between checks */
        uint8_t is_frozen = 0;
        cmd_bdm_freeze(g_usb_dev, &is_frozen);
        if (is_frozen) {
            printf("Target halted after %d ms\n", i + 1);
            halted = 1;
            break;
        }
    }

    /* Force halt if auto-halt didn't work */
    if (!halted) {
        printf("Auto-halt timeout, forcing halt\n");
        cmd_bdm_halt(g_usb_dev);
    }

    /* Step 6: Clear SSM bit */
    if (read_csr(&csr) == 0) {
        csr &= ~CSR_SSM;
        write_csr(csr);
        printf("CSR after step: 0x%08X\n", csr);
    }

    /* Read PC after step and report */
    uint32_t pc_after = 0;
    read_cpu_register(REG_PC, &pc_after);
    printf("PC after step: 0x%08X\n", pc_after);

    if (pc_after == pc_before) {
        printf("WARNING: PC did not advance! Instruction at PC may be invalid.\n");
        /* Read instruction at PC for debugging */
        uint32_t opcode = 0;
        cmd_read_memory_long_addr(g_usb_dev, pc_before, &opcode);
        printf("Opcode at PC: 0x%08X\n", opcode);
        if (opcode == 0xFFFFFFFF) {
            printf("  -> Erased flash (no valid code)\n");
        } else if (opcode == 0x00000000) {
            printf("  -> Zero/uninitialized memory\n");
        }
    } else {
        printf("PC advanced by %d bytes\n", (int)(pc_after - pc_before));
    }

    /* Increment step counter for BDM reset workaround */
    g_step_count++;

    g_target_halted = 1;
    return send_packet(sock, "S05"); /* SIGTRAP - stepped */
}

/* Handle '?' - query halt reason */
static int handle_halt_reason(int sock) {
    /* Report SIGTRAP (05) - indicates breakpoint or single-step */
    return send_packet(sock, "S05");
}

/* Handle 'H' - set thread (we only have one thread) */
static int handle_set_thread(int sock, const char *data) {
    (void)data;
    return send_ok(sock);
}

/* Handle 'q' - general query */
static int handle_query(int sock, const char *data) {
    if (strncmp(data, "Supported", 9) == 0) {
        /* Report supported features */
        /* Report supported features including flash programming and memory map */
        return send_packet(sock, "PacketSize=1000;qXfer:features:read+;qXfer:memory-map:read+;vFlash+");
    }
    else if (strncmp(data, "Attached", 8) == 0) {
        /* We're always attached to an existing process */
        return send_packet(sock, "1");
    }
    else if (strncmp(data, "CRC:", 4) == 0) {
        /* CRC32 of memory region for compare-sections
         * Format: qCRC:addr,length
         * Response: C<crc32>
         */
        const char *params = data + 4;
        char *comma = strchr(params, ',');
        if (!comma) {
            return send_error(sock, 1);
        }

        uint32_t addr = strtoul(params, NULL, 16);
        uint32_t length = strtoul(comma + 1, NULL, 16);

        printf("qCRC: addr=0x%08X, length=0x%08X\n", addr, length);

        /* Allocate buffer for memory read */
        uint8_t *buffer = malloc(length);
        if (!buffer) {
            fprintf(stderr, "qCRC: Failed to allocate %u bytes\n", length);
            return send_error(sock, 2);
        }

        /* Read memory from target in chunks
         * cmd_0717 uses 6-byte-per-4-byte format (1.5x expansion)
         * with 256-byte USB buffer, max actual data is ~160 bytes
         * Use 128 bytes to be safe
         */
        #define MAX_READ_CHUNK 128
        uint32_t offset = 0;
        int chunk_num = 0;
        while (offset < length) {
            uint32_t chunk = length - offset;
            if (chunk > MAX_READ_CHUNK) {
                chunk = MAX_READ_CHUNK;
            }
            int r = cmd_0717_read_memory(g_usb_dev, addr + offset, chunk, buffer + offset, chunk);
            if (r != 0) {
                fprintf(stderr, "qCRC: Failed to read memory at 0x%08X\n", addr + offset);
                free(buffer);
                return send_error(sock, 3);
            }
            /* Debug: show first 16 bytes of each chunk for first few chunks */
            if (chunk_num < 4 && addr == 0) {
                printf("qCRC: Chunk %d @ 0x%08X: ", chunk_num, addr + offset);
                for (int i = 0; i < 16 && i < (int)chunk; i++) {
                    printf("%02X ", buffer[offset + i]);
                }
                printf("\n");
            }
            offset += chunk;
            chunk_num++;
        }

        /* Debug: show first 32 bytes of read data */
        printf("qCRC: First 32 bytes read from target:\n");
        printf("  ");
        for (int i = 0; i < 32 && i < (int)length; i++) {
            printf("%02X ", buffer[i]);
            if ((i + 1) % 16 == 0) printf("\n  ");
        }
        printf("\n");

        /* Debug: save full buffer to file for analysis */
        if (addr == 0 && length == 0x400) {
            FILE *f = fopen("/tmp/qcrc_buffer.bin", "wb");
            if (f) {
                fwrite(buffer, 1, length, f);
                fclose(f);
                printf("qCRC: Saved %u bytes to /tmp/qcrc_buffer.bin\n", length);
            }
        }

        /* Calculate CRC32 using GDB-compatible xcrc32 (init=0xFFFFFFFF) */
        uint32_t crc = xcrc32(buffer, length, 0xFFFFFFFF);
        free(buffer);

        /* Send response: C<crc32> */
        char response[32];
        snprintf(response, sizeof(response), "C%x", crc);
        printf("qCRC: CRC32=0x%08X\n", crc);
        return send_packet(sock, response);
    }
    else if (strncmp(data, "C", 1) == 0) {
        /* Current thread ID - we only have thread 1 */
        return send_packet(sock, "QC1");
    }
    else if (strncmp(data, "fThreadInfo", 11) == 0) {
        /* First thread info query */
        return send_packet(sock, "m1");
    }
    else if (strncmp(data, "sThreadInfo", 11) == 0) {
        /* Subsequent thread info - end of list */
        return send_packet(sock, "l");
    }
    else if (strncmp(data, "Xfer:features:read:target.xml", 29) == 0) {
        /* Target description - ColdFire */
        const char *xml =
            "l<?xml version=\"1.0\"?>"
            "<!DOCTYPE target SYSTEM \"gdb-target.dtd\">"
            "<target version=\"1.0\">"
            "<architecture>m68k:521x</architecture>"
            "</target>";
        return send_packet(sock, xml);
    }
    else if (strncmp(data, "Xfer:memory-map:read:", 21) == 0) {
        /* Memory map - defines flash and RAM regions */
        /* MCF52233: 256KB flash at 0x00000000, 32KB SRAM at 0x20000000 */
        /* IPSBAR: peripheral registers at 0x40000000 (2MB) */
        /* Flash has 8KB hardware sectors, but we chunk erase/program in 0x800 (2KB) blocks */
        const char *xml =
            "l<?xml version=\"1.0\"?>"
            "<!DOCTYPE memory-map PUBLIC \"+//IDN gnu.org//DTD GDB Memory Map V1.0//EN\" "
            "\"http://sourceware.org/gdb/gdb-memory-map.dtd\">"
            "<memory-map>"
            "<memory type=\"flash\" start=\"0x00000000\" length=\"0x40000\">"
            "<property name=\"blocksize\">0x800</property>"
            "</memory>"
            "<memory type=\"ram\" start=\"0x20000000\" length=\"0x8000\"/>"
            "<memory type=\"ram\" start=\"0x40000000\" length=\"0x200000\"/>"
            "</memory-map>";
        return send_packet(sock, xml);
    }
    else if (strncmp(data, "Rcmd,", 5) == 0) {
        /* Monitor command: qRcmd,<hex-encoded-command>
         * Decode the hex string and execute the command
         */
        const char *hex = data + 5;
        char cmd_buf[256];
        int cmd_len = 0;

        /* Decode hex string to ASCII */
        while (*hex && *(hex+1) && cmd_len < (int)sizeof(cmd_buf) - 1) {
            int hi = hex_to_nibble(*hex);
            int lo = hex_to_nibble(*(hex+1));
            if (hi < 0 || lo < 0) break;
            cmd_buf[cmd_len++] = (hi << 4) | lo;
            hex += 2;
        }
        cmd_buf[cmd_len] = '\0';

        printf("Monitor command: '%s'\n", cmd_buf);

        if (strcmp(cmd_buf, "reset") == 0 || strcmp(cmd_buf, "reset halt") == 0) {
            /* Reset target - set PC and SP from vector table */
            printf("Monitor reset...\n");
            fflush(stdout);

            /* Try to read reset vectors from flash */
            uint32_t reset_sp = 0, reset_pc = 0;
            cmd_read_memory_long_addr(g_usb_dev, 0x0, &reset_sp);
            cmd_read_memory_long_addr(g_usb_dev, 0x4, &reset_pc);
            printf("Vectors: SP=0x%08X, PC=0x%08X\n", reset_sp, reset_pc);

            /* Use cached values or defaults if flash read fails */
            if (reset_pc == 0xFFFFFFFF || reset_pc == 0) {
                reset_pc = g_cached_pc ? g_cached_pc : 0x400;
                printf("Using fallback PC=0x%08X\n", reset_pc);
            }
            if (reset_sp == 0xFFFFFFFF || reset_sp == 0) {
                reset_sp = g_cached_sp ? g_cached_sp : 0x20008000;
                printf("Using fallback SP=0x%08X\n", reset_sp);
            }
            fflush(stdout);

            /* Set SP (A7) - BDM store register is 0x018F for active A7 */
            /* We're in supervisor mode, so A7 = SSP */
            cmd_07_14_write_bdm_reg(g_usb_dev, 0x018F, reset_sp);

            /* Set PC last (includes sync) */
            cmd_write_pc(g_usb_dev, reset_pc);

            g_target_halted = 1;
            printf("Reset complete: PC=0x%08X, SP=0x%08X\n", reset_pc, reset_sp);
            fflush(stdout);

            /* Send "Reset OK\n" as hex */
            return send_packet(sock, "5265736574204f4b0a");
        }
        else if (strcmp(cmd_buf, "halt") == 0) {
            /* Halt target */
            printf("Halting target...\n");
            cmd_bdm_halt(g_usb_dev);
            g_target_halted = 1;
            return send_packet(sock, "4f4b0a");  /* "OK\n" */
        }
        else if (strcmp(cmd_buf, "go") == 0) {
            /* Resume target execution */
            printf("Resuming target...\n");
            cmd_enter_mode(g_usb_dev, 0xF8);
            cmd_07_02_bdm_go(g_usb_dev);
            g_target_halted = 0;
            return send_packet(sock, "4f4b0a");  /* "OK\n" */
        }
        else {
            /* Unknown command */
            printf("Unknown monitor command: %s\n", cmd_buf);
            return send_packet(sock, "556e6b6e6f776e20636f6d6d616e640a");  /* "Unknown command\n" */
        }
    }

    /* Unknown query - empty response */
    return send_packet(sock, "");
}

/* Handle 'v' - extended commands */
static int handle_v_command(int sock, const char *data, int data_len) {
    if (strncmp(data, "MustReplyEmpty", 14) == 0) {
        return send_packet(sock, "");
    }
    else if (strncmp(data, "Cont?", 5) == 0) {
        /* Report supported vCont actions */
        return send_packet(sock, "vCont;c;s");
    }
    else if (strncmp(data, "Cont;c", 6) == 0) {
        return handle_continue(sock, NULL);
    }
    else if (strncmp(data, "Cont;s", 6) == 0) {
        return handle_step(sock, NULL);
    }
    else if (strncmp(data, "FlashErase:", 11) == 0) {
        /* Flash erase: vFlashErase:addr,length
         * Format: vFlashErase:XXXXXXXX,YYYYYYYY
         */
        const char *params = data + 11;
        char *comma = strchr(params, ',');
        if (!comma) {
            return send_error(sock, 1);
        }

        uint32_t addr = strtoul(params, NULL, 16);
        uint32_t length = strtoul(comma + 1, NULL, 16);

        printf("vFlashErase: addr=0x%08X, length=0x%08X\n", addr, length);

        /* Validate address range */
        if (addr >= FLASH_SIZE || addr + length > FLASH_SIZE) {
            printf("vFlashErase: Invalid range\n");
            return send_error(sock, 0x0E);
        }

        /* Perform the erase */
        if (flash_erase_region(addr, length) != 0) {
            printf("vFlashErase: Erase failed\n");
            return send_error(sock, 0x10);
        }

        return send_ok(sock);
    }
    else if (strncmp(data, "FlashWrite:", 11) == 0) {
        /* Flash write: vFlashWrite:addr:XX...
         * Format: vFlashWrite:XXXXXXXX:binary_data
         * Note: Data is binary, not hex encoded!
         * Binary data uses RSP escaping: 0x7d followed by (byte XOR 0x20)
         */
        const char *params = data + 11;
        char *colon = strchr(params, ':');
        if (!colon) {
            return send_error(sock, 1);
        }

        uint32_t addr = strtoul(params, NULL, 16);
        const uint8_t *bin_data = (const uint8_t *)(colon + 1);

        /* Calculate binary data length from the packet length.
         * data starts after "FlashWrite:" (11 chars from start of 'v' command data)
         * The binary data starts after the second colon
         * Binary length = total_data_len - (address_part + 2 colons)
         *               = data_len - (colon - data) - 1
         */
        size_t bin_len = data_len - (colon - data) - 1;

        if (bin_len == 0) {
            return send_ok(sock);  /* Empty write is OK */
        }

        /* Allocate temp buffer for un-escaping RSP binary data */
        uint8_t *unescaped = malloc(bin_len);
        if (!unescaped) {
            printf("vFlashWrite: Out of memory for unescape buffer\n");
            return send_error(sock, 0x0C);
        }
        memcpy(unescaped, bin_data, bin_len);
        size_t actual_len = rsp_unescape_binary(unescaped, bin_len);

        printf("vFlashWrite: addr=0x%08X, escaped_len=%zu, actual_len=%zu bytes\n",
               addr, bin_len, actual_len);

        /* Validate address range */
        if (addr >= FLASH_SIZE || addr + actual_len > FLASH_SIZE) {
            printf("vFlashWrite: Invalid range\n");
            free(unescaped);
            return send_error(sock, 0x0E);
        }

        /* Buffer the write data - we'll program on FlashDone */
        if (!flash_state.write_buffer) {
            flash_state.write_capacity = FLASH_SIZE;  /* Max possible */
            flash_state.write_buffer = malloc(flash_state.write_capacity);
            if (!flash_state.write_buffer) {
                printf("vFlashWrite: Out of memory\n");
                free(unescaped);
                return send_error(sock, 0x0C);
            }
            memset(flash_state.write_buffer, 0xFF, flash_state.write_capacity);
            flash_state.write_addr = addr;
            flash_state.write_len = 0;
        }

        /* Copy unescaped data to buffer at appropriate offset */
        uint32_t offset = addr - flash_state.write_addr;
        if (offset + actual_len > flash_state.write_capacity) {
            printf("vFlashWrite: Buffer overflow\n");
            free(unescaped);
            return send_error(sock, 0x0E);
        }

        memcpy(flash_state.write_buffer + offset, unescaped, actual_len);
        if (offset + actual_len > flash_state.write_len) {
            flash_state.write_len = offset + actual_len;
        }

        free(unescaped);
        return send_ok(sock);
    }
    else if (strncmp(data, "FlashDone", 9) == 0) {
        /* Flash done - commit all buffered writes */
        printf("vFlashDone: Committing flash writes\n");

        if (flash_state.write_buffer && flash_state.write_len > 0) {
            /* Program the buffered data */
            if (flash_program_region(flash_state.write_addr,
                                    flash_state.write_buffer,
                                    flash_state.write_len) != 0) {
                printf("vFlashDone: Programming failed\n");
                flash_reset_state();
                return send_error(sock, 0x10);
            }
        }

        /* Re-init target after flash programming */
        flash_reset_state();

        /* Reinitialize target for debugging */
        uint32_t flash_size = 0;
        target_init_full(g_usb_dev, &flash_size);

        printf("vFlashDone: Complete, target reinitialized\n");
        return send_ok(sock);
    }

    return send_packet(sock, "");
}

/* Handle 'Z' - set breakpoint
 * Format: Ztype,addr,kind
 * Types:
 *   0 = software breakpoint (memory breakpoint)
 *   1 = hardware breakpoint (execution)
 *   2 = write watchpoint
 *   3 = read watchpoint
 *   4 = access watchpoint
 * For watchpoints, 'kind' is the number of bytes to watch
 */
static int handle_set_breakpoint(int sock, const char *data) {
    int type = data[0] - '0';

    /* Parse address */
    const char *addr_str = strchr(data, ',');
    if (!addr_str) {
        return send_error(sock, 1);
    }
    addr_str++;

    /* Parse address and kind (length for watchpoints) */
    char *kind_str = NULL;
    uint32_t addr = strtoul(addr_str, &kind_str, 16);
    uint32_t kind = 4;  /* Default to 4 bytes */
    if (kind_str && *kind_str == ',') {
        kind = strtoul(kind_str + 1, NULL, 16);
    }

    printf("Set breakpoint type %d at 0x%08X (kind=%u)\n", type, addr, kind);

    switch (type) {
        case 0:
            /* Software breakpoint - use hardware if available, fall back to software */
            /* Try hardware first (faster, doesn't modify memory) */
            if (set_hw_breakpoint(addr) >= 0) {
                return send_ok(sock);
            }
            /* Fall back to software breakpoint */
            if (set_sw_breakpoint(addr) == 0) {
                return send_ok(sock);
            }
            return send_error(sock, 0x0E);  /* Resource busy */

        case 1:
            /* Hardware execution breakpoint */
            if (set_hw_breakpoint(addr) >= 0) {
                return send_ok(sock);
            }
            return send_error(sock, 0x0E);  /* Resource busy */

        case 2:  /* Write watchpoint */
            if (set_watchpoint(addr, kind, WP_TYPE_WRITE) == 0) {
                return send_ok(sock);
            }
            return send_error(sock, 0x0E);  /* Resource busy */

        case 3:  /* Read watchpoint */
            if (set_watchpoint(addr, kind, WP_TYPE_READ) == 0) {
                return send_ok(sock);
            }
            return send_error(sock, 0x0E);  /* Resource busy */

        case 4:  /* Access watchpoint (read or write) */
            if (set_watchpoint(addr, kind, WP_TYPE_ACCESS) == 0) {
                return send_ok(sock);
            }
            return send_error(sock, 0x0E);  /* Resource busy */

        default:
            return send_error(sock, 1);
    }
}

/* Handle 'z' - remove breakpoint
 * Format: ztype,addr,kind
 */
static int handle_remove_breakpoint(int sock, const char *data) {
    int type = data[0] - '0';

    /* Parse address */
    const char *addr_str = strchr(data, ',');
    if (!addr_str) {
        return send_error(sock, 1);
    }
    addr_str++;

    /* Parse address and kind (length for watchpoints) */
    char *kind_str = NULL;
    uint32_t addr = strtoul(addr_str, &kind_str, 16);
    uint32_t kind = 4;  /* Default to 4 bytes */
    if (kind_str && *kind_str == ',') {
        kind = strtoul(kind_str + 1, NULL, 16);
    }

    printf("Remove breakpoint type %d at 0x%08X (kind=%u)\n", type, addr, kind);

    switch (type) {
        case 0:
            /* Could be hardware or software - try both */
            if (clear_hw_breakpoint(addr) == 0) {
                return send_ok(sock);
            }
            if (clear_sw_breakpoint(addr) == 0) {
                return send_ok(sock);
            }
            /* Breakpoint not found - still return OK per GDB spec */
            return send_ok(sock);

        case 1:
            /* Hardware execution breakpoint */
            clear_hw_breakpoint(addr);
            return send_ok(sock);

        case 2:  /* Write watchpoint */
            clear_watchpoint(addr, kind, WP_TYPE_WRITE);
            return send_ok(sock);

        case 3:  /* Read watchpoint */
            clear_watchpoint(addr, kind, WP_TYPE_READ);
            return send_ok(sock);

        case 4:  /* Access watchpoint */
            clear_watchpoint(addr, kind, WP_TYPE_ACCESS);
            return send_ok(sock);

        default:
            return send_error(sock, 1);
    }
}

/* Process a single GDB RSP command */
static int process_command(int sock, const char *cmd, int len) {
    printf("RX: $%.*s#xx\n", len, cmd);

    switch (cmd[0]) {
        case 'g':
            return handle_read_registers(sock);
        case 'G':
            return handle_write_registers(sock, cmd + 1);
        case 'p':
            return handle_read_register(sock, cmd + 1);
        case 'P':
            return handle_write_register(sock, cmd + 1);
        case 'm':
            return handle_read_memory(sock, cmd + 1);
        case 'M':
            return handle_write_memory(sock, cmd + 1);
        case 'c':
            return handle_continue(sock, cmd + 1);
        case 's':
            return handle_step(sock, cmd + 1);
        case '?':
            return handle_halt_reason(sock);
        case 'H':
            return handle_set_thread(sock, cmd + 1);
        case 'q':
            return handle_query(sock, cmd + 1);
        case 'Q':
            /* Set commands - acknowledge but ignore for now */
            return send_ok(sock);
        case 'v':
            return handle_v_command(sock, cmd + 1, len - 1);
        case 'Z':
            return handle_set_breakpoint(sock, cmd + 1);
        case 'z':
            return handle_remove_breakpoint(sock, cmd + 1);
        case 'k':
            /* Kill request */
            printf("Kill request received\n");
            return 0;
        case 'D':
            /* Detach */
            printf("Detach request received\n");
            return send_ok(sock);
        default:
            /* Unknown command - empty response */
            return send_packet(sock, "");
    }
}

/* Main packet receive loop */
static int handle_client(int sock) {
    char buffer[MAX_PACKET_SIZE];
    int buf_pos = 0;

    while (g_running) {
        /* Use select() with timeout to allow checking g_running */
        fd_set readfds;
        struct timeval tv;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        tv.tv_sec = 1;  /* 1 second timeout */
        tv.tv_usec = 0;

        int sel = select(sock + 1, &readfds, NULL, NULL, &tv);
        if (sel < 0) {
            if (errno == EINTR) continue;  /* Signal interrupted, check g_running */
            perror("select");
            break;
        }
        if (sel == 0) {
            /* Timeout - check g_running and continue */
            continue;
        }

        /* Read data from socket */
        int n = recv(sock, buffer + buf_pos, sizeof(buffer) - buf_pos - 1, 0);
        if (n <= 0) {
            if (n < 0 && errno != EINTR) perror("recv");
            break;
        }
        buf_pos += n;
        buffer[buf_pos] = '\0';  /* For string operations on non-binary parts */

        /* Process complete packets */
        char *ptr = buffer;
        char *buf_end = buffer + buf_pos;
        while (ptr < buf_end) {
            /* Handle interrupt character (Ctrl-C) */
            if (*ptr == 0x03) {
                printf("Interrupt received\n");
                g_target_halted = 1;
                send_packet(sock, "S02");  /* SIGINT */
                ptr++;
                continue;
            }

            /* Handle ACK/NACK */
            if (*ptr == '+') {
                ptr++;
                continue;
            }
            if (*ptr == '-') {
                printf("NACK received - retransmit needed\n");
                ptr++;
                continue;
            }

            /* Look for packet start */
            if (*ptr != '$') {
                ptr++;
                continue;
            }

            /* Find packet end - use memchr for binary safety */
            size_t remaining_bytes = buf_end - ptr - 1;
            char *end = memchr(ptr + 1, '#', remaining_bytes);
            if (!end || (end - buffer + 2) > buf_pos) {
                /* Incomplete packet - wait for more data */
                break;
            }

            /* Extract command */
            int cmd_len = end - ptr - 1;
            char cmd[MAX_PACKET_SIZE];
            memcpy(cmd, ptr + 1, cmd_len);
            cmd[cmd_len] = '\0';

            /* Verify checksum */
            uint8_t recv_cksum = (hex_to_nibble(end[1]) << 4) | hex_to_nibble(end[2]);
            uint8_t calc_cksum = calc_checksum(cmd, cmd_len);

            if (recv_cksum != calc_cksum) {
                printf("Checksum mismatch: recv=%02x calc=%02x\n", recv_cksum, calc_cksum);
                send(sock, "-", 1, 0);  /* NACK */
            } else {
                send(sock, "+", 1, 0);  /* ACK */
                process_command(sock, cmd, cmd_len);
            }

            ptr = end + 3;
        }

        /* Move remaining data to start of buffer */
        int remaining = buf_pos - (ptr - buffer);
        if (remaining > 0) {
            memmove(buffer, ptr, remaining);
        }
        buf_pos = remaining;
    }

    return 0;
}

/* Initialize USB connection to Multilink */
static int init_usb(void) {
    int r = libusb_init(NULL);
    if (r < 0) {
        fprintf(stderr, "Failed to initialize libusb: %s\n", libusb_error_name(r));
        return -1;
    }

    g_usb_dev = libusb_open_device_with_vid_pid(NULL, USB_VENDOR_ID, USB_PRODUCT_ID);
    if (!g_usb_dev) {
        fprintf(stderr, "Could not open Multilink (VID=%04x PID=%04x)\n",
                USB_VENDOR_ID, USB_PRODUCT_ID);
        libusb_exit(NULL);
        return -1;
    }

    r = libusb_claim_interface(g_usb_dev, 0);
    if (r < 0) {
        fprintf(stderr, "Could not claim interface: %s\n", libusb_error_name(r));
        libusb_close(g_usb_dev);
        libusb_exit(NULL);
        return -1;
    }

    printf("Multilink connected\n");
    return 0;
}

/* Initialize target MCU via BDM */
static int init_target(void) {
    printf("Initializing target...\n");

    uint32_t flash_size = 0;
    int r = target_init_full(g_usb_dev, &flash_size);
    if (r != 0) {
        fprintf(stderr, "Failed to initialize target\n");
        return -1;
    }

    /* Initialize breakpoint tracking */
    init_sw_breakpoints();

    /* Initialize watchpoint tracking */
    init_watchpoints();

    /* Clear any existing hardware breakpoints */
    for (int i = 0; i < MAX_HW_BREAKPOINTS; i++) {
        write_pbr(i, 0);
        hw_breakpoints[i] = 0;
        hw_breakpoint_used[i] = 0;
    }

    /* Clear address range registers (watchpoints) */
    write_ablr(0);
    write_abhr(0);

    /* Clear TDR to disable all triggers */
    write_tdr(0);

    printf("Target initialized (flash size: %u KB)\n", flash_size);
    printf("Hardware breakpoints: %d available\n", MAX_HW_BREAKPOINTS);
    printf("Software breakpoints: %d available\n", MAX_SW_BREAKPOINTS);
    printf("Watchpoints: %d available\n", MAX_WATCHPOINTS);
    return 0;
}

/* Cleanup */
static void cleanup(void) {
    if (g_client_socket >= 0) {
        close(g_client_socket);
    }
    if (g_server_socket >= 0) {
        close(g_server_socket);
    }
    if (g_usb_dev) {
        libusb_release_interface(g_usb_dev, 0);
        libusb_close(g_usb_dev);
        libusb_exit(NULL);
    }
}

static void print_usage(const char *prog) {
    printf("OpenLink ColdFire GDB Server / Flash Programmer\n\n");
    printf("Usage: %s [OPTIONS]\n\n", prog);
    printf("Modes:\n");
    printf("  --erase                Erase entire flash (256KB)\n");
    printf("  --program <file>       Erase and program flash from file\n");
    printf("                         Supports: .bin, .elf, .s19/.srec\n");
    printf("  --gdb                  GDB server mode (default)\n");
    printf("\n");
    printf("Options:\n");
    printf("  -p, --port <port>      TCP port for GDB (default: %d)\n", DEFAULT_PORT);
    printf("  -f, --flashloader <path>  Path to flashloader.elf\n");
    printf("  -v, --verify           Verify after programming\n");
    printf("  --base <addr>          Base address for .bin files (default: 0x00000000)\n");
    printf("  -h, --help             Show this help\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s --erase                       Erase entire flash\n", prog);
    printf("  %s --program firmware.bin        Program binary file\n", prog);
    printf("  %s --program firmware.elf -v     Program ELF with verify\n", prog);
    printf("  %s -p 3333                       Start GDB server\n", prog);
    printf("  %s --gdb                         Start GDB server\n", prog);
}

/* Mode 1: Erase only */
static int do_erase_only(void) {
    gpl_flash_state_t flash;
    int ret = -1;

    printf("\n");
    printf("==============================================\n");
    printf("  Erasing entire flash (256KB)\n");
    printf("==============================================\n\n");

    if (gpl_flash_init(&flash, g_usb_dev, NULL) != 0) {
        fprintf(stderr, "Failed to initialize flashloader\n");
        return -1;
    }

    if (gpl_flash_mass_erase(&flash) != 0) {
        fprintf(stderr, "Mass erase failed\n");
        goto cleanup;
    }

    printf("\n");
    printf("==============================================\n");
    printf("  Flash erased successfully!\n");
    printf("==============================================\n");
    ret = 0;

cleanup:
    gpl_flash_cleanup(&flash);
    return ret;
}

/* Mode 2: Program file */
static int do_program_file(const char *filename, uint32_t base_addr, int verify) {
    gpl_flash_state_t flash;
    loaded_file_t file;
    int ret = -1;

    printf("\n");
    printf("==============================================\n");
    printf("  Programming %s\n", filename);
    printf("==============================================\n\n");

    /* Load file */
    if (file_load(filename, base_addr, &file) != 0) {
        fprintf(stderr, "Failed to load file '%s'\n", filename);
        return -1;
    }

    file_print_info(&file);
    printf("\n");

    /* Check flash bounds */
    if (file.max_addr > FLASH_SIZE) {
        fprintf(stderr, "Error: File extends beyond flash (0x%08X > 0x%08X)\n",
                file.max_addr, FLASH_SIZE);
        file_free(&file);
        return -1;
    }

    /* Initialize flashloader */
    if (gpl_flash_init(&flash, g_usb_dev, NULL) != 0) {
        fprintf(stderr, "Failed to initialize flashloader\n");
        file_free(&file);
        return -1;
    }

    /* Get contiguous data */
    uint8_t *data;
    uint32_t data_addr, data_size;
    if (file_get_contiguous(&file, &data_addr, &data, &data_size) != 0) {
        fprintf(stderr, "Failed to get contiguous data\n");
        goto cleanup;
    }

    /* Program with optional verify */
    int r = gpl_flash_program_binary(&flash, data, data_size, data_addr, verify);
    free(data);

    if (r != 0) {
        fprintf(stderr, "Programming failed\n");
        goto cleanup;
    }

    printf("\n");
    printf("==============================================\n");
    printf("  Programming complete!\n");
    if (file.entry_point != 0) {
        printf("  Entry point: 0x%08X\n", file.entry_point);
    }
    printf("==============================================\n");
    ret = 0;

cleanup:
    gpl_flash_cleanup(&flash);
    file_free(&file);
    return ret;
}

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;
    operation_mode_t mode = MODE_GDB;
    const char *program_file = NULL;
    int verify = 0;
    uint32_t base_addr = 0x00000000;

    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0) {
            if (i + 1 < argc) {
                port = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--gdb") == 0) {
            mode = MODE_GDB;
        } else if (strcmp(argv[i], "--erase") == 0) {
            mode = MODE_ERASE;
        } else if (strcmp(argv[i], "--program") == 0) {
            if (i + 1 < argc) {
                mode = MODE_PROGRAM;
                program_file = argv[++i];
            } else {
                fprintf(stderr, "Error: --program requires a filename\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verify") == 0) {
            verify = 1;
        } else if (strcmp(argv[i], "--base") == 0) {
            if (i + 1 < argc) {
                base_addr = strtoul(argv[++i], NULL, 0);
            }
        } else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--flashloader") == 0) {
            if (i + 1 < argc) {
                /* Flashloader path - currently ignored, uses default */
                i++;
            }
        } else if (argv[i][0] != '-') {
            /* Positional argument - treat as file for programming */
            mode = MODE_PROGRAM;
            program_file = argv[i];
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    /* Setup signal handlers */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, SIG_IGN);  /* Ignore SIGPIPE - handle broken pipe in send() */

    /* Initialize USB connection */
    if (init_usb() != 0) {
        return 1;
    }

    /* Initialize target */
    if (init_target() != 0) {
        cleanup();
        return 1;
    }

    /* Handle non-GDB modes */
    if (mode == MODE_ERASE) {
        int ret = do_erase_only();
        cleanup();
        return ret;
    } else if (mode == MODE_PROGRAM) {
        int ret = do_program_file(program_file, base_addr, verify);
        cleanup();
        return ret;
    }

    /* MODE_GDB: Create server socket */
    g_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (g_server_socket < 0) {
        perror("socket");
        cleanup();
        return 1;
    }

    /* Allow address reuse */
    int opt = 1;
    setsockopt(g_server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* Bind to port */
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(g_server_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        cleanup();
        return 1;
    }

    /* Listen for connections */
    if (listen(g_server_socket, 1) < 0) {
        perror("listen");
        cleanup();
        return 1;
    }

    printf("\n");
    printf("==============================================\n");
    printf("  m68k-gdbserver listening on port %d\n", port);
    printf("==============================================\n");
    printf("\n");
    printf("Connect with:\n");
    printf("  m68k-elf-gdb -ex \"set arch m68k:521x\" -ex \"target remote :%d\" program.elf\n", port);
    printf("\n");

    /* Main server loop */
    while (g_running) {
        printf("Waiting for GDB connection...\n");

        /* Use select() with timeout to allow checking g_running */
        fd_set readfds;
        struct timeval tv;
        FD_ZERO(&readfds);
        FD_SET(g_server_socket, &readfds);
        tv.tv_sec = 1;  /* 1 second timeout */
        tv.tv_usec = 0;

        int sel = select(g_server_socket + 1, &readfds, NULL, NULL, &tv);
        if (sel < 0) {
            if (errno == EINTR) continue;  /* Signal interrupted */
            perror("select");
            break;
        }
        if (sel == 0) {
            /* Timeout - check g_running and continue */
            continue;
        }

        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        g_client_socket = accept(g_server_socket, (struct sockaddr *)&client_addr, &client_len);

        if (g_client_socket < 0) {
            if (errno == EINTR) continue;
            perror("accept");
            break;
        }

        /* Disable Nagle's algorithm for immediate packet transmission */
        int nodelay = 1;
        setsockopt(g_client_socket, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));

        printf("GDB connected from %s:%d\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        handle_client(g_client_socket);

        close(g_client_socket);
        g_client_socket = -1;
        printf("GDB disconnected\n");
    }

    cleanup();
    printf("Goodbye!\n");
    return 0;
}
