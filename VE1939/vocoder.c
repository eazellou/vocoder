/*
 * FFT.c
 *
 *  Created on: Oct 10, 2011
 *      Author: GSI
 */


#include <Dsplib.h>
#include <usbstk5515.h>
#include <AIC_func.h>
#include <usbstk5515_interrupts.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include "multiply.h"

#define BUFSIZE 1024
#define BUFCHK	2
#define N_SINE 256
#define FTV 1365
#define PI 3.14159265
Uint32 Counter;
bool keepSample;

// We need three buffers.
// One buffer will be used by the AIC to dump data into (pAIC).  Once that
// buffer is full, that buffer will be used as the input to the FFT.  A third
// buffer is used for the output of the FFT.
DATA bSpeech		[2*BUFSIZE];//Buffer 1
DATA bSpeechFill 	[2*BUFSIZE];//Buffer 2
DATA bSpeechFreq	[2*BUFSIZE];//Buffer 3--FFT output goes here.

DATA bSynth	[2*BUFSIZE];
DATA bSynthFill	[2*BUFSIZE];
DATA bSynthFreq	[2*BUFSIZE];

DATA speechReal[BUFSIZE];
DATA speechImag[BUFSIZE];
DATA synthReal[BUFSIZE];
DATA synthImag[BUFSIZE];
DATA bPhase[BUFSIZE];
DATA bPhaseCos[BUFSIZE];

DATA bVocoded[2*BUFSIZE];//final vocoded output

DATA sineTable [N_SINE];
DATA cosTable [N_SINE];

DATA *pSpeech, *pSpeechFill, *pSpeechFreq, *pSynth, *pSynthFill, *pSynthFreq, *pVocoded;
DATA *psReal, *psImag, *pyReal, *pyImag, *phase, *phaseCos;

void Reset();

void C55_setup();

interrupt void I2S_ISR()
{
	if (keepSample) {
		Int16 right, left, output;
		AIC_read2(&right, &left);

		output = pVocoded[Counter];
		AIC_write2(output, output);

		pSpeechFill[Counter] = right;  //Only use evens for FFT function
		pSpeechFill[Counter+1] = 0;	//No imaginary part

		pSynthFill[Counter] = left;
		pSynthFill[Counter + 1] = 0;

		Counter += 2;
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

void splitRealImag(DATA *input1, DATA *real1, DATA *imag1, DATA *input2, DATA *real2, DATA *imag2) {
	int i = 0;
	for (i = 0; i < BUFSIZE; ++i) {
		real1[i] = input1[2*i];
		imag1[i] = input1[2*i + 1];

		real2[i] = input2[2*i];
		imag2[i] = input2[2*i + 1];
	}
}

void combineRealImag(DATA *real, DATA *imag, DATA *out) {
	int i = 0;
	for (i = 0; i < BUFSIZE; ++i) {
		out[2*i] = real[i];
		out[2*i+1] = imag[i];
	}
}

void square(DATA *input, int length) {
	multiply(input, input, input, length, 0);
}

DATA sinLookup(DATA angle)
{
	Uint16 ind;
	//angle to index
	ind =  floor(angle*N_SINE/PI - 1);
	return sineTable[ind];
}

DATA cosLookup(DATA angle)
{
	Uint16 ind;
	//angle to index
	ind = floor(angle*N_SINE/PI - 1);
	return cosTable[ind];

}

// requires cosOut != input
void vecCosSin(DATA *input, DATA *cosOut, DATA *sinOut, int length) {
	int i = 0;
	for (i = 0; i < length; ++i) {
		cosOut[i] = cosLookup(input[i]);
		sinOut[i] = sinLookup(input[i]);
	}
}

void main(void)
{
	Int16 ctr;
	DATA *temp1;
	pSpeech = &bSpeech[0];
	pSpeechFill = &bSpeechFill[0];
	pSpeechFreq = &bSpeechFreq[0];
	pSynth = &bSynth[0];
	pSynthFill = &bSynthFill[0];
	pSynthFreq = &bSynthFreq[0];
	psReal = &speechReal[0];
	psImag = &speechImag[0];
	pyReal = &synthReal[0];
	pyImag = &synthImag[0];
	phase = &bPhase[0];
	phaseCos = &bPhaseCos[0];
	pVocoded = &bVocoded[0];
	Counter = 0;
	keepSample = true;

	for(ctr=0; ctr<N_SINE; ctr++)  // generate sine and cosine table
	{
		sineTable[ctr]=sin(PI*ctr/N_SINE);
		cosTable[ctr]=cos(PI*ctr/N_SINE);
	}

	USBSTK5515_init();
	AIC_init();
	//C55_setup(16000);
	I2S_interrupt_setup();
	_enable_interrupts();

	while(1)
	{
		if(Counter >= (BUFSIZE*2))
		{
			// Rotate buffers
			temp1 = pSpeechFill;
			pSpeechFill = pSpeech;
			pSpeech = temp1;

			temp1 = pSynthFill;
			pSynthFill = pSynth;
			pSynth = temp1;

			// reset Counter
			Counter = 0;

			// Do cfft with scaling.
			cfft_SCALE(pSpeech, BUFSIZE);
			cbrev(pSpeech, pSpeechFreq, BUFSIZE);

			cfft_SCALE(pSynth, BUFSIZE);
			cbrev(pSynth, pSynthFreq, BUFSIZE);

			splitRealImag(pSpeechFreq, psReal, psImag, pSynthFreq, pyReal, pyImag);

			// get phase of synth
			atan2_16(pyImag, pyReal, phase, BUFSIZE);

			// get magnitudes mag = sqrt(r^2 + i^2)
			square(psReal, BUFSIZE);
			square(psImag, BUFSIZE);
			square(pyReal, BUFSIZE);
			square(pyImag, BUFSIZE);

			// don't scale for now
			add(psReal, psImag, psReal, BUFSIZE, 0);
			sqrt_16(psReal, psReal, BUFSIZE);
			// now psReal has magnitude of speech

			add(pyReal, pyImag, pyReal, BUFSIZE, 0);
			sqrt_16(pyReal, pyReal, BUFSIZE);
			// now pyReal has magnitude of synth

			// multiply magnitudes
			multiply(psReal, pyReal, psReal, BUFSIZE, 0);

			// real = mag * cos(phase)
			// imag = mag * sin(phase)
			vecCosSin(phase, phaseCos, phase, BUFSIZE);

			// real part
			multiply(psReal, phaseCos, psReal, BUFSIZE, 0);
			// imaginary part
			multiply(psReal, phase, psImag, BUFSIZE, 0);

			// now buffers have final real and imaginary
			combineRealImag(psReal, psImag, pVocoded);

			// pVocoded has interleaved real and imag
			cifft(pVocoded, BUFSIZE, SCALE);
			cbrev(pVocoded, pVocoded, BUFSIZE);

			if(Counter >= (BUFSIZE*2))  // If this happens, we are too slow!
				goto TERMINATE;
		}
	}
TERMINATE:
	printf("Not enough time to compute FFT\n");
	printf("Counter: %lu",Counter);
}

