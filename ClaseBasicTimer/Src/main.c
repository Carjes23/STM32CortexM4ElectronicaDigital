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
#include <stm32f4xx.h>
#include "GPIOxDriver.h"
#include "BasicTimer.h"
#include <stdbool.h>

bool flag = 0;

void BasicTimer2_Callback(void){
	flag = SET;
}
/*
 * Funcion principal del programa
 * Esta funcion es el corazon del programa!!
 *
 * */





int main(void){
	GPIO_Handler_t handlerUserLedPin = {0};
	/*
	* Se define el Handler para el pin LD2
	* Con este pin se pretende probar los dos primeros puntos.
	*/


	handlerUserLedPin.pGPIOx = GPIOA; // Aquí se coloca a que GPIO pertenece el pin en este caso es el A
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_5; //El numero del pin en este caso es el
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_OUT; //En que modo se va a utilizar el pin es este caso lo usaremos como salida
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL; //Sí se configura out, en que modo se va a usar, en este caso se usara como push-pull
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING; //Si se le va agregár una resistencia Pull-Up o Pull-down, no  le pondremos ninguna
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_FAST; //A que velocidad se va utilizr la salida del
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinAltFunMode	= AF0; //Ninguna funcion alternativa
	//Carga de la configuración.
	//Se carga la configuración para el Led.
	GPIO_Config(&handlerUserLedPin);


	//se creal el Handler

	BasicTimer_Handler_t handlerTim2 = {0};

	//Se configura el Handeler.

	handlerTim2.ptrTIMx = TIM2;
	handlerTim2.TIMx_Config.TIMx_interruptEnable = 1;
	handlerTim2.TIMx_Config.TIMx_mode = BTIMER_MODE_DOWN;
	handlerTim2.TIMx_Config.TIMx_period = 250;
	handlerTim2.TIMx_Config.TIMx_speed = BTIMER_SPEED_1ms;

	BasicTimer_Config(&handlerTim2);




	while(1){
		if(flag==1){
			flag = 0;
			GPIOxTooglePin(&handlerUserLedPin);
		}

	}

	return 0;
}


