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
/* Librerias que se van a utilizar a lo laro del proyecto. */
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

// Definiciones MACROS para la direccion.
#define RIGHT 	1
#define UP 		2
#define LEFT 	3
#define DOWN 	4
#define PAUSE 	5

//Data de prueba para el USART2
char string[] = "Hola ";

/************************* Configuracion de Perifericos *************************/

PWM_Handler_t pwmadc = { 0 };   // PWM encargado del ADC
ADC_Config_t adcConfig = { 0 }; //Funcion para configurar el ADC

BasicTimer_Handler_t handlerTim2 = { 0 }; // Timer para el blinking
BasicTimer_Handler_t handlerTim5 = { 0 }; // Timer encargado del ADC & Movimiento

GPIO_Handler_t tx2pin = { 0 };	//Pin para configurar la trasmision del USART2
GPIO_Handler_t rx2pin = { 0 };	//Pin para configurar la recepcion del USART2
USART_Handler_t HandlerTerminal = { 0 }; 	//Handler para manejar el USART2

uint8_t usart2image = 0; //Dato recibido por el usart 2:; Imagenes

GPIO_Handler_t handlerUserLedPin = { 0 }; //Led de usuario para el blinking

// Ultimos datos de cada ADC y contadores para su correcto funcionamiento
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

/************************* Configuracion de Variables del programa *************************/

// Variables: Sintonización del MCU
uint8_t trimval = 13;

// Variables: Escenario de Juego
uint16_t backgroundColor = ILI9341_BLACK;
uint16_t snakeColor = ILI9341_DARKGREEN;
uint16_t wallColor = ILI9341_BLUE;
uint8_t  direction = PAUSE;

// Variables: Posiciones de Juego
uint16_t headPositionX = 0;
uint16_t headPositionY = 0;
uint16_t tailPositionX = 0;
uint16_t tailPositionY = 0;

//Score y score temporal para el  aumento de velocidad
uint8_t score = 0;
uint8_t tempScore = 0;

// Variables: Parametros Default de Snake
uint16_t snakeSize = 3;  // tamaño inicial
uint16_t snakePositions[2][300] = { { 16 * 6, 16 * 7, 16 * 8 },
									{ 16 * 7, 16* 7, 16 * 7 } };

// Buffer para envio de datos
char bufferData[128] = { 0 };

// Variables: Parametros Default de Comida
uint16_t foodColor = ILI9341_PINK;

//Tamaño de cada cuadricula en el juego
uint8_t squareSize = 15;


//Iniacialización variables de la comida
uint16_t foodPositionX = 0;
uint16_t foodPositionY = 0;


// Variables: Banderas de Estados
bool banderaComida = false; //Si hay comida en pantalla
bool flagFoodCollision = false; //Si se comio la comida
bool flagDeath = false; //Bandera que se lanza cuando se muere la culebra
bool enableChangeDir = 1; //Bandera para filtar direcciones en el momento cuando nose ha inciado
bool newChangeRecently = 0; //Bandera para filtar direcciones rapidas
bool enableChange = 1; //Bandera para no permitir movimientos mientras carga la pantalla
bool star = 0; //Si ya se inicio el juego o se siguente en men

// Variables: Movimiento & cambio
uint8_t newDirVal = 0; //Nueva variable de movimiento

// Variables: Contadores
uint32_t contadorPan = 0;		//Contador para verificar si la pantalla esta llena
uint8_t contador10Segundos = 0;	//Contador que cuenta 10 segundos
uint8_t contadorMovimiento = 0;	//Contador que ayuda para el movimiento cada ciertas transiciones del ADC
uint8_t contadorNewChange = 0;	//Cotador que cuenta para permitir un nuevo cambio de direccion

// Variables: Configuración ADC
uint32_t periodADC = 25000;		//Periodo "Facil" para el adc el cual afecta directamente la velocidad de la snake
uint16_t duttyValue = 10;		//Duttycycle lo suficientemente bajo para no verse afectado por el ADC

/************************* Configuracion del MenuSnake *************************/

// Seleccion de modo
bool gameMode = 0;	//Si se esta en gameMode o se encuentra en el Menu
uint8_t selected = 1;	//Cual nivel se selecciono

// Parametros de menú
uint16_t menucolor = ILI9341_PURPLE;		//Color de las letras menu
uint16_t menucolorSelect = ILI9341_WHITE; //Color de las letras seleccionadas en el menu

// Opciones de menú textos
char *menu1 = "Easy";
char *menu2 = "Normal";
char *menu3 = "Hard";

// Posiciones de menú posiciones iniciales menu
uint16_t menux = 30;
uint16_t menuy = 30;

// Refresco de menú (Estado de LED)
uint8_t flagRefrescoMenu = 0;

// Inicialización de nivel
uint8_t level = 0;

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

// Cabeceras de funciones
bool checkCollisionFood(uint16_t x, uint16_t y); //Funciones que verifica la cualicion
void drawInitialSnake(void);	//Funcion que se encarga de dibujar el estado inicial de la snake
void advanceSnake(void);		//Funcion que se encarga de avanzar la snake
void changeDirection(void);		//Se ambia la direccion del movimiento
void createFood(void);			//Se crea la comida si no esta creada
void randomFood(void);			//Randomiza en valor de la coida
void checkCollision(void);		//Verifica si hay colisiones
void resetEasyMode(void);		//Se inicia el modo facil
void initSystem(void);			//Funcion de inicio
void USART2Rx_Callback(void);	//Funcion que se encarga del manejo del usart2
void increseSpeed(void);		//Se incrementa la velocidad
void updateMenu(void);			//Para cambiar los valores del menu
void resetNormalMode(void);//Se inicia el modo Normal
void resetHardMode(void);//Se inicia el modo Hard

int main(void) {

	// Iniciamos el programa
	initSystem();

	// Enviamos test de USART
	writeString(&HandlerTerminal, string);

	// Definimos rotación & Fondo de pantalla
	Ili_setRotation(&ili, 3);
	ILI9341_FillRectangle(&ili, 0, 0, ILI9341_TFTHEIGHT - 1,
						ILI9341_TFTWIDTH - 1, backgroundColor);

	while (1) {
		// Selección de modo de Juego
		if (gameMode) {

			// Incrementos de velocidad por puntaje
			if ((tempScore + 1) % 2 == 0) {
				tempScore = 0;
				increseSpeed();
			}

			// Regeneración de Comida
			if (banderaComida == false && flagDeath == 0) {
				randomFood();
				createFood();
			}

			// Avance de Snake
			if (contadorMovimiento > 9 && flagDeath == 0) {
				contadorMovimiento = 0;
				advanceSnake();
			}

			// Reset de parametros de control de movimiento
			if (contadorNewChange > 9) {
				enableChange = 1;
				contadorNewChange = 0;
				newChangeRecently = 0;

			}

			// Cambio de dirección
			if (newDirVal != 0 && flagDeath == 0) {
				changeDirection();
			}

			//Volvemos a colocar la pantalla lista para la recepción de la siguiente imagen
			if (contadorPan == 86400) {

				//Reiniciamos el contador
				contadorPan = 0;

				// cerramos la comunicación de la anterior imange
				Ili_endCom(&ili);

				// Reiniciamos variable de muerte
				flagDeath = 0;

				// Escritura de puntaje
				sprintf(bufferData, "Score %d", score);

				// Inicio de posición
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
			//Si estamos en el menu verificamos si es hora de refrescar

			if (flagRefrescoMenu > 1) {
				//Se pregunta por la varibable nueva direccion para ver donde se mueve
				//Se selecciona el juego con la derecha
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
				//Se reinicia la variable de conteo de meni
				flagRefrescoMenu = 0;
				//Se actualiza los valores del menu
				updateMenu();
			}
			//Al darle a la derecha el level es diferente de cero se selecciona el nivel y se inicia el juego
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

	//Configuración Timer 5 el cual se encarga tanto de lanzar el ADC como de mover la snake .

	handlerTim5.ptrTIMx = TIM5; //El timer que se va a usar
	handlerTim5.TIMx_Config.TIMx_interruptEnable = 1; //Se habilitan las interrupciones
	handlerTim5.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; //Se usara en modo ascendente

	BasicTimer_Config(&handlerTim5); //Se carga la configuración.

	//Se configura el PWM que lanza el ADC
	pwmadc.ptrTIMx = TIM5;
	pwmadc.config.channel = PWM_CHANNEL_1;
	pwmadc.config.duttyCicle = duttyValue;
	pwmadc.config.periodo = periodADC;
	pwmadc.config.prescaler = 80;

	pwm_Config(&pwmadc);
	enableOutput(&pwmadc);
	startPwmSignal(&pwmadc);


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

	handlerDown.pGPIOx = GPIOC; //Se encuentra en el el GPIOC
	handlerDown.GPIO_PinConfig_t.GPIO_PinNumber = PIN_2; //Es el pin numero 2
	handlerDown.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_IN; //En este caso el pin se usara como entrada de datos
	handlerDown.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLUP; //No se usara ninguna resistencia pull-up o pull-down

	handlerExtiDown.pGPIOHandler = &handlerDown; // Se carga la config del pin al handler del EXTI
	handlerExtiDown.edgeType = EXTERNAL_INTERRUPT_FALLING_EDGE; //Se define que se van a detectar flancos de bajada.

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
		writeChar(&HandlerTerminal, 1);
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

//Funcion que se encarga de administrar el ADC
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

	// Definir las cordenadas de los rectangilos
	tailPositionX = snakePositions[0][0], tailPositionY = snakePositions[1][0], headPositionX =
			snakePositions[0][snakeSize - 1], headPositionY =
			snakePositions[1][snakeSize - 1];

	// Se rellena el rectangulo donde se encuentra la snake
	ILI9341_FillRectangle(&ili, tailPositionX, tailPositionY,
			headPositionX + squareSize, headPositionY + squareSize, snakeColor);
	ILI9341_FillRectangle(&ili, headPositionX, headPositionY,
			headPositionX + squareSize, headPositionY + squareSize, snakeColor);

}

void advanceSnake(void) {
	//Funcion que se encarga de cambiar la direccion si se tiene habilitado el enableChandeDir se cambia la direccion
	//Sino solo se avanza todas las funciones de movimiento se paracen
	//Todos las direccines son parecidas primero se verifica si se sigue normalmente, se traspasa o se choca
	//Y asi se calcula donde se dibuja el siguiente rectangulo
	if (enableChangeDir == 1) {
		if (direction == RIGHT) {
			//Se hacen consideracciones de los nniveles ya que en unos se traspasa y en otros se choca contra la parde
			if (level == 1) {
				//Estos If nos permiten determinar traspasar
				if (snakePositions[0][snakeSize - 1] + squareSize < 319) {
					headPositionX = snakePositions[0][snakeSize - 1] + 16;
				} else {
					headPositionX = 0;
				}
				headPositionY = snakePositions[1][snakeSize - 1];
			} else {
				//Estos If nos permiten
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

	//Se verifica si se tiene la bandera de que se choco la snaje

	if (flagDeath) {
		direction = PAUSE;
		enableChangeDir = 0;
		//Iniciamos la comuniación para la recepcion de la primera imagen
		Ili_starCom(&ili);
		//Definimos toda la pantalla para rellenarla por completo.
		ILI9341_SetAddressWindow(&ili, 40, 30, 240 - 1 + 40, 180 - 1 + 30);
	}

	//Si se tiene diferente a pause se dibuja la siguiente posicion se suma
	//el score si se come la comida y el score temproral que lo que nos ayduda es a cambiar la velocidad
	//cada cierto score
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

//Variables encargadas de cambiar la dirrecion
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
	//Se crea la comida si no se encuentra

	ILI9341_FillRectangle(&ili, foodPositionX, foodPositionY,
			foodPositionX + squareSize, foodPositionY + squareSize, foodColor);

	banderaComida = true;
}
//Se verifica si hay colision con la comida
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
//Se verifica la colicion de muerte
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
//Se genera el random para la comida
void randomFood(void) {
	uint8_t tempx = rand() % 20, tempy = rand() % 15;
	if (checkCollisionFood(tempx * 16, tempy * 16) == false) {
		foodPositionX = tempx * 16;
		foodPositionY = tempy * 16;
	} else {
		randomFood();
	}
}
//Se incrementa la velocidad
void increseSpeed(void) {
	float aux = (periodADC * 0.95f);
	periodADC = (int) (aux);
	if (periodADC >= 20) {
		updateFrequency(&pwmadc, periodADC);
	}
}
//Se resetea el modo facil
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
//Se resetea el modo normal
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
//Se resetea el modo dificil
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
//Se dibuja el meni
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

