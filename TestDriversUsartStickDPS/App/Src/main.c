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
#include "USARTxDriver.h"
#include "BasicTimer.h"
#include <stdbool.h>
#include "SysTick.h"
#include <arm_math.h>

bool banderaLedUsuario = 0; //Bandera TIM2 el cual nos da el blinking del LD2

uint8_t usart2DataReceived = 0;

char string[] = "Hola ";
char bufferMsg[64] = {0};

float valueA = 123.4567f;
float valueB = 987.7654f;
float valueC = 0.0;

float32_t sineValue = 0.0;
float32_t sineArgValue = 0.0;

float32_t srcNumber[4] = {-0.987, 32.26, -45.21, -987.321};
float32_t destNumber[4] = {0};
uint32_t dataSize = 4;

BasicTimer_Handler_t handlerTim2 = {0}; //Timer para el blinking
USART_Handler_t USART2Handler = {0};
GPIO_Handler_t handlerUserLedPin = {0};
GPIO_Handler_t tx2pin = {0};
GPIO_Handler_t rx2pin = {0};

void initSystem(void);
void USART2Rx_Callback(void);


int main(void){
	//Activacion cooprocesador matematico
	SCB->CPACR |= (0xF <<20);

	initSystem();

	while(1){

		config_SysTick_ms(HSI_CLOCK_CONFIGURED);

		if(banderaLedUsuario == 1){
			banderaLedUsuario = 0;
//			GPIOxTooglePin(&handlerUserLedPin);
		}

		if(usart2DataReceived == 'A'){
			valueC = valueA + valueB;

			sprintf(bufferMsg, "ValueC= %#.3f \n",valueC);
			writeString(&USART2Handler, bufferMsg);
			usart2DataReceived = 0;
		}

		if(usart2DataReceived == 'B'){
			arm_abs_f32(srcNumber, destNumber, dataSize);

			for(int j = 0; j < 4; j++){
				sprintf(bufferMsg, "ValueC= %#.3f \n",destNumber[j]);
				writeString(&USART2Handler, bufferMsg);
			}

			usart2DataReceived = 0;
		}

		if(usart2DataReceived == 'C'){
			sineArgValue = M_PI /4;

			sineValue = arm_sin_f32(sineArgValue);

			sprintf(bufferMsg, "Seno(%#.2f) = %#.6f  \n", sineArgValue, sineValue);
			writeString(&USART2Handler, bufferMsg);
			usart2DataReceived = 0;
		}


		delay_ms(250);
		GPIOxTooglePin(&handlerUserLedPin);
	}

	return 0;
}

void initSystem(void){

	//Led de usuario usado para el blinking

	handlerUserLedPin.pGPIOx = GPIOA; //Se encuentra en el el GPIOA
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_5; // Es el pin 5.
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_OUT; //Se utiliza como salida
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL; //Salida PushPull
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING; //No se usa ninguna resistencia
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_FAST; //Se usa en velocidad rapida

	GPIO_Config(&handlerUserLedPin); //Se carga la configuración para el Led.

	USART2Handler.ptrUSARTx = USART2;
	USART2Handler.USART_Config.USART_baudrate = USART_BAUDRATE_9600;
	USART2Handler.USART_Config.USART_datasize = USART_DATASIZE_8BIT;
	USART2Handler.USART_Config.USART_mode = USART_MODE_RXTX;
	USART2Handler.USART_Config.USART_parity = USART_PARITY_NONE;
	USART2Handler.USART_Config.USART_stopbits =  USART_STOPBIT_1;
	USART2Handler.USART_Config.USART_RX_Int_Ena = ENABLE;

	USART_Config(&USART2Handler);
	//Configuración Timer 2 que controlara el blinking

	handlerTim2.ptrTIMx = TIM2; //El timer que se va a usar
	handlerTim2.TIMx_Config.TIMx_interruptEnable = 1; //Se habilitan las interrupciones
	handlerTim2.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; //Se usara en modo ascendente
	handlerTim2.TIMx_Config.TIMx_period = 250; //Se define el periodo en este caso el led cambiara cada 250ms
	handlerTim2.TIMx_Config.TIMx_speed = BTIMER_SPEED_1ms; //Se define la "velocidad" que se usara

	BasicTimer_Config(&handlerTim2); //Se carga la configuración.

	//PA2
	tx2pin.pGPIOx = GPIOA;
	tx2pin.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_2;
	tx2pin.GPIO_PinConfig_t.GPIO_PinMode		= GPIO_MODE_ALTFN;
	tx2pin.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_PULLUP;
	tx2pin.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_FAST; //Se usa en velocidad rapida
	tx2pin.GPIO_PinConfig_t.GPIO_PinAltFunMode = 7;

	GPIO_Config(&tx2pin);

	rx2pin.pGPIOx = GPIOA;
	rx2pin.GPIO_PinConfig_t.GPIO_PinNumber        = PIN_3;
	rx2pin.GPIO_PinConfig_t.GPIO_PinMode          = GPIO_MODE_ALTFN;
	rx2pin.GPIO_PinConfig_t.GPIO_PinPuPdControl   = GPIO_PUPDR_PULLUP;
	rx2pin.GPIO_PinConfig_t.GPIO_PinSpeed         = GPIO_OSPEED_FAST;
	rx2pin.GPIO_PinConfig_t.GPIO_PinAltFunMode    = 7;

	GPIO_Config(&rx2pin);


	writeString(&USART2Handler,string);


}

//Calback del timer2 para el blinking
void BasicTimer2_Callback(void){
	banderaLedUsuario = 1;
}

void USART2Rx_Callback(void){
	usart2DataReceived = getRxData();
}


