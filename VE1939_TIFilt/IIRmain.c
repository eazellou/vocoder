/*
 * main.c
 *
 *  Created on: Oct 9, 2011
 *      Author: GSI
 */

#include <usbstk5515.h>
#include <AIC_func.h>
#include <stdio.h>
#include <Dsplib.h>

#define NBIQ 3
#define NBIQ_BP 1
#define NBIQ_MAIN 2
#define BIQ_COEFF 5
#define PI 3.14159265
#define FS 48000
#define halfPiQ 16384
#define Q2  46340
#define Q_BP  131072
//#define Q_BP 262144

//useful for timing routine!
#define TCR0 		*((ioport volatile Uint16 *)0x1810)
#define TIMCNT1_0 	*((ioport volatile Uint16 *)0x1814)
#define TIME_START  0x8001
#define TIME_STOP   0x8000

Uint16 start_time, end_time, delta_time;

Int16 biq[NBIQ * BIQ_COEFF];
LDATA biq_32[NBIQ * BIQ_COEFF];

Int16 delay_buff [NBIQ*2 + 2];

DATA f0[NBIQ];
DATA w0[NBIQ];

DATA sinOut[NBIQ];
DATA cosOut[NBIQ];


LDATA b0[NBIQ], b1[NBIQ], b2[NBIQ], a0[NBIQ], a1[NBIQ], a2[NBIQ], alpha[NBIQ], alphaBP[NBIQ];


//Int32 biq_32[NBIQ * BIQ_COEFF] = {
//		-53768,	22390,	731,	731,	-73,
//		-55822,	26004,	6363,	6363,	-9777,
//		-57921,	29581,	16521,	16521,	-28615,
//		-59463,	31878,	23308,	23308,	-41434};

//Int32 biq_32[NBIQ * BIQ_COEFF] = {
//		-41931, 15650, 1622, 1622, 3243, //LPF
////		-49396, 19882, 26325, 26325, -49396, //BCF
////		-60183, 31380, 31295, 32852, -60183, //BBF
//		-56485, 24823, 28519, 28519, -57038 //HPF


//};


void getFreq(DATA *freqBuf, DATA *angBuf){
	//Ultimately this will call to Arduino but for now fill
	int i;
	for(i=0; i< NBIQ; ++i){
		//Fetch code goes here
	}

	freqBuf[0] = 4000;
	freqBuf[1] = 1500;
	freqBuf[2] = 2700;

	for(i=0; i < NBIQ; ++i){
		angBuf[i] = (((2 * freqBuf[i])/(float)FS) * 32767);

	}

}

void calculateCoeffs(){



	sine(w0, sinOut , NBIQ);

	// add pi/2 for cosine
	int i = 0;
	for (i = 0; i<NBIQ; i++)
	{
		//THIS APPEARS TO WORK BUT HASN'T BEEN UNIT TESTED
		w0[i] = w0[i] + halfPiQ;
	}

	sine(w0, cosOut, NBIQ);

	//Calculating alpha
	for(i = 0; i < NBIQ; ++i){
		alpha[i] = (((LDATA)sinOut[i]) << 15)/Q2;
		alphaBP[i] = (((LDATA)sinOut[i]) << 15)/Q_BP;
	}


	//Calculate LPF Coeffs
	b0[0] = (32767 - (LDATA)cosOut[0])/2;
	b1[0] = (32767 - (LDATA)cosOut[0]);
	b2[0] = (32767 - (LDATA)cosOut[0])/2;
	a0[0] = 32767 + alpha[0];
	a1[0] = (-2*(LDATA)cosOut[0]);
	a2[0] = 32767 - alpha[0];
	//Calculate HPF Coeffs
	b0[1] = (32767 + (LDATA)cosOut[1])/2;
	b1[1] = -1*(32767 + (LDATA)cosOut[1]);
	b2[1] = b0[1];
	a0[1] = 32767 + alpha[1];
	a1[1] = -2*(LDATA)cosOut[1];
	a2[1] = 32767 - alpha[1];
	//Calculate BPF Coeffs
	b0[2] = alphaBP[2];
	b1[2] = 0;
	b2[2] = -1*alphaBP[2];
	a0[2] = 32767 + alphaBP[2];
	a1[2] = -2*(LDATA)cosOut[2];
	a2[2] = 32767 - alphaBP[2];


	//a2[0] = (((long long)a2[0] << 15)/a0[0]);

	//Normalize all coeffs
	for(i = 0; i < NBIQ; ++i){
		b0[i] = (((long long)b0[i]<<15)/a0[i]);
		b1[i] = (((long long)b1[i]<<15)/a0[i]);
		b2[i] = (((long long)b2[i]<<15)/a0[i]);
		a1[i] = (((long long)a1[i]<<15)/a0[i]);
		a2[i] = (((long long)a2[i]<<15)/a0[i]);
	}
}

void processEQ(DATA *in, DATA *out){


	int i;
	for(i=0; i < NBIQ_MAIN; ++i){
		biq[i*BIQ_COEFF] = a1[i] >> 1; //a1
		biq[(i*BIQ_COEFF)+1] =  a2[i];			//a2
		biq[(i*BIQ_COEFF)+2] =  b2[i];			//b2

		biq[(i*BIQ_COEFF)+3] =  b0[i];			//b0
		biq[(i*BIQ_COEFF)+4] = b1[i] >> 1;	//b1
	}
	iircas5(in, biq, out, delay_buff, NBIQ_MAIN, 1);

}

void main(void)

{
	Uint16 i;
	Int16 input, notUsed, output;
	USBSTK5515_init();


	getFreq(f0, w0);
	calculateCoeffs();



//	for(i=0; i<NBIQ_MAIN; i++)
//	{
//		if(i == 1){
//			biq[i*BIQ_COEFF]     = (biq_32[i*BIQ_COEFF] + 1) >> 1;		//a1
//			biq[(i*BIQ_COEFF)+1] =  biq_32[(i*BIQ_COEFF)+1];			//a2
//			biq[(i*BIQ_COEFF)+2] =  biq_32[(i*BIQ_COEFF)+2];			//b2
//
//			biq[(i*BIQ_COEFF)+3] =  biq_32[(i*BIQ_COEFF)+3] >> 1;			//b0
//			biq[(i*BIQ_COEFF)+4] = (biq_32[(i*BIQ_COEFF)+4] + 1) >> 1;	//b1
//		}
//		else{
//			biq[i*BIQ_COEFF]     = (biq_32[i*BIQ_COEFF] + 1) >> 1;		//a1
//			biq[(i*BIQ_COEFF)+1] =  biq_32[(i*BIQ_COEFF)+1];			//a2
//			biq[(i*BIQ_COEFF)+2] =  biq_32[(i*BIQ_COEFF)+2];			//b2
//
//			biq[(i*BIQ_COEFF)+3] =  biq_32[(i*BIQ_COEFF)+3];			//b0
//			biq[(i*BIQ_COEFF)+4] = (biq_32[(i*BIQ_COEFF)+4] + 1) >> 1;	//b1
//		}
//	}

	for(i=0; i< NBIQ_MAIN*2 + 2; i++)
		delay_buff[i] = 0; //Initialize the registers to zero

	AIC_init();
	TCR0 = TIME_STOP;
	TCR0 = TIME_START;

	while(1)
	{
		AIC_read2(&input, &notUsed);
		input = input>>5;
		start_time = TIMCNT1_0;
		processEQ(&input, &output);
		//iircas5(&input, biq, &output, delay_buff, NBIQ, 1);
		end_time = TIMCNT1_0;
		delta_time = (start_time-end_time) << 1;
		output = output<<5;
		//printf("%d\n", delta_time);
		AIC_write2(output, input);
	}
}
