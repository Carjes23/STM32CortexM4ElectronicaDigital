/**
 ******************************************************************************
 * @file           : main.c
 * @author         : dacardenasj
 * @brief          : Configuracion basica de un proyecto
 ******************************************************************************
  *
 ******************************************************************************
 */

#include <stdint.h>
#include "stm32f411xx_hal.h"
/*
 * Funcion principal del programa
 * Esta funcion es el corazon del programa!!
 *
 * */



int main(void){
	RCC->AHB1PENR |= 1;
	GPIOA->MODER |= 1 << (5*2);
	GPIOA->ODR |= 1 << (5*1);
	/*Loop forever */
	while(1){


	}

	return 0;
}
