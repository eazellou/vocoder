/*
 * UART setup code for VE1939 FX C5515
 */

#include <stdint.h>
#include "usbstk5515.h"

#define IVPD *((volatile unsigned *)0x00000049)
#define IVPH *((volatile unsigned *)0x0000004A)
#define IER0 *((volatile unsigned *)0x00000000)
#define IFR0 *((volatile unsigned *)0x00000001)
#define UART_BIT (1<<6)
#define UART_VEC 0x30

void Reset();

volatile uint16_t UART_input = 0;
volatile int16_t UART_input_flag = 0;

typedef enum{IDLE, K1, K2, K3, K4, K5, K6, K7, K8} KnobState;
KnobState UARTstate = IDLE;

volatile int16_t k1 = 0, k2 = 0, k3 = 0, k4 = 0, k5 = 0, k6 = 0, k7 = 0, k8 = 0;


interrupt void uart_interrupt_handler(void){
	int16_t temp;

	temp = UART_RBR;

	if(temp == 255){
		UARTstate = IDLE;
	}

	switch(UARTstate){
	case IDLE:
		if(temp == 255){
			UARTstate = K1;
		}
		break;
	case K1:
		k1 = UART_RBR;
		UARTstate = K2;
		break;
	case K2:
		k2 = UART_RBR;
		UARTstate = K3;
		break;
	case K3:
		k3 = UART_RBR;
		UARTstate = K4;
		break;
	case K4:
		k4 = UART_RBR;
		UARTstate = K5;
		break;
	case K5:
		k5 = UART_RBR;
		UARTstate = K6;
		break;
	case K6:
		k6 = UART_RBR;
		UARTstate = K7;
		break;
	case K7:
		k7 = UART_RBR;
		UARTstate = K8;
		break;
	case K8:
		k8 = UART_RBR;
		UARTstate = IDLE;
		break;
	}
}


//interrupt void uart_interrupt_handler(void){
//	UART_input = UART_RBR;
//	UART_input_flag = 1;
//}

void InitUART(uint16_t baud_divisor)
{
	uint32_t resetloc;

	UART_PWREMU_MGMT = 0x6001;  // enable rcvr, xmtr and free run
	UART_IER = 0x0001;  // enable UART receiver interrupts
	UART_FCR = 0x0084;  // enable FIFOs
	UART_LCR = 0x0007;  // 8-bit characters, 2 stop bits
	UART_MCR = 0x0000;
	UART_DLL = baud_divisor;       // divisor low 8 bits
	UART_DLH = baud_divisor >> 8;  // divisor high 8 bits

	resetloc = (uint32_t)Reset;
	IVPD = (uint16_t)(resetloc>>8);
	IVPH = (uint16_t)(resetloc>>8);

	// link the UART interrupt handler into the interrupt vector

	*((uint16_t *)((resetloc+UART_VEC)>>1))  = (uint16_t)((uint32_t)uart_interrupt_handler>>16);
	*((uint16_t *)((resetloc+UART_VEC+2)>>1))= (uint16_t)(0x0000FFFF&(uint32_t)uart_interrupt_handler);

	UART_input_flag = 0;
	//sw_uart_80 = 0;

	// enable the UART receiver interrupt and clear its flag

	IER0 = IER0 | UART_BIT;
	IFR0 = IFR0 & UART_BIT;
}
