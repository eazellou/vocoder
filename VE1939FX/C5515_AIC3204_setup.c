/* File name: AIC3204_setup.c

    Initial version for use with the C5515 USBstick.  Basic code from
    Spectrum Digital test files.

   04Jun2011 .. initial version .. K.Metzger
   16Dec2014 .. added fs = 8000, 16000 .. KM
   22Dec2014 .. added swing buffered FFT 1 channel interrupt support .. KM
   26Dec2014 .. updated to use <stdint.h> .. KM
   07Feb2015 .. added DDS for testing .. KM
   02Oct2016 .. changed buffering to use FIFO .. KM
*/   

#include <stdint.h>
#define AIC3204_I2C_ADDR 0x18
#include "./C5515_inc/usbstk5515_i2c.h"
#include "./C5515_inc/usbstk5515_gpio.h"
#include "FIFO_builder.h"

// 64 value sine table for DDS

#define N_SINE 64

int16_t SineTable[N_SINE] = { 0, 3212, 6393, 9512, 12539, 15446, 18204, 20787,
 23170, 25329, 27245, 28898, 30273, 31356, 32137, 32609,
 32767, 32609, 32137, 31356, 30273, 28898, 27245, 25329,
 23170, 20787, 18204, 15446, 12539, 9512, 6393, 3212,
 0, -3212, -6393, -9512, -12539, -15446, -18204, -20787,
 -23170, -25329, -27245, -28898, -30273, -31356, -32137, -32609,
 -32767, -32609, -32137, -31356, -30273, -28898, -27245, -25329,
 -23170, -20787, -18204, -15446, -12539, -9512, -6393, -3212};


// Default register contents for 48 kHz sample rate
/*
static uint16_t CR11=0x82;
static uint16_t CR12=0x87;
static uint16_t CR13=0x00;
static uint16_t CR14=0x80;
static uint16_t CR18=0x87;
static uint16_t CR19=0x82;
static uint16_t CR20=0x80;
*/
/* ------------------------------------------------------------------------ *
 *                                                                          *
 *  _AIC3204_rset( regnum, regval )                                         *
 *                                                                          *
 *      Set codec register regnum to value regval                           *
 *                                                                          *
 * ------------------------------------------------------------------------ */
int16_t AIC3204_rset( uint16_t regnum, uint16_t regval )
{
    Uint8 cmd[2];
    cmd[0] = regnum & 0x007F;       // 7-bit Register Address
    cmd[1] = regval;                // 8-bit Register Data

    USBSTK5515_I2C_write( AIC3204_I2C_ADDR, cmd, 2 );
    return 0;
}

#ifdef vc5505
// This is the hardware reset for the VC5505 chip only!!!

void aic3204_hardware_init(void)
{
 	SYS_EXBUSSEL |= 0x0020;  // Select A20/GPIO26 as GPIO26
	USBSTK5515_GPIO_init();
	USBSTK5515_GPIO_setDirection(GPIO26, GPIO_OUT);
	USBSTK5515_GPIO_setOutput( GPIO26, 1 );    // Take AIC3204 chip out of reset
	USBSTK5515_I2C_init( );                    // Initialize I2C
	USBSTK5515_wait( 100 );  // Wait
}
#else
// This is called if using C5515 and has on effect
void aic3204_hardware_init(void)
{
	return;
}
#endif

// supports fs = 48000, 16000 and 8000
// other default to 48000
//
// Note: the AIC is delta sigma.  There will be a startup
// 		 interval when samples are not valid.

void AIC3204_setup(uint16_t fs)
{

	aic3204_hardware_init(); // do this for the C5505!!!!

    /* Configure AIC3204 */

	static uint16_t CR11=0x82;
	static uint16_t CR12=0x87;
	static uint16_t CR13=0x00;
	static uint16_t CR14=0x80;
	static uint16_t CR18=0x87;
	static uint16_t CR19=0x82;
	static uint16_t CR20=0x80;

	switch (fs) {
		case 48000:
			CR11=0x82;	// power up NDAC and set to 2
			CR12=0x87;	// power up MDAC and set to 7
			CR13=0x00;	// DOSR msh
			CR14=0x80;	// DOSR lsh of 128
			CR18=0x87;	// power NADC and set to 7
			CR19=0x82;	// power MADC and set to 2
			CR20=0x80;	// AOSR = 128
			break;
		case 16000:
			CR11=0x86;	// power up NDAC and set to 6
			CR12=0x87;	// power up MDAC and set to 7
			CR13=0x00;	// DOSR msh of 128
			CR14=0x80;  // DOSR lsh of 128
			CR18=0x87;  // power up NADC and set to 7
			CR19=0x86;  // power up MADC and set to 6
			CR20=0x80;	// AOSR = 128
			break;
		case 8000: 		// increasing the D/A OSR to 768
			CR11=0x82; 	// power up NDAC and set to 2
			CR12=0x87; 	// power up MDAC and set to 7
			CR13=0x03; 	// high byte of DOSR .. using 768!!!!!!!!
			CR14=0x00; 	// low byte of DOSR
			CR18=0x80+21; 	// power up NADC and set to 21
			CR19=0x84; 	// power up MADC and set to 4
			CR20=0x80; 	// AOSR .. using 128
			break;
		default:
			break;
	}

    AIC3204_rset( 0, 0 );          // Select page 0
    AIC3204_rset( 1, 1 );          // Reset codec
    AIC3204_rset( 0, 1 );          // Select page 1
    AIC3204_rset( 1, 8 );          // Disable crude AVDD generation from DVDD
    AIC3204_rset( 2, 1 );          // Enable Analog Blocks, use LDO power
    AIC3204_rset( 0, 0 );          // Select page 0
    /* PLL and Clocks config and Power Up  */
    AIC3204_rset( 27, 0x0d );      // BCLK and WCLK is set as o/p to AIC3204(Master)
    AIC3204_rset( 28, 0x00 );      // Data ofset = 0

    							   // MCLK = 12.000 MHz on C5515
    AIC3204_rset( 4, 0x03 );       // PLL setting: PLLCLK <- MCLK, CODEC_CLKIN <-PLL CLK
    AIC3204_rset( 6, 0x07 );       // PLL setting: J=7

    AIC3204_rset( 7, 0x06 );       // PLL setting: HI_BYTE(D=1680)
    AIC3204_rset( 8, 0x90 );       // PLL setting: LO_BYTE(D=1680)
    AIC3204_rset( 30, 0x88 );      // For 32 bit clocks per frame in Master mode ONLY
    							   // BCLK=DAC_CLK/N =(12288000/8) = 1.536MHz = 32*fs
    AIC3204_rset( 5, 0x91 );       // PLL setting: Power up PLL, P=1 and R=1
    AIC3204_rset( 13, CR13 );      // Hi_Byte(DOSR) for DOSR = 128 decimal or 0x0080 DAC oversamppling
    AIC3204_rset( 14, CR14 );      // Lo_Byte(DOSR) for DOSR = 128 decimal or 0x0080
    AIC3204_rset( 20, CR20 );      // AOSR for AOSR = 128 decimal or 0x0080 for decimation filters 1 to 6
    AIC3204_rset( 11, CR11 );      // Power up NDAC and set NDAC value to 2
    AIC3204_rset( 12, CR12 );      // Power up MDAC and set MDAC value to 7
    AIC3204_rset( 18, CR18 );      // Power up NADC and set NADC value to 7
    AIC3204_rset( 19, CR19 );      // Power up MADC and set MADC value to 2

    /* DAC ROUTING and Power Up */
    AIC3204_rset(  0, 0x01 );      // Select page 1
    AIC3204_rset( 12, 0x08 );      // LDAC AFIR routed to HPL
    AIC3204_rset( 13, 0x08 );      // RDAC AFIR routed to HPR
    AIC3204_rset(  0, 0x00 );      // Select page 0
    AIC3204_rset( 64, 0x02 );      // Left vol=right vol
    AIC3204_rset( 65, 0x00 );      // Left DAC gain to 0dB VOL; Right tracks Left
    AIC3204_rset( 63, 0xd4 );      // Power up left,right data paths and set channel
    AIC3204_rset(  0, 0x01 );      // Select page 1
    AIC3204_rset( 16, 0x00 );      // Unmute HPL , 0dB gain
    AIC3204_rset( 17, 0x00 );      // Unmute HPR , 0dB gain
    AIC3204_rset(  9, 0x30 );      // Power up HPL,HPR
    AIC3204_rset(  0, 0x00 );      // Select page 0
    USBSTK5515_wait( 500 );        // Wait
    
    /* ADC ROUTING and Power Up */
    AIC3204_rset( 0, 1 );          // Select page 1
    AIC3204_rset( 0x34, 0x30 );    // STEREO 1 Jack
                                   // IN2_L to LADC_P through 40 kohm
    AIC3204_rset( 0x37, 0x30 );    // IN2_R to RADC_P through 40 kohmm
    AIC3204_rset( 0x36, 3 );       // CM_1 (common mode) to LADC_M through 40 kohm
    AIC3204_rset( 0x39, 0xc0 );    // CM_1 (common mode) to RADC_M through 40 kohm
    AIC3204_rset( 0x3b, 0x17 );    // MIC_PGA_L unmute
    AIC3204_rset( 0x3c, 0x17 );    // MIC_PGA_R unmute
    AIC3204_rset( 0, 0 );          // Select page 0
    AIC3204_rset( 0x51, 0xc0 );    // Powerup Left and Right ADC
    AIC3204_rset( 0x52, 0 );       // Unmute Left and Right ADC
    
    AIC3204_rset( 0, 0 );    
    USBSTK5515_wait( 200 );        // Wait
    /* I2S settings */
    I2S0_SRGR = 0x0;
    I2S0_CR = 0x8010;    // 16-bit word, slave, enable I2C
    I2S0_ICMR = 0x20;    // enable transmission complete interrupt
}

//***********************AIC3204 non-interrupt support************************
//
//  returns left channel in most significant 16 bits and right in low 16 bits

//#define Rcv 0x08
#define XMIT 0x20

uint32_t AIC3204_IO(int16_t LeftValue, int16_t RightValue)
{
	uint16_t data3, data4;

    while((XMIT & I2S0_IR) == 0);  // Wait for interrupt pending flag
    I2S0_W0_MSW_W = LeftValue;  // 16 bit left channel transmit audio data
    I2S0_W0_LSW_W = 0;
    I2S0_W1_MSW_W = RightValue;  // 16 bit right channel transmit audio data
    I2S0_W1_LSW_W = 0;
    /* Read Digital audio */
    data3 = I2S0_W0_MSW_R;  // 16 bit left channel received audio data
    data4 = I2S0_W1_MSW_R;  // 16 bit right channel received audio data

    return ((uint32_t)data3<<16)|(uint32_t)data4;
}


//***********************AIC3204 interrupt support***************************

// Create FIFO for ADC samples.  SIZE must be power of 2.
// Creates: ADC_Fifo_Init, ADC_Fifo_Get, ADC_Fifo_Put, ADC_Fifo_Size.

#define ADC_FIFO_SIZE 1024
AddIndexFifo(ADC_, ADC_FIFO_SIZE, int32_t, 1, 0)  // 1=operation worked, 0=fail

// Create FIFO for DAC values.  Size must be power of 2.

#define DAC_FIFO_SIZE 1024
AddIndexFifo(DAC_, DAC_FIFO_SIZE, int32_t, 1, 0)  // 1=operation worked, 0=fail

// interrupt handler for the AIC3204 .. actually for I2S0

#define GPIO_OUT_0	*(volatile ioport Uint16*)(0x1c0a)
static uint32_t adc_value;
uint16_t adc_full_flag;
static uint32_t dac_value;
uint16_t dac_empty_flag;

static uint16_t PhaseAccumulator = 0, FTvalue = 1 << 13;
int16_t volatile sw_ADC_DDS = 0, sw_DDS_INTERP = 0;
static int16_t delta, sample_0, sample_1;
int16_t FTVoffset = 0x016;


interrupt void adc_interrupt_handler(void)
{
	GPIO_OUT_0 |= 0x4000;								// start timing pulse  (blue LED)
	I2S0_IR;											// reading clears interrupt flags

	if (sw_ADC_DDS) {
		sample_0 = SineTable[PhaseAccumulator >> 10];
		sample_1 = SineTable[((PhaseAccumulator >> 10)+1)&0x003F]; // for when interpolating
		delta = sample_1 - sample_0;
		if (sw_DDS_INTERP) {
			sample_0 = sample_0 + (((int32_t)delta*(PhaseAccumulator&0x03FF))>>10); //Q15 x Q10
		}
		adc_value =(((uint32_t)sample_0)<<16);
		PhaseAccumulator += FTvalue+FTVoffset;
	}
	else {

	// Get sample values from ADC and pack into 32 bit word

		adc_value =(((uint32_t)I2S0_W0_MSW_R)<<16) | (uint32_t)I2S0_W1_MSW_R;
	}

	//	save packed sample values in ADC FIFO

	if((ADC_PutI - ADC_GetI) & ~(ADC_FIFO_SIZE-1)) {
		adc_full_flag = 1; // sticky bit
	}
	else {
		ADC_Fifo[ADC_PutI & (ADC_FIFO_SIZE-1)] = adc_value;
		ADC_PutI++;
	}

	// get sample value from DAC FIFO

	if (DAC_PutI == DAC_GetI) {
		dac_empty_flag = 1;
	}
	else {
		dac_value = DAC_Fifo[DAC_GetI & (DAC_FIFO_SIZE-1)];
		DAC_GetI++;
	}

	// Unpack sample value and send to DACs

   I2S0_W0_MSW_W = dac_value>>16;
   I2S0_W1_MSW_W = dac_value&0x0000FFFF;

   GPIO_OUT_0 &= ~0x4000; // end timing pulse
   return;
}

//******************    call adc_interrupt_initialize if using interrupts**************************

#define IVPD *((volatile unsigned *)0x00000049)
#define IVPH *((volatile unsigned *)0x0000004A)
#define IER0 *((volatile unsigned *)0x00000000)
#define IFR0 *((volatile unsigned *)0x00000001)
#define PROG0 0x0028
#define PROG0_BIT 0x0020

void Reset(); // need to know where the interrupt vector is at

void adc_initialize()
{
	uint32_t resetloc;

	adc_full_flag = 0;
    dac_empty_flag = 0;
    ADC_Fifo_Init();
    DAC_Fifo_Init();

	// initialize the hardware pointers

	resetloc = (uint32_t)Reset;
	IVPD = (uint16_t)(resetloc>>8);
	IVPH = (uint16_t)(resetloc>>8);

	// link the I2S0 interrupt handler into the interrupt vector

	*((uint16_t *)((resetloc+PROG0)>>1))  = (uint16_t)((uint32_t)adc_interrupt_handler>>16);
	*((uint16_t *)((resetloc+PROG0+2)>>1))= (uint16_t)(0x0000FFFF&(uint32_t)adc_interrupt_handler);

	// enable the I2S0 interrupt and clear its flag

	IER0 = IER0 | PROG0_BIT;
	IFR0 = IFR0 & PROG0_BIT;
}
