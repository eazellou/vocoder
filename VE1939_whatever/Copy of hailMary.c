///*
// * freq_fir_filter.c
// *
// *  Created on: Oct 1, 2016
// *      Author: GSI
//*/
//
//#include <Dsplib.h>
//#include <usbstk5515.h>
//#include <AIC_func.h>
//#include <usbstk5515_interrupts.h>
//#include <stdio.h>
//#include <math.h>
//#include <stdbool.h>
//#include "FIFO_builder.h"
//
//#define BUFSIZE 512
//#define ADC_FIFO_SIZE 1024
//#define OVERLAP BUFSIZE/4
//bool keepSample;
//
//AddIndexFifo(Synth_, 1024, Int16, 1, 0)
//AddIndexFifo(Speech_,1024, Int16, 1, 0)
//AddIndexFifo(DAC_,1024, Int16, 1, 0)
//
//DATA inputSpeech [2*BUFSIZE];
//DATA inputSynth  [2*BUFSIZE];
//DATA output [2*BUFSIZE];
//DATA extra [OVERLAP-1];
//Int16 flag;
//
//DATA speechReal[BUFSIZE];
//DATA speechImag[BUFSIZE];
//DATA synthReal[BUFSIZE];
//DATA synthImag[BUFSIZE];
//DATA phase[BUFSIZE];
//DATA phaseSine[BUFSIZE];
//DATA phaseCosine[BUFSIZE];
//
//LDATA speechRealLong[BUFSIZE];
//LDATA speechImagLong[BUFSIZE];
//LDATA synthRealLong[BUFSIZE];
//LDATA synthImagLong[BUFSIZE];
//LDATA phaseSineLong[BUFSIZE];
//LDATA phaseCosineLong[BUFSIZE];
//
//void Reset();
//
//interrupt void I2S_ISR()
//{
//	if (keepSample) {
//		Int16 right, left, out;
//		AIC_read2(&right, &left);
//		Speech_Fifo_Put(right);
//		Synth_Fifo_Put(left);
//		flag = DAC_Fifo_Get(&out);
//		if(flag == 0) out = 0;
//		AIC_write2(out, out);
//		IFR0 &= (1 << I2S_BIT_POS);//Clear interrupt Flag
//
//		keepSample = false;
//	} else {
//		keepSample = true;
//	}
//}
//
//
//// Setup AIC interrupt routine.
//void I2S_interrupt_setup(void)
//{
//	Uint32 reset_loc = (Uint32)Reset;
//	IVPD = reset_loc >> 8;//pointer table points to Reset
//	IVPH = reset_loc >> 8;//Ditto
//
//	*((Uint32*)((reset_loc + I2S_ISR_OFFSET)>>1)) = (Uint32)I2S_ISR;
//
//	SYS_EXBUSSEL |= (0x1 << 8);//Set the External bus select SP0Mode to I2S
//	IER0 |= (1 << I2S_BIT_POS);//Set up the Global interrupt register to flag on I2S receive flag
//	IFR0 &= (1 << I2S_BIT_POS);
//}
//
//void splitRealImag(DATA *input, DATA *real, DATA *imag) {
//	int i = 0;
//	for (i = 0; i < BUFSIZE; ++i) {
//		real[i] = input[2*i];
//		imag[i] = input[2*i + 1];
//	}
//}
//
//void combineRealImag(DATA *real, DATA *imag, DATA *out) {
//	int i = 0;
//	for (i = 0; i < BUFSIZE; ++i) {
//		out[2*i] = real[i];
//		out[2*i+1] = imag[i];
//	}
//}
//
//ushort square(LDATA *input, int length) {
//	return mul32(input, input, input, length);
//}
//
//void vecCastDown4(LDATA *x1, LDATA *x2, LDATA *x3, LDATA *x4, DATA *out1, DATA *out2, DATA *out3, DATA *out4, int length) {
//	int i = 0;
//	for (i = 0; i < length; ++i) {
//		out1[i] = (DATA) x1[i];
//		out2[i] = (DATA) x2[i];
//		out3[i] = (DATA) x3[i];
//		out4[i] = (DATA) x4[i];
//	}
//}
//
//void vecCastUp4(DATA *x1, DATA *x2, DATA *x3, DATA *x4, LDATA *out1, LDATA *out2, LDATA *out3, LDATA *out4, int length) {
//	int i = 0;
//	for (i = 0; i < length; ++i) {
//		out1[i] = (LDATA) x1[i];
//		out2[i] = (LDATA) x2[i];
//		out3[i] = (LDATA) x3[i];
//		out4[i] = (LDATA) x4[i];
//	}
//}
//
//LDATA pointFive[BUFSIZE];
//
//void main(void)
//{
//
//    DAC_Fifo_Init();
//    Speech_Fifo_Init();
//    Synth_Fifo_Init();
//
//    keepSample = true;
//    int j = 0;
//    for(j = 0; j < BUFSIZE; j++){
//    	pointFive[j] = 0.5 * 32768;
//    }
//
//	Int16 ctr;
//	Int32 insize = BUFSIZE - OVERLAP + 1;
//    //Initialize to all zeros
//    for(ctr=0; ctr<2*BUFSIZE; ctr++)
//    {   inputSpeech[ctr] = 0;
//    	inputSynth[ctr] = 0;
//        if(ctr < OVERLAP-1) extra[ctr] = 0;
//    }
//
//	USBSTK5515_init();
//	AIC_init();
//	I2S_interrupt_setup();
//	_enable_interrupts();
//
//	while(1)
//	{
//		if(Speech_Fifo_Size() >= insize && Synth_Fifo_Size() >= insize)
//		{
//
//            //Throw input data into zero-padded frame
//            for(ctr=0; ctr<insize; ctr++) {
//               Speech_Fifo_Get(&inputSpeech[2*ctr]);
//               Synth_Fifo_Get(&inputSynth[2*ctr]);
//            }
//
//			//Do cfft with scaling.
//			cfft_SCALE(inputSpeech, BUFSIZE);
//			cbrev(inputSpeech, inputSpeech, BUFSIZE);
//
//			//Do cfft with scaling.
//			cfft_SCALE(inputSynth, BUFSIZE);
//			cbrev(inputSynth, inputSynth, BUFSIZE);
//
//			splitRealImag(inputSynth, synthReal, synthImag);
//			splitRealImag(inputSpeech, speechReal, speechImag);
//
////			// get phase of synth
////			atan2_16(synthImag, synthReal, phase, BUFSIZE);
////			// get magnitudes mag = sqrt(r^2 + i^2)
//
//			// cast up for multiply
//			vecCastUp4(speechReal, speechImag, synthReal, synthImag,
//					   speechRealLong, speechImagLong, synthRealLong, synthImagLong, BUFSIZE);
////			square(speechRealLong, BUFSIZE);
////			square(speechImagLong, BUFSIZE);
////			square(synthRealLong, BUFSIZE);
////			square(synthImagLong, BUFSIZE);
//
//			mul32(speechRealLong, pointFive, speechRealLong, BUFSIZE);
//			mul32(speechImagLong, pointFive, speechImagLong, BUFSIZE);
//
//			// cast back down
//			vecCastDown4(speechRealLong, speechImagLong, synthRealLong, synthImagLong,
//					     speechReal, speechImag, synthReal, synthImag, BUFSIZE);
//
////			// don't scale for now
////			add(speechReal, speechImag, speechReal, BUFSIZE, 0);
////			sqrt_16(speechReal, speechReal, BUFSIZE);
////			// now speechReal has magnitude of speech
////
////			add(synthReal, synthImag, synthReal, BUFSIZE, 0);
////			sqrt_16(synthReal, synthReal, BUFSIZE);
////			// now synthReal has magnitude of synth
////
////			// real = mag * cos(phase)
////			// imag = mag * sin(phase)
////
////			// put sine of phase into phaseSine and cosine of phase into phase
////			sine(phase, phaseSine, BUFSIZE);
////			// add pi/2 for cosine
////			int i = 0;
////			for (i = 0; i<BUFSIZE; i++)
////			{
////				phaseCosine[i] = phase[i] + .5;
////			}
////			sine(phaseCosine, phaseCosine, BUFSIZE);
////
////			//cast up for multiply
////			vecCastUp4(synthReal, speechReal, phaseCosine, phaseSine, synthRealLong, speechRealLong, phaseCosineLong, phaseSineLong, BUFSIZE);
////
////			// multiply magnitudes
////			mul32(speechRealLong, synthRealLong, speechRealLong, BUFSIZE);
////
////			// real part
////			mul32(speechRealLong, phaseCosineLong, synthRealLong, BUFSIZE);
////			// imaginary part
////			mul32(speechRealLong, phaseSineLong, synthImagLong, BUFSIZE);
////
////
////
////			// cast down to be safe
////			for (i = 0; i < BUFSIZE; i++) {
////				synthReal[i] = (DATA)synthRealLong[i];
////				synthImag[i] = (DATA)synthImagLong[i];
////			}
//
//
//			//combineRealImag(synthReal, synthImag, output);
//
//			combineRealImag(speechReal, speechImag, output);
//
//
//
//
//
//
//
//
////            for(ctr=0; ctr < BUFSIZE; ctr++)
////            {
////				output[2*ctr] = inputSpeech[2*ctr];// + inputSynth[2*ctr];
////
////				output[2*ctr + 1] = inputSpeech[2*ctr+1];// + inputSynth[2*ctr];
////            }
//
//
//			//Do inverse cfft with scaling
//			cifft_NOSCALE(output,BUFSIZE);
//            cbrev(output, output, BUFSIZE);
//
//            //TODO: overlap add
//            //In this section you will need to perform the overlap add.
//            //We have left the logic sections and you need to fill in
//            //which values are being set in each section. Use the "extra"
//            //buffer to handle the values that will overlap into the next
//            //filter computation. Make sure to write values to the DAC_Fifo with
//            //the command "DAC_Fifo_Put(value)".
//            //Also remember that your output alternates between real and imaginary values
//            //for each index
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
//
//            //Reset input to all zeros
//            for(ctr=0; ctr<2*BUFSIZE; ctr++){
//                inputSpeech[ctr] = 0;
//                inputSynth[ctr] = 0;
//            }
//
//            if(Speech_Fifo_Size() >= insize || Synth_Fifo_Size() >= insize) printf("error\n");
//		}
//	}
//
//}
//
//
//
//
