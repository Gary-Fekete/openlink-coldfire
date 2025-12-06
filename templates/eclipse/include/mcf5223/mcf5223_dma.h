/*
 * File:    mcf5223_dma.h
 * Purpose: Register and bit definitions for the DMA Controller Module
 *
 * Based on MCF52235 Reference Manual, Chapter 20
 */

#ifndef __MCF5223_DMA_H__
#define __MCF5223_DMA_H__

#include <stdint.h>

/*********************************************************************
*
* DMA Controller Module (DMA)
*
*********************************************************************/


/* Register read/write macros */
#define MCF_DMA_DMAREQC            (*(volatile uint32_t *)(IPSBAR + 0x000014))
#define MCF_DMA_SAR0               (*(volatile uint32_t *)(IPSBAR + 0x000100))
#define MCF_DMA_SAR1               (*(volatile uint32_t *)(IPSBAR + 0x000110))
#define MCF_DMA_SAR2               (*(volatile uint32_t *)(IPSBAR + 0x000120))
#define MCF_DMA_SAR3               (*(volatile uint32_t *)(IPSBAR + 0x000130))
#define MCF_DMA_SAR(x)             (*(volatile uint32_t *)(IPSBAR + 0x000100 + ( (x)*0x10 ) ))
#define MCF_DMA_DAR0               (*(volatile uint32_t *)(IPSBAR + 0x000104))
#define MCF_DMA_DAR1               (*(volatile uint32_t *)(IPSBAR + 0x000114))
#define MCF_DMA_DAR2               (*(volatile uint32_t *)(IPSBAR + 0x000124))
#define MCF_DMA_DAR3               (*(volatile uint32_t *)(IPSBAR + 0x000134))
#define MCF_DMA_DAR(x)             (*(volatile uint32_t *)(IPSBAR + 0x000104 + ( (x)*0x10 ) ))
#define MCF_DMA_DSR0               (*(volatile uint8_t *)(IPSBAR + 0x000108))
#define MCF_DMA_DSR1               (*(volatile uint8_t *)(IPSBAR + 0x000118))
#define MCF_DMA_DSR2               (*(volatile uint8_t *)(IPSBAR + 0x000128))
#define MCF_DMA_DSR3               (*(volatile uint8_t *)(IPSBAR + 0x000138))
#define MCF_DMA_DSR(x)             (*(volatile uint8_t *)(IPSBAR + 0x000108 + ( (x)*0x10 ) ))
#define MCF_DMA_BCR0               (*(volatile uint32_t *)(IPSBAR + 0x000108))
#define MCF_DMA_BCR1               (*(volatile uint32_t *)(IPSBAR + 0x000118))
#define MCF_DMA_BCR2               (*(volatile uint32_t *)(IPSBAR + 0x000128))
#define MCF_DMA_BCR3               (*(volatile uint32_t *)(IPSBAR + 0x000138))
#define MCF_DMA_BCR(x)             (*(volatile uint32_t *)(IPSBAR + 0x000108 + ( (x)*0x10 ) ))
#define MCF_DMA_DCR0               (*(volatile uint32_t *)(IPSBAR + 0x00010C))
#define MCF_DMA_DCR1               (*(volatile uint32_t *)(IPSBAR + 0x00011C))
#define MCF_DMA_DCR2               (*(volatile uint32_t *)(IPSBAR + 0x00012C))
#define MCF_DMA_DCR3               (*(volatile uint32_t *)(IPSBAR + 0x00013C))
#define MCF_DMA_DCR(x)             (*(volatile uint32_t *)(IPSBAR + 0x00010C + ( (x)*0x10 ) ))


/* Bit definitions and macros for MCF_DMA_DMAREQC */
#define MCF_DMA_DMAREQC_DMAC0(x)        (((x)&0x0000000F)<<0)
#define MCF_DMA_DMAREQC_DMAC1(x)        (((x)&0x0000000F)<<4)
#define MCF_DMA_DMAREQC_DMAC2(x)        (((x)&0x0000000F)<<8)
#define MCF_DMA_DMAREQC_DMAC3(x)        (((x)&0x0000000F)<<12)
#define MCF_DMA_DMAREQC_DMAREQC_EXT(x)  (((x)&0x0000000F)<<16)

/* Bit definitions and macros for MCF_DMA_SAR */
#define MCF_DMA_SAR_SAR(x)              (((x)&0xFFFFFFFF)<<0)

/* Bit definitions and macros for MCF_DMA_DAR */
#define MCF_DMA_DAR_DAR(x)              (((x)&0xFFFFFFFF)<<0)

/* Bit definitions and macros for MCF_DMA_DSR */
#define MCF_DMA_DSR_DONE                (0x01)
#define MCF_DMA_DSR_BSY                 (0x02)
#define MCF_DMA_DSR_REQ                 (0x04)
#define MCF_DMA_DSR_BED                 (0x10)
#define MCF_DMA_DSR_BES                 (0x20)
#define MCF_DMA_DSR_CE                  (0x40)

/* Bit definitions and macros for MCF_DMA_BCR/DSR (located in the same 32-bit register) */
#define MCF_DMA_BCR_BCR(x)              (((x)&0x00FFFFFF)<<0)
#define MCF_DMA_BCR_DSR(x)              (((x)&0x000000FF)<<24)

/* Bit definitions and macros for MCF_DMA_DCR */
#define MCF_DMA_DCR_LCH2(x)             (((x)&0x00000003)<<0)
#define MCF_DMA_DCR_LCH1(x)             (((x)&0x00000003)<<2)
#define MCF_DMA_DCR_LINKCC(x)           (((x)&0x00000003)<<4)
#define MCF_DMA_DCR_D_REQ               (0x00000080)
#define MCF_DMA_DCR_DMOD(x)             (((x)&0x0000000F)<<8)
#define MCF_DMA_DCR_SMOD(x)             (((x)&0x0000000F)<<12)
#define MCF_DMA_DCR_START               (0x00010000)
#define MCF_DMA_DCR_DSIZE(x)            (((x)&0x00000003)<<17)
#define MCF_DMA_DCR_DINC                (0x00080000)
#define MCF_DMA_DCR_SSIZE(x)            (((x)&0x00000003)<<20)
#define MCF_DMA_DCR_SINC                (0x00400000)
#define MCF_DMA_DCR_BWC(x)              (((x)&0x00000007)<<25)
#define MCF_DMA_DCR_AA                  (0x10000000)
#define MCF_DMA_DCR_CS                  (0x20000000)
#define MCF_DMA_DCR_EEXT                (0x40000000)
#define MCF_DMA_DCR_INT                 (0x80000000)
#define MCF_DMA_DCR_BWC_16K             (0x1)
#define MCF_DMA_DCR_BWC_32K             (0x2)
#define MCF_DMA_DCR_BWC_64K             (0x3)
#define MCF_DMA_DCR_BWC_128K            (0x4)
#define MCF_DMA_DCR_BWC_256K            (0x5)
#define MCF_DMA_DCR_BWC_512K            (0x6)
#define MCF_DMA_DCR_BWC_1024K           (0x7)
#define MCF_DMA_DCR_DMOD_DIS            (0x0)
#define MCF_DMA_DCR_DMOD_16             (0x1)
#define MCF_DMA_DCR_DMOD_32             (0x2)
#define MCF_DMA_DCR_DMOD_64             (0x3)
#define MCF_DMA_DCR_DMOD_128            (0x4)
#define MCF_DMA_DCR_DMOD_256            (0x5)
#define MCF_DMA_DCR_DMOD_512            (0x6)
#define MCF_DMA_DCR_DMOD_1K             (0x7)
#define MCF_DMA_DCR_DMOD_2K             (0x8)
#define MCF_DMA_DCR_DMOD_4K             (0x9)
#define MCF_DMA_DCR_DMOD_8K             (0xA)
#define MCF_DMA_DCR_DMOD_16K            (0xB)
#define MCF_DMA_DCR_DMOD_32K            (0xC)
#define MCF_DMA_DCR_DMOD_64K            (0xD)
#define MCF_DMA_DCR_DMOD_128K           (0xE)
#define MCF_DMA_DCR_DMOD_256K           (0xF)
#define MCF_DMA_DCR_SMOD_DIS            (0x0)
#define MCF_DMA_DCR_SMOD_16             (0x1)
#define MCF_DMA_DCR_SMOD_32             (0x2)
#define MCF_DMA_DCR_SMOD_64             (0x3)
#define MCF_DMA_DCR_SMOD_128            (0x4)
#define MCF_DMA_DCR_SMOD_256            (0x5)
#define MCF_DMA_DCR_SMOD_512            (0x6)
#define MCF_DMA_DCR_SMOD_1K             (0x7)
#define MCF_DMA_DCR_SMOD_2K             (0x8)
#define MCF_DMA_DCR_SMOD_4K             (0x9)
#define MCF_DMA_DCR_SMOD_8K             (0xA)
#define MCF_DMA_DCR_SMOD_16K            (0xB)
#define MCF_DMA_DCR_SMOD_32K            (0xC)
#define MCF_DMA_DCR_SMOD_64K            (0xD)
#define MCF_DMA_DCR_SMOD_128K           (0xE)
#define MCF_DMA_DCR_SMOD_256K           (0xF)
#define MCF_DMA_DCR_SSIZE_LONG          (0x0)
#define MCF_DMA_DCR_SSIZE_BYTE          (0x1)
#define MCF_DMA_DCR_SSIZE_WORD          (0x2)
#define MCF_DMA_DCR_SSIZE_LINE          (0x3)
#define MCF_DMA_DCR_DSIZE_LONG          (0x0)
#define MCF_DMA_DCR_DSIZE_BYTE          (0x1)
#define MCF_DMA_DCR_DSIZE_WORD          (0x2)
#define MCF_DMA_DCR_DSIZE_LINE          (0x3)
#define MCF_DMA_DCR_LCH1_CH0            (0x0)
#define MCF_DMA_DCR_LCH1_CH1            (0x1)
#define MCF_DMA_DCR_LCH1_CH2            (0x2)
#define MCF_DMA_DCR_LCH1_CH3            (0x3)
#define MCF_DMA_DCR_LCH2_CH0            (0x0)
#define MCF_DMA_DCR_LCH2_CH1            (0x1)
#define MCF_DMA_DCR_LCH2_CH2            (0x2)
#define MCF_DMA_DCR_LCH2_CH3            (0x3)

/********************************************************************/

#endif /* __MCF5223_DMA_H__ */
