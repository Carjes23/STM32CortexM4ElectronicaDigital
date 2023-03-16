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

//Funciones adicionales, para el funcionamiento de la tarea 2

// La funcion delay es una función que "pausa" o "duerme"
//el codigo aproximadamente a los segundos que se le coloque
void delay(int secs);

//La función GPIOxTooglePin nos cambia de estada un pin, lo que quiere decir
//que si este se encuentra apagado se prende y viceversa.
void GPIOxTooglePin(GPIO_Handler_t *pPinHandler);







#endif /* GPIOXDRIVER_H_ */
