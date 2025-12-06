/*
 * File:    mcf5223_uart.h
 * Purpose: UART register definitions for MCF5223
 * Based on MCF52235 Reference Manual, Chapter 26
 *
 * OpenLink ColdFire - Open Source ColdFire/M68K Debug Tools
 *
 * This file is part of OpenLink ColdFire.
 * Licensed under the GNU General Public License v3.0
 */

#ifndef MCF5223_UART_H
#define MCF5223_UART_H

#include <stdint.h>

/*********************************************************************
 * Universal Asynchronous Receiver Transmitter (UART)
 * Base Addresses:
 *   UART0: IPSBAR + 0x000200
 *   UART1: IPSBAR + 0x000240
 *   UART2: IPSBAR + 0x000280
 *********************************************************************/

#define MCF_UART0_BASE          (IPSBAR + 0x000200)
#define MCF_UART1_BASE          (IPSBAR + 0x000240)
#define MCF_UART2_BASE          (IPSBAR + 0x000280)

/* Generic UART base address macro */
#define MCF_UART_BASE(n)        (IPSBAR + 0x000200 + ((n) * 0x40))

/*********************************************************************
 * UART0 Register definitions
 *********************************************************************/

#define MCF_UART0_UMR           (*(volatile uint8_t *)(MCF_UART0_BASE + 0x00))
#define MCF_UART0_USR           (*(volatile uint8_t *)(MCF_UART0_BASE + 0x04))
#define MCF_UART0_UCSR          (*(volatile uint8_t *)(MCF_UART0_BASE + 0x04))
#define MCF_UART0_UCR           (*(volatile uint8_t *)(MCF_UART0_BASE + 0x08))
#define MCF_UART0_URB           (*(volatile uint8_t *)(MCF_UART0_BASE + 0x0C))
#define MCF_UART0_UTB           (*(volatile uint8_t *)(MCF_UART0_BASE + 0x0C))
#define MCF_UART0_UIPCR         (*(volatile uint8_t *)(MCF_UART0_BASE + 0x10))
#define MCF_UART0_UACR          (*(volatile uint8_t *)(MCF_UART0_BASE + 0x10))
#define MCF_UART0_UISR          (*(volatile uint8_t *)(MCF_UART0_BASE + 0x14))
#define MCF_UART0_UIMR          (*(volatile uint8_t *)(MCF_UART0_BASE + 0x14))
#define MCF_UART0_UBG1          (*(volatile uint8_t *)(MCF_UART0_BASE + 0x18))
#define MCF_UART0_UBG2          (*(volatile uint8_t *)(MCF_UART0_BASE + 0x1C))
#define MCF_UART0_UIP           (*(volatile uint8_t *)(MCF_UART0_BASE + 0x34))
#define MCF_UART0_UOP1          (*(volatile uint8_t *)(MCF_UART0_BASE + 0x38))
#define MCF_UART0_UOP0          (*(volatile uint8_t *)(MCF_UART0_BASE + 0x3C))

/*********************************************************************
 * UART1 Register definitions
 *********************************************************************/

#define MCF_UART1_UMR           (*(volatile uint8_t *)(MCF_UART1_BASE + 0x00))
#define MCF_UART1_USR           (*(volatile uint8_t *)(MCF_UART1_BASE + 0x04))
#define MCF_UART1_UCSR          (*(volatile uint8_t *)(MCF_UART1_BASE + 0x04))
#define MCF_UART1_UCR           (*(volatile uint8_t *)(MCF_UART1_BASE + 0x08))
#define MCF_UART1_URB           (*(volatile uint8_t *)(MCF_UART1_BASE + 0x0C))
#define MCF_UART1_UTB           (*(volatile uint8_t *)(MCF_UART1_BASE + 0x0C))
#define MCF_UART1_UIPCR         (*(volatile uint8_t *)(MCF_UART1_BASE + 0x10))
#define MCF_UART1_UACR          (*(volatile uint8_t *)(MCF_UART1_BASE + 0x10))
#define MCF_UART1_UISR          (*(volatile uint8_t *)(MCF_UART1_BASE + 0x14))
#define MCF_UART1_UIMR          (*(volatile uint8_t *)(MCF_UART1_BASE + 0x14))
#define MCF_UART1_UBG1          (*(volatile uint8_t *)(MCF_UART1_BASE + 0x18))
#define MCF_UART1_UBG2          (*(volatile uint8_t *)(MCF_UART1_BASE + 0x1C))
#define MCF_UART1_UIP           (*(volatile uint8_t *)(MCF_UART1_BASE + 0x34))
#define MCF_UART1_UOP1          (*(volatile uint8_t *)(MCF_UART1_BASE + 0x38))
#define MCF_UART1_UOP0          (*(volatile uint8_t *)(MCF_UART1_BASE + 0x3C))

/*********************************************************************
 * UART2 Register definitions
 *********************************************************************/

#define MCF_UART2_UMR           (*(volatile uint8_t *)(MCF_UART2_BASE + 0x00))
#define MCF_UART2_USR           (*(volatile uint8_t *)(MCF_UART2_BASE + 0x04))
#define MCF_UART2_UCSR          (*(volatile uint8_t *)(MCF_UART2_BASE + 0x04))
#define MCF_UART2_UCR           (*(volatile uint8_t *)(MCF_UART2_BASE + 0x08))
#define MCF_UART2_URB           (*(volatile uint8_t *)(MCF_UART2_BASE + 0x0C))
#define MCF_UART2_UTB           (*(volatile uint8_t *)(MCF_UART2_BASE + 0x0C))
#define MCF_UART2_UIPCR         (*(volatile uint8_t *)(MCF_UART2_BASE + 0x10))
#define MCF_UART2_UACR          (*(volatile uint8_t *)(MCF_UART2_BASE + 0x10))
#define MCF_UART2_UISR          (*(volatile uint8_t *)(MCF_UART2_BASE + 0x14))
#define MCF_UART2_UIMR          (*(volatile uint8_t *)(MCF_UART2_BASE + 0x14))
#define MCF_UART2_UBG1          (*(volatile uint8_t *)(MCF_UART2_BASE + 0x18))
#define MCF_UART2_UBG2          (*(volatile uint8_t *)(MCF_UART2_BASE + 0x1C))
#define MCF_UART2_UIP           (*(volatile uint8_t *)(MCF_UART2_BASE + 0x34))
#define MCF_UART2_UOP1          (*(volatile uint8_t *)(MCF_UART2_BASE + 0x38))
#define MCF_UART2_UOP0          (*(volatile uint8_t *)(MCF_UART2_BASE + 0x3C))

/*********************************************************************
 * Generic UART register macros (n = 0, 1, or 2)
 *********************************************************************/

#define MCF_UART_UMR(n)         (*(volatile uint8_t *)(MCF_UART_BASE(n) + 0x00))
#define MCF_UART_USR(n)         (*(volatile uint8_t *)(MCF_UART_BASE(n) + 0x04))
#define MCF_UART_UCSR(n)        (*(volatile uint8_t *)(MCF_UART_BASE(n) + 0x04))
#define MCF_UART_UCR(n)         (*(volatile uint8_t *)(MCF_UART_BASE(n) + 0x08))
#define MCF_UART_URB(n)         (*(volatile uint8_t *)(MCF_UART_BASE(n) + 0x0C))
#define MCF_UART_UTB(n)         (*(volatile uint8_t *)(MCF_UART_BASE(n) + 0x0C))
#define MCF_UART_UIPCR(n)       (*(volatile uint8_t *)(MCF_UART_BASE(n) + 0x10))
#define MCF_UART_UACR(n)        (*(volatile uint8_t *)(MCF_UART_BASE(n) + 0x10))
#define MCF_UART_UISR(n)        (*(volatile uint8_t *)(MCF_UART_BASE(n) + 0x14))
#define MCF_UART_UIMR(n)        (*(volatile uint8_t *)(MCF_UART_BASE(n) + 0x14))
#define MCF_UART_UBG1(n)        (*(volatile uint8_t *)(MCF_UART_BASE(n) + 0x18))
#define MCF_UART_UBG2(n)        (*(volatile uint8_t *)(MCF_UART_BASE(n) + 0x1C))
#define MCF_UART_UIP(n)         (*(volatile uint8_t *)(MCF_UART_BASE(n) + 0x34))
#define MCF_UART_UOP1(n)        (*(volatile uint8_t *)(MCF_UART_BASE(n) + 0x38))
#define MCF_UART_UOP0(n)        (*(volatile uint8_t *)(MCF_UART_BASE(n) + 0x3C))

/*********************************************************************
 * UMR1 bit definitions - Mode Register 1 (after reset or RESET_MR)
 *********************************************************************/

#define MCF_UART_UMR1_BC(x)         (((x) & 0x03) << 0)  /* Bits per character */
#define MCF_UART_UMR1_BC_5          (0x00)
#define MCF_UART_UMR1_BC_6          (0x01)
#define MCF_UART_UMR1_BC_7          (0x02)
#define MCF_UART_UMR1_BC_8          (0x03)

#define MCF_UART_UMR1_PT            (0x04)              /* Parity type (0=even, 1=odd) */
#define MCF_UART_UMR1_PM(x)         (((x) & 0x03) << 3) /* Parity mode */
#define MCF_UART_UMR1_PM_EVEN       (0x00)
#define MCF_UART_UMR1_PM_ODD        (0x04)
#define MCF_UART_UMR1_PM_FORCE_LO   (0x08)
#define MCF_UART_UMR1_PM_FORCE_HI   (0x0C)
#define MCF_UART_UMR1_PM_NONE       (0x10)
#define MCF_UART_UMR1_PM_MULTI_DATA (0x18)
#define MCF_UART_UMR1_PM_MULTI_ADDR (0x1C)

#define MCF_UART_UMR1_ERR           (0x20)              /* Error mode */
#define MCF_UART_UMR1_RXIRQ         (0x40)              /* Receiver interrupt select */
#define MCF_UART_UMR1_RXRTS         (0x80)              /* Receiver RTS control */

/*********************************************************************
 * UMR2 bit definitions - Mode Register 2 (after first write to UMR)
 *********************************************************************/

#define MCF_UART_UMR2_SB(x)         (((x) & 0x0F) << 0) /* Stop bits */
#define MCF_UART_UMR2_SB_1          (0x07)              /* 1 stop bit */
#define MCF_UART_UMR2_SB_15         (0x08)              /* 1.5 stop bits */
#define MCF_UART_UMR2_SB_2          (0x0F)              /* 2 stop bits */

#define MCF_UART_UMR2_TXCTS         (0x10)              /* Transmitter CTS control */
#define MCF_UART_UMR2_TXRTS         (0x20)              /* Transmitter RTS control */
#define MCF_UART_UMR2_CM(x)         (((x) & 0x03) << 6) /* Channel mode */
#define MCF_UART_UMR2_CM_NORMAL     (0x00)
#define MCF_UART_UMR2_CM_ECHO       (0x40)
#define MCF_UART_UMR2_CM_LOCAL_LOOP (0x80)
#define MCF_UART_UMR2_CM_REMOTE_LOOP (0xC0)

/*********************************************************************
 * USR bit definitions - Status Register (read)
 *********************************************************************/

#define MCF_UART_USR_RXRDY          (0x01)  /* Receiver ready */
#define MCF_UART_USR_FFULL          (0x02)  /* FIFO full */
#define MCF_UART_USR_TXRDY          (0x04)  /* Transmitter ready */
#define MCF_UART_USR_TXEMP          (0x08)  /* Transmitter empty */
#define MCF_UART_USR_OE             (0x10)  /* Overrun error */
#define MCF_UART_USR_PE             (0x20)  /* Parity error */
#define MCF_UART_USR_FE             (0x40)  /* Framing error */
#define MCF_UART_USR_RB             (0x80)  /* Received break */

/*********************************************************************
 * UCSR bit definitions - Clock Select Register (write)
 *********************************************************************/

#define MCF_UART_UCSR_TCS(x)        (((x) & 0x0F) << 0) /* Transmitter clock select */
#define MCF_UART_UCSR_RCS(x)        (((x) & 0x0F) << 4) /* Receiver clock select */
#define MCF_UART_UCSR_TCS_SYS_CLK   (0x0D)
#define MCF_UART_UCSR_TCS_CTM16     (0x0E)
#define MCF_UART_UCSR_TCS_CTM       (0x0F)
#define MCF_UART_UCSR_RCS_SYS_CLK   (0xD0)
#define MCF_UART_UCSR_RCS_CTM16     (0xE0)
#define MCF_UART_UCSR_RCS_CTM       (0xF0)

/*********************************************************************
 * UCR bit definitions - Command Register (write)
 *********************************************************************/

#define MCF_UART_UCR_RXC(x)         (((x) & 0x03) << 0) /* Receiver command */
#define MCF_UART_UCR_RX_ENABLED     (0x01)
#define MCF_UART_UCR_RX_DISABLED    (0x02)

#define MCF_UART_UCR_TXC(x)         (((x) & 0x03) << 2) /* Transmitter command */
#define MCF_UART_UCR_TX_ENABLED     (0x04)
#define MCF_UART_UCR_TX_DISABLED    (0x08)

#define MCF_UART_UCR_MISC(x)        (((x) & 0x07) << 4) /* Miscellaneous command */
#define MCF_UART_UCR_NONE           (0x00)
#define MCF_UART_UCR_RESET_MR       (0x10)              /* Reset mode register pointer */
#define MCF_UART_UCR_RESET_RX       (0x20)              /* Reset receiver */
#define MCF_UART_UCR_RESET_TX       (0x30)              /* Reset transmitter */
#define MCF_UART_UCR_RESET_ERROR    (0x40)              /* Reset error status */
#define MCF_UART_UCR_BKCHGINT       (0x50)              /* Reset break change interrupt */
#define MCF_UART_UCR_START_BREAK    (0x60)              /* Start break */
#define MCF_UART_UCR_STOP_BREAK     (0x70)              /* Stop break */

/*********************************************************************
 * UISR/UIMR bit definitions - Interrupt Status/Mask Register
 *********************************************************************/

#define MCF_UART_UISR_TXRDY         (0x01)  /* Transmitter ready */
#define MCF_UART_UISR_RXRDY_FU      (0x02)  /* Receiver ready/FIFO full */
#define MCF_UART_UISR_DB            (0x04)  /* Delta break */
#define MCF_UART_UISR_RXFTO         (0x08)  /* Receiver FIFO timeout */
#define MCF_UART_UISR_TXFIFO        (0x10)  /* Transmitter FIFO empty */
#define MCF_UART_UISR_RXFIFO        (0x20)  /* Receiver FIFO full */
#define MCF_UART_UISR_COS           (0x80)  /* Change of state */

#define MCF_UART_UIMR_TXRDY         (0x01)
#define MCF_UART_UIMR_RXRDY_FU      (0x02)
#define MCF_UART_UIMR_DB            (0x04)
#define MCF_UART_UIMR_COS           (0x80)

/*********************************************************************
 * UIP bit definitions - Input Port Register
 *********************************************************************/

#define MCF_UART_UIP_CTS            (0x01)  /* CTS input state */

/*********************************************************************
 * UOP bit definitions - Output Port Registers
 *********************************************************************/

#define MCF_UART_UOP_RTS            (0x01)  /* RTS output */

/*********************************************************************
 * Baud rate calculation
 *
 * Baud Rate = System Clock / (32 * Divisor)
 * Divisor = (UBG1 << 8) | UBG2
 *
 * For 60MHz system clock and 115200 baud:
 *   Divisor = 60000000 / (32 * 115200) = 16.28 ≈ 16
 *   UBG1 = 0, UBG2 = 16
 *********************************************************************/

#define MCF_UART_CALC_BAUD(sysclk, baud) ((sysclk) / (32 * (baud)))

#endif /* MCF5223_UART_H */
