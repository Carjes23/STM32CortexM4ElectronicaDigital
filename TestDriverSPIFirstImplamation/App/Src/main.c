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
 * Codigo que permite probar los drivers vistos en paro, y mandar imagenes a la pantalla
 * a traves del USART2
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
#include "PLLDriver.h"
#include "ExtiDriver.h"

bool banderaLedUsuario = 0; //Bandera TIM2 el cual nos da el blinking del LD2

uint8_t usart2DataReceived = 0; //Dato recibido por el usart dos

char string[] = "Hola "; //String de prueba para el ussart
char bufferMsg[64] = { 0 }; //Para guardar l configuracion a mandar por el usar

//Variables para demostrar el funcionamiento del modulo de procesamiento matematico.
float valueA = 123.4567f;
float valueB = 987.7654f;
float valueC = 0.0;

//Funciones utilizadas para el DSP
float32_t sineValue = 0.0;
float32_t sineArgValue = 0.0;

float32_t srcNumber[4] = { -0.987, 32.26, -45.21, -987.321 };
float32_t destNumber[4] = { 0 };
uint32_t dataSize = 4;

//Handlers utilizados para manipular los perifericos
BasicTimer_Handler_t handlerTim2 = { 0 }; //Timer para el blinking
USART_Handler_t USART2Handler = { 0 }; //Para manejar el USART2
GPIO_Handler_t handlerUserLedPin = { 0 }; // Para manipular el led de usuario
GPIO_Handler_t tx2pin = { 0 }; //Para manipular el pin de trasminsión de datos del USART
GPIO_Handler_t rx2pin = { 0 }; //Para manipular el pin de recepcioón de datos del USART
PWM_Handler_t pwmtim3 = { 0 }; //Para configurar el PWM enn el timer 3
GPIO_Handler_t pwmpin = { 0 }; //Pin que nos entrega la señal PWM
GPIO_Handler_t CSPrueba = { 0 }; // CS para la pantalla (chip select)
GPIO_Handler_t RSTPrueba = { 0 }; // RST para la pantalla (Reset)
GPIO_Handler_t DCPrueba = { 0 }; //DC para la pantalla (Data or command)
SPI_Handler_t SPIPrueba = { 0 }; //Handler para usar el SPI 2.
GPIO_Handler_t MOSIPrueba = { 0 }; //MOSI para la pantalla (Master Output Slave Input)
GPIO_Handler_t SCKPrueba = { 0 }; //SCK para la pantalla (Clock del SPI)
ILI9341_Handler_t ili = { 0 }; //Handler para manejar la pantalla

/*
 * EXTI
 */

GPIO_Handler_t handlerLeft = {0};
EXTI_Config_t handlerExtiLeft = {0};

GPIO_Handler_t handlerRight = {0};
EXTI_Config_t handlerExtiRight = {0};

GPIO_Handler_t handlerUp = {0};
EXTI_Config_t handlerExtiUp = {0};

GPIO_Handler_t handlerDown = {0};
EXTI_Config_t handlerExtiDown = {0};

uint8_t selected = 1;
uint16_t menucolor = ILI9341_PURPLE;
uint16_t menucolorSelect = ILI9341_WHITE;
char *menu1 = "Easy";
char *menu2 = "Normal";
char *menu3 = "Hard";
uint16_t menux = 30;
uint16_t menuy = 30;
uint16_t backgroundColor = 0x0000; // negro en 16-bit RGB565

uint16_t duttyValue = 2000; //Valor del dutty inicial
long contadorPan = 0; // COntador para saber cuando la pantalla se lleno

void initSystem(void); // Función quue inicializa el sistema
void USART2Rx_Callback(void); //Definir el callback
void updateMenu(void);

int main(void) {

	//Iniciamos sistma
	initSystem();
	//Colocamos la rotacion 3 para el llenado de datos.
	Ili_setRotation(&ili, 3);

	// Vamos a rellenar la pantalla de negro.

	// Rellenar la pantalla con el color de background
	ILI9341_FillRectangle(&ili, 0, 0, ILI9341_TFTHEIGHT - 1,
			ILI9341_TFTWIDTH - 1, backgroundColor);
//	// Rellenar la pantalla con el color de background
//	ILI9341_FillRectangle(&ili, 0, 0, 32,
//			32, ILI9341_RED);

//    //Iniciamos la comuniación para la recepcion de la primera imagen
//	Ili_starCom(&ili);
//	//Definimos toda la pantalla para rellenarla por completo.
//    ILI9341_SetAddressWindow(&ili, 40, 30, 240 - 1 + 40, 180 - 1 + 30);

	ILI9341_WriteString(&ili, menux, menuy, menucolorSelect, backgroundColor,
			menu1, 4, 4);
	ILI9341_WriteString(&ili, menux, menuy * 3, menucolor, backgroundColor,
			menu2, 4, 4);
	ILI9341_WriteString(&ili, menux, menuy * 5, menucolor, backgroundColor,
			menu3, 4, 4);

	while (1) {

		//Utilizamos el led de usuario cada 250 ms
		if (banderaLedUsuario == 1) {
			banderaLedUsuario = 0;
			GPIOxTooglePin(&handlerUserLedPin);
			updateMenu();
		}
		//Volvemos a colocar la pantalla lista para la recepción de la siguiente imagen
		if (contadorPan == 0x25800) {
			//Reiniciamos el contador
			contadorPan = 0;
			// cerramos la comunicación de la anterior imange
			Ili_endCom(&ili);
			//Abrimos la comunicación para la siguiente imagen
			Ili_starCom(&ili);
			//Definimos toda la pantalla
			ILI9341_SetAddressWindow(&ili, 0, 0, ILI9341_TFTHEIGHT - 1,
					ILI9341_TFTWIDTH - 1);

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

	USART2Handler.ptrUSARTx = USART2;
	USART2Handler.USART_Config.USART_baudrate = 921600;
	USART2Handler.USART_Config.USART_datasize = USART_DATASIZE_8BIT;
	USART2Handler.USART_Config.USART_mode = USART_MODE_RXTX;
	USART2Handler.USART_Config.USART_parity = USART_PARITY_NONE;
	USART2Handler.USART_Config.USART_stopbits = USART_STOPBIT_1;
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

	writeString(&USART2Handler, string);

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

	//Boton de usuario para el cambio de tasa de resfresco.

	handlerLeft.pGPIOx = GPIOC; //Se encuentra en el el GPIOC
	handlerLeft.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_0; //Es el pin numero 13
	handlerLeft.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_IN;//En este caso el pin se usara como entrada de datos
	handlerLeft.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_PULLUP; //No se usara ninguna resistencia pull-up o pull-down

	handlerExtiLeft.pGPIOHandler = &handlerLeft; // Se carga la config del pin al handler del EXTI
	handlerExtiLeft.edgeType = EXTERNAL_INTERRUPT_FALLING_EDGE; //Se define que se van a detectar flancos de bajada.

	extInt_Config(&handlerExtiLeft); //Se carga la configuracion usando la funcion de la libreria.

	//Boton de usuario para el cambio de tasa de resfresco.

	handlerRight.pGPIOx = GPIOC; //Se encuentra en el el GPIOC
	handlerRight.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_1; //Es el pin numero 13
	handlerRight.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_IN;//En este caso el pin se usara como entrada de datos
	handlerRight.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_PULLUP; //No se usara ninguna resistencia pull-up o pull-down

	handlerExtiRight.pGPIOHandler = &handlerRight; // Se carga la config del pin al handler del EXTI
	handlerExtiRight.edgeType = EXTERNAL_INTERRUPT_FALLING_EDGE; //Se define que se van a detectar flancos de bajada.

	extInt_Config(&handlerExtiRight); //Se carga la configuracion usando la funcion de la libreria.


	//Boton de usuario para el cambio de tasa de resfresco.

	handlerUp.pGPIOx = GPIOC; //Se encuentra en el el GPIOC
	handlerUp.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_3; //Es el pin numero 13
	handlerUp.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_IN;//En este caso el pin se usara como entrada de datos
	handlerUp.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_PULLUP; //No se usara ninguna resistencia pull-up o pull-down

	handlerExtiUp.pGPIOHandler = &handlerUp; // Se carga la config del pin al handler del EXTI
	handlerExtiUp.edgeType = EXTERNAL_INTERRUPT_FALLING_EDGE; //Se define que se van a detectar flancos de bajada.

	extInt_Config(&handlerExtiUp); //Se carga la configuracion usando la funcion de la libreria.


	//Boton de usuario para el cambio de tasa de resfresco.

	handlerDown.pGPIOx = GPIOC; //Se encuentra en el el GPIOC
	handlerDown.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_2; //Es el pin numero 13
	handlerDown.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_IN;//En este caso el pin se usara como entrada de datos
	handlerDown.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_PULLUP; //No se usara ninguna resistencia pull-up o pull-down

	handlerExtiDown.pGPIOHandler = &handlerDown; // Se carga la config del pin al handler del EXTI
	handlerExtiDown.edgeType = EXTERNAL_INTERRUPT_FALLING_EDGE; //Se define que se van a detectar flancos de bajada.

	extInt_Config(&handlerExtiDown); //Se carga la configuracion usando la funcion de la libreria.

	changeTrim(14);

}

//Calback del timer2 para el blinking
void BasicTimer2_Callback(void) {
	banderaLedUsuario = 1;
}
//Recibimos los datos de la imagen y los mandamos a la pantalla, aumentamos el contador para poder
//reiniciarla una vez se llene la imagen.
void USART2Rx_Callback(void) {
	usart2DataReceived = getRxData();
	ILI9341_WriteData(&ili, &usart2DataReceived, 1);
	contadorPan++;
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
	}
	else{
		ILI9341_WriteString(&ili, menux, menuy * 5, menucolor, backgroundColor,
				menu3, 4, 4);
		ILI9341_WriteString(&ili, menux, menuy * 3, menucolor, backgroundColor,
				menu2, 4, 4);
		ILI9341_WriteString(&ili, menux, menuy, menucolorSelect,
				backgroundColor, menu1, 4, 4);
	}
}

//Callback EXTI 2 para el boton del encodner
void callback_extInt0(void){
	__NOP();

}
//Callback EXTI 2 para el boton del encodner
void callback_extInt1(void){
	__NOP();
}
//Callback EXTI 2 para el boton del encodner
void callback_extInt2(void){
	if(selected != 1){
		selected--;
	}else{
		selected = 3;
	}
}
//Callback EXTI 2 para el boton del encodner
void callback_extInt3(void){
if(selected != 3){
		selected++;
	}else{
		selected = 1;
	}
}
