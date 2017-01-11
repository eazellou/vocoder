/*  File name: C5505.h


    11Nov2009 .. initial version .. KMetzger
    14Nov2009 .. added I2S1 .. KM
    15Nov2010 .. added UART .. KM
    11Feb2011 .. more UART additions .. KM
    13Mar2011 .. added I2S .. KM
	27Dec2014 .. more UART work .. KM

*/

#ifndef C5505h

#define C5505h


// Idle Control, Status and System Registers

#define SPICG     0x0002
#define UARTCG    0x0004
#define I2S0CG    0x0100
#define I2S1CG    0x0200
#define I2S2CG    0x0400

#define EBSR      *((volatile ioport unsigned *)0x1C00)
#define PCGCR1    *((volatile ioport unsigned *)0x1C02)
#define PCGCR2    *((volatile ioport unsigned *)0x1C03)
#define PSRCR 	  *((volatile ioport unsigned *)0x1C04)
#define PRCR	  *((volatile ioport unsigned *)0x1C05)

// Pull-up/Pulldown inhibit registers

#define PDINHIBR1   *((volatile ioport unsigned *)0x1C17)
#define PDINHIBR2   *((volatile ioport unsigned *)0x1C18)
#define PDINHIBR3   *((volatile ioport unsigned *)0x1C19)

// Clock stop register

#define CLKSTOP    *((volatile ioport unsigned *)0x1C3A)
#define URTCLKSTPREQ  0x0010
#define URTCLKSTPACK  0x0200

// Interrupt support registers

#define IER0 *((volatile unsigned *)0x00000000)
#define IFR0 *((volatile unsigned *)0x00000001)
#define ST0_55 *((volatile unsigned *)0x00000002)
#define ST1_55 *((volatile unsigned *)0x00000003)
#define ST2_55 *((volatile unsigned *)0x0000004B)
#define ST3_55 *((volatile unsigned *)0x00000004)
#define IVPD *((volatile unsigned *)0x00000049)
#define IVPH *((volatile unsigned *)0x0000004A)

#define IER1 *((volatile unsigned *)0x00000045)
#define IFR1 *((volatile unsigned *)0x00000046)

// Interrupt vector offsets

#define INT0_VEC   0x00000010
#define INT1_VEC   0x00000018
#define TINT       0x00000020
#define PROG0_VEC  0x00000028
#define UART_VEC   0x00000030
#define PROG1_VEC  0x00000038
#define DMA_VEC    0x00000040
#define PROG2_VEC  0x00000048
#define PROG3_VEC  0x00000058
#define LCD_VEC    0x00000060
#define SAR_VEC    0x00000068
#define XMT2_VEC   0x00000070
#define RCV2_VEC   0x00000078
#define XMT3_VEC   0x00000080
#define RCV3_VEC   0x00000088
#define RTC_VEC    0x00000090
#define SPI_VEC    0x00000098
#define USB_VEC    0x000000A0
#define GPIO_VEC   0x000000A8
#define EMIF_VEC   0x000000B0
#define I2C_VEC    0x000000B8


// IFR0 and IER0 definitions

#define RCV2_B  0x8000
#define XMT2_B  0x4000
#define SAR_B   0x2000
#define LCD_B   0x1000
#define PROG3_B 0x0800
#define PROG2_B 0x0200
#define DMA_B   0x0100
#define PROG1_B 0x0080
#define UART_B  0x0040
#define PROG0_B 0x0020
#define TINT_B  0x0010
#define INT1_B  0x0008
#define INT0_B  0x0004

// IFR1 and IER1 definitions

#define RTOS_B 0x0400
#define DLOG_B 0x0200
#define BERR_B 0x0100
#define I2C_B  0x0080
#define EMIF_B 0x0040
#define GPIO_B 0x0020
#define USB_B  0x0010
#define SPI_B  0x0008
#define RTC_B  0x0004
#define RCV3_B 0x0002
#define XMT3_B 0x0001


// SPI definitions

#define SPICDR    *((volatile ioport unsigned *)0x3000)
#define SPICCR    *((volatile ioport unsigned *)0x3001)
#define SPIDCR1   *((volatile ioport unsigned *)0x3002)
#define SPIDCR2   *((volatile ioport unsigned *)0x3003)
#define SPICMD1   *((volatile ioport unsigned *)0x3004)
#define SPICMD2   *((volatile ioport unsigned *)0x3005)
#define SPISTAT1  *((volatile ioport unsigned *)0x3006)
#define SPISTAT2  *((volatile ioport unsigned *)0x3007)
#define SPIDAT1   *((volatile ioport unsigned *)0x3008)
#define SPIDAT2   *((volatile ioport unsigned *)0x3009)

#define SPI_RST      0x4000
#define SPI_CLKEN    0x8000 

// GPIO definitions

#define IODIR1      *((volatile ioport unsigned *)0x1C06)
#define IODIR2      *((volatile ioport unsigned *)0x1C07)
#define IOINDATA1   *((volatile ioport unsigned *)0x1C08)
#define IOINDATA2   *((volatile ioport unsigned *)0x1C09)
#define IODATAOUT1  *((volatile ioport unsigned *)0x1C0A)
#define IODATAOUT2  *((volatile ioport unsigned *)0x1C0B)
#define IOINTEDG1   *((volatile ioport unsigned *)0x1C0C)
#define IOINTEDG2   *((volatile ioport unsigned *)0x1C0D)
#define IOINTEN1    *((volatile ioport unsigned *)0x1C0E)
#define IOINTEN2    *((volatile ioport unsigned *)0x1C0F)
#define IOINTFLG1   *((volatile ioport unsigned *)0x1C10)
#define IOINTFLG2   *((volatile ioport unsigned *)0x1C11)

// I2S0 definitions

#define I2S0SCTRL    *((volatile ioport unsigned *)0x2800)
#define I2S0SRATE    *((volatile ioport unsigned *)0x2804)
#define I2S0TXLT0    *((volatile ioport unsigned *)0x2808)
#define I2S0TXLT1    *((volatile ioport unsigned *)0x2809)
#define I2S0TXRT0    *((volatile ioport unsigned *)0x280C)
#define I2S0TXRT1    *((volatile ioport unsigned *)0x280D)
#define I2S0INTFL    *((volatile ioport unsigned *)0x2810)
#define I2S0INTMASK  *((volatile ioport unsigned *)0x2814)
#define I2S0RXLT0    *((volatile ioport unsigned *)0x2828)
#define I2S0RXLT1    *((volatile ioport unsigned *)0x2829)
#define I2S0RXRT0    *((volatile ioport unsigned *)0x282C)
#define I2S0RXRT1    *((volatile ioport unsigned *)0x282D)

// I2S1 definitions

#define I2S1SCTRL    *((volatile ioport unsigned *)0x2900)
#define I2S1SRATE    *((volatile ioport unsigned *)0x2904)
#define I2S1TXLT0    *((volatile ioport unsigned *)0x2908)
#define I2S1TXLT1    *((volatile ioport unsigned *)0x2909)
#define I2S1TXRT0    *((volatile ioport unsigned *)0x290C)
#define I2S1TXRT1    *((volatile ioport unsigned *)0x290D)
#define I2S1INTFL    *((volatile ioport unsigned *)0x2910)
#define I2S1INTMASK  *((volatile ioport unsigned *)0x2914)
#define I2S1RXLT0    *((volatile ioport unsigned *)0x2928)
#define I2S1RXLT1    *((volatile ioport unsigned *)0x2929)
#define I2S1RXRT0    *((volatile ioport unsigned *)0x292C)
#define I2S1RXRT1    *((volatile ioport unsigned *)0x292D)

// I2S2 definitions

#define I2S2SCTRL    *((volatile ioport unsigned *)0x2A00)
#define I2S2SRATE    *((volatile ioport unsigned *)0x2A04)
#define I2S2TXLT0    *((volatile ioport unsigned *)0x2A08)
#define I2S2TXLT1    *((volatile ioport unsigned *)0x2A09)
#define I2S2TXRT0    *((volatile ioport unsigned *)0x2A0C)
#define I2S2TXRT1    *((volatile ioport unsigned *)0x2A0D)
#define I2S2INTFL    *((volatile ioport unsigned *)0x2A10)
#define I2S2INTMASK  *((volatile ioport unsigned *)0x2A14)
#define I2S2RXLT0    *((volatile ioport unsigned *)0x2A28)
#define I2S2RXLT1    *((volatile ioport unsigned *)0x2A29)
#define I2S2RXRT0    *((volatile ioport unsigned *)0x2A2C)
#define I2S2RXRT1    *((volatile ioport unsigned *)0x2A2D)

#define I2SENABLE     0x8000
#define I2SMONO       0x1000
#define I2SFSPOL      0x0400
#define I2SCLKPOL     0x0200
#define I2SWDLENGTH8  0x0000
#define I2SWDLENGTH16 0x0010
#define I2SMODE       0x0002
#define I2SFRMT       0x0001
#define I2SXMITMONFL  0x0010
#define I2SXMITSTFL   0x0020
#define I2SRCVMONFL   0x0004
#define I2SDATADLY    0x0100

// UART definitions

#define RBR          *((volatile ioport unsigned *)0x1B00)
#define THR          *((volatile ioport unsigned *)0x1B00)
#define IER          *((volatile ioport unsigned *)0x1B02)
#define IIR          *((volatile ioport unsigned *)0x1B04)
#define FCR          *((volatile ioport unsigned *)0x1B04)
#define LCR          *((volatile ioport unsigned *)0x1B06)
#define MCR          *((volatile ioport unsigned *)0x1B08)
#define LSR          *((volatile ioport unsigned *)0x1B0A)
#define SCR          *((volatile ioport unsigned *)0x1B0E)
#define DLL          *((volatile ioport unsigned *)0x1B10)
#define DLH          *((volatile ioport unsigned *)0x1B12)
#define PWREMU_MGMT  *((volatile ioport unsigned *)0x1B18)
#define UART_BIT		(1<<6)

// Assumes use of 100 MHz clock.

#define BAUD2400    2604
#define BAUD4800    1302
#define BAUD9600     651
#define BAUD19200    326
#define BAUD38400    163
#define BAUD56000    112
#define BAUD128000    49

#endif

