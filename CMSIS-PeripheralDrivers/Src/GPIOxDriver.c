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
/*
 * En este programa se escribe el control, par que sea lo más generico
 * posible
 */

#include "GPIOxDriver.h"
/*
 * Para cualquier prefierico hay unos pasos estrictos que siempre
 * se tienen que seguir en un orden. Lo primero sería activar la señal
 * de reloj
 */

void GPIO_Config (GPIO_Handler_t *pGPIOHandler){
	//Variable para hacer  paso a paso.
	uint32_t auxConfig = 	0;
	uint32_t auxPosition = 	0;
	// 1) activar el periferico.
	//Verificar que pin.
	if(pGPIOHandler -> pGPIOx == GPIOA){
		RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	}
	else if(pGPIOHandler -> pGPIOx == GPIOB){
		RCC -> AHB1ENR |=  RCC_AHB1ENR_GPIOBEN;
	}
	else if(pGPIOHandler -> pGPIOx == GPIOC){
		RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	}
	else if(pGPIOHandler -> pGPIOx == GPIOD){
		RCC -> AHB1ENR |=  RCC_AHB1ENR_GPIODEN;
	}
	else if(pGPIOHandler -> pGPIOx == GPIOE){
		RCC -> AHB1ENR |=  RCC_AHB1ENR_GPIOEEN;
	}
	else if(pGPIOHandler -> pGPIOx == GPIOH){
		RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOHEN;
	}

	//Despues de activado podemos comenzar a configurar.
	// 2) Configurar el registro GPIOx_MODER
	auxConfig = (pGPIOHandler -> GPIO_PinConfig_t.GPIO_PinMode << 2 * pGPIOHandler -> GPIO_PinConfig_t.GPIO_PinNumber);
	//Antes de cargar el nuevo valor, limpiamos los bits especificos de ese registro
	//para lo cual aplicamos.
	pGPIOHandler -> pGPIOx -> MODER &= ~(0b11 << 2 * pGPIOHandler->GPIO_PinConfig_t.GPIO_PinNumber);

	//Cargamos a auxConfig en el registro MODER
	pGPIOHandler -> pGPIOx -> MODER |= auxConfig;

	// 3)configurando el registro OTYPER
	auxConfig = (pGPIOHandler -> GPIO_PinConfig_t.GPIO_PinOPType << pGPIOHandler -> GPIO_PinConfig_t.GPIO_PinNumber);
	//limpiamos
	pGPIOHandler -> pGPIOx -> OTYPER &= ~(SET << pGPIOHandler->GPIO_PinConfig_t.GPIO_PinNumber);
	// cargamos el resultado
	pGPIOHandler -> pGPIOx -> OTYPER |= auxConfig;

	// 4) Configurando ahora la velocidad.
	//Realizando el mismo proceso
	auxConfig = (pGPIOHandler -> GPIO_PinConfig_t.GPIO_PinSpeed << 2 * pGPIOHandler -> GPIO_PinConfig_t.GPIO_PinNumber);
	pGPIOHandler -> pGPIOx -> OSPEEDR &= ~(0b11 << 2 * pGPIOHandler->GPIO_PinConfig_t.GPIO_PinNumber);
	pGPIOHandler -> pGPIOx -> OSPEEDR |= auxConfig;

	// 5) confiurando si se desea pull-up, pull-down o flotante.
	auxConfig = (pGPIOHandler -> GPIO_PinConfig_t.GPIO_PinPuPdControl << 2 * pGPIOHandler -> GPIO_PinConfig_t.GPIO_PinNumber);
	pGPIOHandler -> pGPIOx -> PUPDR &= ~(0b11 << 2 * pGPIOHandler->GPIO_PinConfig_t.GPIO_PinNumber);
	pGPIOHandler -> pGPIOx -> PUPDR |= auxConfig;

	//Esta es la parte para la configuración de las funciones alternativs
	if(pGPIOHandler->GPIO_PinConfig_t.GPIO_PinMode == GPIO_MODE_ALTFN){

		//Seleccionamos primero si se debe utilizar el registro bajo (AFRL) o alto (AFRH)
		if(pGPIOHandler->GPIO_PinConfig_t.GPIO_PinNumber < 8){
			//Estamos en el registro AFRL, que controla los pines del 0 al 7
			auxPosition = 4 * pGPIOHandler->GPIO_PinConfig_t.GPIO_PinNumber;

			//limpiamos
			pGPIOHandler->pGPIOx->AFR[0] &= ~(0b111<<auxPosition);

			// Y escribimos el valor configurado en la posicion seleccionada
			pGPIOHandler->pGPIOx->AFR[0] |= (pGPIOHandler->GPIO_PinConfig_t.GPIO_PinAltFunMode << auxPosition);
		} else {
			//Estamos en el registro AFRH, que controla los pines del 8 al 15
			auxPosition = 4 * (pGPIOHandler->GPIO_PinConfig_t.GPIO_PinNumber - 8);

			//Limpiamos
			pGPIOHandler->pGPIOx->AFR[1] &= ~(0b111<<auxPosition);

			// Y escribimos el valor configurado en la posicion seleccionada
						pGPIOHandler->pGPIOx->AFR[1] |= (pGPIOHandler->GPIO_PinConfig_t.GPIO_PinAltFunMode << auxPosition);
		}
	}
} //Fin del GPIO_config

/**
 * Funcion utilizada para cambiar el estado el pin enregado en el hander, asignando
 * el valor entregado en el variable newState
 */

void GPIO_WritePin(GPIO_Handler_t *pPinHandler, uint8_t newState){
	//Limpiamos la posicion que deseamos
//	pPinHandler -> pGPIOx -> ODR &= ~(SET << pPinHandler->GPIO_PinConfig_t.GPIO_PinNumber);
	if(newState == SET){
		//Trabajando con la parte baja del registro
		pPinHandler->pGPIOx->BSRR |= (SET << pPinHandler->GPIO_PinConfig_t.GPIO_PinNumber);
	} else{
		//trabajando con la parte alta del registro
		pPinHandler->pGPIOx->BSRR |= (SET << (pPinHandler->GPIO_PinConfig_t.GPIO_PinNumber + 16));
	}
}
//Funcion para leer el estado de un pin.
uint32_t GPIO_ReadPin(GPIO_Handler_t *pPinHandler){
	// creamos variable auxiliar para retornarla
	uint32_t pinValue = pPinHandler->pGPIOx->IDR;

	//Cargamos el valor del registro IDR, desplzadado a derecha tantas veces como la ubicacion del pin
	//especifico
	//limpiar los demas valores
	//Antes:
	// pinValue = (pPinHandler->pGPIOx->IDR>>pPinHandler->GPIO_PinConfig_t.GPIO_PinNumber);
	pinValue &= (SET<<pPinHandler->GPIO_PinConfig_t.GPIO_PinNumber);
	pinValue >>= pPinHandler->GPIO_PinConfig_t.GPIO_PinNumber;
	return pinValue;
}

//Funciones adicionales, para el funcionamiento de la tarea



/*
 * La función GPIOxTooglePin nos cambia de estada un pin, lo que quiere decir
 * que si este se encuentra apagado se prende y viceversa, se noto que al implementar
 * un XOR con el valor actual y un set se consigue este resultado
 * se puede evidenciar mucho mejor al ver su tabla.
 * 	Valor pin | SET | XOR entre los 2
 * 		1	  |  1	|		 0
 * 		0	  |  1	| 	 	 1
 */


void GPIOxTooglePin(GPIO_Handler_t *pPinHandler){
	GPIO_WritePin(pPinHandler,SET^GPIO_ReadPin(pPinHandler));
}

