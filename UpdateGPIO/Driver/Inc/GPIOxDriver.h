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

#ifndef GPIOXDRIVER_H_
#define GPIOXDRIVER_H_

//Incluyendo este archivo estamos incluyendo también el correspondiente al GPIOx.
#include <stm32f4xx.h>
/**
 * @GPIO_PinNumber -> ajlajsalkjdaksj
 */

typedef struct
{
	uint8_t GPIO_PinNumber;	//PIN con el que deseamos trabajar
	uint8_t GPIO_PinMode;	//Modo de la cofigración: entrada, salida, alanogo, f. alternativa
	uint8_t GPIO_PinSpeed;	//Velocidad d respuesta del PIN (solo para digital)
	uint8_t GPIO_PinPuPdControl;	//Seleccionamos si deseamos una salida pull up pull down o "libre"
	uint8_t GPIO_PinOPType;	//Trabaja de las mano con el anterior, selecciona salida PIPD o OpenDrain
	uint8_t GPIO_PinAltFunMode;	//Selecciona cual es a funcion alternativa que se esta configurando.
}	GPIO_PinConfig_t;

/*
 * Esta es na estructura que contiene dos elementos:
 * -	La direccion del puerto que se esta utilizando(la referencia al puerto)
 * -	La configuración especifica del PIN que se está actualizando.
 */

typedef struct
{
	GPIO_TypeDef	*pGPIOx; /*!<Direccion del puerto al que el pin corresponde*/
	GPIO_PinConfig_t GPIO_PinConfig_t; /*configuracion del pin*/
}	GPIO_Handler_t;

// Definición de las cabeceras de las funciones del GPIOxDriver.
void GPIO_Config (GPIO_Handler_t *pGPIOHandler);
void GPIO_WritePin(GPIO_Handler_t *pPinHander, uint8_t newState);
uint32_t GPIO_ReadPin (GPIO_Handler_t *pPinHander);

//Valores estandar para ls configuracioenes  configuración del pin
//8.4.1 GPIOx MODDER (dos bit por cada pin)
#define GPIO_MODE_IN			0
#define GPIO_MODE_OUT			1
#define GPIO_MODE_ALTFN			2
#define GPIO_MODE_ANALOG		3 //ver imagen pag 156 reference manual.

//8.4.2 GPIOx OTYPER (un bit por cada pin) solo cuando se usiliza como salida
#define GPIO_OTYPE_PUSHPULL			0 //saca 0 y 1
#define GPIO_OTYPE_OPENDRAIN		1 //valor de alta impedancia

//8.4.3 GPIOx OSPEEDR (dos bit por cada pin)
//Controla la velocidad de operacion
#define GPIO_OSPEED_LOW				0
#define GPIO_OSPEED_MEDIUM			1
#define GPIO_OSPEED_FAST			2
#define GPIO_OSPEED_HIGH			3

//8.4.3 GPIOx PUPDR (dos bit por cada pin)
//
#define GPIO_PUPDR_NOTHING			0
#define GPIO_PUPDR_PULLUP			1
#define GPIO_PUPDR_PULLDOWN			2
#define GPIO_PUPDR_RESERVED			3

//	Definicion de los nombre de los pines
#define PIN_0			0
#define PIN_1			1
#define PIN_2			2
#define PIN_3			3
#define PIN_4			4
#define PIN_5			5
#define PIN_6			6
#define PIN_7			7
#define PIN_8			8
#define PIN_9			9
#define PIN_10			10
#define PIN_11			11
#define PIN_12			12
#define PIN_13			13
#define PIN_14			14
#define PIN_15			15

//	Definicion de las funciones alternativas

#define AF0   		 0b0000
#define AF1   		 0b0001
#define AF2   		 0b0010
#define AF3   		 0b0011
#define AF4   		 0b0100
#define AF5   		 0b0101
#define AF6   		 0b0110
#define AF7   		 0b0111
#define AF8   		 0b1000
#define AF9   		 0b1001
#define AF10   		 0b1010
#define AF11   		 0b1011
#define AF12   		 0b1100
#define AF13   		 0b1101
#define AF14   		 0b1110
#define AF15   		 0b1111

//Funciones adicionales, para el funcionamiento de la tarea 2

//La función GPIOxTooglePin nos cambia de estada un pin, lo que quiere decir
//que si este se encuentra apagado se prende y viceversa.
void GPIOxTooglePin(GPIO_Handler_t *pPinHandler);







#endif /* GPIOXDRIVER_H_ */
