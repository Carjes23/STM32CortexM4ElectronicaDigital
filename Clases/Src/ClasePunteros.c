/**
 ******************************************************************************
 * @file           : main.c
 * @author         : dacardenasj
 * @brief          : Main program body
 *
 ******************************************************************************
 * @authors
 *
 * Taller V. Electronica analoga & Microcontroladores
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

#include <stdint.h>
#include <stm32f4xx.h>

uint32_t counter = 0;
uint32_t auxCounter = 0;
uint32_t  *ptrCounter = 0;

uint8_t byteVariable;
uint8_t *ptrByteVariable;

int main(void){

	counter = 12345;

	auxCounter = counter;
	ptrCounter = &counter;

	*ptrCounter = 43658709;
	ptrCounter ++;

	*ptrCounter = 43658709;

	byteVariable = 32;
	ptrByteVariable = &byteVariable;
	*ptrByteVariable = 123;

	auxCounter =(uint32_t) &counter;
	ptrByteVariable = (uint8_t *) auxCounter;
	*ptrByteVariable = 1;

	while(1){

	};
}
