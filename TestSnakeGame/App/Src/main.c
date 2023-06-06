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
#include "SPIxDriver.h"
#include "ILI9341.h"
#include <stdlib.h>
#include "PLLDriver.h"

//Macros para la direccion.

#define RIGHT 	0
#define UP 		1
#define LEFT 	2
#define DOWN 	3
#define PAUSE 	4

//funciones necesarias.
void drawInitialSnake(void);
void advanceSnake(void);
void changeDirection(void);
void createFood(void);
bool checkCollisionFood(uint16_t x,uint16_t y);
void randomFood(void);
void checkCollision(void);


bool banderaLedUsuario = 0; //Bandera TIM2 el cual nos da el blinking del LD2

uint8_t usart2DataReceived = 0; //Dato recibido por el USART2

char string[] = "Hola "; //Datos de prueba para el USART2



BasicTimer_Handler_t handlerTim2 = {0}; //Timer para el blinking
USART_Handler_t USART2Handler = {0}; 	//Handler para manejar el USART2
GPIO_Handler_t handlerUserLedPin = {0}; //Led de usuario para el blinking
GPIO_Handler_t tx2pin = {0};			//Pin para configurar la trasmision del USART2
GPIO_Handler_t rx2pin = {0};			//Pin para configurar la recepcion del USART2
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
GPIO_Handler_t CSPrueba = {0};
GPIO_Handler_t RSTPrueba = {0};
GPIO_Handler_t DCPrueba = {0};
SPI_Handler_t SPIPrueba = {0};
GPIO_Handler_t MOSIPrueba = {0};
GPIO_Handler_t SCKPrueba = {0};
ILI9341_Handler_t ili = {0};

//DEfinimos variables que vamos a utilizar
uint16_t snakeColor = 0x001F; // 16-bit Azul (RGB 565)
uint16_t headPositionX = 0;
uint16_t headPositionY = 0;
uint16_t tailPositionX = 0;
uint16_t tailPositionY = 0;
uint16_t snakeSize = 3;
uint16_t snakePositions[2][300] = {{16*6,16*7,16*8},{16*7,16*7,16*7}};
uint8_t direction = PAUSE;
// Set the background color to black
uint16_t backgroundColor = 0x0; // 16-bit color value for black
uint8_t squareSize = 15;
bool banderaComida = false;
uint16_t foodPositionX = 0;
uint16_t foodPositionY = 0;
uint16_t foodColor = 0xfc20;
bool flagFoodCollision = false;
bool flagDeath = false;



uint16_t duttyValue = 2000;

void initSystem(void);
void USART2Rx_Callback(void);

int main(void){
	//Activacion cooprocesador matematico
	SCB->CPACR |= (0xF <<20);

	initSystem();

	Ili_setRotation(&ili, 3);



    // Fill the entire screen with the background color
    ILI9341_FillRectangle(&ili, 0, 0, ILI9341_TFTHEIGHT - 1, ILI9341_TFTWIDTH - 1, backgroundColor);

    drawInitialSnake();




	while(1){

		if(banderaComida == false){
			randomFood();
			createFood();
		}


		if(banderaLedUsuario == 1){
			banderaLedUsuario = 0;
			GPIOxTooglePin(&handlerUserLedPin);
			advanceSnake();
		}

		if(usart2DataReceived != 0){
			changeDirection();
		}



	}

	return 0;
}

void initSystem(void){

	configPLL(80);


	config_SysTick(getFreqPLL());

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
	handlerTim2.TIMx_Config.TIMx_period = 2500; //Se define el periodo en este caso el led cambiara cada 250ms
	handlerTim2.TIMx_Config.TIMx_speed = BTIMER_SPEED_100us; //Se define la "velocidad" que se usara

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
	SPIPrueba.SPI_Config.BaudRatePrescaler				= SPI_BAUDRATE_DIV8;
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
}

void drawInitialSnake(void){


	// Define coordinates for the rectangle
    tailPositionX = snakePositions[0][0], tailPositionY = snakePositions[1][0], headPositionX = snakePositions[0][snakeSize-1], headPositionY = snakePositions[1][snakeSize-1];

	// Fill the rectangle with the specified color
    ILI9341_FillRectangle(&ili, tailPositionX, tailPositionY, headPositionX+squareSize, headPositionY+squareSize, snakeColor);
    ILI9341_FillRectangle(&ili, headPositionX, headPositionY, headPositionX+squareSize, headPositionY+squareSize, snakeColor);


}

void advanceSnake(void){
	if(direction == RIGHT){

		if(snakePositions[0][snakeSize-1] + squareSize < 319){
				headPositionX = snakePositions[0][snakeSize-1] + 16;
			} else {
				headPositionX = 0;
			}
		headPositionY = snakePositions[1][snakeSize-1];

		checkCollision();


		if(flagFoodCollision == false){
			tailPositionX = snakePositions[0][0], tailPositionY = snakePositions[1][0];
			//Limpiar el valor de la cola
			for(int i = 0; i < snakeSize - 1; i++){
				snakePositions[0][i] = snakePositions[0][i+1];
				snakePositions[1][i] = snakePositions[1][i+1];
			}
			snakePositions[1][snakeSize-1] = headPositionY;
			snakePositions[0][snakeSize-1] = headPositionX;


		}

		else{
			snakeSize++;
			snakePositions[1][snakeSize-1] = headPositionY;
			snakePositions[0][snakeSize-1] = headPositionX;
		}



	}

	else if(direction == DOWN){

		if(snakePositions[1][snakeSize-1] + squareSize < 239){
				headPositionY = snakePositions[1][snakeSize-1] + 16;
			} else {
				headPositionY = 0;
			}
		headPositionX = snakePositions[0][snakeSize-1];

		checkCollision();

		if(flagFoodCollision == false){
			tailPositionX = snakePositions[0][0], tailPositionY = snakePositions[1][0];

			for(int i = 0; i < snakeSize - 1; i++){
				snakePositions[0][i] = snakePositions[0][i+1];
				snakePositions[1][i] = snakePositions[1][i+1];
			}
			snakePositions[1][snakeSize-1] = headPositionY;
			snakePositions[0][snakeSize-1] = headPositionX;
		} else{
			snakeSize++;
			snakePositions[1][snakeSize-1] = headPositionY;
			snakePositions[0][snakeSize-1] = headPositionX;
		}

	}

	else if(direction == UP){

		if(snakePositions[1][snakeSize-1] == 0){
				headPositionY = 240-16;
			} else {
				headPositionY = snakePositions[1][snakeSize-1] - 16;
			}

		headPositionX = snakePositions[0][snakeSize-1];

		checkCollision();

		if(flagFoodCollision == false){
			tailPositionX = snakePositions[0][0], tailPositionY = snakePositions[1][0];
			//Limpiar el valor de la cola
			for(int i = 0; i < snakeSize - 1; i++){
				snakePositions[0][i] = snakePositions[0][i+1];
				snakePositions[1][i] = snakePositions[1][i+1];
			}

			snakePositions[0][snakeSize-1] = headPositionX;
			snakePositions[1][snakeSize-1] = headPositionY;

		} else{
			snakeSize++;
			snakePositions[1][snakeSize-1] = headPositionY;
			snakePositions[0][snakeSize-1] = headPositionX;
		}

	}

	else if(direction == LEFT){

		if(snakePositions[0][snakeSize-1] == 0){
				headPositionX = 320-16;
			} else {
				headPositionX = snakePositions[0][snakeSize-1] - 16;
			}

		headPositionY = snakePositions[1][snakeSize-1];

		checkCollision();

		if(flagFoodCollision == false){


			tailPositionX = snakePositions[0][0], tailPositionY = snakePositions[1][0];
			//Limpiar el valor de la cola
			for(int i = 0; i < snakeSize - 1; i++){
				snakePositions[0][i] = snakePositions[0][i+1];
				snakePositions[1][i] = snakePositions[1][i+1];
			}
			snakePositions[1][snakeSize-1] = headPositionY;
			snakePositions[0][snakeSize-1] = headPositionX;
		} else{
			snakeSize++;
			snakePositions[1][snakeSize-1] = headPositionY;
			snakePositions[0][snakeSize-1] = headPositionX;
		}

	}


	if(flagDeath){
		direction = PAUSE;
		delay_ms(100);
		 NVIC_SystemReset();
	}

	if(direction != PAUSE){
		if(flagFoodCollision == false){
			ILI9341_FillRectangle(&ili, tailPositionX, tailPositionY, tailPositionX+squareSize, tailPositionY+squareSize, backgroundColor);
			ILI9341_FillRectangle(&ili, headPositionX, headPositionY, headPositionX+squareSize, headPositionY+squareSize, snakeColor);
		}
		else{
			ILI9341_FillRectangle(&ili, headPositionX, headPositionY, headPositionX+squareSize, headPositionY+squareSize, snakeColor);
			flagFoodCollision = false;
			banderaComida = false;
		}

	}

	else{
		__NOP();
	}
}

void changeDirection(void){
			if(usart2DataReceived == 'W' || usart2DataReceived == 'w'){
				if(direction!=DOWN){
					direction = UP;
				}
			}

			else if(usart2DataReceived == 'A' || usart2DataReceived == 'a'){
				if(direction!=RIGHT){
					direction = LEFT;
				}
			}

			else if(usart2DataReceived == 'S' || usart2DataReceived == 's'){
				if(direction!=UP){
					direction = DOWN;
				}
			}

			else if(usart2DataReceived == 'd' || usart2DataReceived == 'D'){
				if(direction!=LEFT){
					direction = RIGHT;
				}
			}

			else if(usart2DataReceived == 'p' || usart2DataReceived == 'P'){
				direction = PAUSE;
			}
			usart2DataReceived = 0;
}


void createFood(void){

	ILI9341_FillRectangle(&ili, foodPositionX, foodPositionY, foodPositionX+squareSize, foodPositionY+squareSize, foodColor);

	banderaComida = true;
}

bool checkCollisionFood(uint16_t x,uint16_t y){
	for(int i = 0; i < snakeSize; i++){
			if(x == snakePositions[0][i] && y == snakePositions[1][i]){
				return true;
			}
		}
	if(x == headPositionX && y == headPositionY){
		return true;
	}
	return false;
}

void checkCollision(void){
	for(int i = 0; i < snakeSize; i++){
			if(foodPositionX == snakePositions[0][i] && foodPositionY == snakePositions[1][i]){
				flagFoodCollision = true;
			}
			if(headPositionX == snakePositions[0][i] && headPositionY == snakePositions[1][i]){
				flagDeath = true;
			}
		}
	if(foodPositionX == headPositionX && foodPositionY == headPositionY){
		flagFoodCollision = true;
	}
}

void randomFood(void){
	uint8_t tempx = rand()%20, tempy = rand()%15;
	if(checkCollisionFood(tempx * 16, tempy * 16)==false){
		foodPositionX = tempx * 16;
		foodPositionY = tempy * 16;
	} else{randomFood();}
}

