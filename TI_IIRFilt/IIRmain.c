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

#define NUM_SEC 2
#define BIQ_COEFF 5

//useful for timing routine!
#define TCR0 		*((ioport volatile Uint16 *)0x1810)
#define TIMCNT1_0 	*((ioport volatile Uint16 *)0x1814)
#define TIME_START  0x8001
#define TIME_STOP   0x8000

Uint16 start_time, end_time, delta_time;

Int16 biq[NUM_SEC * BIQ_COEFF];

Int16 delay_buff [NUM_SEC*2 + 2];

//Int32 biq_32[NUM_SEC * BIQ_COEFF] = {
//		-53768,	22390,	731,	731,	-73,
//		-55822,	26004,	6363,	6363,	-9777,
//		-57921,	29581,	16521,	16521,	-28615,
//		-59463,	31878,	23308,	23308,	-41434};

Int32 biq_32[NUM_SEC * BIQ_COEFF] = {
		-41931, 15650, 1622, 1622, 3243, //LPF
//		-60665, 31297, 31107, 32958, -60665, //BPF
		-56485, 24823, 28519, 28519, -57038 //HPF

};

void main(void)

{
	Uint16 i;
	Int16 input, notUsed, output;
	USBSTK5515_init();

	for(i=0; i<NUM_SEC; i++)
	{
//		if(i == 1){
			biq[i*BIQ_COEFF]     = (biq_32[i*BIQ_COEFF] + 1) >> 1;		//a1
			biq[(i*BIQ_COEFF)+1] =  biq_32[(i*BIQ_COEFF)+1];			//a2
			biq[(i*BIQ_COEFF)+2] =  biq_32[(i*BIQ_COEFF)+2];			//b2

			biq[(i*BIQ_COEFF)+3] =  biq_32[(i*BIQ_COEFF)+3];			//b0
			biq[(i*BIQ_COEFF)+4] = (biq_32[(i*BIQ_COEFF)+4] + 1) >> 1;	//b1
//		}
//		else{
//			biq[i*BIQ_COEFF]     = (biq_32[i*BIQ_COEFF] + 1) >> 1;		//a1
//			biq[(i*BIQ_COEFF)+1] =  biq_32[(i*BIQ_COEFF)+1];			//a2
//			biq[(i*BIQ_COEFF)+2] =  biq_32[(i*BIQ_COEFF)+2];			//b2
//
//			biq[(i*BIQ_COEFF)+3] =  biq_32[(i*BIQ_COEFF)+3];			//b0
//			biq[(i*BIQ_COEFF)+4] = (biq_32[(i*BIQ_COEFF)+4] + 1) >> 1;	//b1
//		}
	}

	for(i=0; i< NUM_SEC*2 + 2; i++)
		delay_buff[i] = 0; //Initialize the registers to zero

	AIC_init();
	TCR0 = TIME_STOP;
	TCR0 = TIME_START;

	while(1)
	{
		AIC_read2(&input, &notUsed);
		input = input>>5;
		start_time = TIMCNT1_0;
		iircas5(&input, biq, &output, delay_buff, NUM_SEC, 1);
		end_time = TIMCNT1_0;
		delta_time = (start_time-end_time) << 1;
		output = output<<5;
		//printf("%d\n", delta_time);
		AIC_write2(output, input);
	}
}
