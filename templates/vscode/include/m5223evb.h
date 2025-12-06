/*
 * File:		m5223evb.h
 * Purpose:		Evaluation board definitions and memory map information
 *
 * Notes:
 */

#ifndef _M5271EVB_H
#define _M5271EVB_H

/********************************************************************/

#include "mcf5xxx.h"
/*#include "io.h"*/

/********************************************************************/

/*
 * Debug prints ON (#undef) or OFF (#define)
 */
#undef DEBUG

/* 
 * System Bus Clock Info 
 */
#define	SYSTEM_CLOCK			60		/* system bus frequency in MHz */
#define PERIOD					20	/* system bus period in ns */
#define UART_BAUD				115200	/*  19200*/
#define TERMINAL_PORT			0

/*
 * LED Info
 */
//#undef HAS_LEDS
#define HAS_LEDS 1

/*
 * Ethernet Port Info
 */
#define FEC_PHY0            (0x00)

/*
 * Memory Map Info
 * Note: Using fixed addresses instead of linker symbols for compatibility
 * with OpenLink ColdFire headers.
 */
#ifndef IPSBAR
#define IPSBAR                  0x40000000
#endif
#define IPSBAR_ADDRESS          IPSBAR

#define SRAM_ADDRESS            0x20000000
#define SRAM_SIZE               0x00008000  /* 32KB */

/*
 *	Interrupt Controller Definitions
 */
#define TIMER_NETWORK_LEVEL		3
#define FEC_LEVEL				4

/*
 *	Timer period info
 */
#define TIMER_NETWORK_PERIOD	1000000000/0x10000	/* 1 sec / max timeout */

/********************************************************************/
/********************************************************************/
/********************************************************************/
#ifdef HAS_LEDS /* { */

	#define LED0_TOGGLE     MCF_GPIO_PORTTC = (uint8)(MCF_GPIO_PORTTC ^ MCF_GPIO_PORTTC_PORTTC0)
	#define LED1_TOGGLE     MCF_GPIO_PORTTC = (uint8)(MCF_GPIO_PORTTC ^ MCF_GPIO_PORTTC_PORTTC1);
	#define LED2_TOGGLE     MCF_GPIO_PORTTC = (uint8)(MCF_GPIO_PORTTC ^ MCF_GPIO_PORTTC_PORTTC2);
	#define LED3_TOGGLE     MCF_GPIO_PORTTC = (uint8)(MCF_GPIO_PORTTC ^ MCF_GPIO_PORTTC_PORTTC3);

	#define LED_INIT()		Leds_Init()
#else  /* No LEDS  */
	#define LED_INIT()		void()
#endif
	#define RXLED_TOGGLE     MCF_GPIO_PORTLD = (uint8)(MCF_GPIO_PORTLD ^ MCF_GPIO_PORTLD_PORTLD5);
	#define TXLED_TOGGLE     MCF_GPIO_PORTLD = (uint8)(MCF_GPIO_PORTLD ^ MCF_GPIO_PORTLD_PORTLD6);
	#define ACTLED_TOGGLE    MCF_GPIO_PORTLD = (uint8)(MCF_GPIO_PORTLD ^ MCF_GPIO_PORTLD_PORTLD0);
/********************************************************************/
/********************************************************************/
//void Leds_Init();
//void board_led_display(uint8 number);
/********************************************************************/
/********************************************************************/
#endif /* _M5271EVB_H */
