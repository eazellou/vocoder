/*
 * main.c
 */

#include <Dsplib.h>
#include <usbstk5515.h>
#include <AIC_func.h>
#include <usbstk5515_interrupts.h>
#include <stdint.h>
#include <stdio.h>

extern volatile int16_t k1, k2;

void C55_setup();

int main(void) {
	

	C55_setup();
	printf("Hi2\n");
	_enable_interrupts();

	//printf("Hi3\n");
	while(1){
		printf("K1 value: %d\n", k1);
		printf("K2 value: %d\n", k2);

	}

	return 0;
}
