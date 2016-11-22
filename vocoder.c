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
#include <stdbool.h>
#include <math.h>

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
DATA bSpeech	[2*BUFSIZE];//Buffer 1
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

// LDATA buffers
LDATA bPhaseL[BUFSIZE];
LDATA bPhaseCosine[BUFSIZE];
LDATA bSRealL[BUFSIZE];
LDATA bYRealL[BUFSIZE];
LDATA bSImagL[BUFSIZE];
LDATA bYImagL[BUFSIZE];

LDATA bVocoded[2*BUFSIZE];//final vocoded output

LDATA sineTable [N_SINE];
LDATA cosTable [N_SINE];

DATA *pSpeech, *pSpeechFill, *pSpeechFreq, *pSynth, *pSynthFill, *pSynthFreq;
DATA *psReal, *psImag, *pyReal, *pyImag, *phase;
LDATA *phaseLong, *phaseCosLong, *pSRealL, *pYRealL, *pSImagL, *pYImagL, *pVocoded;

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

void splitRealImag(DATA *input, DATA *real, DATA *imag) {
	int i = 0;
	for (i = 0; i < BUFSIZE; ++i) {
		real[i] = input[2*i];
		imag[i] = input[2*i + 1];
	}
}

void combineRealImag(LDATA *real, LDATA *imag, LDATA *out) {
	int i = 0;
	for (i = 0; i < BUFSIZE; ++i) {
		out[2*i] = real[i];
		out[2*i+1] = imag[i];
	}
}

void square(LDATA *input, int length) {
	mul32(input, input, input, length);
}

LDATA sineLookup(LDATA angle)
{
	Uint16 ind;
	//angle to index
	ind =  floor(angle*N_SINE/PI - 1);
	return sineTable[ind];
}

LDATA cosLookup(LDATA angle)
{
	Uint16 ind;
	//angle to index
	ind = floor(angle*N_SINE/PI - 1);
	return cosTable[ind];

}

void vecCos(LDATA *input, LDATA *output, int length) {
	int i = 0;
	for (i = 0; i < length; ++i) {
		output[i] = cosLookup(input[i]);
	}
}

void vecSin(LDATA *input, LDATA *output, int length) {
	int i = 0;
	for (i = 0; i < length; ++i) {
		output[i] = sineLookup(input[i]);
	}
}

void vecCastDown4(LDATA *x1, LDATA *x2, LDATA *x3, LDATA *x4, DATA *out1, DATA *out2, DATA *out3, DATA *out4, int length) {
	int i = 0;
	for (i = 0; i < length; ++i) {
		out1[i] = (DATA) x1[i];
		out2[i] = (DATA) x2[i];
		out3[i] = (DATA) x3[i];
		out4[i] = (DATA) x4[i];
	}
}

void vecCastUp4(DATA *x1, DATA *x2, DATA *x3, DATA *x4, LDATA *out1, LDATA *out2, LDATA *out3, LDATA *out4, int length) {
	int i = 0;
	for (i = 0; i < length; ++i) {
		out1[i] = (LDATA) x1[i];
		out2[i] = (LDATA) x2[i];
		out3[i] = (LDATA) x3[i];
		out4[i] = (LDATA) x4[i];
	}
}

void vecCastUp3(DATA *x1, DATA *x2, DATA *x3, LDATA *out1, LDATA *out2, LDATA *out3, int length) {
	int i = 0;
	for (i = 0; i < length; ++i){
		out1[i] = (LDATA) x1[i];
		out2[i] = (LDATA) x2[i];
		out3[i] = (LDATA) x3[i];
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
	pVocoded = &bVocoded[0];
	Counter = 0;
	keepSample = true;

	// LDATA
	phaseLong = &bPhaseL[0];
	phaseCosLong = &bPhaseCosine[0];
	pSRealL = &bSRealL[0];
	pYRealL = &bYRealL[0];
	pSImagL = &bSImagL[0];
	pYImagL = &bYImagL[0];

	for(ctr=0; ctr<N_SINE; ctr++)  // generate sine and cosine table
	{
		sineTable[ctr]=(LDATA)sin(PI*ctr/N_SINE);
		cosTable[ctr]=(LDATA)cos(PI*ctr/N_SINE);
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

			splitRealImag(pSpeechFreq, psReal, psImag);
			splitRealImag(pSynthFreq, pyReal, pyImag);

			// get phase of synth
			atan2_16(pyImag, pyReal, phase, BUFSIZE);

			// get magnitudes mag = sqrt(r^2 + i^2)

			// cast up for multiply
			vecCastUp4(psReal, psImag, pyReal, pyImag, pSRealL, pSImagL, pYRealL, pYImagL, BUFSIZE);
			square(pSRealL, BUFSIZE);
			square(pSImagL, BUFSIZE);
			square(pYRealL, BUFSIZE);
			square(pYImagL, BUFSIZE);

			// cast back down
			vecCastDown4(pSRealL, pSImagL, pYRealL, pYImagL, psReal, psImag, pyReal, pyImag, BUFSIZE);

			// don't scale for now
			add(psReal, psImag, psReal, BUFSIZE, 0);
			sqrt_16(psReal, psReal, BUFSIZE);
			// now psReal has magnitude of speech

			add(pyReal, pyImag, pyReal, BUFSIZE, 0);
			sqrt_16(pyReal, pyReal, BUFSIZE);
			// now pyReal has magnitude of synth

			// real = mag * cos(phase)
			// imag = mag * sin(phase)
			//vecCos(phaseLong, phaseCosLong, BUFSIZE);
			// TODO: move casting around
			// TODO: figure out math to convert for cosine
			sine(phase, phase, BUFSIZE);


			//cast up for multiply
			vecCastUp3(pyReal, psReal, phase, pYRealL, pSRealL, phaseLong, BUFSIZE);

			// multiply magnitudes
			mul32(pSRealL, pYRealL, pSRealL, BUFSIZE);




			// real part
			mul32(pSRealL, phaseCosLong, pYRealL, BUFSIZE);
			// imaginary part
			mul32(pSRealL, phaseLong, pSRealL, BUFSIZE);

			// now buffers have final real and imaginary
			combineRealImag(pYRealL, pSRealL, pVocoded);

			// pSynthFreq has interleaved real and imag
			cifft32(pVocoded, BUFSIZE, SCALE);
			cbrev32(pVocoded, pVocoded, BUFSIZE);

			if(Counter >= (BUFSIZE*2))  // If this happens, we are too slow!
				goto TERMINATE;
		}
	}
TERMINATE:
	printf("Not enough time to compute FFT\n");
	printf("Counter: %lu",Counter);
}
