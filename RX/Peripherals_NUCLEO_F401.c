/*-----------------------------------------------------------------------------
 * Name:    LED_NUCLEO_F401.c
 * Purpose: LED interface for NUCLEO-L152RE evaluation board
 * Rev.:    1.00
 *----------------------------------------------------------------------------*/

#include "stm32f4xx.h"                         // Device header
#include <stdint.h>

#define POCET_VYVODU_BRANY  16
#define LED  5


int32_t LED_Initialize (uint32_t NUM)                  // void mozno nahradit promennou
{    RCC->AHB1ENR |= (1ul << 0); // Povoleni hodinového signálu pro GPIOA
    // Nastaveni vývodu PA.5 (Zelena LED) na výstup push-pull
    // bez upnutí k napájení nebo zemi// bez upnutí k napájení nebo zemi
    GPIOA->MODER &= ~((3ul << 2*NUM)); // Stav po nulovaní (rušeni předchozího stavu)
    GPIOA->MODER |= ((1ul << 2*NUM)); // Vystup
    GPIOA->OTYPER &= ~((1ul << NUM)); // Push-Pull
    GPIOA->OSPEEDR &= ~((3ul << 2*NUM)); // Rušeni předchozího stavu
    GPIOA->OSPEEDR |= ((1ul << 2*NUM)); // Medium speed
    GPIOA->PUPDR &= ~((3ul << 2*NUM)); // Bez Pull DOWN i Pull UP
    return (0);
}



int32_t GPIO_Initialize(GPIO_TypeDef *GPIO, uint32_t PIN, unsigned char MODER, unsigned char clkPort)
{
    RCC->AHB1ENR  |= (1ul << clkPort);			// Peripheral Clk Enable | 0:A, 1:B, 2:C ...
	GPIO->MODER   &= ~((3ul << 2*PIN));         // MODER reset
    GPIO->MODER   |=  ((MODER << 2*PIN));       // MODER set based on rec value (only accepts 00/01/10/11)
    GPIO->OSPEEDR &= ~(3ul << 2*PIN);           // OSPEEDR reset
    GPIO->OSPEEDR |=  (1ul << 2*PIN);           // OSPEEDR set, (static medium speed for now sufficient)
    GPIO->PUPDR   &= ~(3ul << 2*PIN);           // PUPDR reset
    GPIO->PUPDR   |=  (1ul << 2*PIN);           // PUPDR set to (static pull up, sufficient for now)

    return(0);
}

void TIM2_Initialize(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;					// enable clock for TIM2
	TIM2->PSC = 240;									// prescaler division factor (max is 65536), assumes sysClk 48MHz
	TIM2->ARR = 250; 									// count to xxx debug x1000
	TIM2->DIER |= TIM_DIER_UIE;							// enable interrupt from TIM2
	NVIC_EnableIRQ(TIM2_IRQn);							// enable interrupt from TIM2 in NVIC
	TIM2->CR1 |= TIM_CR1_CEN;							// start clock
	TIM2->SR &= ~TIM_SR_UIF;							// clear IRQ flag from TIM2
}

int32_t Buttons_Initialize(char P, int8_t NUM)
{
    if (P=='A'){
    	RCC->AHB1ENR |= (1ul << 0);				// povoleni hodinoveho signalu pro GPIOA
        GPIOA->MODER	&= ~(3ul << 2*NUM);		// vstup
        GPIOA->OSPEEDR	&= ~(3ul << 2*NUM);		// ruseni predchoziho stavu
        GPIOA->OSPEEDR	|=  (1ul << 2*NUM);		// Medium speed
        GPIOA->PUPDR	&= ~(3ul << 2*NUM);		// bez upinacich odporu
    }
    else {
    	RCC->AHB1ENR |= (1ul << 1);				// povoleni hodinoveho signalu pro GPIOB
        GPIOB->MODER	&= ~(3ul << 2*NUM);		// vstup
        GPIOB->OSPEEDR	&= ~(3ul << 2*NUM);		// ruseni predchoziho stavu
        GPIOB->OSPEEDR	|=  (1ul << 2*NUM);		// Medium speed
        GPIOB->PUPDR	&= ~(3ul << 2*NUM);		// bez upinacich odporu
    }

	return(0);
}

int32_t Buttons_AF_Initialize(char P, int8_t NUM)
{
    if (P=='A'){
    	RCC->AHB1ENR |= (1ul << 0);				// povoleni hodinoveho signalu pro GPIOA
        GPIOA->MODER	&= ~(3ul << 2*NUM);		// vstup
        GPIOA->MODER   |=  (2ul << 2*NUM);      // Nastaveni alternativni funkce
        GPIOA->OSPEEDR	&= ~(3ul << 2*NUM);		// ruseni predchoziho stavu
        GPIOA->OSPEEDR	|=  (1ul << 2*NUM);		// Medium speed
        GPIOA->PUPDR	&= ~(3ul << 2*NUM);		// bez upinacich odporu
        // vybrat AFR
    }
    else {
    	RCC->AHB1ENR |= (1ul << 1);				// povoleni hodinoveho signalu pro GPIOB
        GPIOB->MODER	&= ~(3ul << 2*NUM);		// vstup
        GPIOB->MODER   |=  (2ul << 2*NUM);      // Nastaveni alternativni funkce
        GPIOB->OSPEEDR	&= ~(3ul << 2*NUM);		// ruseni predchoziho stavu
        GPIOB->OSPEEDR	|=  (1ul << 2*NUM);		// Medium speed
        GPIOB->PUPDR	&= ~(3ul << 2*NUM);		// bez upinacich odporu

        GPIOB->AFR[0] |= GPIO_AFRL_AFRL0 & 0x00000002;	// AFRL0 (PB0) na AF2 	// dle tabulky v datasheetu

    }

	return(0);
}

void Button1_IRQ_Init(void)			// zobecnit pro dalsi tlacitka
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ;				// povoleni hodinoveho signalu pro GPIOA
	RCC->APB2ENR|= RCC_APB2ENR_SYSCFGEN;				// povoleni hod signalu v sys kontroleru

	EXTI->FTSR |= EXTI_FTSR_TR1;						// reakce na sestupnou hranu

	SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI1;			// odepnuti EXTI1 od jakehokoli GPIO
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI1_PA;		// prirazeni preruseni od brany 1 na EXTI line 1
	EXTI->IMR |= EXTI_IMR_MR1;							// povoleni preruseni od exti l 1
	NVIC_EnableIRQ(EXTI1_IRQn);						// povoleni externiho preruseni v kontroleru NVIC
	NVIC_SetPriority(EXTI1_IRQn, 0); 				// Set priority (0 is the highest)

}

void Button2_IRQ_Init(void)			// zobecnit pro dalsi tlacitka
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN ;				// povoleni hodinoveho signalu pro GPIOA
	RCC->APB2ENR|= RCC_APB2ENR_SYSCFGEN;				// povoleni hod signalu v sys kontroleru

	EXTI->FTSR |= EXTI_FTSR_TR4;						// reakce na sestupnou hranu

	SYSCFG->EXTICR[1] &= ~SYSCFG_EXTICR2_EXTI4;			// odepnuti EXTI1 od jakehokoli GPIO
	SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI4_PA;		// prirazeni preruseni od brany 4 na EXTI line 4
	EXTI->IMR |= EXTI_IMR_MR4;							// povoleni preruseni od exti l 4
	NVIC_EnableIRQ(EXTI4_IRQn);						// povoleni externiho preruseni v kontroleru NVIC
	NVIC_SetPriority(EXTI4_IRQn, 0); 				// Set priority (0 is the highest)

}

uint32_t Buttons3_GetState(void)
{
    if ((GPIOB->IDR & (1ul << 0)) == 0) return(1);
    else return(0);
}


int32_t GPIO_High(GPIO_TypeDef *GPIO, uint32_t NUM) {

	GPIO->BSRR |= (1ul << NUM);	// 1 v BSRRi zapisuje 1 na ODRi

	return(0);
}

int32_t GPIO_Low(GPIO_TypeDef *GPIO, uint32_t NUM) {

	GPIO->BSRR |= ((1ul << NUM)<<16);	// 1 v BSRRi+size zapisuje 0 na ODRi

	return(0);
}


void dispDigit(int8_t segSel, int8_t i_seg, int8_t i_dot, int8_t data) {
	// nejdriv odzadu seriove poslu data byte
	// pak segSel byte
	// 595 dela shiftleft, takze posilame v opacnem poradi nez chceme cist

	// pridani tecky
	if (i_seg == i_dot) {
		data |= 0x80;
	}

	// pro rozsviceni ale chci Low, pro zhasnuti High, data pin (PA9)
	for (int8_t i=7; i>=0; i--) {
		if (data & (0x1<<i)) GPIO_Low(GPIOA, 9);
		else GPIO_High(GPIOA,9);
		// pro kazdy poslany bit vygeneruj pulz na SHCP (PA8)
		GPIO_High(GPIOA, 8), GPIO_Low(GPIOA, 8);
	}

	for (int8_t i=7; i>=0; i--) {
		if (segSel & (0x1<<i)) GPIO_High(GPIOA, 9);
		else GPIO_Low(GPIOA,9);
		// pro kazdy poslany bit vygeneruj pulz na SHCP (PA8)
		GPIO_High(GPIOA,8), GPIO_Low(GPIOA, 8);
	}

	// po naposouvani celych 2 Bytu vygeneruj Latch Pulz (PB5)
	GPIO_High(GPIOB,5), GPIO_Low(GPIOB,5);

}

void USART2_Initialize(void) {
	RCC->APB1ENR |= (0x1ul << 17);			// Enable clock for UART2
	USART2->CR1  |= (0x1ul << 13); 			// Enable USART2
	USART2->CR1  |= (0x1ul << 2);  	    	// Enable RX
	USART2->CR1  |= (0x1ul << 3);			// Enable TX
	USART2->BRR   = 0x4E2;					// USART_DIV for baudrate 9600 and clock 12 MHz

    GPIO_Initialize(GPIOA, 2, 0x2, 0);			// USART2_TX - initialize + MODER set for AF
    GPIO_Initialize(GPIOA, 3, 0x2, 0);			// USART2_RX - initailze + MODER set for AF

    GPIOA->AFR[0] = 0x0700 ; 				// AF AF7 na PA2
    GPIOA->AFR[0] |= (0x07 << 12);			// AF AF7 na PA3

}



void InputCapture_Initialize(void) // Odvozena z časovače TIM3
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;				// Povoleni hodin časovače TIM3
	TIM3->CR1 |= TIM_CR1_ARPE|TIM_CR1_URS;			// Reset 0000, CKD=00, ARPE-buffred,
													// CMS=00, DIR=UP, OPM=0, USR=jen
													// přetečení, UDIS=enable
													// CEN=zatím nepovolen
	TIM3->PSC = 24000;								// prescaler division factor (max is 65536), assumes sysClk 24MHz
	TIM3->ARR = 100000;								// count to xxx

	TIM3->CCMR2 |= TIM_CCMR2_CC3S_0; 				// Aktivace kanálu 3 na vstup TI3
	TIM3->CCER &= ~(TIM_CCER_CC3NP | TIM_CCER_CC3P); // Nulování bitu CC3NP a CC3P
	TIM3->CCER |= TIM_CCER_CC3P; 					// Reakce na sestupnou hranu
	TIM3->SMCR |=TIM_SMCR_SMS&0x0; 					// Interní hodiny přímo do předdělice
	TIM3->CCER |= TIM_CCER_CC3E; 					// Povoleni zachyceni citace TIM3
	TIM3->SR &= ~TIM_SR_UIF; 						// Nulovaní příznaku události
													// od časovače TIM3
	TIM3->SR &= ~TIM_SR_CC3IF; 						// Nulování příznaku zachycení v kanálu 3
	TIM3->CCMR2 |= TIM_CCMR2_CC3S&0x01; 			// IC3 mapovaný na TI3
	TIM3->DIER |= TIM_DIER_CC3IE; 					// Povoleni přerušení při zachycení CC3

	TIM3->CR1 |= TIM_CR1_CEN; 						// Povoleni časovače TIM3 (start clock)
}

void SystemCoreClockSetHSI(void) {

  RCC->CR |= ((uint32_t)RCC_CR_HSION);                     // Enable HSI
  while ((RCC->CR & RCC_CR_HSIRDY) == 0);                  // Wait for HSI Ready

  RCC->CFGR = RCC_CFGR_SW_HSI;                             // HSI is system clock
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);  // Wait for HSI used as system clock

  FLASH->ACR  = FLASH_ACR_PRFTEN;                          // Enable Prefetch Buffer
  FLASH->ACR |= FLASH_ACR_ICEN;                            // Instruction cache enable
  FLASH->ACR |= FLASH_ACR_DCEN;                            // Data cache enable
  FLASH->ACR |= FLASH_ACR_LATENCY_5WS;                     // Flash 5 wait state

  RCC->CFGR |= RCC_CFGR_HPRE_DIV1;                         // HCLK = SYSCLK
  RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;                        // APB1 = HCLK/4
  RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;                        // APB2 = HCLK/2

  RCC->CR &= ~RCC_CR_PLLON;                                // Disable PLL

  // PLL configuration:  VCO = HSI/M * N,  Sysclk = VCO/P
  RCC->PLLCFGR = ( 16ul                   |                // PLL_M =  16
                 (384ul <<  6)            |                // PLL_N = 384
                 (  3ul << 16)            |                // PLL_P =   8
                 (RCC_PLLCFGR_PLLSRC_HSI) |                // PLL_SRC = HSI
                 (  8ul << 24)             );              // PLL_Q =   8

  RCC->CR |= RCC_CR_PLLON;                                 // Enable PLL
  while((RCC->CR & RCC_CR_PLLRDY) == 0) __NOP();           // Wait till PLL is ready

  RCC->CFGR &= ~RCC_CFGR_SW;                               // Select PLL as system clock source
  RCC->CFGR |=  RCC_CFGR_SW_PLL;
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);  // Wait till PLL is system clock src
}

void ADC_Initialize(void) {
	// Initializace HSI hodin je již v podprogramu SystemCoreClockSetHSI

	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;		// povoleni hodin pro ADC1
	ADC->CCR &= ~ADC_CCR_TSVREFE; 			// Zakázání teplotního senzoru
	ADC->CCR |= ADC_CCR_ADCPRE_1; 			// Předdělič ADC = HSI/4
	// ADC1->SQR1 &= ~ADC_SQR1_L; 				// Jeden převod
	ADC1->SQR3 &= ~(3u << 1);				// first conversion in regular sequence from channel 0 PA0
											// reset state - all conversions from channel0 (PA_0)
	ADC1->CR1 &= ~(3ul << 24 );				// 12 bitová konfigurace  ?? kdyztak zkopirovat z prezentace
    ADC1->SMPR2 &= ~(7ul);			        // channel0 sample rate: 3 cycles

	ADC1->CR2 |= ADC_CR2_ADON; 				// Povolení ADC převodníku
}


