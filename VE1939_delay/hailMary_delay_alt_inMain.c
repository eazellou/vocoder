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

#include "delay.h"
#include "FIFO_builder.h"

#define BUFSIZE 64
#define ADC_FIFO_SIZE 128 //BUFSIZE * 2
#define OVERLAP 32 //BUFSIZE / 2
#define MAXDELAYSIZE 32768
bool keepSample;

AddIndexFifo(ADC_, 128, Int16, 1, 0)
AddIndexFifo(DAC_,128, Int16, 1, 0)

DATA input [2*BUFSIZE];
DATA output [2*BUFSIZE];
DATA extra [OVERLAP-1];
Int16 flag;

void Reset();

interrupt void I2S_ISR()
{
	if (keepSample) {
		Int16 right, left, out;
		AIC_read2(&right, &left);
		ADC_Fifo_Put(right);
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

void main(void)
 {
    DAC_Fifo_Init();
    ADC_Fifo_Init();

    keepSample = true;
    DATA test = 0;

	Int16 ctr;
	Int32 insize = BUFSIZE - OVERLAP + 1;
    //Initialize to all zeros    
    for(ctr=0; ctr<2*BUFSIZE; ctr++)
    {   input[ctr] = 0;
        if(ctr < OVERLAP-1) extra[ctr] = 0;
    }

	USBSTK5515_init();
	AIC_init();
	I2S_interrupt_setup();
	_enable_interrupts();


	while(1)
	{
		if(ADC_Fifo_Size() >= insize)
		{

            //Throw input data into zero-padded frame
            for(ctr=0; ctr<insize; ctr++) {
               ADC_Fifo_Get(&input[2*ctr]);
            }

            processDelay(input, output, 10000, BUFSIZE);

//            for(i = 0; i < BUFSIZE*2; i++)
//            {
//            	output[i] = input[i];
//            }


            for(ctr=0; ctr < BUFSIZE; ctr++)
            {
                if(ctr >= insize)
                {
                	// this is the section that will overlap with
                	// the beginning of the next segment
                	extra[ctr - insize] = output[2*ctr];
                }
                else if(ctr < OVERLAP - 1)
                {
                	// outputting the sum of the extra overhang from the
                	// previous convolution with the results of this one
                	DAC_Fifo_Put(output[2*ctr] + extra[ctr]);
                }
                else
                {
                	// this section does not overlap and can be output directly
                	DAC_Fifo_Put(output[2*ctr]);
                }
            }

            for(ctr=0; ctr<2*BUFSIZE; ctr++){
            	input[ctr] = 0;
            }

            if(ADC_Fifo_Size() >= insize) printf("error\n");
		}
	}

}




