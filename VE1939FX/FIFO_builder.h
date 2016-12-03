// File name: FIFO_Builder.h
//
// Macros for building FIFOs.  Taken from
// "Real-TimeOperating Systems for ARM Cortex-M
// Microcontrollers" by Jonathon W. Valvano.
//
// 15Jul2014 .. file created .. K.Metzger
//
// macro to create a power of 2 index FIFO

#ifndef FIFO_BUILDER
#define FIFO_BUILDER

//	SIZE must be a power of two.
#define AddIndexFifo(NAME, SIZE, TYPE, SUCCESS, FAIL) \
uint32_t volatile NAME ## PutI; 			\
uint32_t volatile NAME ## GetI;				\
static TYPE NAME ## Fifo [SIZE]; 			\
void NAME ## Fifo_Init(void) { 				\
	NAME ## PutI = NAME ## GetI = 0;		\
}											\
int16_t NAME ## Fifo_Put (TYPE data){ 		\
	if((NAME ## PutI - NAME ## GetI) & ~(SIZE-1)) { \
		return(FAIL);						\
	}										\
	NAME ## Fifo[NAME ## PutI & (SIZE-1)] = data;	\
	NAME ## PutI++;						\
	return(SUCCESS);						\
}											\
int16_t NAME ## Fifo_Get (TYPE *datapt) {	\
	if (NAME ## PutI == NAME ## GetI) {		\
		return(FAIL);						\
	}										\
	*datapt = NAME ## Fifo[NAME ## GetI & (SIZE-1)]; \
	NAME ## GetI++;						\
	return(SUCCESS);						\
}											\
uint16_t NAME ## Fifo_Size (void) {			\
	return((uint16_t)(NAME ## PutI - NAME ## GetI)); \
}
#endif
