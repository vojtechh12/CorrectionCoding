/*
 * LED_NUCLEO_F401.h
 *
 *  Created on: Nov 3, 2023
 *      Author: vojtechh
 */

#ifndef SRC_PERIPHERALS_NUCLEO_F401_H_
#define SRC_PERIPHERALS_NUCLEO_F401_H_

int32_t LED_Initialize (uint32_t NUM);
int32_t GPIO_Initialize(GPIO_TypeDef *GPIO, uint32_t NUM, unsigned char MODER, unsigned char clkPort);
int32_t GPIO_High (GPIO_TypeDef *GPIO, uint32_t NUM);
int32_t GPIO_Low (GPIO_TypeDef *GPIO, uint32_t NUM);
void dispDigit(int8_t segSel, int8_t i_seg, int8_t i_dot, int8_t data);
void TIM2_Initialize(void);
int32_t Buttons_Initialize (char P, int8_t NUM);
int32_t Buttons_AF_Initialize(char P, int8_t NUM);
void Button1_IRQ_Init(void);
void Button2_IRQ_Init(void);
uint32_t Buttons3_GetState(void);
void InputCapture_Initialize(void);
void USART2_Initialize(void);
void ADC_Initialize(void);

void SystemCoreClockSetHSI(void);

#endif /* SRC_PERIPHERALS_NUCLEO_F401_H_ */
