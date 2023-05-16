/*
 ******************************************************************************
 * @file           : SysTick.c
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

#include <stm32f4xx.h>
#include "SysTick.h"

uint64_t ticks = 0;
uint64_t ticks_start = 0;
uint64_t ticks_counting = 0;

void config_SysTick_ms(uint8_t systemClock){
	// Reiniciamos la variable que cuenta el tiempo
	ticks = 0;

	// Cargamos el valor del limite de incrementos que representan 1ms.

	switch(systemClock){
		//Caso para el reloj HSI -> 16 MHz

		case 0:
			SysTick->LOAD = SYSTICK_LOAD_VALUE_16MHz_1ms;
			break;
		//Caso para el reloj HSE -> 16 MHz

		case 1:
			SysTick->LOAD = SYSTICK_LOAD_VALUE_16MHz_1ms;
			break;

		//Caso para el reloj HSI -> 16 MHz

		case 2:
			SysTick->LOAD = SYSTICK_LOAD_VALUE_100MHz_1ms;
			break;
		//En cualquier otro caso

		default:
			SysTick->LOAD = SYSTICK_LOAD_VALUE_16MHz_1ms;
			break;
	}

	// Limpiamos el valor actual del SysTick
	SysTick->VAL = 0;

	// Configuramos el reloj intero del timer.
	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;

	//Desactivamos las interrupciones globales
	__disable_irq();

	//Matriculamos la interrućuión en el NVIC
	NVIC_EnableIRQ(SysTick_IRQn);

	//Activamos la intrerrupción debida al conteo a cero del SysTick.
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;

	//Activamos el timer
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

	// Activamos las interrupciones globales
	__enable_irq();
}

uint64_t getTicks_ms(void){
	return ticks;
}

void delay_ms(uint32_t wait_time_ms){
	//Captura el primer valor de tiempo para comparar.
	ticks_start = getTicks_ms();

	//Captrua el segundo valor para compara comienzan en el mismo valor
	ticks_counting = getTicks_ms();

	//Compara el valor counting con el delay + start.

	while(ticks_counting < (ticks_start + (uint64_t) wait_time_ms)){

		//Actualizar el valor
		ticks_counting = getTicks_ms();
	}
}

void SysTick_Handler(void){
	//Verificamos que la interrupcion se lanzo
	if(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk){

		//limpiamos la bandera
		SysTick->CTRL &= ~SysTick_CTRL_COUNTFLAG_Msk;

		//incrementamos en 1 el contador
		ticks++;
	}
}

