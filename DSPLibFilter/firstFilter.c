
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
#define NBIQ 1
#define FS 48000
#define halfPiQ 16384
//#define F0 1000

#define Q 10
Uint32 Counter;
bool keepSample;

// We need three buffers.
// One buffer will be used by the AIC to dump data into (pAIC).  Once that
// buffer is full, that buffer will be used as the input to the FFT.  A third
// buffer is used for the output of the FFT.

DATA bInput          [BUFSIZE];
DATA bInputFill      [BUFSIZE];
DATA bInputFreq      [BUFSIZE]; //FFT/Filter output goes here.

DATA bqCoeff         [5*NBIQ];

DATA dbuffer         [2*NBIQ+1];

DATA f0              [NBIQ];
DATA w0              [NBIQ];

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

DATA *pInput, *pInputFill, *pInputFreq, *pbqCoeff, *pdbuffer;

void Reset();

void C55_setup();

interrupt void I2S_ISR()
{
	Int16 right, left, output;
	AIC_read2(&right, &left);

	output = pInput[Counter];
	AIC_write2(output, output);

	pInputFill[Counter] = right;  //Only use evens for FFT function
	//pInputFill[Counter+1] = 0;	//No imaginary part

	Counter += 1;
	IFR0 &= (1 << I2S_BIT_POS);//Clear interrupt Flag

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



void getFrequencies(void){
	//This is ultimately where a call to check arduino values will go, but for now just fill
	int i = 0;
	for(i = 0; i < NBIQ; ++i){
		f0[i] = 1000;
		//Leave out pi because dsplib sine expects values normalized to pi
		w0[i] = ((2 * f0[i])/(float)FS) * 32767;
	}
}

void calculateLPCoeff(DATA *bqCoeff){
	DATA b0[NBIQ], b1[NBIQ], b2[NBIQ], a0[NBIQ], a1[NBIQ], a2[NBIQ];
	LDATA b0_32[NBIQ], b1_32[NBIQ], b2_32[NBIQ], a0_32[NBIQ], a1_32[NBIQ], a2_32[NBIQ];

	DATA cosW0[NBIQ], cosVal[NBIQ], sinVal[NBIQ];

	//Calculate house keeping constants

	// put sine of phase into phaseSine and cosine of phase into phase
	sine(w0, sinVal, NBIQ);
	// add pi/2 for cosine
	int i = 0;
	for (i = 0; i<NBIQ; i++)
	{
		//THIS APPEARS TO WORK BUT HASN'T BEEN UNIT TESTED
		cosW0[i] = w0[i] + halfPiQ;
	}

	sine(cosW0, cosVal, NBIQ);

	//w0 = 2*PI*(F0/FS);
	//cosVal = (DATA)(cos(w0) * 32767);
	//sinVal = (DATA)(sin(w0) * 32767);

	//Calculate raw coeffs
	for(i = 0; i < NBIQ; ++i){
//		b0_32[i] = ((((1<<14) - (cosVal[i]>>1))<<1))>>1;
//		b1_32[i] = ((1<<14) - (cosVal[i]>>1))<<1;
//		b2_32[i] = ((((1<<14) - (cosVal[i]>>1))<<1))>>1;
//		a0_32[i] = ((1<<14) + ((((LDATA)sinVal[i]<<11)/(((LDATA)(20)<<11)))<<1))<<1;
//		a1_32[i] = ((-2<<14) * ((LDATA)cosVal[i]>>1)>>13);
//		a2_32[i] = 1 - (sinVal[i]/(2*Q));

		b0_32[i] = (1<<15) - (cosVal[i]>>2);
		b1_32[i] = (1<<15) - (cosVal[i]);
		b2_32[i] = (1<<15) - (cosVal[i]>>2);
		a0_32[i] = (1<<15) + ((sinVal[i]>>15)/((LDATA)2*Q)>>15);
		a1_32[i] = (((long)-2<<15) * ((LDATA)cosVal[i]))>>15;
		a2_32[i] = (1<<15) + ((sinVal[i]>>15)/((LDATA)2*Q)>>15);

		//Normalize coeffs to a0
		DATA divisor = a0[i];

//		b0[i] = (b0_32[i] << 15)/divisor;
//		b1[i] = (b1_32[i] << 15)/divisor;
//		b2[i] = (b2_32[i] << 15)/divisor;
//		a1[i] = (a1_32[i] << 15)/divisor;
//		a2[i] = (a2_32[i] << 15)/divisor;

		b0[i] = 3199;
		b1[i] = 6398;
		b2[i] = 3199;
		a1[i] = -30894;
		a2[i] = 10923;

		//Assign coeffs to storage vector in order dictacted b C515DSP_LIB_guide
		bqCoeff[5*i] = b0[i];
		bqCoeff[5*i+1] = b1[i];
		bqCoeff[5*i+2] = b2[i];
		bqCoeff[5*i+3] = a1[i];
		bqCoeff[5*i+4] = a2[i];
	}

}

void main(void)
{
	Int16 ctr;
	short i;

	pInput = &bInput[0];
	pInputFill = &bInputFill[0];
	pInputFreq = &bInputFreq[0];
	pbqCoeff = &bqCoeff[0];
	pdbuffer = &dbuffer[0];

	DATA *temp1;
	Counter = 0;
	keepSample = true;

	USBSTK5515_init();
	AIC_init();

	getFrequencies();

	calculateLPCoeff(pbqCoeff);

	I2S_interrupt_setup();
	_enable_interrupts();

	while(1)
	{
		if(Counter >= (BUFSIZE))
		{
			// reset Counter
			Counter = 0;

			//Clear delay buffer
			for (i=0; i<5*NBIQ; i++) dbuffer[i] = 0;

			// Rotate buffers
			temp1 = pInputFill;
			pInputFill = pInput;
			pInput = temp1;



			// Do cfft with scaling. Do I need to do this?
			//cfft_SCALE(pInput, BUFSIZE);
			//cbrev(pInput, pInputFreq, BUFSIZE);

			//Filter fft result
			iircas51(pInput, bqCoeff, pInput, dbuffer, NBIQ, BUFSIZE);

			// pVocoded has interleaved real and imag
			//cifft(pVocoded, BUFSIZE, SCALE);
			//cbrev(pVocoded, pVocoded, BUFSIZE);

			if(Counter >= (BUFSIZE))  // If this happens, we are too slow!
				goto TERMINATE;
		}
	}
TERMINATE:
	printf("Not enough time to compute FFT\n");
	printf("Counter: %lu",Counter);
}

