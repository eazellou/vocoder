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
#include "triWindowQ.h"

#define BUFSIZE 512
#define ADC_FIFO_SIZE 1024
#define OVERLAP  256  //BUFSIZE/4
#define halfPiQ 16384//.5 * 32767 // pi/2 in q15
bool keepSample;

AddIndexFifo(Synth_, 1024, Int16, 1, 0)
AddIndexFifo(Speech_,1024, Int16, 1, 0)
AddIndexFifo(DAC_,1024, Int16, 1, 0)

DATA inputSpeech [2*BUFSIZE];
DATA inputSynth  [2*BUFSIZE];
DATA output [2*BUFSIZE];
DATA extra [OVERLAP-1];
Int16 flag;

DATA speechReal[BUFSIZE];
DATA speechImag[BUFSIZE];
DATA speechAdd[BUFSIZE]; //hold added speech buffs for sqrt
DATA speechMagnitude[BUFSIZE]; //hold the speech magnitude, instead of overwriting speechReal
DATA synthReal[BUFSIZE];
DATA synthImag[BUFSIZE];
DATA synthAdd[BUFSIZE]; //hold added synth buffs for sqrt
DATA synthMagnitude[BUFSIZE]; //hold the synth magnitude
DATA phase[BUFSIZE];
DATA phaseSine[BUFSIZE];
DATA phaseCosine[BUFSIZE];


void Reset();

interrupt void I2S_ISR()
{
	if (keepSample) {
		Int16 right, left, out;
		AIC_read2(&right, &left);
		Speech_Fifo_Put(right);
		Synth_Fifo_Put(left);
		//Synth_Fifo_Put(right);
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

void splitRealImag(DATA *input, DATA *real, DATA *imag, int length) {
	int i = 0;
	for (i = 0; i < length; ++i) {
		real[i] = input[2*i];
		imag[i] = input[2*i + 1];
	}
}

void combineRealImag(DATA *real, DATA *imag, DATA *out, int length) {
	int i = 0;
	for (i = 0; i < length; ++i) {
		out[2*i] = real[i];
		out[2*i+1] = imag[i];
	}
}

void multiply (DATA *x, DATA *y, DATA *out, int length)
{
	int i = 0;
	for (i = 0; i < length; i++)
	{
		out[i] = (DATA)(((LDATA)x[i]*(LDATA)y[i])>>15);
	}
}

void multiplyReal (DATA *x, DATA *y, DATA *out, int winLength) {
	int i = 0;
	for (i = 0; i < winLength; i++) {
		out[2*i] = (DATA)(((LDATA)x[2*i]*(LDATA)y[2*i])>>15);
	}
}

void addition(DATA *x, DATA *y, DATA *out, int length) {
	int i = 0;
	for (i = 0; i < length; i++) {
		out[i] = x[i] + y[i];
	}
}

void square(DATA *input, int length) {
	multiply(input, input, input, length);
}

	//TODO: doesn't saturate without float, too slow with float
void applyGain(DATA *input, int gainFactor, int length){
	int i = 0;
	for(i = 0; i < length; ++i){
		input[i] *= gainFactor;
	}
}


void main(void)
{
	unsigned int tmp;
    DAC_Fifo_Init();
    Speech_Fifo_Init();
    Synth_Fifo_Init();

    keepSample = true;

	Int16 ctr;
	Int32 insize = BUFSIZE - OVERLAP + 1;
    //Initialize to all zeros    
    for(ctr=0; ctr<2*BUFSIZE; ctr++)
    {   inputSpeech[ctr] = 0;
    	inputSynth[ctr] = 0;
        if(ctr < OVERLAP-1) extra[ctr] = 0;
    }

	USBSTK5515_init();
	AIC_init();
	I2S_interrupt_setup();
	_enable_interrupts();


	while(1)
	{
		if(Speech_Fifo_Size() >= insize && Synth_Fifo_Size() >= insize)
		{

            //Throw input data into zero-padded frame
            for(ctr=0; ctr<insize; ctr++) {
               Speech_Fifo_Get(&inputSpeech[2*ctr]);
               Synth_Fifo_Get(&inputSynth[2*ctr]);
               inputSpeech[2*ctr + 1] = 0;
               inputSynth[2*ctr + 1] = 0;
            }

            //TODO
//            //Apply pre-gain to synth and speech equally
//            applyGain(inputSpeech, 2, BUFSIZE);
//            applyGain(inputSynth, 2, BUFSIZE);
//            tmp = Speech_Fifo_Size();

            //First window
            multiplyReal(inputSpeech,triWindow,inputSpeech,BUFSIZE);
            multiplyReal(inputSynth,triWindow,inputSynth,BUFSIZE);

			//Do cfft with scaling.
			cfft_SCALE(inputSpeech, BUFSIZE);
			cbrev(inputSpeech, inputSpeech, BUFSIZE);

			//Do cfft with scaling.
			cfft_SCALE(inputSynth, BUFSIZE);
			cbrev(inputSynth, inputSynth, BUFSIZE);

			splitRealImag(inputSynth, synthReal, synthImag, BUFSIZE);
			splitRealImag(inputSpeech, speechReal, speechImag, BUFSIZE);

			// get phase of synth
			atan2_16(synthReal, synthImag, phase, BUFSIZE);
			// put sine of phase into phaseSine and cosine of phase into phase
			sine(phase, phaseSine, BUFSIZE);
			// add pi/2 for cosine
			int i = 0;
			for (i = 0; i<BUFSIZE; i++)
			{
				//THIS APPEARS TO WORK BUT HASN'T BEEN UNIT TESTED
				phaseCosine[i] = phase[i] + halfPiQ;
//				if(phase[i] < halfPiQ){
//					phaseCosine[i] = phase[i] + halfPiQ;
//				} else {
//					phaseCosine[i] = phase[i] - 3 * halfPiQ;
//				}
			}

			sine(phaseCosine, phaseCosine, BUFSIZE);

			// get magnitudes mag = sqrt(r^2 + i^2)
			square(speechReal, BUFSIZE);
			square(speechImag, BUFSIZE);
			square(synthReal, BUFSIZE);
			square(synthImag, BUFSIZE);

			addition(speechReal, speechImag, speechAdd, BUFSIZE);
			sqrt_16(speechAdd, speechMagnitude, BUFSIZE);
			// now speechReal has magnitude of speech

			addition(synthReal, synthImag, synthAdd, BUFSIZE);
			sqrt_16(synthAdd, synthMagnitude, BUFSIZE);
			// now synthReal has magnitude of synth

			// multiply magnitudes
			multiply(speechMagnitude, synthMagnitude, speechReal, BUFSIZE);
			applyGain(speechReal, 100, BUFSIZE);

			// real = mag * cos(phase)
			// imag = mag * sin(phase)

			// real part
			multiply(speechReal, phaseCosine, synthReal, BUFSIZE);
			// imaginary part
			multiply(speechReal, phaseSine, synthImag, BUFSIZE);

			combineRealImag(synthReal, synthImag, output, BUFSIZE);

			//Do inverse cfft without scaling
			cifft_NOSCALE(output,BUFSIZE);
            cbrev(output, output, BUFSIZE);

            //window output
            multiplyReal(output,triWindow,output,BUFSIZE);
            //applyGain(output,100,BUFSIZE);

            //TODO: overlap add
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

            if(Speech_Fifo_Size() >= insize || Synth_Fifo_Size() >= insize) printf("error\n");
		}
	}

}




