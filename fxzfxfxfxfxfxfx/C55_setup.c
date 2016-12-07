/*
 * C5515_setup.c
 *
 *  Created on: Oct 7, 2011 .. KM
 *  Modified: Dec 2, 2015 .. KM
 *  Modified TxPut 28Sep2016 .. KM
 *
 *      To replace C5510 AIC23 support with minimal disruption.
 *      For use with the real time FFT.  Not all code is originally
 *      mine.
 */

#include <stdint.h>
#include "usbstk5515_led.h"

uint16_t InitializeCPU(uint16_t CPU_clock_rate, uint16_t CPU_id);
void ConfigPort();
void USBSTK5515_I2C_init(void);
void AIC3204_setup(uint16_t);
void InitSPI(void);
void InitUART(unsigned int baud_divisor);
void adc_initialize();


#ifdef vc5505
	#define C55XX_CLOCK 100
	#define C55XX_CPU 5505
	#define C55XX_UART_DIVIDE 3
#else
	#define C55XX_CLOCK 120
	#define C55XX_CPU 5515
	#define C55XX_UART_DIVIDE 781
#endif

void C55_setup()
{
	if (InitializeCPU(C55XX_CLOCK, C55XX_CPU)==0) while(1);
	ConfigPort(); 			// configure port use
    USBSTK5515_I2C_init( ); // Initialize I2C
	AIC3204_setup(0);		// initialize CODEC to use default
	adc_initialize(); 	// initialize the ADC/I2S0 interrupt handler
									// note: ADC has a start up transient!!!!
	InitUART(C55XX_UART_DIVIDE);	// set the UART baud rate for proper processor

#ifdef c5515
	USBSTK5515_ULED_init( ); // Configure user LEDs
#endif

}
