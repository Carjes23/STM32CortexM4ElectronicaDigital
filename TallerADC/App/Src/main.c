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
#include "PLLDriver.h"

//Generación Handlers
//GPIO

GPIO_Handler_t handlerUserLedPin = { 0 };
GPIO_Handler_t rx2pin = { 0 };
GPIO_Handler_t tx2pin = { 0 };
GPIO_Handler_t pwprueba = { 0 };

//Timers

BasicTimer_Handler_t handlerTimer2 = { 0 };
BasicTimer_Handler_t handlerTimer5 = { 0 };

USART_Handler_t handlerTerminal = { 0 };

uint8_t rxData = 0;

void initSystem(void);
void VerificarUsartOcupado(void);

ADC_Config_t channnel_0 = { 0 };

uint8_t ADCISCOMPLETE = 0;
uint16_t adcData[2][256] = { 0 };

bool banderaLedUsuario = 0;
bool banderaADC = 0;
bool adcIsComplete = 0;
bool banderaImprimir = 0;
bool imprimirready = 1;
uint8_t adcIsCompleteCount = 0;

uint32_t count256 = 0;

PWM_Handler_t pwmadc = { 0 }; //Para configurar el PWM en el timer 3 para X

char bufferMsg[128] = { 0 };

int main(void) {
	initSystem();
	writeString(&handlerTerminal, "Hola estoy funcionando \n");

	while (1) {
		if (banderaLedUsuario == 1) {
			GPIOxTooglePin(&handlerUserLedPin);
			banderaLedUsuario = 0;
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
				sprintf(bufferMsg, "%u\t%u \n", adcData[0][i], adcData[1][i]);
				writeString(&handlerTerminal, bufferMsg);
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
}
//Calback del timer2 para el blinking
void BasicTimer2_Callback(void) {
	banderaLedUsuario = 1;
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

void VerificarUsartOcupado(void) {
	bool flagNewData = getFlagNewData();
	while (flagNewData != 0) { //No deja ingresar nuevos datos hasta que se envien por completo
		__NOP();
		flagNewData = getFlagNewData();
	}
}
