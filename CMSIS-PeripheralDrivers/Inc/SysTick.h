/*
 ******************************************************************************
 * @file           : SysTick.h
 * @author         : dacardenasj
 * @brief          : Main program body
 ******************************************************************************
 * @credits
 *
 * Taller V. Electronica Digital & Microcontroladores
 * Facultad de Ciencias
 * Ing. Física
 *
 * Universidad Nacional de Colombia
 * Sede Medellín
 * 2023-01S
 *
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
 ******************************************************************************
 */


#ifndef INC_SYSTICK_H_
#define INC_SYSTICK_H_

#include <stm32f4xx.h>
extern volatile uint32_t ticksDown;

#define SYSTICK_LOAD_VALUE_16MHz_1ms	16000 	// Cada 1ms
#define SYSTICK_LOAD_VALUE_100MHz_1ms	100000	// Cada 1ms
#define HSI_CLOCK_CONFIGURED 	0
#define HSE_CLOCK_CONFIGURED 	1
#define PLL_CLOCK_CONFIGURED 	2


void config_SysTick(void);
uint64_t getTicks_ms(void);
uint64_t getTicks_us(void);
void delay_ms(uint32_t wait_time_ms);
void delay_100us(uint32_t wait_time_ms);

#define SYSTICK_ticksDown()				(ticksDown)

#define SYSTICK_setTicksDown(time)		(ticksDown = (time))



#endif /* INC_SYSTICK_H_ */
