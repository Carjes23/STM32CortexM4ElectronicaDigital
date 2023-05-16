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
#include "PwmDriver.h"
#include "SPIxDriver.h"
#include "ILI9341.h"

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
PWM_Handler_t pwmtim3 = {0};
GPIO_Handler_t pwmpin = {0};
GPIO_Handler_t CSPrueba = {0};
GPIO_Handler_t RSTPrueba = {0};
GPIO_Handler_t DCPrueba = {0};
SPI_Handler_t SPIPrueba = {0};
GPIO_Handler_t MOSIPrueba = {0};
GPIO_Handler_t SCKPrueba = {0};
ILI9341_Handler_t ili = {0};

uint16_t duttyValue = 2000;

void initSystem(void);
void USART2Rx_Callback(void);

int main(void){
	//Activacion cooprocesador matematico
	SCB->CPACR |= (0xF <<20);

	initSystem();

	Ili_setRotation(&ili, 3);

	// Set the background color to black
    uint16_t backgroundColor = 0x001F; // 16-bit color value for black

    // Fill the entire screen with the background color
    ILI9341_FillRectangle(&ili, 0, 0, ILI9341_TFTHEIGHT - 1, ILI9341_TFTWIDTH - 1, backgroundColor);

    //Imagen
//    ILI9341_SendImage(&ili,0,0,ILI9341_TFTHEIGHT - 1, ILI9341_TFTWIDTH - 1, imagen);
//
    // Define coordinates for the rectangle
    uint16_t x0 = 10, y0 = 10, x1 = 100, y1 = 100;

    // Define the color for the rectangle (e.g., red)
    uint16_t rectangleColor = 0xF800; // 16-bit color value for red

    // Fill the rectangle with the specified color
    ILI9341_FillRectangle(&ili, x0, y0, x1, y1, rectangleColor);

    // Set the address window on the ILI9341 display
	Ili_starCom(&ili);
    ILI9341_SetAddressWindow(&ili, 0, 0, ILI9341_TFTHEIGHT - 1, ILI9341_TFTWIDTH - 1);


	while(1){



		if(banderaLedUsuario == 1){
			banderaLedUsuario = 0;
			GPIOxTooglePin(&handlerUserLedPin);
		}

	}

	return 0;
}

void initSystem(void){

	RCC -> PLLCFGR &= ~(RCC_PLLCFGR_PLLSRC);

	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLM); // Limpiamos
	RCC->PLLCFGR |= (RCC_PLLCFGR_PLLM_3); // Ponemos un 8 en el PLLM

	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLN); // Limpiamos
	/* Ponemos PLLN en 100 */
	RCC->PLLCFGR |= (RCC_PLLCFGR_PLLN_2);
	RCC->PLLCFGR |= (RCC_PLLCFGR_PLLN_5);
	RCC->PLLCFGR |= (RCC_PLLCFGR_PLLN_6);

	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLP); // Limpiamos

	/* Latencia del FLASH_ACR para que se demore y pueda hacer el registro */
	// 3 Wait states
	FLASH -> ACR &= ~(FLASH_ACR_LATENCY); // Limpiamos
	FLASH -> ACR |= (FLASH_ACR_LATENCY_1WS);
	FLASH -> ACR |= (FLASH_ACR_LATENCY_2WS);

	/* Configuramos el MC01 (PIN A8 como funcion alternativa 00) */

	// Seleccionamos la senal PLL
	RCC -> CFGR |= RCC_CFGR_MCO1_0;
	RCC -> CFGR |= RCC_CFGR_MCO1_1;

	// Utilizamos un prescaler para poder ver la senal en el osciloscopio
	RCC -> CFGR |= RCC_CFGR_MCO1PRE_0;
	RCC -> CFGR |= RCC_CFGR_MCO1PRE_1;
	RCC -> CFGR |= RCC_CFGR_MCO1PRE_2;

	// Encender el PLL

	RCC->CR |= RCC_CR_PLLON;

	// Esperamos que el PLL se cierre (estabilizacion)
	while (!(RCC->CR & RCC_CR_PLLRDY)){
		__NOP();
	}

	// Cambiamos el CPU Clock source cambiando los SW bits (System Clock Switch)
	/* System Clock Switch para PLL */
	RCC -> CFGR &= ~(RCC_CFGR_SW); // Limpiamos
	RCC -> CFGR |= (RCC_CFGR_SW_1);

	config_SysTick_ms(HSI_CLOCK_CONFIGURED);

	//Led de usuario usado para el blinking

	handlerUserLedPin.pGPIOx = GPIOA; //Se encuentra en el el GPIOA
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_5; // Es el pin 5.
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_OUT; //Se utiliza como salida
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL; //Salida PushPull
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING; //No se usa ninguna resistencia
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_FAST; //Se usa en velocidad rapida

	GPIO_Config(&handlerUserLedPin); //Se carga la configuración para el Led.

	USART2Handler.ptrUSARTx = USART2;
	USART2Handler.USART_Config.USART_baudrate = USART_BAUDRATE_115200;
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

	pwmpin.pGPIOx 								  = GPIOC;
	pwmpin.GPIO_PinConfig_t.GPIO_PinNumber        = PIN_7;
	pwmpin.GPIO_PinConfig_t.GPIO_PinMode          = GPIO_MODE_ALTFN;
	pwmpin.GPIO_PinConfig_t.GPIO_PinOPType		  = GPIO_OTYPE_PUSHPULL;
	pwmpin.GPIO_PinConfig_t.GPIO_PinPuPdControl   = GPIO_PUPDR_NOTHING;
	pwmpin.GPIO_PinConfig_t.GPIO_PinSpeed         = GPIO_OSPEED_FAST;
	pwmpin.GPIO_PinConfig_t.GPIO_PinAltFunMode    = AF2;

	GPIO_Config(&pwmpin);


	pwmtim3.ptrTIMx = TIM3;
	pwmtim3.config.channel = PWM_CHANNEL_2;
	pwmtim3.config.duttyCicle = duttyValue;
	pwmtim3.config.periodo = 20000;
	pwmtim3.config.prescaler = 16;

	pwm_Config(&pwmtim3);

	enableOutput(&pwmtim3);
	startPwmSignal(&pwmtim3);


	writeString(&USART2Handler,string);


	CSPrueba.pGPIOx 							    	= GPIOA;
	CSPrueba.GPIO_PinConfig_t.GPIO_PinNumber        	= PIN_8;
	CSPrueba.GPIO_PinConfig_t.GPIO_PinMode         		= GPIO_MODE_OUT;
	CSPrueba.GPIO_PinConfig_t.GPIO_PinOPType		  	= GPIO_OTYPE_PUSHPULL;
	CSPrueba.GPIO_PinConfig_t.GPIO_PinPuPdControl   	= GPIO_PUPDR_NOTHING;
	CSPrueba.GPIO_PinConfig_t.GPIO_PinSpeed         	= GPIO_OSPEED_HIGH;

	GPIO_Config(&CSPrueba);
	GPIO_WritePin(&CSPrueba, SET);

	RSTPrueba.pGPIOx 							    	= GPIOB;
	RSTPrueba.GPIO_PinConfig_t.GPIO_PinNumber        	= PIN_10;
	RSTPrueba.GPIO_PinConfig_t.GPIO_PinMode         	= GPIO_MODE_OUT;
	RSTPrueba.GPIO_PinConfig_t.GPIO_PinOPType		  	= GPIO_OTYPE_PUSHPULL;
	RSTPrueba.GPIO_PinConfig_t.GPIO_PinPuPdControl   	= GPIO_PUPDR_NOTHING;
	RSTPrueba.GPIO_PinConfig_t.GPIO_PinSpeed         	= GPIO_OSPEED_HIGH;

	GPIO_Config(&RSTPrueba);
	GPIO_WritePin(&RSTPrueba, SET);

	DCPrueba.pGPIOx 							    	= GPIOB;
	DCPrueba.GPIO_PinConfig_t.GPIO_PinNumber        	= PIN_4;
	DCPrueba.GPIO_PinConfig_t.GPIO_PinMode         		= GPIO_MODE_OUT;
	DCPrueba.GPIO_PinConfig_t.GPIO_PinOPType		  	= GPIO_OTYPE_PUSHPULL;
	DCPrueba.GPIO_PinConfig_t.GPIO_PinPuPdControl   	= GPIO_PUPDR_NOTHING;
	DCPrueba.GPIO_PinConfig_t.GPIO_PinSpeed         	= GPIO_OSPEED_HIGH;

	GPIO_Config(&DCPrueba);
	GPIO_WritePin(&DCPrueba, SET);



	MOSIPrueba.pGPIOx 								 	= GPIOB;
	MOSIPrueba.GPIO_PinConfig_t.GPIO_PinNumber        	= PIN_5;
	MOSIPrueba.GPIO_PinConfig_t.GPIO_PinMode          	= GPIO_MODE_ALTFN;
	MOSIPrueba.GPIO_PinConfig_t.GPIO_PinOPType		  	= GPIO_OTYPE_PUSHPULL;
	MOSIPrueba.GPIO_PinConfig_t.GPIO_PinPuPdControl   	= GPIO_PUPDR_NOTHING;
	MOSIPrueba.GPIO_PinConfig_t.GPIO_PinSpeed         	= GPIO_OSPEED_HIGH;
	MOSIPrueba.GPIO_PinConfig_t.GPIO_PinAltFunMode    	= AF5;

	GPIO_Config(&MOSIPrueba);
	GPIO_WritePin(&MOSIPrueba, SET);


	SCKPrueba.pGPIOx 								 	= GPIOB;
	SCKPrueba.GPIO_PinConfig_t.GPIO_PinNumber        	= PIN_3;
	SCKPrueba.GPIO_PinConfig_t.GPIO_PinMode          	= GPIO_MODE_ALTFN;
	SCKPrueba.GPIO_PinConfig_t.GPIO_PinOPType		  	= GPIO_OTYPE_PUSHPULL;
	SCKPrueba.GPIO_PinConfig_t.GPIO_PinPuPdControl   	= GPIO_PUPDR_NOTHING;
	SCKPrueba.GPIO_PinConfig_t.GPIO_PinSpeed         	= GPIO_OSPEED_HIGH;
	SCKPrueba.GPIO_PinConfig_t.GPIO_PinAltFunMode    	= AF5;

	GPIO_Config(&SCKPrueba);
	GPIO_WritePin(&SCKPrueba, SET);

	SPIPrueba.ptrSPIx 									= SPI1;
	SPIPrueba.SPI_Config.BaudRatePrescaler				= SPI_BAUDRATE_DIV2;
	SPIPrueba.SPI_Config.CPOLCPHA						= SPI_CPOLCPHA_MODE_00;
	SPIPrueba.SPI_Config.DataSize 						= SPI_DATASIZE_8BIT;
	SPIPrueba.SPI_Config.FirstBit						= SPI_FIRSTBIT_MSB;
	SPIPrueba.SPI_Config.Mode 							= SPI_MODE_MASTER;

	ili.spi_handler = &SPIPrueba;
	ili.cs_pin 		= &CSPrueba;
	ili.dc_pin		= &DCPrueba;
	ili.reset_pin	= &RSTPrueba;

	ILI9341_Init(&ili);



}

//Calback del timer2 para el blinking
void BasicTimer2_Callback(void){
	banderaLedUsuario = 1;
}

void USART2Rx_Callback(void){
	usart2DataReceived = getRxData();
	ILI9341_WriteData(&ili, &usart2DataReceived, 1);
}
