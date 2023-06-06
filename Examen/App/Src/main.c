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

#include "GPIOxDriver.h"
#include "USARTxDriver.h"
#include "BasicTimer.h"
#include "PwmDriver.h"
#include "AdcDriver.h"
#include <stdbool.h>
#include <string.h>
#include "PLLDriver.h"

//Generación Handlers
//GPIO

GPIO_Handler_t handlerUserLedPin = { 0 };
GPIO_Handler_t rx2pin = { 0 };
GPIO_Handler_t tx2pin = { 0 };
GPIO_Handler_t pwprueba = { 0 };
GPIO_Handler_t handlerPinMCO = { 0 };

//Timers

char bufferReception[64] = { 0 };
char cmd[64] = { 0 };
unsigned int firstParameter = 0;
unsigned int secondParameter = 0;
char userMsg[64] = { 0 };

BasicTimer_Handler_t handlerTimer2 = { 0 };
BasicTimer_Handler_t handlerTimer5 = { 0 };

USART_Handler_t handlerTerminal = { 0 };

uint8_t rxData = 0;

void initSystem(void);
void VerificarUsartOcupado(void);
void parseCommands(char *ptrBufferReception);

ADC_Config_t channnel_0 = { 0 };

uint8_t ADCISCOMPLETE = 0;
uint16_t adcData[2][256] = { 0 };
uint8_t trimValue = 16;

bool banderaADC = 0;
bool adcIsComplete = 0;
bool banderaImprimir = 0;
bool imprimirready = 1;
bool stringComplete = 0;
uint8_t adcIsCompleteCount = 0;
uint16_t counterReception = 0;
bool pllTrue = 0;

uint32_t count256 = 0;

PWM_Handler_t pwmadc = { 0 }; //Para configurar el PWM en el timer 3 para X

char bufferData[128] = { 0 };

int main(void) {
	initSystem();
	writeString(&handlerTerminal, "Hola estoy funcionando \n");

	while (1) {

		// El caracter '@' nos indica que es el final de la cadena
		if (rxData != '\0') {
			bufferReception[counterReception] = rxData;
			counterReception++;

			// If the incoming character is a newline, set a flag
			// so the main loop can do something about it
			if (rxData == '@') {
				stringComplete = 1;

				//Agrego esta linea para crear el string con el null al final
				bufferReception[counterReception] = '\0';
				counterReception = 0;
			}
			//Para que no vuelva entrar. Solo cambia debido a la interrupcion
			rxData = '\0';
		}

		//Hacemos un analisis de la cadena de datos obtenida
		if (stringComplete) {
			parseCommands(bufferReception);
			stringComplete = 0;
		}



//		if(adcIsComplete == 1){
//			adcIsComplete=0;
//
//			sprintf(bufferMsg, "%u\t%u \n", adcData1[0], adcData2[0]);
//			writeString(&handlerTerminal, bufferMsg);
//
//		}
		if (banderaImprimir && imprimirready) {
			for (int i = 0; i < 256; i++) {
				sprintf(bufferData, "%u\t%u \n", adcData[0][i], adcData[1][i]);
				writeString(&handlerTerminal, bufferData);
			}
			banderaImprimir = 0;
			imprimirready = 0;
		}
	}
	return 0;
}

void initSystem(void) {

	//Activacion cooprocesador matematico(importante para esta tarea)
	SCB->CPACR |= (0xF << 20);

	//Configuramos el PLL a 80 MHz

	configPLL(100);

	handlerUserLedPin.pGPIOx = GPIOA; //Se encuentra en el el GPIOA
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinNumber = PIN_5; // Es el pin 5.
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_OUT; //Se utiliza como salida
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL; //Salida PushPull
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING; //No se usa ninguna resistencia
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_FAST; //Se usa en velocidad rapida

	GPIO_Config(&handlerUserLedPin); //Se carga la configuración para el Led.

	//PA2
	tx2pin.pGPIOx = GPIOA;
	tx2pin.GPIO_PinConfig_t.GPIO_PinNumber = PIN_2;
	tx2pin.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	tx2pin.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLUP;
	tx2pin.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_FAST; //Se usa en velocidad rapida
	tx2pin.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF7;

	GPIO_Config(&tx2pin);

	rx2pin.pGPIOx = GPIOA;
	rx2pin.GPIO_PinConfig_t.GPIO_PinNumber = PIN_3;
	rx2pin.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	rx2pin.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLUP;
	rx2pin.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_FAST;
	rx2pin.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF7;

	GPIO_Config(&rx2pin);

	handlerTimer2.ptrTIMx = TIM2; //El timer que se va a usar
	handlerTimer2.TIMx_Config.TIMx_interruptEnable = 1; //Se habilitan las interrupciones
	handlerTimer2.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; //Se usara en modo ascendente
	handlerTimer2.TIMx_Config.TIMx_period = 2500; //Se define el periodo en este caso el led cambiara cada 250ms
	handlerTimer2.TIMx_Config.TIMx_speed = BTIMER_SPEED_100us; //Se define la "velocidad" que se usara

	BasicTimer_Config(&handlerTimer2); //Se carga la configuración.

	handlerTerminal.ptrUSARTx = USART2;
	handlerTerminal.USART_Config.USART_baudrate = USART_BAUDRATE_115200;
	handlerTerminal.USART_Config.USART_datasize = USART_DATASIZE_8BIT;
	handlerTerminal.USART_Config.USART_mode = USART_MODE_RXTX;
	handlerTerminal.USART_Config.USART_parity = USART_PARITY_NONE;
	handlerTerminal.USART_Config.USART_stopbits = USART_STOPBIT_1;
	handlerTerminal.USART_Config.USART_RX_Int_Ena = ENABLE;

	USART_Config(&handlerTerminal);
	//Configuración Timer 2 que controlara el blinking

	pwmadc.ptrTIMx = TIM5;
	pwmadc.config.channel = PWM_CHANNEL_1;
	pwmadc.config.duttyCicle = 100;
	pwmadc.config.periodo = 500;
	pwmadc.config.prescaler = 10;

	pwm_Config(&pwmadc);
	enableOutput(&pwmadc);
	startPwmSignal(&pwmadc);

	pwprueba.pGPIOx = GPIOA;
	pwprueba.GPIO_PinConfig_t.GPIO_PinNumber = PIN_0;
	pwprueba.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	pwprueba.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	pwprueba.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	pwprueba.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_FAST;
	pwprueba.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF2;

	GPIO_Config(&pwprueba);

	uint8_t channels[2] = { ADC_CHANNEL_1, ADC_CHANNEL_4 };
	channnel_0.channels = channels;
	channnel_0.dataAlignment = ADC_ALIGNMENT_RIGHT;
	channnel_0.numberOfChannels = 2;
	uint8_t samplingPeriod[2] = { 0 };
	samplingPeriod[0] = ADC_SAMPLING_PERIOD_480_CYCLES
	;
	samplingPeriod[1] = ADC_SAMPLING_PERIOD_480_CYCLES
	;
	channnel_0.samplingPeriod = samplingPeriod;
	channnel_0.resolution = ADC_RESOLUTION_12_BIT;
	channnel_0.externType = EXTEN_RISING_TIMER5_CC1;

	adc_Config(&channnel_0);

	/*Configuracion del Pin para ver la velocidad */
	handlerPinMCO.pGPIOx = GPIOA;
	handlerPinMCO.GPIO_PinConfig_t.GPIO_PinNumber = PIN_8;
	handlerPinMCO.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	handlerPinMCO.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	handlerPinMCO.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_FAST;
	handlerPinMCO.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	handlerPinMCO.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF0;

	/* Cargamos la configuracion del Pin en los registros*/
	GPIO_Config(&handlerPinMCO);

	configChannelMCO1(MCO1_HSI_CHANNEL);
	configPresMCO1(0);

}
//Calback del timer2 para el blinking
void BasicTimer2_Callback(void) {
	GPIOxTooglePin(&handlerUserLedPin);
}

void USART2Rx_Callback(void) {
	rxData = getRxData();
}

void adcComplete_Callback(void) {
	adcData[adcIsCompleteCount][count256] = getADC();
	adcIsCompleteCount++;
	if (adcIsCompleteCount > 1) {
		adcIsCompleteCount = 0;
		adcIsComplete = 1;
		if (banderaImprimir == 0) {
			count256++;
		}
	}

	if (count256 > 255) {
		count256 = 0;
		banderaImprimir = 1;
	}
}

void parseCommands(char *ptrBufferReception) {

	/* Lee la cadena de caracteres a la que apunta el "ptrBufferReception
	 * y almacena en tres elementos diferentes: un string llamado "cmd",
	 * y dos integer llamados "firstParameter" y "secondParameter"
	 * De esta forma podemos introducir informacion al micro desde el puerto
	 */
	sscanf(ptrBufferReception, "%s %u %u %s", cmd, &firstParameter,
			&secondParameter, userMsg);
	if (strcmp(cmd, "help") == 0) {
		writeString(&handlerTerminal, "Help Menu CMDs: \n");
		writeString(&handlerTerminal, "1)  Help -> Print this menu \n");
		writeString(&handlerTerminal, "Default MCO -> HSI div0\n");
		writeString(&handlerTerminal,
				"2)  MCO_prescaler -> #div(0,2,3,4,5); warning PLL > 2\n");
		writeString(&handlerTerminal,
				"3)  MCO_channel #(LSE=0, PLL=1, HSI=2); warning PLL default prescaler 2\n");
		writeString(&handlerTerminal, "4) Changetrim #(0-> disminuir, 1-> aumentar) \n");
		writeString(&handlerTerminal, "4)  RTC \n");
		writeString(&handlerTerminal, "5)  RTC \n");
		writeString(&handlerTerminal, "6)  RTC \n");
		writeString(&handlerTerminal, "7)  RTC \n");
		writeString(&handlerTerminal,
				"8)  Datos analogos: Velocidad de muestreo \n");
		writeString(&handlerTerminal,
				"9)  Datos analogos: Presentacion de los arreglos \n");
		writeString(&handlerTerminal, "10) Acelerometro: Captura de datos \n");
		writeString(&handlerTerminal, "11) Acelerometro: Frecuencias \n");

	}

	else if (strcmp(cmd, "MCO_prescaler") == 0) {
		if (firstParameter == 0 && pllTrue == 0) {
			writeString(&handlerTerminal,
					"CMD: MCO_prescaler sin division no permitido en PLL \n");
			configPresMCO1(0);
		} else if (firstParameter == 0 && pllTrue == 1) {
			writeString(&handlerTerminal,
					"CMD: MCO_prescaler sin division no permitido en PLL \n");
		} else if (firstParameter == 2) {
			writeString(&handlerTerminal, "CMD: MCO_prescaler divison 2 \n");
			configPresMCO1(MCO1_PRESCALER_DIV_2);
		} else if (firstParameter == 3) {
			writeString(&handlerTerminal, "CMD: MCO_prescaler divison 3 \n");
			configPresMCO1(MCO1_PRESCALER_DIV_3);
		} else if (firstParameter == 4) {
			writeString(&handlerTerminal, "CMD: MCO_prescaler divison 4 \n");
			configPresMCO1(MCO1_PRESCALER_DIV_4);
		} else if (firstParameter == 5) {
			writeString(&handlerTerminal, "CMD: MCO_prescaler divison 5 \n");
			configPresMCO1(MCO1_PRESCALER_DIV_5);
		} else {
			writeString(&handlerTerminal, "Wrong number \n");
		}
	}

	else if (strcmp(cmd, "MCO_channel") == 0) {

		if (firstParameter == 0) {
			pllTrue = 0;
			writeString(&handlerTerminal, "CMD: MCO_channel LSE \n");
			configChannelMCO1(MCO1_LSE_CHANNEL);
		} else if (firstParameter == 1) {
			pllTrue = 1;
			writeString(&handlerTerminal,
					"CMD: MCO_channel PLL se coloca como prescaler default 2 \n");
			configPresMCO1(MCO1_PRESCALER_DIV_2);
			configChannelMCO1(MCO1_PLL_CHANNEL);

		} else if (firstParameter == 2) {
			pllTrue = 0;
			writeString(&handlerTerminal, "CMD: MCO_channel HSI \n");
			configChannelMCO1(MCO1_HSI_CHANNEL);
		} else {
			pllTrue = 0;
			writeString(&handlerTerminal, "Wrong number \n");
		}
	}

	else if (strcmp(cmd, "Changetrim") == 0) {

		if (firstParameter == 0) {
			writeString(&handlerTerminal, "CMD: Disminuir trim\n");
			trimValue++;
			changeTrim(trimValue);
		} else if (firstParameter == 1) {
			writeString(&handlerTerminal, "CMD: Aumentar trim\n");
			trimValue--;
			changeTrim(trimValue);

		} else {
			pllTrue = 0;
			writeString(&handlerTerminal, "Wrong number \n");
		}
	}
	// El comando dummy sirve para entender como funciona la recepcion de numeros
	//enviados desde la consola
	else if (strcmp(cmd, "dummy") == 0) {
		writeString(&handlerTerminal, "CMD: dummy \n");
		// Cambiando el formato para presentar el numero por el puerto serial
		sprintf(bufferData, "number A = %u \n", firstParameter);
		writeString(&handlerTerminal, bufferData);
		sprintf(bufferData, "number B = %u \n", secondParameter);
		writeString(&handlerTerminal, bufferData);
	}
	// El comando usermsg sirve para entender como funciona la recepcion de strings
	//enviados desde la consola
	else if (strcmp(cmd, "usermsg") == 0) {
		writeString(&handlerTerminal, "CMD: usermsg \n");
		writeString(&handlerTerminal, userMsg);
		writeString(&handlerTerminal, "\n");
	}
//	else if (strcmp(cmd, "setPeriod") == 0) {
//		writeString(&handlerTerminal, "CMD: setPeriod \n");
//	}
	else {
		// Se imprime el mensaje "Wrong CMD" si la escritura no corresponde a los CMD implementados
		writeString(&handlerTerminal, "Wrong CMD \n");
	}
}
