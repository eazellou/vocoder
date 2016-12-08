#ifndef NEWDELAY_H
#define NEWDELAY_H

#define NEWDELAYSIZE 16384
#define halfQ 16384

//include a table for changing delay time
//this could be random values (noise) between -30 and 30, perhaps. (integers)
//this represents how the delay length changes over time
//this could be probably 10000 values long (Whatever2)
//this should sum to zero aross the whole thing, so that delay isn't
//   slowly increasing or decreasing with each loop through it
#include "varyTable2.h"

//this doesn't have to be that long, like 1024 or so (if delay length is around 500)
DATA delayIn[1024];

int writeIndexCh = 700;
int readIndexCh = 0;
int varyIndexCh = 0;

int x = 0;

int hexDelayIn = 0x3FF;
int hexVaryTable = 0x1FFF;

//this is a global variable that keeps track of if a delay is done
//
//for example:
//   if the delay is 500 samples long
//   this should count up to 500 and change the delay length at the end
int delayCount = 0;
int delayLengthSampCh = 500;

//calculates where the readIndex is based on where writeIndex is
void calculateReadIndex()
{
	if (writeIndexCh < delayLengthSampCh)
	{
		readIndexCh = NEWDELAYSIZE - delayLengthSampCh + writeIndexCh;
	}
	else if (writeIndexCh >= delayLengthSampCh)
	{
		readIndexCh = writeIndexCh - delayLengthSampCh;
	}
}

//this is a single delay line
// the first two lines first fill the delay line
//  then the output with very short delay (~500 samples)
//
//then the indices are incremented
//
//delayCount is used to go into another section of code after a single full delay
// where the delayLength is changed
void processNewChorus(DATA *input, DATA *output, DATA feedbackCh, int BUFSIZE)
{
	for (x = 0; x < BUFSIZE / 2 + 1; x++)
	{
		if (feedbackCh > 10000)
		{
			//this fills the delay line with scaled input and feedback
			delayIn[writeIndexCh] = (DATA)((((LDATA)(input[2*x]) / 2 + (LDATA)(delayIn[readIndexCh])) * feedbackCh)>>15);

			//this fills the output with input (dry signal) and delayed signal
			output[2*x] = input[2*x] + delayIn[readIndexCh];
		}
		else if (feedbackCh <= 10000)
		{
			output[2*x] = input[2*x];
		}


		//increment indices and delayCount
		writeIndexCh++;
		readIndexCh++;
		delayCount++;

		//this rounds the indices for circular buffering
		writeIndexCh &= hexDelayIn;
		readIndexCh &= hexDelayIn;

		//this changes the delay length at the end of the delay
		if (delayCount >= delayLengthSampCh)
		{
		  //changes the delayLength
		  //WAIT
		  //This might be better just as:
		  delayLengthSampCh = 200 + varyTable[varyIndexCh];
		  //!!!
		  //otherwise this might end up as less than 500 at points in the middle of varyTable!!!
		  ///!!!
			//OLD
		  //delayLengthSampCh += varyTable[varyIndexCh];

		  //increment and circular buffer rounding
		  varyIndexCh++;
		  varyIndexCh &= hexVaryTable;

		  //very important here! this recalculates where the readIndex is
		  calculateReadIndex();

		  //reset counter
		  delayCount = 0;
		}
	}
}

#endif


