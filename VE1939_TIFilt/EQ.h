#ifndef EQ_H
#define EQ_H

#include <usbstk5515.h>
#include <AIC_func.h>
#include <stdio.h>
#include <Dsplib.h>

#define NBIQ 3
#define NBIQ_MAIN 2
#define NBIQ_BP 1
#define BIQ_COEFF 5
#define FS 48000
#define halfPiQ 16384
#define Q2  46340
#define Q_BP  131072

Int16 biq_main[NBIQ_MAIN * BIQ_COEFF];
Int16 biq_bp[NBIQ_BP * BIQ_COEFF];


Int16 delay_buff_main [NBIQ_MAIN*2 + 2];
Int16 delay_buff_bp [NBIQ_BP*2 + 2];

DATA f0_main[NBIQ_MAIN];
DATA w0_main[NBIQ_MAIN];
DATA f0_bp[NBIQ_BP];
DATA w0_bp[NBIQ_BP];

DATA sinOut_main[NBIQ_MAIN];
DATA cosOut_main[NBIQ_MAIN];
DATA sinOut_bp[NBIQ_BP];
DATA cosOut_bp[NBIQ_BP];

LDATA b0_main[NBIQ_MAIN], b1_main[NBIQ_MAIN], b2_main[NBIQ_MAIN], a0_main[NBIQ_MAIN], a1_main[NBIQ_MAIN], a2_main[NBIQ_MAIN], alpha_main[NBIQ_MAIN];
LDATA b0_bp[NBIQ_BP], b1_bp[NBIQ_BP], b2_bp[NBIQ_BP], a0_bp[NBIQ_BP], a1_bp[NBIQ_BP], a2_bp[NBIQ_BP], alpha_bp[NBIQ_BP];


void initEQ(){
	//Clear delay buffers
	int i;
	for(i=0; i< NBIQ_MAIN*2 + 2; i++)
			delay_buff_main[i] = 0; //Initialize the registers to zero

	for(i=0; i< NBIQ_BP*2 + 2; i++)
			delay_buff_bp[i] = 0; //Initialize the registers to zero

}
void convertFreq(DATA* freqBuf, DATA size){
	int i;
	for(i = 0; i < size; ++i){
		freqBuf[i] = (((2 * freqBuf[i])/(float)FS) * 32767);
	}
}

void calculateCoeffs(){

	sine(f0_main, sinOut_main , NBIQ_MAIN);
	sine(f0_bp, sinOut_bp, NBIQ_BP);

	int i = 0;
	//main filters
	for (i = 0; i<NBIQ_MAIN; i++)
	{
		//THIS APPEARS TO WORK BUT HASN'T BEEN UNIT TESTED
		f0_main[i] = f0_main[i] + halfPiQ;
	}
	//bp filter
	for (i = 0; i<NBIQ_BP; i++)
		{
			//THIS APPEARS TO WORK BUT HASN'T BEEN UNIT TESTED
			f0_bp[i] = f0_bp[i] + halfPiQ;
		}

	sine(f0_main, cosOut_main , NBIQ_MAIN);
	sine(f0_bp, cosOut_bp, NBIQ_BP);

	//Calculating alpha
	//main filters
	for(i = 0; i < NBIQ_MAIN; ++i){
		alpha_main[i] = (((LDATA)sinOut_main[i]) << 15)/Q2;
	}
	//bp filter
	for(i = 0; i < NBIQ_BP; ++i){
		alpha_bp[i] = (((LDATA)sinOut_bp[i]) << 15)/Q_BP;
	}

	//First process "main" filters
	//LPF
	b0_main[0] = (32767 - (LDATA)cosOut_main[0])/2;
	b1_main[0] = (32767 - (LDATA)cosOut_main[0]);
	b2_main[0] = (32767 - (LDATA)cosOut_main[0])/2;
	a0_main[0] = 32767 + alpha_main[0];
	a1_main[0] = (-2*(LDATA)cosOut_main[0]);
	a2_main[0] = 32767 - alpha_main[0];
	//HPF
	b0_main[1] = (32767 + (LDATA)cosOut_main[1])/2;
	b1_main[1] = -1*(32767 + (LDATA)cosOut_main[1]);
	b2_main[1] = b0_main[1];
	a0_main[1] = 32767 + alpha_main[1];
	a1_main[1] = -2*(LDATA)cosOut_main[1];
	a2_main[1] = 32767 - alpha_main[1];


	//Now process bp filter
	//BPF
	b0_bp[0] = alpha_bp[0];
	b1_bp[0] = 0;
	b2_bp[0] = -1*alpha_bp[0];
	a0_bp[0] = 32767 + alpha_bp[0];
	a1_bp[0] = -2*(LDATA)cosOut_bp[0];
	a2_bp[0] = 32767 - alpha_bp[0];


	//Normalize main coeffs
	for(i = 0; i < NBIQ_MAIN; ++i){
		b0_main[i] = (((long long)b0_main[i]<<15)/a0_main[i]);
		b1_main[i] = (((long long)b1_main[i]<<15)/a0_main[i]);
		b2_main[i] = (((long long)b2_main[i]<<15)/a0_main[i]);
		a1_main[i] = (((long long)a1_main[i]<<15)/a0_main[i]);
		a2_main[i] = (((long long)a2_main[i]<<15)/a0_main[i]);
	}

	//Normalize bp coeffs
	for(i = 0; i < NBIQ_BP; ++i){
		b0_bp[i] = (((long long)b0_bp[i]<<15)/a0_bp[i]);
		b1_bp[i] = (((long long)b1_bp[i]<<15)/a0_bp[i]);
		b2_bp[i] = (((long long)b2_bp[i]<<15)/a0_bp[i]);
		a1_bp[i] = (((long long)a1_bp[i]<<15)/a0_bp[i]);
		a2_bp[i] = (((long long)a2_bp[i]<<15)/a0_bp[i]);
	}

}

void assignCoeffs(){
	int i = 0;
	//main filters
	for(i=0; i < NBIQ_MAIN; ++i){
		biq_main[i*BIQ_COEFF] = a1_main[i] >> 1; //a1
		biq_main[(i*BIQ_COEFF)+1] =  a2_main[i];			//a2
		biq_main[(i*BIQ_COEFF)+2] =  b2_main[i];			//b2

		biq_main[(i*BIQ_COEFF)+3] =  b0_main[i];			//b0
		biq_main[(i*BIQ_COEFF)+4] = b1_main[i] >> 1;	//b1
	}

	//bp filters
	for(i=0; i < NBIQ_BP; ++i){
		biq_bp[i*BIQ_COEFF] = a1_bp[i] >> 1; //a1
		biq_bp[(i*BIQ_COEFF)+1] =  a2_bp[i];			//a2
		biq_bp[(i*BIQ_COEFF)+2] =  b2_bp[i];			//b2

		biq_bp[(i*BIQ_COEFF)+3] =  b0_bp[i];			//b0
		biq_bp[(i*BIQ_COEFF)+4] = b1_bp[i] >> 1;	//b1
	}
}

void eqGain (DATA *x, DATA y, DATA *out, int length)
{
	int i = 0;
	for (i = 0; i < length; i++)
	{
		out[i] = (DATA)(((LDATA)x[i]*(LDATA)y)>>15);
	}
}

void processEQ(DATA *in, DATA *out, DATA lpFreq, DATA hpFreq, DATA bpFreq, DATA bpGain, DATA length){
	DATA out_main[1024], out_bp[1024];
	int i;
	for(i = 0; i < length; ++i){
		in[i] = in[i] >> 5;
	}

	for(i=0; i< NBIQ_MAIN*2 + 2; i++)
			delay_buff_main[i] = 0; //Initialize the registers to zero

	for(i=0; i< NBIQ_BP*2 + 2; i++)
			delay_buff_bp[i] = 0; //Initialize the registers to zero


	//Store input frequency values
	f0_main[0] = lpFreq;
	f0_main[1] = hpFreq;
	f0_bp[0] = bpFreq;

	//Convert frequency to angular frequency
	convertFreq(f0_main, NBIQ_MAIN);
	convertFreq(f0_bp, NBIQ_BP);

	//Calculate and assign coefficients
	calculateCoeffs();
	assignCoeffs();


	//Gain is calculated assuming a Q15 number for gain
	//filter with main filters
	iircas5(in, biq_main, out, delay_buff_main, NBIQ_MAIN, length);
	//eqGain(out_main, 32737-bpGain, out_main, length);
	//filter with bp filters
//	iircas5(in, biq_bp, out_bp, delay_buff_main, NBIQ_BP, length);
//	eqGain(out_bp, bpGain, out_bp, length);
//
//	for(i = 0; i < length; ++i){
//		out[i] = out_main[i] + out_bp[i];
//	}

	for(i = 0; i < length; ++i){
		out[i] = out[i] << 5;
	}
}

#endif
