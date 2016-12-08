/*
 * freq_fir_filter.c
 *
 *  Created on: Oct 1, 2016
 *      Author: GSI
*/

#include <Dsplib.h>
#include <usbstk5515.h>
#include <AIC_func.h>
#include <usbstk5515_interrupts.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include "FIFO_builder.h"
#include "bitCrush.h"
#include "delay.h"
#include "newChorus.h"

#define BUFSIZE 1024
#define ADC_FIFO_SIZE 2048
#define OVERLAP  512
bool keepSample;

AddIndexFifo(ADC_Dry_, 2048, Int16, 1, 0)
AddIndexFifo(ADC_Wet_, 2048, Int16, 1, 0)
AddIndexFifo(DAC_,2048, Int16, 1, 0)

DATA input [2*BUFSIZE];
DATA inputDry [2*BUFSIZE];
DATA output [2*BUFSIZE];
DATA extra [OVERLAP-1];
Int16 flag;

extern volatile int16_t k1, k2, k3, k4, k5, k6, k7, k8, k9;
extern volatile int16_t s1, f1;
/*
 * 1 - delay feedback
 * 2 - delay time
 * 3 - chorus
 * 4 - distortion
 * 5-8 - eq
 * 9 - mix
 */

uint16_t InitializeCPU(uint16_t CPU_clock_rate, uint16_t CPU_id);
void ConfigPort(void);

void Reset();
void InitUART(unsigned int baud_divisor);
#define C55XX_UART_DIVIDE 434

interrupt void I2S_ISR()
{
	if (keepSample) {
		Int16 right, left, out;
		AIC_read2(&right, &left);
		ADC_Wet_Fifo_Put(right);
		ADC_Dry_Fifo_Put(left);
		flag = DAC_Fifo_Get(&out);
		if(flag == 0) out = 0;
		AIC_write2(out, out);
		IFR0 &= (1 << I2S_BIT_POS);//Clear interrupt Flag

		keepSample = false;
	} else {
		keepSample = true;
	}
}

// Setup AIC interrupt routine.
void I2S_interrupt_setup(void)
{
	Uint32 reset_loc = (Uint32)Reset;
	IVPD = reset_loc >> 8;//pointer table points to Reset
	IVPH = reset_loc >> 8;//Ditto

	*((Uint32*)((reset_loc + I2S_ISR_OFFSET)>>1)) = (Uint32)I2S_ISR;

	SYS_EXBUSSEL |= (0x1 << 8);//Set the External bus select SP0Mode to I2S
	IER0 |= (1 << I2S_BIT_POS);//Set up the Global interrupt register to flag on I2S receive flag
	IFR0 &= (1 << I2S_BIT_POS);
}

void applyGain(DATA *input, DATA gainFactor, int length){
	int i = 0;
	for(i = 0; i < length; ++i){
		input[i] = (DATA)(((LDATA)input[i] * (LDATA)gainFactor)>>15);
	}
}

void addition(DATA *x, DATA *y, DATA *out, int length) {
	int i = 0;
	for (i = 0; i < length; i++) {
		out[i] = x[i] + y[i];
	}
}

void mixDryWet(DATA* inDry, DATA *inWet, DATA *out, DATA wetAmount, int length) {

	DATA dryAmount = ((32767 - wetAmount - 1) > 0) ? 32767 - wetAmount - 1 : 0;

	applyGain(inWet, wetAmount, length);
	applyGain(inDry, dryAmount, length);
	addition(inWet, inDry, out, length);
}

DATA normalizeDryWet(int knob) {
	return ((DATA)(((LDATA)(knob)<<15) / 255));
}

DATA normalizeChorusFeedback(int knob) {
	// want about 0-30000, get 0-255
	return (DATA)(117 * knob);
}

DATA normalizeMasterGain(int knob) {
	if(knob > 0){
		knob = 255;
	}
	return 32767 - ((DATA)(((LDATA)(knob)<<15) / 255));
}

int normalizeBitDepth(int knob){
	return 16 - ((int)(knob / 18) + 1);
}

int normalizeDelayFeedback(int knob){
	return (knob*120);
}

int normalizeDelayTime(int knob){
	return (knob*120);
}

void main(void)
 {
    DAC_Fifo_Init();
    ADC_Dry_Fifo_Init();
    ADC_Wet_Fifo_Init();

    keepSample = true;
    DATA test = 0;

	Int16 ctr;
	Int32 insize = BUFSIZE - OVERLAP + 1;
    //Initialize to all zeros    
    for(ctr=0; ctr<2*BUFSIZE; ctr++)
    {   input[ctr] = 0;
        if(ctr < OVERLAP-1) extra[ctr] = 0;
    }

	//USBSTK5515_init();
	InitializeCPU(100, 5515);
	ConfigPort();
	AIC_init();
	I2S_interrupt_setup();
	InitUART(C55XX_UART_DIVIDE);	// set the UART baud rate for proper processor
	_enable_interrupts();


	while(1)
	{
		//printf("S1 value: %d\n", s1);
		//printf("K2 value: %d\n", k2);

		if(ADC_Wet_Fifo_Size() >= insize && ADC_Dry_Fifo_Size() >= insize)
		{

            //Throw input data into zero-padded frame
            for(ctr=0; ctr<insize; ctr++) {
               ADC_Wet_Fifo_Get(&input[2*ctr]);
               ADC_Dry_Fifo_Get(&inputDry[2*ctr]);
            }

            // mix
            mixDryWet(inputDry, input, output, normalizeDryWet(k9), 2*BUFSIZE);

//            processNewChorus(output, output, normalizeChorusFeedback(k3), BUFSIZE);
			processBitCrush(output, output, normalizeBitDepth(k4), 2*BUFSIZE);

			processDelay(output, output, normalizeDelayTime(k2), normalizeDelayFeedback(k1), BUFSIZE);

//			int i = 0;
//            for (i = 0; i < 2*BUFSIZE; i++){
//            	output[i] = input[i];
//            }

            // gain
//            DATA test = normalizeMasterGain(f1);
//            applyGain(output, normalizeMasterGain(f1), 2*BUFSIZE);

//            for(ctr=0; ctr < BUFSIZE; ctr++)
//            {
//                if(ctr >= insize)
//                {
//                	// this is the section that will overlap with
//                	// the beginning of the next segment
//                	extra[ctr - insize] = output[2*ctr];
//                }
//                else if(ctr < OVERLAP - 1)
//                {
//                	// outputting the sum of the extra overhang from the
//                	// previous convolution with the results of this one
//                	DAC_Fifo_Put(output[2*ctr] + extra[ctr]);
//                }
//                else
//                {
//                	// this section does not overlap and can be output directly
//                	DAC_Fifo_Put(output[2*ctr]);
//                }
//            }

            for(ctr=0; ctr<insize; ctr++)
            {
            	DAC_Fifo_Put(output[2*ctr]);
            }

            for(ctr=0; ctr<2*BUFSIZE; ctr++){
            	input[ctr] = 0;
            	inputDry[ctr] = 0;
            	output[ctr] = 0;
            }

            if(ADC_Wet_Fifo_Size() >= insize || ADC_Dry_Fifo_Size() >= insize) printf("error\n");
		}
	}

}
