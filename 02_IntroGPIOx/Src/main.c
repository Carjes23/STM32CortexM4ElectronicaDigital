/*
 ******************************************************************************
 * @file           : main.c
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

#include <stdint.h>
#include <unistd.h>

#include "stm32f411xx_hal.h"
#include "GPIOxDriver.h"
void my_sleep(int secs);


int main(void){

	//Definimos el handler para el PIN que deseamos configurar.
	GPIO_Handler_t handlerUserLedPin = {0};
	GPIO_Handler_t handlerUserButton = {0};

	//Deseamos trabajar con el puerto GPIOA
	handlerUserLedPin.pGPIOx = GPIOA;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_5;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_MEDIUM;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinAltFunMode	= AF0; //Ninuna funcion.

	//Deseamos trabajar con el puerto GPIOA
	handlerUserButton.pGPIOx = GPIOC;
	handlerUserButton.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_13;
	handlerUserButton.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_IN;
	handlerUserButton.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerUserButton.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerUserButton.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_MEDIUM;
	handlerUserButton.GPIO_PinConfig_t.GPIO_PinAltFunMode	= AF0; //Ninuna funcion.

	//Cargamos al configuracion del PIN especifico.
	GPIO_Config(&handlerUserLedPin);
	GPIO_Config(&handlerUserButton);



	//Este es el ciclo pincipal
	while(1){
		uint32_t variable = GPIO_ReadPin(&handlerUserButton);
		if (variable == CLEAR){
			if (GPIO_ReadPin(&handlerUserLedPin) == SET){
				GPIO_WritePin(&handlerUserLedPin, RESET);
				my_sleep(1);
			} else{
				GPIO_WritePin(&handlerUserLedPin, SET);
				my_sleep(1);
			}
		}
	}
}

void my_sleep(int secs) {
  #define STEPS_PER_SEC 800000
  unsigned int i,s;
  for (s=0; s < secs; s++) {
    for (i=0; i < STEPS_PER_SEC; i++) {
       // skip CPU cycle or any other statement(s) for making loop
       // untouched by C compiler code optimizations
       NOP();
    }
  }
}

