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

#define SYSTICK_LOAD_VALUE_16MHz_1ms 	// Cada 1ms
#define SYSTICK_LOAD_VALUE_100MHz_1ms	// Cada 1ms

void config_SysTick_ms(uint8_t systemClock);
uint64_t getTicks_ms(void);
void delay_ms(uint32_t wait_time_ms);



#endif /* INC_SYSTICK_H_ */
