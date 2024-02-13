/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************/

#include "main.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "stm32f4xx.h"                  // Device header

#include "Peripherals_NUCLEO_F401.h"

#define setbit(reg, bit) ((reg) |= (1U << (bit)))
#define clearbit(reg, bit) ((reg) &= (~(1U << (bit))))
#define togglebit(reg, bit) ((reg) ^= (1U << (bit)))
#define getbit(reg, bit) ((reg & (1U << bit)) >> bit)

int8_t segSelect = 0x01;
int8_t iSeg = 0;								// select segments as 0,1,2,3
char dispChar[4] = {61,61,61,61};				// array of symbols on each 7-seg
char tmpDispChar[4] = {61,61,61,61};		    // temp  array of symbols.
char tmpChar = '0';
char lowNibble = 61;
char highNibble = 61;
int8_t iDot = 4;								// selects which segment lights up the dot
int8_t i = 0;									// select disp segment


// specifikace PINu ve strukture
struct Pin {
	char abcd;
	uint32_t num;
};

// jake piny pouzivam
struct Pin PA2, PA3; // PA5 is for debuging

void Initialize_Pins() {
	PA2.abcd = 'A', PA2.num = 2;			// USART2_TX
	PA3.abcd = 'A', PA3.num = 3;			// USART2_RX
}

// segments order: hgfedcba
const char NumbersASCII[14] = {
  0x3F, /* 0 */
  0x06, /* 1 */
  0x5B, /* 2 */
  0x4F, /* 3 */
  0x66, /* 4 */
  0x6D, /* 5 */
  0x7D, /* 6 */
  0x07, /* 7 */
  0x7F, /* 8 */
  0x6F, /* 9 */
  0x40, /* - */
  0x79, /* E */
  0x50, /* r */
  0x00, /* nothing */
};

// transposed parity check matrix H^T
char mxHt[7] = {0x05, 0x07, 0x06, 0x03, 0x04, 0x02, 0x01};

// decoding (syndrome calculation)
char syndrome(char x) {
    // hamming code syndrome calculation
    char tmp;
    char state = 0x00;
    for (int8_t i=0; i<7; i++) {
        if (((x>>(6-i)) & 0x01) == 0) {         // masking columns of parity check matrix
            continue;
        }
        tmp = mxHt[i];
        if (state != 0x00){
            tmp ^= state;
        }
        state = tmp;
    }

    return tmp;
}

void TIM2_IRQHandler(void)
{
	TIM2->SR &= ~TIM_SR_UIF;				// clear IRQ flag
	// when counter reaches 0.25 ms, multiplex the displays
	if (segSelect == 0x08){
		 segSelect = 0x01;
		 iSeg = 0;
	}
	else {
		segSelect <<= 1;
		iSeg++;
	}


	// vetveni co zobrazuju
	dispDigit(segSelect, iSeg, iDot, NumbersASCII[dispChar[iSeg] - '0']);

}

char read_char(void) {
	while(((USART2->SR) & USART_SR_RXNE) == 0);	// wait for char receive
	return (USART2->DR);
}

void flip_data(void) {							// flips data from tmp array to array displaying data
	for (int8_t i=0; i<4; i++){
		dispChar[i] = tmpDispChar[i];
		tmpDispChar[i] = 61;		// debug. off every cipher between flips
	}
	iDot = 4;
}

void clc_tmpChar(void){			// clear every cipher (off)
	for (int8_t i=0; i<4; i++){
		tmpDispChar[i] = 61;
	}
}

void disp_error(void){
	// dispChar only gives index to NumbersASCII
	clc_tmpChar();

	dispChar[0] = 59;	// E
	dispChar[1] = 60;	// r
	dispChar[2] = 60;	// r
	dispChar[3] = 61;	// OFF
}

int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();


  /* Configure the system clock */
  SystemCoreClockSetHSI();
  SystemCoreClockUpdate();


  /* Initialize all configured peripherals */
  //Initialize_Pins();

  GPIO_Initialize(GPIOA, 8, 0x1, 0);	// SHCP pin, output, clkEn
  GPIO_Initialize(GPIOA, 9, 0x1, 0);	// DS pin, output, clkEK
  GPIO_Initialize(GPIOB, 5, 0x1, 1);	// STCP (latch) pin, output, clkEn
  TIM2_Initialize();			// display multiplexing and segment control
  USART2_Initialize();			// USART2 initialize



  while (1)
  {
	  i = 0;									// disp digit position
	  char s = 0x00;							// syndrome for error corr

	  lowNibble = read_char();					// receive low nibble
	  // lowNibble >>= 3;						    // shift to discard 3 parity bits (result is 4 bits of data)
	  s = syndrome(lowNibble);

	    // error correction
	    switch (s)
	    {
	    case 0x00:
	        // errorless transmission
	        break;
	    case 0x01:
	        lowNibble ^= 0x01;
	        break;
	    case 0x02:
	    	lowNibble ^= 0x02;
	        break;
	    case 0x04:
	    	lowNibble ^= 0x04;
	        break;
	    case 0x03:  // error on bit n3
	    	lowNibble ^= 0x08;
	        break;
	    case 0x06:  // error on bit n4
	    	lowNibble ^= 0x10;
	        break;
	    case 0x07:  // error on bit n5
	    	lowNibble ^= 0x20;
	        break;
	    case 0x05:	// error on bit n6
	    	lowNibble ^= 0x40;
	        break;
	    default:
	        // throw flag for retransmission (send NACK)
	        break;
	    }
	  lowNibble >>= 3;						    // shift to discard 3 parity bits (result is 4 bits of data)

	  highNibble = read_char();
	  s = syndrome(highNibble);

	    // error correction
	    switch (s)
	    {
	    case 0x00:
	        // errorless transmission
	        break;
	    case 0x01:
	    	highNibble ^= 0x01;
	        break;
	    case 0x02:
	    	highNibble ^= 0x02;
	        break;
	    case 0x04:
	    	highNibble ^= 0x04;
	        break;
	    case 0x03:  // error on bit n3
	    	highNibble ^= 0x08;
	        break;
	    case 0x06:  // error on bit n4
	    	highNibble ^= 0x10;
	        break;
	    case 0x07:  // error on bit n5
	    	highNibble ^= 0x20;
	        break;
	    case 0x05:	// error on bit n6
	    	highNibble ^= 0x40;
	        break;
	    default:
	        // throw flag for retransmission (send NACK)
	        break;
	    }
	  highNibble >>= 3;								// shift to discard 3 parity bits (result is 4 bits of data)
	  highNibble <<= 4;								// shift to get high nibble

	  // concatenate both nibbles to complete a byte of sent data
	  lowNibble |= highNibble;

	  tmpDispChar[i] = lowNibble;

	  flip_data();

  }
}
