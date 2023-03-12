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
#include <stdbool.h>

#include "stm32f411xx_hal.h"
#include "GPIOxDriver.h"

void my_sleep(int secs);
void GPIOxTooglePin(GPIO_Handler_t *pPinHandler);


int main(void){
	//EJEMPLO PARA LOS DOS PRIMEROS PUNTOS

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

	//Deseamos trabajar con el puerto GPIOC
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

	//DEFINICION PINES PUNTO 3.
	GPIO_Handler_t handlerPC9= {0};
	handlerUserLedPin.pGPIOx = GPIOC;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_9;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_MEDIUM;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinAltFunMode	= AF0; //Ninuna funcion.

	GPIO_Handler_t handlerPC6= {0};
	handlerUserLedPin.pGPIOx = GPIOC;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_6;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_MEDIUM;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinAltFunMode	= AF0; //Ninuna funcion.

	GPIO_Handler_t handlerPB8= {0};
	handlerUserLedPin.pGPIOx = GPIOB;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_8;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_MEDIUM;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinAltFunMode	= AF0; //Ninuna funcion.

	GPIO_Handler_t handlerPA6= {0};
	handlerUserLedPin.pGPIOx = GPIOA;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_6;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_MEDIUM;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinAltFunMode	= AF0; //Ninuna funcion.

	GPIO_Handler_t handlerPC7= {0};
	handlerUserLedPin.pGPIOx = GPIOC;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_7;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_MEDIUM;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinAltFunMode	= AF0; //Ninuna funcion.

	GPIO_Handler_t handlerPC8= {0};
	handlerUserLedPin.pGPIOx = GPIOC;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_8;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_MEDIUM;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinAltFunMode	= AF0; //Ninuna funcion.

	GPIO_Handler_t handlerPA7= {0};
	handlerUserLedPin.pGPIOx = GPIOA;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_7;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_MEDIUM;
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinAltFunMode	= AF0; //Ninuna funcion.

	/*
	 * 1)
	 * El error en GPIO_ReadPin venia dado porque no se tenian en cuenta los valores
	 * que estaban por delante del pin, por lo que si se tenian valores diferentes
	 * a 0, delante del pin objetivo (el que queremos leer) se entragaria un valor
	 * erroneo, por esto se tiene que realizar un limpiado previo, este se realiza con
	 * un and sobre el registro junto con una mascara con 0 en todas las posiciones
	 * excepto en la poisicion objetivo, en el GPIOxDrive.c se pueden ver los cambios que
	 * se realizaron(luego mientras se hacia la tarea se descubrio que bastaba solo con un
	 * agragarle un & con el SET para  consevar solo el primer valor, de todos modos se deja
	 * con la implementacion anterior en el GPIOxDriver.c)
	 *  y el siguiente codigo demuestra su funcionamiento al programar
	 * el boton de usuario (azul) para que prenda y apague el LED LD2, leyendo el valor
	 * tanto del boton azul, como del led, se crea una funcion sleep() que me permita
	 * "dormir" el codigo por unos segundos para que halla como un tiempo donde no se cambie
	 * el valor, para poder ver los cambios (los pasos por segundos definidos son completamente
	 * empiricos a ensayo y error).
	 * 2) En el mismo codigo que prende y apaga el led con el boton se utiliza la funcion
	 * GGPIOxTooglePin
	 */
	uint8_t contador = 1;
	bool PC9 = 0, PC6 = 0, PB8 = 0, PA6 = 0, PC7 = 0, PC8 = 0, PA7 = 0;
	while(1){
		//CODIGO PARA LOS 2 PRIMEROS PUNTOS
		//COMENTARLO AL MOMENTO DE REVISAR EL PUNTO 3
//		uint32_t variable = GPIO_ReadPin(&handlerUserButton);
//		if (variable == CLEAR){
//			GPIOxTooglePin(&handlerUserLedPin);
//			my_sleep(1);
//		}
		//CODIGO PARA EL 3
		PC9 = (contador >> 6) & SET;
		PC6 = (contador >> 5) & SET;
		PB8 = (contador >> 4) & SET;
		PA6 = (contador >> 3) & SET;
		PC7 = (contador >> 2) & SET;
		PC8 = (contador >> 1) & SET;
		PA7 = (contador) & SET;
		GPIO_WritePin(&handlerPC9,PC9);
		GPIO_WritePin(&handlerPC6,PC6);
		GPIO_WritePin(&handlerPB8,PB8);
		GPIO_WritePin(&handlerPA6,PA6);
		GPIO_WritePin(&handlerPC7,PC7);
		GPIO_WritePin(&handlerPC8,PC8);
		GPIO_WritePin(&handlerPA7,PA7);
		uint32_t variable = GPIO_ReadPin(&handlerUserButton);
		my_sleep(1);
		if (variable == SET){
			if(contador == 60){
				contador = 1;
			} else {contador++;}
		} else{
			if(contador == 1){
				contador = 60;
			} else{contador--;}
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

void GPIOxTooglePin(GPIO_Handler_t *pPinHandler){
	GPIO_WritePin(pPinHandler,SET^GPIO_ReadPin(pPinHandler));
}

