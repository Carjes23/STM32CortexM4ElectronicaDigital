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
#include "AdcDriver.h"
#include "PwmDriver.h"
#include "ExtiDriver.h"

//Macros para la direccion.

#define RIGHT 	1
#define UP 		2
#define LEFT 	3
#define DOWN 	4
#define PAUSE 	5

uint8_t usart2DataReceived = 0; //Dato recibido por el USART2

char string[] = "Hola "; //Datos de prueba para el USART2

//Funcion para cuadrar el ADC
ADC_Config_t adcConfig = { 0 };

BasicTimer_Handler_t handlerTim2 = { 0 }; //Timer para el blinking
BasicTimer_Handler_t handlerTim5 = { 0 }; //Timer para el blinking
USART_Handler_t HandlerTerminal = { 0 }; 	//Handler para manejar el USART2
GPIO_Handler_t handlerUserLedPin = { 0 }; //Led de usuario para el blinking
GPIO_Handler_t tx2pin = { 0 };	//Pin para configurar la trasmision del USART2
GPIO_Handler_t rx2pin = { 0 };	//Pin para configurar la recepcion del USART2
uint8_t usart2image = 0; //Dato recibido por el usart dos
//Ultimos datos de cada ADC
uint16_t adcLastData1 = 0;
uint16_t adcLastData2 = 0;
uint8_t adcIsCompleteCount = 0;

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
PWM_Handler_t pwmadc = { 0 }; //Para configurar el PWM en el timer 3 para X

//DEfinimos variables que vamos a utilizar
uint16_t snakeColor = ILI9341_DARKGREEN; // 16-bit Azul (RGB 565)
uint16_t wallColor = ILI9341_BLUE;
uint16_t headPositionX = 0;
uint16_t headPositionY = 0;
uint16_t tailPositionX = 0;
uint16_t tailPositionY = 0;
uint16_t snakeSize = 3;
uint16_t snakePositions[2][300] = { { 16 * 6, 16 * 7, 16 * 8 }, { 16 * 7, 16
		* 7, 16 * 7 } };
char bufferData[128] = { 0 };	//para crear mensajes
uint8_t direction = PAUSE;
// Set the background color to black
uint16_t backgroundColor = 0x0; // 16-bit color value for black
uint8_t squareSize = 15;
bool banderaComida = false;
uint16_t foodPositionX = 0;
uint16_t foodPositionY = 0;
uint16_t foodColor = ILI9341_PINK;
bool flagFoodCollision = false;
bool flagDeath = false;
uint8_t trimval = 13;
bool star = 0;
uint8_t score = 0;
uint8_t tempScore = 0;
bool enableChangeDir = 1;
uint32_t contadorPan = 0;
uint8_t newDirVal = 0;
uint8_t newYval = 0;
uint8_t contador10Segundos = 0;
uint8_t contadorMovimiento = 0;
uint8_t contadorNewChange = 0;
bool newChangeRecently = 0;
bool enableChange = 1;
uint32_t periodADC = 25000;

uint16_t duttyValue = 10;
/*
 * Variables relacionadas al MenuSnake
 */
bool gameMode = 0;
uint8_t selected = 1;
uint16_t menucolor = ILI9341_PURPLE;
uint16_t menucolorSelect = ILI9341_WHITE;
char *menu1 = "Easy";
char *menu2 = "Normal";
char *menu3 = "Hard";
uint16_t menux = 30;
uint16_t menuy = 30;
uint8_t flagRefrescoMenu = 0; //Bandera TIM2 el cual nos da el blinking del LD2
uint8_t level = 0;

/*
 * EXTI
 */

GPIO_Handler_t handlerLeft = { 0 };
EXTI_Config_t handlerExtiLeft = { 0 };

GPIO_Handler_t handlerRight = { 0 };
EXTI_Config_t handlerExtiRight = { 0 };

GPIO_Handler_t handlerUp = { 0 };
EXTI_Config_t handlerExtiUp = { 0 };

GPIO_Handler_t handlerDown = { 0 };
EXTI_Config_t handlerExtiDown = { 0 };

//funciones necesarias.
void drawInitialSnake(void);
void advanceSnake(void);
void changeDirection(void);
void createFood(void);
bool checkCollisionFood(uint16_t x, uint16_t y);
void randomFood(void);
void checkCollision(void);
void resetEasyMode(void);
void initSystem(void);
void USART2Rx_Callback(void);
void increseSpeed(void);
void updateMenu(void);
void resetNormalMode(void);
void resetHardMode(void);

int main(void) {

	initSystem();

	writeString(&HandlerTerminal, string);

	Ili_setRotation(&ili, 3);

	// Fill the entire screen with the background color
	ILI9341_FillRectangle(&ili, 0, 0, ILI9341_TFTHEIGHT - 1,
	ILI9341_TFTWIDTH - 1, backgroundColor);

	while (1) {
		if (gameMode) {
			if ((tempScore + 1) % 2 == 0) {
				tempScore = 0;
				increseSpeed();
			}

			if (banderaComida == false && flagDeath == 0) {
				randomFood();
				createFood();
			}

			if (contadorMovimiento > 9 && flagDeath == 0) {
				contadorMovimiento = 0;
				advanceSnake();
			}
			if (contadorNewChange > 9) {
				enableChange = 1;
				contadorNewChange = 0;
				newChangeRecently = 0;

			}

			if (newDirVal != 0 && flagDeath == 0) {
				changeDirection();
			}
			//Volvemos a colocar la pantalla lista para la recepción de la siguiente imagen
			if (contadorPan == 86400) {
				//Reiniciamos el contador
				contadorPan = 0;
				// cerramos la comunicación de la anterior imange
				Ili_endCom(&ili);

				flagDeath = 0;
				sprintf(bufferData, "Score %d", score);
				uint16_t x = 100;
				uint16_t y = 170;
				uint16_t color = ILI9341_PURPLE;
				ILI9341_WriteString(&ili, x, y, color, backgroundColor,
						bufferData, 3, 3);
				delay_ms(1000);
				if (level == 1) {
					resetEasyMode();
				} else if (level == 2) {
					resetNormalMode();
				} else if (level == 3) {
					resetHardMode();
				} else {
					resetEasyMode();
				}

			}
		} else {

			if (flagRefrescoMenu > 1) {
				if (newDirVal == RIGHT) {
					level = selected;
					newDirVal = 0;
				} else if (newDirVal == DOWN) {
					if (selected < 3) {
						selected++;
					} else {
						selected = 1;
					}
					newDirVal = 0;
				} else if (newDirVal == UP) {
					if (selected > 1) {
						selected--;
					} else {
						selected = 3;
					}
					newDirVal = 0;
				}
				flagRefrescoMenu = 0;
				updateMenu();
			}

			if (level != 0) {
				gameMode = 1;
				if (level == 1) {
					resetEasyMode();
				} else if (level == 2) {
					resetNormalMode();
				} else if (level == 3) {
					resetHardMode();
				} else {
					resetEasyMode();
				}

			}
		}

	}

	return 0;
}

void initSystem(void) {
	//Activacion cooprocesador matematico
	SCB->CPACR |= (0xF << 20);
	configPLL(80);

	config_SysTick();

	//Led de usuario usado para el blinking

	handlerUserLedPin.pGPIOx = GPIOA; //Se encuentra en el el GPIOA
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinNumber = PIN_5; // Es el pin 5.
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_OUT; //Se utiliza como salida
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL; //Salida PushPull
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING; //No se usa ninguna resistencia
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_FAST; //Se usa en velocidad rapida

	GPIO_Config(&handlerUserLedPin); //Se carga la configuración para el Led.

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

	//PA2
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

	ili.spi_handler = &SPIPrueba;
	ili.cs_pin = &CSPrueba;
	ili.dc_pin = &DCPrueba;
	ili.reset_pin = &RSTPrueba;

	ILI9341_Init(&ili);

	//Configuración Timer 4 que controla el filtro.
	//En este caso al usarlo a 25ms las interrupciones de rebote que se den en ese intervalo son desestimadas.

	handlerTim5.ptrTIMx = TIM5; //El timer que se va a usar
	handlerTim5.TIMx_Config.TIMx_interruptEnable = 1; //Se habilitan las interrupciones
	handlerTim5.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; //Se usara en modo ascendente

	BasicTimer_Config(&handlerTim5); //Se carga la configuración.

	pwmadc.ptrTIMx = TIM5;
	pwmadc.config.channel = PWM_CHANNEL_1;
	pwmadc.config.duttyCicle = duttyValue;
	pwmadc.config.periodo = periodADC;
	pwmadc.config.prescaler = 80;

	pwm_Config(&pwmadc);
	enableOutput(&pwmadc);
	startPwmSignal(&pwmadc);

	//Boton de usuario para el cambio de tasa de resfresco.

	handlerLeft.pGPIOx = GPIOC; //Se encuentra en el el GPIOC
	handlerLeft.GPIO_PinConfig_t.GPIO_PinNumber = PIN_0; //Es el pin numero 13
	handlerLeft.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_IN; //En este caso el pin se usara como entrada de datos
	handlerLeft.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLDOWN; //No se usara ninguna resistencia pull-up o pull-down

	handlerExtiLeft.pGPIOHandler = &handlerLeft; // Se carga la config del pin al handler del EXTI
	handlerExtiLeft.edgeType = EXTERNAL_INTERRUPT_RISING_EDGE; //Se define que se van a detectar flancos de bajada.

	extInt_Config(&handlerExtiLeft); //Se carga la configuracion usando la funcion de la libreria.

	//Boton de usuario para el cambio de tasa de resfresco.

	handlerRight.pGPIOx = GPIOC; //Se encuentra en el el GPIOC
	handlerRight.GPIO_PinConfig_t.GPIO_PinNumber = PIN_1; //Es el pin numero 13
	handlerRight.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_IN; //En este caso el pin se usara como entrada de datos
	handlerRight.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLDOWN; //No se usara ninguna resistencia pull-up o pull-down

	handlerExtiRight.pGPIOHandler = &handlerRight; // Se carga la config del pin al handler del EXTI
	handlerExtiRight.edgeType = EXTERNAL_INTERRUPT_RISING_EDGE; //Se define que se van a detectar flancos de bajada.

	extInt_Config(&handlerExtiRight); //Se carga la configuracion usando la funcion de la libreria.

	//Boton de usuario para el cambio de tasa de resfresco.

	handlerUp.pGPIOx = GPIOC; //Se encuentra en el el GPIOC
	handlerUp.GPIO_PinConfig_t.GPIO_PinNumber = PIN_3; //Es el pin numero 13
	handlerUp.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_IN; //En este caso el pin se usara como entrada de datos
	handlerUp.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLDOWN; //No se usara ninguna resistencia pull-up o pull-down

	handlerExtiUp.pGPIOHandler = &handlerUp; // Se carga la config del pin al handler del EXTI
	handlerExtiUp.edgeType = EXTERNAL_INTERRUPT_RISING_EDGE; //Se define que se van a detectar flancos de bajada.

	extInt_Config(&handlerExtiUp); //Se carga la configuracion usando la funcion de la libreria.

	//Boton de usuario para el cambio de tasa de resfresco.

	handlerDown.pGPIOx = GPIOC; //Se encuentra en el el GPIOC
	handlerDown.GPIO_PinConfig_t.GPIO_PinNumber = PIN_2; //Es el pin numero 13
	handlerDown.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_IN; //En este caso el pin se usara como entrada de datos
	handlerDown.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLDOWN; //No se usara ninguna resistencia pull-up o pull-down

	handlerExtiDown.pGPIOHandler = &handlerDown; // Se carga la config del pin al handler del EXTI
	handlerExtiDown.edgeType = EXTERNAL_INTERRUPT_RISING_EDGE; //Se define que se van a detectar flancos de bajada.

	extInt_Config(&handlerExtiDown); //Se carga la configuracion usando la funcion de la libreria.

	//COnfiugracion ADC se pones un samplig period de 56 ya que ~= 100 mhz / 56 ~= 1.78 Mhz que es más que suficiente
	uint8_t channels[2] = { ADC_CHANNEL_1, ADC_CHANNEL_4 };
	adcConfig.channels = channels;
	adcConfig.dataAlignment = ADC_ALIGNMENT_RIGHT;
	adcConfig.numberOfChannels = 2;
	uint8_t samplingPeriod[2] = { 0 };
	samplingPeriod[0] = ADC_SAMPLING_PERIOD_56_CYCLES
	;
	samplingPeriod[1] = ADC_SAMPLING_PERIOD_56_CYCLES
	;
	adcConfig.samplingPeriod = samplingPeriod;
	adcConfig.resolution = ADC_RESOLUTION_12_BIT;
	adcConfig.externType = EXTEN_RISING_TIMER5_CC1;

	adc_Config(&adcConfig);

	changeTrim(trimval);

}

//Calback del timer2 para el blinking
void BasicTimer2_Callback(void) {
	GPIOxTooglePin(&handlerUserLedPin);
	if (flagDeath) {
		writeChar(&HandlerTerminal, '1');
	}
	if (gameMode == 0) {
		flagRefrescoMenu++;
	}

}

//Calback del timer5 para el movimiento
void BasicTimer5_Callback(void) {
	contadorMovimiento++;
	if (newChangeRecently == 1) {
		contadorNewChange++;
	}
}

void adcComplete_Callback(void) {
	adcIsCompleteCount++;
	if (adcIsCompleteCount == 1) {
		adcLastData1 = getADC();
		if (adcLastData1 > 3686) {
			newDirVal = DOWN;
		} else if (adcLastData1 < 410) {
			newDirVal = UP;
		}
	} else {
		adcLastData2 = getADC();
		if (adcLastData2 > 3686) {
			newDirVal = LEFT;
		} else if (adcLastData2 < 410) {
			newDirVal = RIGHT;
		}
		adcIsCompleteCount = 0;
	}
}

void USART2Rx_Callback(void) {
	if (flagDeath) {
		usart2image = getRxData();
		ILI9341_WriteData(&ili, &usart2image, 1);
		contadorPan++;
	}
}

void drawInitialSnake(void) {

	// Define coordinates for the rectangle
	tailPositionX = snakePositions[0][0], tailPositionY = snakePositions[1][0], headPositionX =
			snakePositions[0][snakeSize - 1], headPositionY =
			snakePositions[1][snakeSize - 1];

	// Fill the rectangle with the specified color
	ILI9341_FillRectangle(&ili, tailPositionX, tailPositionY,
			headPositionX + squareSize, headPositionY + squareSize, snakeColor);
	ILI9341_FillRectangle(&ili, headPositionX, headPositionY,
			headPositionX + squareSize, headPositionY + squareSize, snakeColor);

}

void advanceSnake(void) {
	if (enableChangeDir == 1) {
		if (direction == RIGHT) {
			if (level == 1) {
				if (snakePositions[0][snakeSize - 1] + squareSize < 319) {
					headPositionX = snakePositions[0][snakeSize - 1] + 16;
				} else {
					headPositionX = 0;
				}
				headPositionY = snakePositions[1][snakeSize - 1];
			} else {
				if (snakePositions[0][snakeSize - 1] + squareSize < 319 - 16) {
					headPositionX = snakePositions[0][snakeSize - 1] + 16;
				} else {
					flagDeath = 1;
				}
			}

			checkCollision();

			if (flagFoodCollision == false) {
				tailPositionX = snakePositions[0][0], tailPositionY =
						snakePositions[1][0];
				//Limpiar el valor de la cola
				for (int i = 0; i < snakeSize - 1; i++) {
					snakePositions[0][i] = snakePositions[0][i + 1];
					snakePositions[1][i] = snakePositions[1][i + 1];
				}
				snakePositions[1][snakeSize - 1] = headPositionY;
				snakePositions[0][snakeSize - 1] = headPositionX;

			}

			else {
				snakeSize++;
				snakePositions[1][snakeSize - 1] = headPositionY;
				snakePositions[0][snakeSize - 1] = headPositionX;
			}

		}

		else if (direction == DOWN) {
			if (level == 1) {
				if (snakePositions[1][snakeSize - 1] + squareSize < 239) {
					headPositionY = snakePositions[1][snakeSize - 1] + 16;
				} else {
					headPositionY = 0;
				}
				headPositionX = snakePositions[0][snakeSize - 1];
			}

			else {
				if (snakePositions[1][snakeSize - 1] + squareSize < 239 - 16) {
					headPositionY = snakePositions[1][snakeSize - 1] + 16;
				} else {
					flagDeath = 1;
				}
				headPositionX = snakePositions[0][snakeSize - 1];
			}

			checkCollision();

			if (flagFoodCollision == false) {
				tailPositionX = snakePositions[0][0], tailPositionY =
						snakePositions[1][0];

				for (int i = 0; i < snakeSize - 1; i++) {
					snakePositions[0][i] = snakePositions[0][i + 1];
					snakePositions[1][i] = snakePositions[1][i + 1];
				}
				snakePositions[1][snakeSize - 1] = headPositionY;
				snakePositions[0][snakeSize - 1] = headPositionX;
			} else {
				snakeSize++;
				snakePositions[1][snakeSize - 1] = headPositionY;
				snakePositions[0][snakeSize - 1] = headPositionX;
			}

		}

		else if (direction == UP) {
			if (level == 1) {
				if (snakePositions[1][snakeSize - 1] == 0) {
					headPositionY = 240 - 16;
				} else {
					headPositionY = snakePositions[1][snakeSize - 1] - 16;
				}
			} else {
				if (snakePositions[1][snakeSize - 1] <= 16) {
					flagDeath = 1;
				} else {
					headPositionY = snakePositions[1][snakeSize - 1] - 16;
				}
			}

			headPositionX = snakePositions[0][snakeSize - 1];

			checkCollision();

			if (flagFoodCollision == false) {
				tailPositionX = snakePositions[0][0], tailPositionY =
						snakePositions[1][0];
				//Limpiar el valor de la cola
				for (int i = 0; i < snakeSize - 1; i++) {
					snakePositions[0][i] = snakePositions[0][i + 1];
					snakePositions[1][i] = snakePositions[1][i + 1];
				}

				snakePositions[0][snakeSize - 1] = headPositionX;
				snakePositions[1][snakeSize - 1] = headPositionY;

			} else {
				snakeSize++;
				snakePositions[1][snakeSize - 1] = headPositionY;
				snakePositions[0][snakeSize - 1] = headPositionX;
			}

		}

		else if (direction == LEFT) {
			if (level == 1) {
				if (snakePositions[0][snakeSize - 1] == 0) {
					headPositionX = 320 - 16;
				} else {
					headPositionX = snakePositions[0][snakeSize - 1] - 16;
				}
			} else {
				if (snakePositions[0][snakeSize - 1] <= 16) {
					flagDeath = 1;
				} else {
					headPositionX = snakePositions[0][snakeSize - 1] - 16;
				}
			}

			headPositionY = snakePositions[1][snakeSize - 1];

			checkCollision();

			if (flagFoodCollision == false) {

				tailPositionX = snakePositions[0][0], tailPositionY =
						snakePositions[1][0];
				//Limpiar el valor de la cola
				for (int i = 0; i < snakeSize - 1; i++) {
					snakePositions[0][i] = snakePositions[0][i + 1];
					snakePositions[1][i] = snakePositions[1][i + 1];
				}
				snakePositions[1][snakeSize - 1] = headPositionY;
				snakePositions[0][snakeSize - 1] = headPositionX;
			} else {
				snakeSize++;
				snakePositions[1][snakeSize - 1] = headPositionY;
				snakePositions[0][snakeSize - 1] = headPositionX;
			}
		}
	}

	if (flagDeath) {
		direction = PAUSE;
		enableChangeDir = 0;
		//Iniciamos la comuniación para la recepcion de la primera imagen
		Ili_starCom(&ili);
		//Definimos toda la pantalla para rellenarla por completo.
		ILI9341_SetAddressWindow(&ili, 40, 30, 240 - 1 + 40, 180 - 1 + 30);
	}

	if (direction != PAUSE) {
		if (flagFoodCollision == false) {
			ILI9341_FillRectangle(&ili, tailPositionX, tailPositionY,
					tailPositionX + squareSize, tailPositionY + squareSize,
					backgroundColor);
			ILI9341_FillRectangle(&ili, headPositionX, headPositionY,
					headPositionX + squareSize, headPositionY + squareSize,
					snakeColor);
		} else {
			ILI9341_FillRectangle(&ili, headPositionX, headPositionY,
					headPositionX + squareSize, headPositionY + squareSize,
					snakeColor);
			flagFoodCollision = false;
			banderaComida = false;
			score++;
			tempScore++;
		}

	}

	else {
		__NOP();
	}
}

void changeDirection(void) {
	if (enableChange == 1) {
		if (newDirVal == UP) {
			if (direction != DOWN) {
				direction = UP;
				star = 1;
				newDirVal = 0;
			}
		} else if (newDirVal == LEFT && star == 1) {
			if (direction != RIGHT) {
				direction = LEFT;
				newDirVal = 0;
			}
		} else if (newDirVal == DOWN) {
			if (direction != UP) {
				direction = DOWN;
				star = 1;
				newDirVal = 0;
			}

		} else if (newDirVal == RIGHT) {
			if (direction != LEFT) {
				direction = RIGHT;
				star = 1;
				newDirVal = 0;
			}

		}
		newDirVal = 0;
		enableChange = 0;
		newChangeRecently = 1;
	} else {
		newDirVal = 0;
	}

}

void createFood(void) {

	ILI9341_FillRectangle(&ili, foodPositionX, foodPositionY,
			foodPositionX + squareSize, foodPositionY + squareSize, foodColor);

	banderaComida = true;
}

bool checkCollisionFood(uint16_t x, uint16_t y) {
	for (int i = 0; i < snakeSize; i++) {
		if (x == snakePositions[0][i] && y == snakePositions[1][i]) {
			return true;

		}
	}
	if (x == headPositionX && y == headPositionY) {
		return true;

	}
	if (level != 1) {
		if (x <= 15 || y <= 15 || x >= 319 - 16 || y >= 239 - 16) {
			return true;
		}
	}
	return false;
}

void checkCollision(void) {
	for (int i = 0; i < snakeSize; i++) {
		if (foodPositionX == snakePositions[0][i]
				&& foodPositionY == snakePositions[1][i]) {
			flagFoodCollision = true;
		}
		if (headPositionX == snakePositions[0][i]
				&& headPositionY == snakePositions[1][i]) {
			flagDeath = true;
		}
	}
	if (foodPositionX == headPositionX && foodPositionY == headPositionY) {
		flagFoodCollision = true;
	}
}

void randomFood(void) {
	uint8_t tempx = rand() % 20, tempy = rand() % 15;
	if (checkCollisionFood(tempx * 16, tempy * 16) == false) {
		foodPositionX = tempx * 16;
		foodPositionY = tempy * 16;
	} else {
		randomFood();
	}
}

void increseSpeed(void) {
	float aux = (periodADC * 0.95f);
	periodADC = (int) (aux);
	if (periodADC >= 20) {
		updateFrequency(&pwmadc, periodADC);
	}
}

void resetEasyMode(void) {
	score = 0;
	star  = 0;
	periodADC = 25000;
	banderaComida = false;
	updateFrequency(&pwmadc, periodADC);
	snakeSize = 3;
	snakePositions[0][0] = 16 * 6;
	snakePositions[0][1] = 16 * 7;
	snakePositions[0][2] = 16 * 8;
	snakePositions[1][0] = 16 * 7;
	snakePositions[1][1] = 16 * 7;
	snakePositions[1][2] = 16 * 7;
	direction = PAUSE;
	newDirVal = PAUSE;
	adcIsCompleteCount = 0;
	//COnfiugracion ADC se pones un samplig period de 56 ya que ~= 100 mhz / 56 ~= 1.78 Mhz que es más que suficiente
	uint8_t channels[2] = { ADC_CHANNEL_1, ADC_CHANNEL_4 };
	adcConfig.channels = channels;
	adcConfig.dataAlignment = ADC_ALIGNMENT_RIGHT;
	adcConfig.numberOfChannels = 2;
	uint8_t samplingPeriod[2] = { 0 };
	samplingPeriod[0] = ADC_SAMPLING_PERIOD_56_CYCLES
	;
	samplingPeriod[1] = ADC_SAMPLING_PERIOD_56_CYCLES
	;
	adcConfig.samplingPeriod = samplingPeriod;
	adcConfig.resolution = ADC_RESOLUTION_12_BIT;
	adcConfig.externType = EXTEN_RISING_TIMER5_CC1;

	adc_Config(&adcConfig);
	// Fill the entire screen with the background color
	ILI9341_FillRectangle(&ili, 0, 0, ILI9341_TFTHEIGHT - 1,
	ILI9341_TFTWIDTH - 1, backgroundColor);

	drawInitialSnake();
	enableChangeDir = 1;
}

void resetNormalMode(void) {
	score = 0;
	star =0;
	periodADC = 25000;
	banderaComida = false;
	updateFrequency(&pwmadc, periodADC);
	snakeSize = 3;
	snakePositions[0][0] = 16 * 6;
	snakePositions[0][1] = 16 * 7;
	snakePositions[0][2] = 16 * 8;
	snakePositions[1][0] = 16 * 7;
	snakePositions[1][1] = 16 * 7;
	snakePositions[1][2] = 16 * 7;
	direction = PAUSE;
	newDirVal = PAUSE;
	adcIsCompleteCount = 0;
	//COnfiugracion ADC se pones un samplig period de 56 ya que ~= 100 mhz / 56 ~= 1.78 Mhz que es más que suficiente
	uint8_t channels[2] = { ADC_CHANNEL_1, ADC_CHANNEL_4 };
	adcConfig.channels = channels;
	adcConfig.dataAlignment = ADC_ALIGNMENT_RIGHT;
	adcConfig.numberOfChannels = 2;
	uint8_t samplingPeriod[2] = { 0 };
	samplingPeriod[0] = ADC_SAMPLING_PERIOD_56_CYCLES
	;
	samplingPeriod[1] = ADC_SAMPLING_PERIOD_56_CYCLES
	;
	adcConfig.samplingPeriod = samplingPeriod;
	adcConfig.resolution = ADC_RESOLUTION_12_BIT;
	adcConfig.externType = EXTEN_RISING_TIMER5_CC1;

	adc_Config(&adcConfig);
	// Fill the entire screen with the background color
	ILI9341_FillRectangle(&ili, 0, 0, ILI9341_TFTHEIGHT - 1,
	ILI9341_TFTWIDTH - 1, wallColor);
	ILI9341_FillRectangle(&ili, 16, 16, ILI9341_TFTHEIGHT - 1 - 16,
	ILI9341_TFTWIDTH - 1 - 16, backgroundColor);

	drawInitialSnake();
	enableChangeDir = 1;
}

void resetHardMode(void) {
	score = 0;
	star = 0;
	periodADC = 10000;
	banderaComida = false;
	updateFrequency(&pwmadc, periodADC);
	snakeSize = 3;
	snakePositions[0][0] = 16 * 6;
	snakePositions[0][1] = 16 * 7;
	snakePositions[0][2] = 16 * 8;
	snakePositions[1][0] = 16 * 7;
	snakePositions[1][1] = 16 * 7;
	snakePositions[1][2] = 16 * 7;
	direction = PAUSE;
	newDirVal = PAUSE;
	adcIsCompleteCount = 0;
	//COnfiugracion ADC se pones un samplig period de 56 ya que ~= 100 mhz / 56 ~= 1.78 Mhz que es más que suficiente
	uint8_t channels[2] = { ADC_CHANNEL_1, ADC_CHANNEL_4 };
	adcConfig.channels = channels;
	adcConfig.dataAlignment = ADC_ALIGNMENT_RIGHT;
	adcConfig.numberOfChannels = 2;
	uint8_t samplingPeriod[2] = { 0 };
	samplingPeriod[0] = ADC_SAMPLING_PERIOD_56_CYCLES
	;
	samplingPeriod[1] = ADC_SAMPLING_PERIOD_56_CYCLES
	;
	adcConfig.samplingPeriod = samplingPeriod;
	adcConfig.resolution = ADC_RESOLUTION_12_BIT;
	adcConfig.externType = EXTEN_RISING_TIMER5_CC1;

	adc_Config(&adcConfig);
	// Fill the entire screen with the background color
	ILI9341_FillRectangle(&ili, 0, 0, ILI9341_TFTHEIGHT - 1,
	ILI9341_TFTWIDTH - 1, wallColor);
	ILI9341_FillRectangle(&ili, 16, 16, ILI9341_TFTHEIGHT - 1 - 16,
	ILI9341_TFTWIDTH - 1 - 16, backgroundColor);

	drawInitialSnake();
	enableChangeDir = 1;
}

void updateMenu(void) {
	if (selected == 1) {
		ILI9341_WriteString(&ili, menux, menuy * 3, menucolor, backgroundColor,
				menu2, 4, 4);
		ILI9341_WriteString(&ili, menux, menuy * 5, menucolor, backgroundColor,
				menu3, 4, 4);
		ILI9341_WriteString(&ili, menux, menuy, menucolorSelect,
				backgroundColor, menu1, 4, 4);
	} else if (selected == 2) {
		ILI9341_WriteString(&ili, menux, menuy, menucolor, backgroundColor,
				menu1, 4, 4);
		ILI9341_WriteString(&ili, menux, menuy * 5, menucolor, backgroundColor,
				menu3, 4, 4);
		ILI9341_WriteString(&ili, menux, menuy * 3, menucolorSelect,
				backgroundColor, menu2, 4, 4);
	} else if (selected == 3) {
		ILI9341_WriteString(&ili, menux, menuy, menucolor, backgroundColor,
				menu1, 4, 4);
		ILI9341_WriteString(&ili, menux, menuy * 3, menucolor, backgroundColor,
				menu2, 4, 4);
		ILI9341_WriteString(&ili, menux, menuy * 5, menucolorSelect,
				backgroundColor, menu3, 4, 4);
	} else {
		ILI9341_WriteString(&ili, menux, menuy * 5, menucolor, backgroundColor,
				menu3, 4, 4);
		ILI9341_WriteString(&ili, menux, menuy * 3, menucolor, backgroundColor,
				menu2, 4, 4);
		ILI9341_WriteString(&ili, menux, menuy, menucolorSelect,
				backgroundColor, menu1, 4, 4);
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

