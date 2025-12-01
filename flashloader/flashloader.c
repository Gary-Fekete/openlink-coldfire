/*
 * OpenLink ColdFire Flashloader
 *
 * This code runs on the target MCF52233 CPU to perform flash operations.
 * It is loaded into SRAM and executed via BDM.
 *
 * The flashloader communicates with the host through a parameter block
 * in SRAM. The host writes parameters, sets PC to entry point, and
 * resumes execution. The flashloader performs the operation and halts.
 *
 * Memory Layout:
 *   0x20000000 - Parameter block (operation, addresses, sizes, results)
 *   0x20000100 - Data buffer (for programming)
 *   0x20000500 - Flashloader code
 *
 * Operations:
 *   0 = Initialize (set clock divider, disable protection)
 *   1 = Mass Erase (erase entire 256KB flash)
 *   2 = Sector Erase (erase 8KB sector at specified address)
 *   3 = Program (write data buffer to flash)
 *   4 = Blank Check (verify flash is erased)
 *   5 = Verify (compare flash with data buffer)
 *
 * License: GPL v3
 */

#include <stdint.h>

/* Parameter block structure at 0x20000000 */
typedef struct {
    uint32_t operation;     /* 0x00: Operation to perform */
    uint32_t flash_addr;    /* 0x04: Flash address for operation */
    uint32_t length;        /* 0x08: Length in bytes */
    uint32_t result;        /* 0x0C: Result code (0=success) */
    uint32_t status;        /* 0x10: CFMUSTAT value after operation */
    uint32_t reserved[3];   /* 0x14-0x1F: Reserved */
} params_t;

/* Result codes */
#define RESULT_SUCCESS          0x00000000
#define RESULT_ERROR_ACCERR     0x00000001  /* Access error */
#define RESULT_ERROR_PVIOL      0x00000002  /* Protection violation */
#define RESULT_ERROR_NOT_BLANK  0x00000003  /* Blank check failed */
#define RESULT_ERROR_VERIFY     0x00000004  /* Verify failed */
#define RESULT_ERROR_TIMEOUT    0x00000005  /* Timeout */
#define RESULT_ERROR_UNKNOWN_OP 0x000000FF  /* Unknown operation */

/* CFM Register addresses */
#define CFMCR       (*(volatile uint16_t*)0x401D0000)  /* Control Register (16-bit) */
#define CFMCLKD     (*(volatile uint8_t*) 0x401D0002)  /* Clock Divider (8-bit) */
#define CFMPROT     (*(volatile uint32_t*)0x401D0010)  /* Protection Register (32-bit) */
#define CFMSACC     (*(volatile uint32_t*)0x401D0014)  /* Supervisor Access (32-bit) */
#define CFMDACC     (*(volatile uint32_t*)0x401D0018)  /* Data Access (32-bit) */
#define CFMUSTAT    (*(volatile uint8_t*) 0x401D0020)  /* User Status (8-bit) */
#define CFMCMD      (*(volatile uint8_t*) 0x401D0024)  /* Command Register (8-bit) */

/* CFMUSTAT bit masks */
#define CBEIF   0x80  /* Command Buffer Empty */
#define CCIF    0x40  /* Command Complete */
#define PVIOL   0x20  /* Protection Violation */
#define ACCERR  0x10  /* Access Error */
#define BLANK   0x04  /* Blank Check Result */

/* Command launch value - CBEIF | PVIOL | ACCERR (clears error flags) */
#define LAUNCH_CMD  0xF4

/* CFM Commands */
#define CMD_BLANK_CHECK     0x05
#define CMD_PAGE_ERASE      0x40
#define CMD_MASS_ERASE      0x41
#define CMD_PROGRAM         0x20

/* Flash backdoor address */
#define FLASH_BACKDOOR      0x44000000

/* Clock divider for 60MHz system clock */
#define FLASH_CLKDIV        0x66

/* Fixed addresses */
#define PARAMS_ADDR         0x20000000
#define DATA_BUFFER_ADDR    0x20000100

/* Pointer to parameters */
#define params ((volatile params_t*)PARAMS_ADDR)
#define data_buffer ((volatile uint32_t*)DATA_BUFFER_ADDR)

/*
 * Wait for command buffer to be empty (ready for new command)
 */
static void wait_cbeif(void) {
    while (!(CFMUSTAT & CBEIF)) {
        /* spin */
    }
}

/*
 * Wait for command to complete (with timeout counter for debugging)
 */
static int wait_ccif(void) {
    volatile uint32_t timeout = 0x00FFFFFF;  /* ~16 million iterations */
    while (!(CFMUSTAT & CCIF)) {
        if (--timeout == 0) {
            return -1;  /* Timeout */
        }
    }
    return 0;
}

/*
 * Clear any error flags
 */
static void clear_errors(void) {
    if (CFMUSTAT & (PVIOL | ACCERR)) {
        CFMUSTAT = (PVIOL | ACCERR);  /* Write 1 to clear */
    }
}

/*
 * Initialize flash module
 */
static uint32_t flash_init(void) {
    /* Disable flash module */
    CFMCR = 0;

    /* Set clock divider */
    CFMCLKD = FLASH_CLKDIV;

    /* Disable all protection */
    CFMPROT = 0x00000000;
    CFMSACC = 0x00000000;
    CFMDACC = 0x00000000;

    /* Clear any pending errors */
    clear_errors();

    return RESULT_SUCCESS;
}

/*
 * Mass erase entire flash (256KB)
 */
static uint32_t flash_mass_erase(void) {
    volatile uint32_t *flash_ptr = (volatile uint32_t*)FLASH_BACKDOOR;

    /* Initialize flash module */
    flash_init();

    /* Wait for ready */
    wait_cbeif();

    /* Write 0 to flash address (required to start sequence) */
    *flash_ptr = 0;

    /* Issue mass erase command */
    CFMCMD = CMD_MASS_ERASE;

    /* Launch command (0xF4 clears any error flags too) */
    CFMUSTAT = LAUNCH_CMD;

    /* Wait for completion (can take several seconds) */
    if (wait_ccif() != 0) {
        params->status = CFMUSTAT;
        return RESULT_ERROR_TIMEOUT;
    }

    /* Save status */
    params->status = CFMUSTAT;

    /* Check for errors */
    if (CFMUSTAT & PVIOL) {
        return RESULT_ERROR_PVIOL;
    }
    if (CFMUSTAT & ACCERR) {
        return RESULT_ERROR_ACCERR;
    }

    return RESULT_SUCCESS;
}

/*
 * Erase single sector (8KB) at specified address
 */
static uint32_t flash_sector_erase(uint32_t addr) {
    volatile uint32_t *flash_ptr = (volatile uint32_t*)(FLASH_BACKDOOR + addr);

    /* Initialize flash module */
    flash_init();

    /* Wait for ready */
    wait_cbeif();

    /* Write 0 to sector address */
    *flash_ptr = 0;

    /* Issue page erase command */
    CFMCMD = CMD_PAGE_ERASE;

    /* Launch command - use 0x90 for page erase per reference code */
    CFMUSTAT = 0x90;

    /* Wait for ready (not busy) */
    wait_cbeif();

    /* Save status */
    params->status = CFMUSTAT;

    /* Check for errors */
    if (CFMUSTAT & PVIOL) {
        return RESULT_ERROR_PVIOL;
    }
    if (CFMUSTAT & ACCERR) {
        return RESULT_ERROR_ACCERR;
    }

    return RESULT_SUCCESS;
}

/*
 * Program flash from data buffer
 * addr: flash address (must be longword aligned)
 * length: number of bytes to program (must be multiple of 4)
 */
static uint32_t flash_program(uint32_t addr, uint32_t length) {
    volatile uint32_t *flash_ptr = (volatile uint32_t*)(FLASH_BACKDOOR + addr);
    uint32_t num_words = length / 4;
    uint32_t i;

    /* Initialize flash module */
    flash_init();

    /* Program each longword */
    for (i = 0; i < num_words; i++) {
        /* Wait for ready */
        wait_cbeif();

        /* Write data to flash address */
        flash_ptr[i] = data_buffer[i];

        /* Issue program command */
        CFMCMD = CMD_PROGRAM;

        /* Launch command - use 0x90 for program per reference code */
        CFMUSTAT = 0x90;
    }

    /* Wait for last command to complete */
    wait_ccif();

    /* Save status */
    params->status = CFMUSTAT;

    /* Check for errors */
    if (CFMUSTAT & PVIOL) {
        return RESULT_ERROR_PVIOL;
    }
    if (CFMUSTAT & ACCERR) {
        return RESULT_ERROR_ACCERR;
    }

    return RESULT_SUCCESS;
}

/*
 * Blank check - verify flash is erased (all 0xFF)
 * addr: start address
 * length: number of bytes to check
 */
static uint32_t flash_blank_check(uint32_t addr, uint32_t length) {
    volatile uint32_t *flash_ptr = (volatile uint32_t*)(FLASH_BACKDOOR + addr);

    /* Unused parameter - blank check checks entire flash array */
    (void)length;

    /* Initialize flash module */
    flash_init();

    /* Wait for ready */
    wait_cbeif();

    /* Write 0 to flash address */
    *flash_ptr = 0;

    /* Issue blank check command */
    CFMCMD = CMD_BLANK_CHECK;

    /* Launch command (0xF4 clears error flags) */
    CFMUSTAT = LAUNCH_CMD;

    /* Wait for completion */
    wait_ccif();

    /* Save status */
    params->status = CFMUSTAT;

    /* Check blank flag */
    if (!(CFMUSTAT & BLANK)) {
        return RESULT_ERROR_NOT_BLANK;
    }

    /* Check for errors */
    if (CFMUSTAT & PVIOL) {
        return RESULT_ERROR_PVIOL;
    }
    if (CFMUSTAT & ACCERR) {
        return RESULT_ERROR_ACCERR;
    }

    return RESULT_SUCCESS;
}

/*
 * Verify - compare flash contents with data buffer
 */
static uint32_t flash_verify(uint32_t addr, uint32_t length) {
    volatile uint32_t *flash_ptr = (volatile uint32_t*)addr;  /* Direct access, not backdoor */
    uint32_t num_words = length / 4;
    uint32_t i;

    for (i = 0; i < num_words; i++) {
        if (flash_ptr[i] != data_buffer[i]) {
            return RESULT_ERROR_VERIFY;
        }
    }

    return RESULT_SUCCESS;
}

/*
 * Main entry point - called by BDM after loading to SRAM
 */
void __attribute__((noreturn)) flashloader_entry(void) {
    uint32_t result;

    /* Read operation from parameter block */
    switch (params->operation) {
        case 0:  /* Initialize */
            result = flash_init();
            break;

        case 1:  /* Mass Erase */
            result = flash_mass_erase();
            break;

        case 2:  /* Sector Erase */
            result = flash_sector_erase(params->flash_addr);
            break;

        case 3:  /* Program */
            result = flash_program(params->flash_addr, params->length);
            break;

        case 4:  /* Blank Check */
            result = flash_blank_check(params->flash_addr, params->length);
            break;

        case 5:  /* Verify */
            result = flash_verify(params->flash_addr, params->length);
            break;

        default:
            result = RESULT_ERROR_UNKNOWN_OP;
            break;
    }

    /* Store result */
    params->result = result;

    /* Re-enable protection */
    CFMPROT = 0xFFFFFFFF;

    /* Halt - BDM will detect this */
    while (1) {
        __asm__ volatile ("halt");
    }
}

/*
 * Reset vector and startup code
 * The flashloader is position-independent, loaded at 0x20000500
 */
void __attribute__((section(".vectors"))) _start(void) {
    /* Set stack pointer to top of SRAM and jump to main entry */
    __asm__ volatile (
        "move.l  #0x20007FF0, %%sp\n"
        "jmp     flashloader_entry\n"
        ::: "memory"
    );
    /* Never reached, but prevents compiler warning */
    while(1);
}
