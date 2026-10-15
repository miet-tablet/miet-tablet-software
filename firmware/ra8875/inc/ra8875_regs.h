#pragma once

/* 4-wire SPI: the first byte of every transfer selects the cycle type */
#define RA8875_SPI_DATA_WRITE   0x00
#define RA8875_SPI_DATA_READ    0x40
#define RA8875_SPI_CMD_WRITE    0x80
#define RA8875_SPI_STATUS_READ  0xC0

/* Status register (RA8875_SPI_STATUS_READ) */
#define RA8875_STATUS_MEM_BUSY  0x80

/* REG[00h] always reads back 0x75 on RA8875, used as a presence check */
#define RA8875_REG_ID           0x00
#define RA8875_CHIP_ID          0x75

/* Registers ending in 0 hold the low byte of a 16-bit value, the next one the high byte */

#define RA8875_PWRR             0x01
#define RA8875_PWRR_NORMAL      0x00
#define RA8875_PWRR_DISPON      0x80
#define RA8875_PWRR_DISPOFF     0x00

#define RA8875_MRWC             0x02

#define RA8875_PCSR             0x04
#define RA8875_PCSR_PDATL       0x80
#define RA8875_PCSR_2CLK        0x01

#define RA8875_SYSR             0x10
#define RA8875_SYSR_16BPP       0x0C
#define RA8875_SYSR_MCU8        0x00

/* Panel timing */
#define RA8875_HDWR             0x14
#define RA8875_HNDFTR           0x15
#define RA8875_HNDFTR_DE_HIGH   0x00
#define RA8875_HNDR             0x16
#define RA8875_HSTR             0x17
#define RA8875_HPWR             0x18
#define RA8875_HPWR_LOW         0x00
#define RA8875_VDHR0            0x19
#define RA8875_VNDR0            0x1B
#define RA8875_VSTR0            0x1D
#define RA8875_VPWR             0x1F
#define RA8875_VPWR_LOW         0x00

/* Text: font control and cursor */
#define RA8875_FNCR0            0x21
#define RA8875_FNCR0_CGRAM      0x80
#define RA8875_FNCR0_EXT_FONT   0x20
#define RA8875_FNCR1            0x22
#define RA8875_FNCR1_TRANSPARENT 0x40
#define RA8875_FNCR1_SCALE_MASK 0x0F
#define RA8875_F_CURX0          0x2A
#define RA8875_F_CURY0          0x2C

/* Active window */
#define RA8875_HSAW0            0x30
#define RA8875_VSAW0            0x32
#define RA8875_HEAW0            0x34
#define RA8875_VEAW0            0x36

#define RA8875_MWCR0            0x40
#define RA8875_MWCR0_TXTMODE    0x80

/* Memory write cursor */
#define RA8875_CURH0            0x46
#define RA8875_CURV0            0x48

/* Background and foreground colour, R/G/B in three consecutive registers */
#define RA8875_BGCR0            0x60
#define RA8875_FGCR0            0x63

#define RA8875_PLLC1            0x88
#define RA8875_PLLC1_PLLDIV1    0x00
#define RA8875_PLLC2            0x89
#define RA8875_PLLC2_DIV4       0x02

/* PWM1 drives the backlight */
#define RA8875_P1CR             0x8A
#define RA8875_P1CR_ENABLE      0x80
#define RA8875_PWM_CLK_DIV1024  0x0A
#define RA8875_P1DCR            0x8B

#define RA8875_MCLR             0x8E
#define RA8875_MCLR_START       0x80
#define RA8875_MCLR_FULL        0x00

/* Drawing engine */
#define RA8875_DCR                    0x90
#define RA8875_DCR_LINESQUTRI_START   0x80
#define RA8875_DCR_LINESQUTRI_STATUS  0x80
#define RA8875_DCR_CIRCLE_START       0x40
#define RA8875_DCR_CIRCLE_STATUS      0x40
#define RA8875_DCR_FILL               0x20
#define RA8875_DCR_DRAWSQUARE         0x10
#define RA8875_DCR_DRAWTRIANGLE       0x01
#define RA8875_DCR_DRAWLINE           0x00

/* Shape points: line/rectangle/triangle start and end, circle centre, radius */
#define RA8875_DLHSR0           0x91
#define RA8875_DLVSR0           0x93
#define RA8875_DLHER0           0x95
#define RA8875_DLVER0           0x97
#define RA8875_DCHR0            0x99
#define RA8875_DCVR0            0x9B
#define RA8875_DCRR             0x9D
#define RA8875_DTPH0            0xA9
#define RA8875_DTPV0            0xAB

#define RA8875_GPIOX            0xC7
