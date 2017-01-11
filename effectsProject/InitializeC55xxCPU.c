// File name: InitializeC55xxCPU.c
//
// Replaces GEL setup for C5505 and C5515.   Initializes:
//
// 		the CPU clock/PLL,
//		the port signals available on processor,
//		peripheral resets,
//		clocks to all of the peripherals.
//
//	Caveats:  	Does not configure any peripherals,
//
//				All peripheral clocks are turned on, this
//				might not be desired if battery life is being
//				maximized.
//
//				The default is to set the cpu clock t0 100 MHz.
//				This is the max for the C5505 and the rate used
//				by CCS.  The C5515 can be operated using a clock
//				of 120 MHz.  Note many peripheral clock rates
//				are divided down from the CPU clock.  For example,
//				the UART baud rate divider setting depends on the
//				desired baud rate and the CPU clock setting.
//
//	Example use:    if (InitializeCPU(100, 5515) == 0) while(1);
//
//		A return value of 0 indicates that the requested setting is
//		not supported on the specified processor.  Supported processors
//		are 5515 and 5505.

#include <stdint.h>

// a minimal set of register defines

#define CONFIG_MSW 		*(ioport volatile unsigned *)0x1C1F    // Clock Configuration MSW Register
#define PLL_CNTL1  		*(ioport volatile unsigned *)0x1C20    // PLL Control Register #1
#define PLL_CNTL2 		*(ioport volatile unsigned *)0x1C21    // PLL Control Register #2
#define PLL_CNTL3 		*(ioport volatile unsigned *)0x1C22    // PLL Control Register #3
#define PLL_CNTL4  		*(ioport volatile unsigned *)0x1C23    // PLL Control Register #4
#define EBSR      		*((volatile ioport unsigned *)0x1C00)
#define PCGCR1    		*((volatile ioport unsigned *)0x1C02)
#define PCGCR2    		*((volatile ioport unsigned *)0x1C03)
#define PER_RSTCOUNT	*(ioport volatile unsigned *)0x1C04 	// Peripheral Software Reset Counter Register
#define PER_RESET 		*(ioport volatile unsigned *)0x1C05 	// Peripheral Reset Register


uint16_t InitializeCPU(uint16_t CPU_clock_rate, uint16_t CPU_id)
{
    uint16_t ctr;

    // PLL set up from RTC

    CONFIG_MSW = 0x0;	// bypass PLL

    if (CPU_id == 5505) {				// CPU_id is defined in CCS
    	if (CPU_clock_rate == 100) {
    		PLL_CNTL2 = 0x8000;
    		PLL_CNTL4 = 0x0000;
    		PLL_CNTL1 = 0x82FA; // for C5505
    	}
    	else return 0;
    }
    else if (CPU_id == 5515) {
    	switch (CPU_clock_rate) {
    	case 100:
    	    PLL_CNTL2 = 0x8000;
    	    PLL_CNTL4 = 0x0000;
    		PLL_CNTL1 = 0x8BE8; // for C5515
    		break;
    	case 120:
    	    PLL_CNTL2 = 0x8000;
    	    PLL_CNTL4 = 0x0000;
    	    PLL_CNTL3 = 0x0806;
    	    PLL_CNTL1 = 0x8E4A;
    		break;
    	default:
    		return 0;
    	}
    }
	while ( (PLL_CNTL3 & 0x0008) == 0);
	CONFIG_MSW = 0x1;	// Switch to PLL clk

	PCGCR1 = 0x0; // enable clocks to peripherals
	PCGCR2 = 0x0; // enable clocks to more peripherals

	// reset peripherals
	PER_RSTCOUNT = 0x20;  // PRSCR reset pulse is 16 cycles
	PER_RESET = 0x00BF;   // PSCR
	for (ctr=0; ctr< 200; ctr++); // allow lots of time to reset

    return(1); // return non-zero if succrssful
}

//  configure port peripheral IO lines

void ConfigPort(void)
{
    EBSR = 0x1500;	// used to enable UART (disable OLED screen)
//	EBSR = 0x6100;  // used to enable OLED screen (disables UART)
}
