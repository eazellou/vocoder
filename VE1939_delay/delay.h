#ifndef DELAY_H
#define DELAY_H

//DATA bitDepths[15] = {0xC000, 0xE000, 0xF000, 0xF800, 0xFC00, 0xFE00, 0xFF00, 0xFF80, 0xFFC0, 0xFFE0, 0xFFF0, 0xFFF8, 0xFFFC, 0xFFFE, 0xFFFF};
#define MAXDELAYSIZE 32768
#define halfQ 16384

Uint16 writeIndex = 0;
Uint16 readIndex = 0;
Uint16 delayCounter = 0;
Uint16 delayLengthSamp = 0;

DATA test2 = 28000;

DATA delayBuffer[MAXDELAYSIZE]; //this should be some maximum, dependent on how much memory is available

DATA hexDelaySize = 0x7FFF;

//void processBitCrush(DATA *in, DATA *out, int delayLength, int length) {
//	DATA bitEnforcer = bitDepths[bitDepth - 1];
//	int i;
//	for (i = 0; i < length; i++) {
//		out[i] = in[i] & bitEnforcer;
//	}
//}

//increment read/write indices, and make it circular by looping back to beginning
//may be more effecient to make DELAYSIZE a power of two, and use masking instead of if statements
//maybe use a uint_16 (max value 65536)
void circIncReadWrite()
{
	++writeIndex;
	++readIndex;
//   if(writeIndex >= MAXDELAYSIZE)
//   {
//      writeIndex = 0;
//   }
//   if(readIndex >= MAXDELAYSIZE)
//   {
//      readIndex = 0;
//   }
	writeIndex &= hexDelaySize;
	readIndex &= hexDelaySize;
}

//this will place the readIndex in the correct place behind the writeIndex depending on delayLengthSamp
//also wraps the read index around depending on where the writeIndex is
void calculateRead()
{
	if (writeIndex < delayLengthSamp)
	{
		readIndex = MAXDELAYSIZE - delayLengthSamp + writeIndex;
	}
	else if (writeIndex >= delayLengthSamp)
	{
		readIndex = writeIndex - delayLengthSamp;
	}
}

void processDelay(DATA *in, DATA *out, Uint16 delayLengthSampIn, int length)
{
	if (delayLengthSampIn != delayLengthSamp)
	{
		delayLengthSamp = delayLengthSampIn;
		calculateRead();
	}

	//this loops and puts samples in delayBuffer from inputBuffer,
	// then puts samples into the output buffer from the delayBuffer
	for (delayCounter = 0; delayCounter < length / 2 + 1; ++delayCounter)
	{
		delayBuffer[writeIndex] = (DATA)((((LDATA)(in[2*delayCounter]) + (LDATA)(delayBuffer[readIndex])) * (LDATA)(test2))>>15); //write input + feedback
		out[2*delayCounter] =  in[2*delayCounter] + delayBuffer[readIndex]; //read input plus delay

		++writeIndex;
		++readIndex;
		writeIndex &= hexDelaySize;
		readIndex &= hexDelaySize;
	}
}

#endif


