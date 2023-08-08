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
 * Proyecto final el cual consiste en inegrar una pantalla en fucionamiento
 * la cual recibe instrucciones desde el micro lo que incluye rellenar rectangulos de
 * colores solidos y también rellenar imagenes.
 *
 ******************************************************************************
 */
/* Librerias que */
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <stm32f4xx.h>

#include "GPIOxDriver.h"
#include "USARTxDriver.h"
#include "BasicTimer.h"
#include "SysTick.h"
#include "SPIxDriver.h"
#include "PLLDriver.h"
#include "AdcDriver.h"
#include "PwmDriver.h"
#include "ExtiDriver.h"
#include "ILI9341.h"
#include "images.h"

// Definiciones MACROS para la direccion.
#define RIGHT 	1
#define UP 		2
#define LEFT 	3
#define DOWN 	4
#define PAUSE 	5

/************************* Configuracion de Perifericos *************************/

BasicTimer_Handler_t handlerTim2 = { 0 }; // Timer para el blinking
BasicTimer_Handler_t handlerTim3 = { 0 };
BasicTimer_Handler_t handlerTim5 = { 0 };

GPIO_Handler_t tx2pin = { 0 };	//Pin para configurar la trasmision del USART2
GPIO_Handler_t rx2pin = { 0 };	//Pin para configurar la recepcion del USART2
USART_Handler_t HandlerTerminal = { 0 }; 	//Handler para manejar el USART2

uint8_t usart2image = 0; //Dato recibido por el usart 2:; Imagenes

GPIO_Handler_t handlerUserLedPin = { 0 }; //Led de usuario para el blinking

/*
 * Pines y handlers necesarios para manipular la pantalla
 * CS -> Chip Select
 * RST -> Reset
 * DC -> Data / Command (Seleccionamos que estamos mandando si datos o comandos)
 * SIPI -> Que SPI se va a utilizar en este caso el 2
 * MOSI -> Master Output Slave Input -> El micro envia datos que la pantalla recibe
 * SCK -> Clock del SPI.
 * ILI -> Handler de la pantalla
 */

GPIO_Handler_t CSPrueba = { 0 };
GPIO_Handler_t RSTPrueba = { 0 };
GPIO_Handler_t DCPrueba = { 0 };
SPI_Handler_t SPIPrueba = { 0 };
GPIO_Handler_t MOSIPrueba = { 0 };
GPIO_Handler_t SCKPrueba = { 0 };
ILI9341_Handler_t ili = { 0 };

/************************* Configuracion de Variables del programa *************************/

// Variables: Sintonización del MCU
uint8_t trimval = 13;

// Variables: Escenario de Juego
uint16_t backgroundColor = ILI9341_BLACK;

// Buffer para envio de datos
char bufferData[128] = { 0 };

/************************* Configuracion de Botones *************************/
/*
 * Configuracion de los botones con sus repectivos Exti
 */
GPIO_Handler_t handlerLeft = { 0 };
EXTI_Config_t handlerExtiLeft = { 0 };

GPIO_Handler_t handlerRight = { 0 };
EXTI_Config_t handlerExtiRight = { 0 };

GPIO_Handler_t handlerUp = { 0 };
EXTI_Config_t handlerExtiUp = { 0 };

GPIO_Handler_t handlerDown = { 0 };
EXTI_Config_t handlerExtiDown = { 0 };

uint8_t newDirVal = 0;

uint16_t timer5Count = 0;
uint16_t timChangeIm = 1; //Multipolso de 5 sg
uint16_t currentImage = 1;
bool flagChange = 0;
uint16_t limitImage = 3;
bool initialState = 1;

// Cabeceras de funciones
void initSystem(void);			//Funcion de inicio
void USART2Rx_Callback(void);	//Funcion que se encarga del manejo del usart2

uint32_t contadorPan = 0;
bool enableChange = 0;

int main(void) {

	// Iniciamos el programa
	initSystem();

	// Definimos rotación & Fondo de pantalla
	Ili_setRotation(&ili, 3);

	timer5Count = 0;
	flagChange = 0;
	imageSelector(&ili, currentImage);
	handlerTim5.ptrTIMx = TIM5; //El timer que se va a usar
	handlerTim5.TIMx_Config.TIMx_interruptEnable = 1; //Se habilitan las interrupciones
	handlerTim5.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; //Se usara en modo ascendente
	handlerTim5.TIMx_Config.TIMx_period = 50000; //Se define el periodo en este caso el led cambiara cada 250ms
	handlerTim5.TIMx_Config.TIMx_speed = BTIMER_SPEED_100us; //Se define la "velocidad" que se usara

	BasicTimer_Config(&handlerTim5); //Se carga la configuración.

	while (1) {
		if (newDirVal && enableChange) {
			enableChange = 0;
			if (newDirVal == LEFT) {
				TurnOffTimer(&handlerTim5);
				timer5Count = 0;
				if (currentImage <= 1) {
					currentImage = limitImage;
				} else {
					currentImage--;
				}
				imageSelector(&ili, currentImage);
				TurnOnTimer(&handlerTim5);

			} else if (newDirVal == RIGHT) {
				TurnOffTimer(&handlerTim5);
				timer5Count = 0;
				if (currentImage <= limitImage) {
					currentImage++;
				} else {
					currentImage = 1;
				}
				imageSelector(&ili, currentImage);
				TurnOnTimer(&handlerTim5);
			}
			if (newDirVal == UP) {
				timChangeIm++;
			} else if (newDirVal == DOWN) {
				if (timChangeIm != 1) {
					timChangeIm--;
				}
			}
			newDirVal = 0;
			TurnOnTimer(&handlerTim3);
		}
		if (flagChange) {
			flagChange = 0;
			newDirVal = RIGHT;
		}
	}

	return 0;
}

void initSystem(void) {
	//Activacion cooprocesador matematico
	SCB->CPACR |= (0xF << 20);
	//Se conifgura el PLL en 80
	configPLL(80);
	//Se configura el Systick automaticamente
	config_SysTick();

	//Led de usuario usado para el blinking

	handlerUserLedPin.pGPIOx = GPIOA; //Se encuentra en el el GPIOA
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinNumber = PIN_5; // Es el pin 5.
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_OUT; //Se utiliza como salida
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL; //Salida PushPull
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING; //No se usa ninguna resistencia
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_FAST; //Se usa en velocidad rapida

	GPIO_Config(&handlerUserLedPin); //Se carga la configuración para el Led.

	handlerTim3.ptrTIMx = TIM3; //El timer que se va a usar
	handlerTim3.TIMx_Config.TIMx_interruptEnable = 1; //Se habilitan las interrupciones
	handlerTim3.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; //Se usara en modo ascendente
	handlerTim3.TIMx_Config.TIMx_period = 2500 * 2; //Se define el periodo en este caso el default es 50 ms.
	handlerTim3.TIMx_Config.TIMx_speed = BTIMER_SPEED_100us; //Se define la "velocidad" que se usara
	handlerTim3.TIMx_Config.TIMx_OPM = ENABLE; //Funcion que nos permite que el timer cuente solo una vez y toque habilitarlo para volver a usarlo.

	BasicTimer_Config(&handlerTim3); //Se carga la configuración.

	//Handeler para manejar el USAR2
	HandlerTerminal.ptrUSARTx = USART2;
	HandlerTerminal.USART_Config.USART_baudrate = 921600;
	HandlerTerminal.USART_Config.USART_datasize = USART_DATASIZE_8BIT;
	HandlerTerminal.USART_Config.USART_mode = USART_MODE_RXTX;
	HandlerTerminal.USART_Config.USART_parity = USART_PARITY_NONE;
	HandlerTerminal.USART_Config.USART_stopbits = USART_STOPBIT_1;
	HandlerTerminal.USART_Config.USART_RX_Int_Ena = ENABLE;

	USART_Config(&HandlerTerminal);
	//Configuración Timer 2 que controlara el blinking

	handlerTim2.ptrTIMx = TIM2; //El timer que se va a usar
	handlerTim2.TIMx_Config.TIMx_interruptEnable = 1; //Se habilitan las interrupciones
	handlerTim2.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; //Se usara en modo ascendente
	handlerTim2.TIMx_Config.TIMx_period = 2500; //Se define el periodo en este caso el led cambiara cada 250ms
	handlerTim2.TIMx_Config.TIMx_speed = BTIMER_SPEED_100us; //Se define la "velocidad" que se usara

	BasicTimer_Config(&handlerTim2); //Se carga la configuración.

	//Pines necesarios para el uso del USART2
	tx2pin.pGPIOx = GPIOA;
	tx2pin.GPIO_PinConfig_t.GPIO_PinNumber = PIN_2;
	tx2pin.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	tx2pin.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLUP;
	tx2pin.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_FAST; //Se usa en velocidad rapida
	tx2pin.GPIO_PinConfig_t.GPIO_PinAltFunMode = 7;

	GPIO_Config(&tx2pin);

	rx2pin.pGPIOx = GPIOA;
	rx2pin.GPIO_PinConfig_t.GPIO_PinNumber = PIN_3;
	rx2pin.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	rx2pin.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLUP;
	rx2pin.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_FAST;
	rx2pin.GPIO_PinConfig_t.GPIO_PinAltFunMode = 7;

	GPIO_Config(&rx2pin);

	//Pines necesarios para el uso de la pantalla

	CSPrueba.pGPIOx = GPIOA;
	CSPrueba.GPIO_PinConfig_t.GPIO_PinNumber = PIN_8;
	CSPrueba.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_OUT;
	CSPrueba.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	CSPrueba.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	CSPrueba.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_HIGH;

	GPIO_Config(&CSPrueba);
	GPIO_WritePin(&CSPrueba, SET);

	RSTPrueba.pGPIOx = GPIOB;
	RSTPrueba.GPIO_PinConfig_t.GPIO_PinNumber = PIN_10;
	RSTPrueba.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_OUT;
	RSTPrueba.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	RSTPrueba.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	RSTPrueba.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_HIGH;

	GPIO_Config(&RSTPrueba);
	GPIO_WritePin(&RSTPrueba, SET);

	DCPrueba.pGPIOx = GPIOB;
	DCPrueba.GPIO_PinConfig_t.GPIO_PinNumber = PIN_4;
	DCPrueba.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_OUT;
	DCPrueba.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	DCPrueba.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	DCPrueba.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_HIGH;

	GPIO_Config(&DCPrueba);
	GPIO_WritePin(&DCPrueba, SET);

	MOSIPrueba.pGPIOx = GPIOB;
	MOSIPrueba.GPIO_PinConfig_t.GPIO_PinNumber = PIN_5;
	MOSIPrueba.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	MOSIPrueba.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	MOSIPrueba.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	MOSIPrueba.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_HIGH;
	MOSIPrueba.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF5;
	GPIO_Config(&MOSIPrueba);
	GPIO_WritePin(&MOSIPrueba, SET);

	SCKPrueba.pGPIOx = GPIOB;
	SCKPrueba.GPIO_PinConfig_t.GPIO_PinNumber = PIN_3;
	SCKPrueba.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	SCKPrueba.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	SCKPrueba.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	SCKPrueba.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_HIGH;
	SCKPrueba.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF5;

	GPIO_Config(&SCKPrueba);
	GPIO_WritePin(&SCKPrueba, SET);

	SPIPrueba.ptrSPIx = SPI1;
	SPIPrueba.SPI_Config.BaudRatePrescaler = SPI_BAUDRATE_DIV8;
	SPIPrueba.SPI_Config.CPOLCPHA = SPI_CPOLCPHA_MODE_00;
	SPIPrueba.SPI_Config.DataSize = SPI_DATASIZE_8BIT;
	SPIPrueba.SPI_Config.FirstBit = SPI_FIRSTBIT_MSB;
	SPIPrueba.SPI_Config.Mode = SPI_MODE_MASTER;

	//Handler de la pantalla

	ili.spi_handler = &SPIPrueba;
	ili.cs_pin = &CSPrueba;
	ili.dc_pin = &DCPrueba;
	ili.reset_pin = &RSTPrueba;

	ILI9341_Init(&ili);

	//Handlers y exti de los botones
	handlerLeft.pGPIOx = GPIOC; //Se encuentra en el el GPIOC
	handlerLeft.GPIO_PinConfig_t.GPIO_PinNumber = PIN_0; //Es el pin numero 0
	handlerLeft.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_IN; //En este caso el pin se usara como entrada de datos
	handlerLeft.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLUP; //No se usara ninguna resistencia pull-up o pull-down

	handlerExtiLeft.pGPIOHandler = &handlerLeft; // Se carga la config del pin al handler del EXTI
	handlerExtiLeft.edgeType = EXTERNAL_INTERRUPT_FALLING_EDGE; //Se define que se van a detectar flancos de bajada.

	extInt_Config(&handlerExtiLeft); //Se carga la configuracion usando la funcion de la libreria.

	//Boton de usuario para el cambio de tasa de resfresco.

	handlerRight.pGPIOx = GPIOC; //Se encuentra en el el GPIOC
	handlerRight.GPIO_PinConfig_t.GPIO_PinNumber = PIN_1; //Es el pin numero 1
	handlerRight.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_IN; //En este caso el pin se usara como entrada de datos
	handlerRight.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLUP; //No se usara ninguna resistencia pull-up o pull-down

	handlerExtiRight.pGPIOHandler = &handlerRight; // Se carga la config del pin al handler del EXTI
	handlerExtiRight.edgeType = EXTERNAL_INTERRUPT_FALLING_EDGE; //Se define que se van a detectar flancos de bajada.

	extInt_Config(&handlerExtiRight); //Se carga la configuracion usando la funcion de la libreria.

	//Boton de usuario para el cambio de tasa de resfresco.

	handlerUp.pGPIOx = GPIOC; //Se encuentra en el el GPIOC
	handlerUp.GPIO_PinConfig_t.GPIO_PinNumber = PIN_3; //Es el pin numero 3
	handlerUp.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_IN; //En este caso el pin se usara como entrada de datos
	handlerUp.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLUP; //No se usara ninguna resistencia pull-up o pull-down

	handlerExtiUp.pGPIOHandler = &handlerUp; // Se carga la config del pin al handler del EXTI
	handlerExtiUp.edgeType = EXTERNAL_INTERRUPT_FALLING_EDGE; //Se define que se van a detectar flancos de bajada.

	extInt_Config(&handlerExtiUp); //Se carga la configuracion usando la funcion de la libreria.

	//Boton de usuario para el cambio de tasa de resfresco.

	handlerDown.pGPIOx = GPIOC; //Se encuentra en el el GPIO
	handlerDown.GPIO_PinConfig_t.GPIO_PinNumber = PIN_2; //Es el pin numero 2
	handlerDown.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_IN; //En este caso el pin se usara como entrada de datos
	handlerDown.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLUP; //No se usara ninguna resistencia pull-up o pull-down

	handlerExtiDown.pGPIOHandler = &handlerDown; // Se carga la config del pin al handler del EXTI
	handlerExtiDown.edgeType = EXTERNAL_INTERRUPT_FALLING_EDGE; //Se define que se van a detectar flancos de bajada.

	extInt_Config(&handlerExtiDown); //Se carga la configuracion usando la funcion de la libreria.

}

//Calback del timer2 para el blinking
void BasicTimer2_Callback(void) {
	GPIOxTooglePin(&handlerUserLedPin);
}

//Calback del timer2 para el blinking
void BasicTimer3_Callback(void) {
	enableChange = 1;
}

//Calback del timer2 para el blinking
void BasicTimer5_Callback(void) {
	timer5Count++;
	if (timer5Count >= timChangeIm) {
		flagChange = 1;
	}
}

//Movimiento izquierda
void callback_extInt0(void) {
	newDirVal = LEFT;

}
//Movimiento derecha
void callback_extInt1(void) {
	newDirVal = RIGHT;
}
//Movimiento arriba
void callback_extInt2(void) {
	newDirVal = DOWN;
}
//Movimiento abajo
void callback_extInt3(void) {
	newDirVal = UP;
}

