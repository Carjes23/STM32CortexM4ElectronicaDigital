/*
 * PLLDriver.h
 *
 *  Created on: May 21, 2023
 *      Author: daniel
 */

#ifndef PLLDRIVER_H_
#define PLLDRIVER_H_

#define MCO1_PRESCALER_DIV_2	0b100
#define MCO1_PRESCALER_DIV_3	0b101
#define MCO1_PRESCALER_DIV_4	0b110
#define MCO1_PRESCALER_DIV_5	0b111

#define MCO1_HSI_CHANNEL	0b00
#define MCO1_LSE_CHANNEL	0b01
#define MCO1_HSE_CHANNEL	0b10
#define MCO1_PLL_CHANNEL	0b11

#define HSI_CLOCK_CONFIGURED		0
#define HSE_CLOCK_CONFIGURED		1
#define PLL_CLOCK_CONFIGURED		2

#define PLL_STANDARD_FREQ	80

void configPLL(uint16_t freqPLL);
uint8_t getConfigPLL(void);
uint16_t getFreqPLL(void);
void configPresMCO1(uint8_t prescaler);
void configChannelMCO1(uint8_t channel);
void changeTrim(uint8_t newValue);

#endif /* PLLDRIVER_H_ */
