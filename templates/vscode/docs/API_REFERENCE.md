# OpenLink ColdFire API Reference

## MCF52233/MCF52235 Template Library

This document provides a comprehensive API reference for the OpenLink ColdFire template library, supporting the M52233DEMO development board and MCF5223x family processors.

---

## Table of Contents

1. [Header Files](#header-files)
2. [Data Types](#data-types)
3. [CPU Core Functions (mcf5xxx)](#cpu-core-functions-mcf5xxx)
   - [Interrupt Control](#interrupt-control)
   - [Exception Handling](#exception-handling)
   - [Special Register Access](#special-register-access)
4. [MCF52235 Peripheral Access](#mcf52235-peripheral-access)
   - [Memory Map](#memory-map)
   - [GPIO](#gpio)
   - [LED Control (M52233DEMO)](#led-control-m52233demo)
   - [UART](#uart)
   - [Flash Module (CFM)](#flash-module-cfm)
   - [Timers](#timers)
5. [Constants and Macros](#constants-and-macros)
6. [Usage Examples](#usage-examples)

---

## Header Files

| Header | Purpose |
|--------|---------|
| `mcf5xxx.h` | Common ColdFire CPU definitions and functions |
| `mcf52235.h` | MCF52233/MCF52235 specific peripheral registers |

### Include Options

```c
// Option 1: Simple - basic register access
#include "mcf52235.h"

// Option 2: Full - comprehensive peripheral headers
#define MCF5223_FULL_HEADERS
#include "mcf52235.h"

// Option 3: Direct - specific peripheral only
#include "mcf5223/mcf5223_uart.h"
```

---

## Data Types

### Standard Types (mcf5xxx.h)

| Type | Size | Description |
|------|------|-------------|
| `uint8` | 8-bit | Unsigned byte |
| `uint16` | 16-bit | Unsigned short |
| `uint32` | 32-bit | Unsigned long |
| `int8` | 8-bit | Signed byte |
| `int16` | 16-bit | Signed short |
| `int32` | 32-bit | Signed long |
| `vuint8` | 8-bit | Volatile unsigned byte |
| `vuint16` | 16-bit | Volatile unsigned short |
| `vuint32` | 32-bit | Volatile unsigned long |
| `ADDRESS` | 32-bit | Memory address type |
| `INSTRUCTION` | 16-bit | CPU instruction type |

### Alternative Type Names

```c
BYTE    // unsigned char (8-bit)
WORD    // unsigned short (16-bit)
LWORD   // unsigned long (32-bit)
UINT8, INT8, UINT16, INT16, UINT32, INT32
tU08, tU16, tU32, tS08, tS16, tS32
```

---

## CPU Core Functions (mcf5xxx)

### Interrupt Control

#### `mcf5xxx_irq_enable()`
```c
void mcf5xxx_irq_enable(void);
```
Enable all maskable interrupts by setting the Interrupt Priority Level (IPL) to 0.

**Example:**
```c
mcf5xxx_irq_enable();  // Allow all interrupts
```

---

#### `mcf5xxx_irq_disable()`
```c
void mcf5xxx_irq_disable(void);
```
Disable all maskable interrupts by setting IPL to 7 (maximum priority level).

**Example:**
```c
mcf5xxx_irq_disable();  // Block all maskable interrupts
// Critical section code here
mcf5xxx_irq_enable();
```

---

#### `mcf5xxx_get_ipl()`
```c
int mcf5xxx_get_ipl(void);
```
Get the current Interrupt Priority Level.

**Returns:** Current IPL value (0-7)

**Example:**
```c
int current_ipl = mcf5xxx_get_ipl();
```

---

#### `mcf5xxx_set_ipl()`
```c
int mcf5xxx_set_ipl(int ipl);
```
Set the Interrupt Priority Level.

**Parameters:**
- `ipl` - New priority level (0-7). Only the lower 3 bits are used.

**Returns:** Previous IPL value

**Example:**
```c
int old_ipl = mcf5xxx_set_ipl(5);  // Block interrupts level 5 and below
// ... do work ...
mcf5xxx_set_ipl(old_ipl);          // Restore previous level
```

---

#### `asm_set_ipl()`
```c
int asm_set_ipl(uint32 ipl);
```
Low-level assembly function to set IPL. Directly modifies the Status Register.

**Parameters:**
- `ipl` - New priority level (0-7)

**Returns:** Previous IPL value

**Note:** Prefer using `mcf5xxx_set_ipl()` which wraps this function.

---

### Exception Handling

#### `mcf5xxx_exception_handler()`
```c
void mcf5xxx_exception_handler(void *framep);
```
C-level exception handler called from the assembly wrapper `asm_exception_handler`.

**Parameters:**
- `framep` - Pointer to the exception stack frame

**Stack Frame Format:**
```
           8 +----------------+----------------+
             |         Program Counter         |
           4 +----------------+----------------+
             |FS/Fmt/Vector/FS|      SR        |
   SP -->  0 +----------------+----------------+
```

**Note:** This function is called automatically by the exception handling system. Override `cpu_handle_interrupt()` for custom interrupt handling.

---

#### `mcf5xxx_set_handler()`
```c
ADDRESS mcf5xxx_set_handler(int vector, ADDRESS handler);
```
Install a custom exception or interrupt handler.

**Parameters:**
- `vector` - Exception vector number (0-255)
- `handler` - Address of the new handler function

**Returns:** Address of the previous handler (0 if none)

**Example:**
```c
void my_timer_handler(void) {
    // Handle timer interrupt
    // Clear interrupt flag
}

// Install handler for PIT0 interrupt (vector 119)
ADDRESS old = mcf5xxx_set_handler(119, (ADDRESS)my_timer_handler);
```

**Vector Numbers:**
| Range | Purpose |
|-------|---------|
| 0-1 | Initial SP/PC |
| 2-15 | CPU exceptions |
| 16-23 | Reserved |
| 24 | Spurious interrupt |
| 25-31 | Autovector interrupts |
| 32-47 | TRAP instructions |
| 48-63 | Reserved |
| 64-255 | Peripheral interrupts |

---

#### `cpu_handle_interrupt()`
```c
void cpu_handle_interrupt(int irq);
```
Weak default interrupt handler. Override this in your application to handle peripheral interrupts.

**Parameters:**
- `irq` - Interrupt number (vector - 64)

**Example:**
```c
// Override the weak default
void cpu_handle_interrupt(int irq) {
    switch (irq) {
        case 55:  // PIT0
            handle_pit0();
            break;
        case 13:  // UART0 RX
            handle_uart_rx();
            break;
    }
}
```

---

#### `mcf5xxx_enter_supervisor()`
```c
void mcf5xxx_enter_supervisor(void);
```
Ensure the CPU is in supervisor mode by setting the S bit in the Status Register.

---

### Special Register Access

These functions write to ColdFire special purpose registers. They are implemented in assembly (`mcf5xxx.S`) using the `movec` instruction.

#### Vector Base Register
```c
void mcf5xxx_wr_vbr(uint32 value);
```
Set the Vector Base Register (exception table location).

---

#### RAM/ROM Base Address Registers
```c
void mcf5xxx_wr_rambar0(uint32 value);
void mcf5xxx_wr_rambar1(uint32 value);
void mcf5xxx_wr_rombar0(uint32 value);
void mcf5xxx_wr_rombar1(uint32 value);
```
Configure SRAM and ROM base addresses and attributes.

**RAMBAR Value Format:**
```c
// Example: SRAM at 0x20000000, valid, read/write enabled
uint32 rambar = 0x20000221;
mcf5xxx_wr_rambar0(rambar);
```

---

#### Cache Control
```c
void mcf5xxx_wr_cacr(uint32 value);
void mcf5xxx_wr_acr0(uint32 value);
void mcf5xxx_wr_acr1(uint32 value);
void mcf5xxx_wr_acr2(uint32 value);
void mcf5xxx_wr_acr3(uint32 value);
```
Configure cache and access control registers.

---

#### MAC Unit (if present)
```c
void mcf5xxx_wr_macsr(uint32 value);
void mcf5xxx_wr_mask(uint32 value);
void mcf5xxx_wr_acc0(uint32 value);
void mcf5xxx_wr_acc1(uint32 value);
void mcf5xxx_wr_acc2(uint32 value);
void mcf5xxx_wr_acc3(uint32 value);
void mcf5xxx_wr_accext01(uint32 value);
void mcf5xxx_wr_accext23(uint32 value);
```
Configure the Multiply-Accumulate unit.

---

#### Other Registers
```c
void mcf5xxx_wr_sr(uint32 value);       // Status Register
void mcf5xxx_wr_pc(uint32 value);       // Program Counter
void mcf5xxx_wr_mbar(uint32 value);     // Module Base Address Register
void mcf5xxx_wr_other_a7(uint32 value); // Alternate Stack Pointer
void mcf5xxx_wr_asid(uint32 value);     // Address Space ID
void mcf5xxx_wr_mmubar(uint32 value);   // MMU Base Address Register
```

---

#### Debug Support
```c
void mcf5xxx_exe_wdebug(void *cmd);
```
Execute a WDEBUG instruction for hardware debug support.

---

## MCF52235 Peripheral Access

### Memory Map

| Base Address | Peripheral |
|--------------|------------|
| `0x00000000` | Flash (256KB) |
| `0x20000000` | SRAM (32KB) |
| `0x40000000` | IPSBAR (Peripheral Base) |
| `0x40000200` | UART0 |
| `0x40000240` | UART1 |
| `0x40000280` | UART2 |
| `0x40100000` | GPIO |
| `0x40120000` | Clock Module |
| `0x40150000` | PIT0 |
| `0x40160000` | PIT1 |
| `0x401D0000` | Flash Module (CFM) |

---

### GPIO

#### Port TC (LEDs on M52233DEMO)
```c
PORTTC   // Port TC Data Register
DDRTC    // Port TC Data Direction (1=output)
SETTC    // Port TC Set (write 1 to set bit)
CLRTC    // Port TC Clear (write 1 to clear bit)
PTCPAR   // Port TC Pin Assignment (0=GPIO)
```

#### Port TD
```c
PORTTD   // Port TD Data Register
DDRTD    // Port TD Data Direction
SETTD    // Port TD Set
CLRTD    // Port TD Clear
PTDPAR   // Port TD Pin Assignment
```

---

### LED Control (M52233DEMO)

The M52233DEMO board has 4 LEDs connected to Port TC bits 0-3.

#### Macros
```c
LED1_ON()      // Turn LED1 on
LED1_OFF()     // Turn LED1 off
LED1_TOGGLE()  // Toggle LED1

LED2_ON()      LED2_OFF()      LED2_TOGGLE()
LED3_ON()      LED3_OFF()      LED3_TOGGLE()
LED4_ON()      LED4_OFF()      LED4_TOGGLE()
```

#### Initialization
```c
void leds_init(void);
```
Initialize LED GPIO pins as outputs with all LEDs off.

**Example:**
```c
leds_init();
LED1_ON();
delay(100000);
LED1_OFF();
```

---

### Utility Functions

#### `delay()`
```c
static inline void delay(volatile uint32_t count);
```
Simple busy-wait delay loop.

**Parameters:**
- `count` - Number of NOP iterations

**Note:** Timing is approximate and depends on clock speed.

---

### UART

#### UART0 Registers
```c
UART0_UMR    // Mode Register (dual access)
UART0_USR    // Status Register (read)
UART0_UCSR   // Clock Select Register (write)
UART0_UCR    // Command Register
UART0_URB    // Receive Buffer (read)
UART0_UTB    // Transmit Buffer (write)
UART0_UBG1   // Baud Rate Generator MSB
UART0_UBG2   // Baud Rate Generator LSB
UART0_UISR   // Interrupt Status Register
UART0_UIMR   // Interrupt Mask Register
```

---

### Flash Module (CFM)

#### Registers
```c
CFMMCR    // Flash Module Configuration (16-bit)
CFMCLKD   // Flash Clock Divider (8-bit, write-once)
CFMSEC    // Flash Security Register (32-bit)
CFMPROT   // Flash Protection Register (32-bit)
CFMSACC   // Supervisor Access Register (32-bit)
CFMDACC   // Data Access Register (32-bit)
CFMUSTAT  // Flash User Status Register (8-bit)
CFMCMD    // Flash Command Register (8-bit)
```

#### Status Bits (CFMUSTAT)
```c
CFMUSTAT_CBEIF   0x80  // Command Buffer Empty
CFMUSTAT_CCIF    0x40  // Command Complete
CFMUSTAT_PVIOL   0x20  // Protection Violation
CFMUSTAT_ACCERR  0x10  // Access Error
CFMUSTAT_BLANK   0x04  // Array is Blank
```

#### Commands (CFMCMD)
```c
CFM_CMD_BLANK_CHECK  0x05  // Verify flash is erased
CFM_CMD_PAGE_ERASE   0x40  // Erase single page (2KB)
CFM_CMD_MASS_ERASE   0x41  // Erase entire flash
CFM_CMD_PROGRAM      0x20  // Program flash word
```

---

### Timers

#### PIT0 (Programmable Interrupt Timer)
```c
PIT0_PCSR   // Control/Status Register (16-bit)
PIT0_PMR    // Modulus Register (16-bit)
PIT0_PCNTR  // Counter Register (16-bit, read-only)
```

---

## Constants and Macros

### Status Register Bits
```c
MCF5XXX_SR_T      0x8000  // Trace Enable
MCF5XXX_SR_S      0x2000  // Supervisor Mode
MCF5XXX_SR_M      0x1000  // Master/Interrupt State
MCF5XXX_SR_IPL    0x0700  // Interrupt Priority Level Mask
MCF5XXX_SR_IPL_0  0x0000  // IPL = 0 (all enabled)
MCF5XXX_SR_IPL_7  0x0700  // IPL = 7 (all masked)
MCF5XXX_SR_X      0x0010  // Extend Flag
MCF5XXX_SR_N      0x0008  // Negative Flag
MCF5XXX_SR_Z      0x0004  // Zero Flag
MCF5XXX_SR_V      0x0002  // Overflow Flag
MCF5XXX_SR_C      0x0001  // Carry Flag
```

### RAMBAR Configuration
```c
MCF5XXX_RAMBAR_BA(a)    // Base Address (aligned)
MCF5XXX_RAMBAR_WP       // Write Protect
MCF5XXX_RAMBAR_CI       // Cache Inhibit
MCF5XXX_RAMBAR_V        // Valid
```

### Exception Frame Macros
```c
MCF5XXX_RD_SF_FORMAT(PTR)  // Get frame format
MCF5XXX_RD_SF_VECTOR(PTR)  // Get vector number
MCF5XXX_RD_SF_FS(PTR)      // Get fault status
MCF5XXX_SF_SR(PTR)         // Access SR in frame
MCF5XXX_SF_PC(PTR)         // Access PC in frame
```

### D0/D1 Reset Value Decoding
```c
MCF5XXX_D0_PF(x)      // Processor Family
MCF5XXX_D0_VER(x)     // Version
MCF5XXX_D0_REV(x)     // Revision
MCF5XXX_D0_MAC(x)     // MAC present
MCF5XXX_D0_DIV(x)     // Hardware divide present
MCF5XXX_D0_EMAC(x)    // EMAC present
MCF5XXX_D0_FPU(x)     // FPU present
MCF5XXX_D0_MMU(x)     // MMU present
MCF5XXX_D0_ISA(x)     // ISA revision
MCF5XXX_D0_DEBUG(x)   // Debug revision
```

---

## Usage Examples

### Basic LED Blink
```c
#include "mcf52235.h"

int main(void) {
    leds_init();

    while (1) {
        LED1_TOGGLE();
        delay(100000);
    }
}
```

### Critical Section with Interrupt Control
```c
#include "mcf52235.h"
#include "mcf5xxx.h"

volatile int shared_counter = 0;

void safe_increment(void) {
    int old_ipl = mcf5xxx_set_ipl(7);  // Disable interrupts
    shared_counter++;
    mcf5xxx_set_ipl(old_ipl);          // Restore
}
```

### Custom Interrupt Handler
```c
#include "mcf52235.h"
#include "mcf5xxx.h"

void timer_isr(void) {
    // Clear interrupt flag
    PIT0_PCSR |= 0x0004;  // Write 1 to PIF to clear

    // Toggle LED
    LED1_TOGGLE();
}

int main(void) {
    leds_init();

    // Install timer interrupt handler (vector 119 = PIT0)
    mcf5xxx_set_handler(119, (ADDRESS)timer_isr);

    // Configure PIT0
    PIT0_PMR = 0xFFFF;           // Maximum period
    PIT0_PCSR = 0x0F0F;          // Enable, prescale, interrupt

    // Enable interrupts
    mcf5xxx_irq_enable();

    while (1) {
        // Main loop
    }
}
```

---

## Files Reference

| File | Location | Purpose |
|------|----------|---------|
| `mcf5xxx.h` | include/ | ColdFire CPU definitions |
| `mcf5xxx.c` | src/ | CPU support functions (C) |
| `mcf5xxx.S` | startup/ | CPU support functions (ASM) |
| `mcf52235.h` | include/ | MCF52235 peripheral registers |
| `startup.S` | startup/ | Reset vector and startup code |
| `mcf52235.ld` | ldscripts/ | Linker script |

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0.0 | 2025-12-06 | Initial release |

---

## License

OpenLink ColdFire - Open Source ColdFire/M68K Debug Tools
Copyright (C) 2025 Gary Fekete
GPL v3 License
