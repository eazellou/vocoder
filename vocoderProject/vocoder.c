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
#include "triWindow1024.h"
#include "noiseFreqBuffs.h"
#include "logGain.h"

#define BUFSIZE 1024
#define ADC_FIFO_SIZE 2048
#define OVERLAP  512
#define halfPiQ 16384//.5 * 32767 // pi/2 in q15
bool keepSample;

AddIndexFifo(Synth_, 2048, Int16, 1, 0)
AddIndexFifo(Speech_,2048, Int16, 1, 0)
AddIndexFifo(DACVocoder_,2048, Int16, 1, 0)
AddIndexFifo(DACClean_,2048,Int16,1,0)

DATA inputSpeech [2*BUFSIZE];
DATA inputSynth  [2*BUFSIZE];
DATA speechPassThrough [2*BUFSIZE];
DATA output [2*BUFSIZE];
DATA extra [OVERLAP-1];
Int16 flag1,flag2;

DATA speechReal[BUFSIZE];
DATA speechImag[BUFSIZE];
DATA speechAdd[BUFSIZE]; //hold added speech buffs for sqrt
DATA speechMagnitude[BUFSIZE]; //hold the speech magnitude, instead of overwriting speechReal
DATA synthRealb[BUFSIZE];
DATA synthImagb[BUFSIZE];
DATA synthAdd[BUFSIZE]; //hold added synth buffs for sqrt
DATA synthMagnitude[BUFSIZE]; //hold the synth magnitude
DATA phase[BUFSIZE];
DATA phaseSine[BUFSIZE];
DATA phaseCosine[BUFSIZE];

DATA *synthReal, *synthImag;

uint16_t InitializeCPU(uint16_t CPU_clock_rate, uint16_t CPU_id);
void ConfigPort(void);

void Reset();

interrupt void I2S_ISR()
{
	if (keepSample) {
		Int16 right, left, outVocode, outClean;
		AIC_read2(&right, &left);
		Speech_Fifo_Put(right);
		Synth_Fifo_Put(left);
		//Synth_Fifo_Put(right);
		flag1 = DACVocoder_Fifo_Get(&outVocode);
		flag2 = DACClean_Fifo_Get(&outClean);
		if(flag1 == 0) outVocode = 0;
		if(flag2 == 0) outClean = 0;
		AIC_write2(outVocode, outClean);
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
		out[2*i] = (DATA)(((LDATA)x[2*i]*(LDATA)y[i])>>15);
	}
}

void addition(DATA *x, DATA *y, DATA *out, int length) {
	int i = 0;
	for (i = 0; i < length; i++) {
		out[i] = x[i] + y[i];
	}
}
DATA temporary;
bool selectiveAddition(DATA *x, int bin1, int bin2, int threshold) {
	short output = 0;
	int i;

	for (i = bin1; i < bin2; i++)
	{
		temporary = x[i];
		output = output + temporary/100;
	}
	return output > threshold;
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

void applyLogGain(DATA *input, DATA*output, int length){
	int i = 0;
	for (i=0; i <length; i++){
		output[i] = input[i] * logGain[i];
	}
}

void main(void)
 {
	synthReal = &synthRealb[0];
	synthImag = &synthImagb[0];
	unsigned int tmp;
    DACVocoder_Fifo_Init();
    DACClean_Fifo_Init();
    Speech_Fifo_Init();
    Synth_Fifo_Init();

    keepSample = true;

	Int16 ctr;
	Int32 insize = BUFSIZE - OVERLAP + 1;
    //Initialize to all zeros    
    for(ctr=0; ctr<2*BUFSIZE; ctr++)
    {   inputSpeech[ctr] = 0;
    	inputSynth[ctr] = 0;
    	speechPassThrough[ctr] = 0;
        if(ctr < OVERLAP-1) extra[ctr] = 0;
    }

	//USBSTK5515_init();
	InitializeCPU(100, 5515);
	ConfigPort();
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
               speechPassThrough[2*ctr] = inputSpeech[2*ctr] / 2;
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

			applyGain(inputSpeech, 30, 2*BUFSIZE);
			applyLogGain(inputSpeech,inputSpeech,2*BUFSIZE);

			splitRealImag(inputSpeech, speechReal, speechImag, BUFSIZE);

			square(speechReal, BUFSIZE);
			square(speechImag, BUFSIZE);

			addition(speechReal, speechImag, speechAdd, BUFSIZE);
			sqrt_16(speechAdd, speechMagnitude, BUFSIZE);
			// now speechReal has magnitude of speech

			//Check frequency content of speech
			//This adds the selected bins, and if the result is above a threshold
			//fft and use noise
			//otherwise fft and use synth
//			if (selectiveAddition(speechMagnitude, 384, 512, 200))
//			{
//				//use noise FFTs to use for synth
//				synthReal = &realNoise[0];
//				synthImag = &imagNoise[0];
//			}
//			else
//			{
				//Do cfft with scaling.
				cfft_SCALE(inputSynth, BUFSIZE);
				cbrev(inputSynth, inputSynth, BUFSIZE);
				synthReal = &synthRealb[0];
				synthImag = &synthImagb[0];
				splitRealImag(inputSynth, synthReal, synthImag, BUFSIZE);
			//}

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
			square(synthReal, BUFSIZE);
			square(synthImag, BUFSIZE);

			addition(synthReal, synthImag, synthAdd, BUFSIZE);
			sqrt_16(synthAdd, synthMagnitude, BUFSIZE);
			// now synthReal has magnitude of synth

			// multiply magnitudes
			multiply(speechMagnitude, synthMagnitude, speechReal, BUFSIZE);
			//applyGain(speechReal, 100, BUFSIZE);

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

            applyGain(output, 5, 2*BUFSIZE);

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
                	DACVocoder_Fifo_Put(output[2*ctr] + extra[ctr]);
                	DACClean_Fifo_Put(speechPassThrough[2*ctr]);
                }
                else
                {
                	// this section does not overlap and can be output directly
                	DACVocoder_Fifo_Put(output[2*ctr]);
                	DACClean_Fifo_Put(speechPassThrough[2*ctr]);
                }
            }

            for(ctr=0; ctr<2*BUFSIZE; ctr++){
            	inputSpeech[ctr] = 0;
            	inputSynth[ctr] = 0;
            }

            if(Speech_Fifo_Size() >= insize || Synth_Fifo_Size() >= insize) printf("error\n");
		}
	}

}




