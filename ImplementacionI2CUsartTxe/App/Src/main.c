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
#include "I2CxDriver.h"

bool banderaLedUsuario = 0; //Bandera TIM2 el cual nos da el blinking del LD2

uint8_t usart2DataReceived = 0; //Dato recibido por el usart dos
bool flagDataSend = 0; //Dato recibido por el usart dos

char string[] = "Hola "; //String de prueba para el ussart
char bufferMsg[64] = {0}; //Para guardar l configuracion a mandar por el usar
uint8_t i2cBuffer = {0};
//Variables para demostrar el funcionamiento del modulo de procesamiento matematico.
float valueA = 123.4567f;
float valueB = 987.7654f;
float valueC = 0.0;

//Funciones utilizadas para el DSP
float32_t sineValue = 0.0;
float32_t sineArgValue = 0.0;

float32_t srcNumber[4] = {-0.987, 32.26, -45.21, -987.321};
float32_t destNumber[4] = {0};
uint32_t dataSize = 4;

//Handlers utilizados para manipular los perifericos
BasicTimer_Handler_t handlerTim2 = {0}; //Timer para el blinking
USART_Handler_t USART2Handler = {0}; //Para manejar el USART2
GPIO_Handler_t handlerUserLedPin = {0}; // Para manipular el led de usuario
GPIO_Handler_t tx2pin = {0}; //Para manipular el pin de trasminsión de datos del USART
GPIO_Handler_t rx2pin = {0}; //Para manipular el pin de recepcioón de datos del USART
PWM_Handler_t pwmtim3 = {0}; //Para configurar el PWM enn el timer 3
GPIO_Handler_t pwm3pin = {0}; //Pin que nos entrega la señal PWM
GPIO_Handler_t CSPrueba = {0}; // CS para la pantalla (chip select)
GPIO_Handler_t RSTPrueba = {0}; // RST para la pantalla (Reset)
GPIO_Handler_t DCPrueba = {0}; //DC para la pantalla (Data or command)
SPI_Handler_t SPIPrueba = {0}; //Handler para usar el SPI 2.
GPIO_Handler_t MOSIPrueba = {0}; //MOSI para la pantalla (Master Output Slave Input)
GPIO_Handler_t SCKPrueba = {0}; //SCK para la pantalla (Clock del SPI)
ILI9341_Handler_t ili = {0}; //Handler para manejar la pantalla

PWM_Handler_t pwmtim4 = {0}; //Para configurar el PWM enn el timer 3
GPIO_Handler_t pwm4pin = {0}; //Pin que nos entrega la señal PWM

PWM_Handler_t pwmtim5 = {0}; //Para configurar el PWM enn el timer 3
GPIO_Handler_t pwm5pin = {0}; //Pin que nos entrega la señal PWM

uint16_t duttyValue1 = 2000; //Valor del dutty inicial
uint16_t duttyValue2 = 5000; //Valor del dutty inicial
uint16_t duttyValue3 = 10000; //Valor del dutty inicial

I2C_Handler_t i2cprueba = {0};

GPIO_Handler_t I2cSDA = {0};
GPIO_Handler_t I2cSCL = {0};


long contadorPan = 0; // COntador para saber cuando la pantalla se lleno

void initSystem(void); // Función quue inicializa el sistema
void USART2Rx_Callback(void); //Definir el callback

#define ACCEL_ADDRESS 0x1D; //0xD -> Rireccion del Accel con Logic_1
#define ACCEL_XOUT_H 50
#define ACCEL_XOUT_L 51
#define ACCEL_YOUT_H 52
#define ACCEL_YOUT_L 53
#define ACCEL_ZOUT_H 54
#define ACCEL_ZOUT_L 55

//#define PWR_MGMT_1 0
#define WHO_AM_I 0



int main(void){
	//Activacion cooprocesador matematico
	SCB->CPACR |= (0xF <<20);

	//Iniciamos sistma
	initSystem();


	while(1){
		if(banderaLedUsuario == 1){
			banderaLedUsuario = 0;
			GPIOxTooglePin(&handlerUserLedPin);
		}

		if(usart2DataReceived == 'w' || usart2DataReceived == 'W'){
			sprintf(bufferMsg, "Who_am_I? (r)\n");
			writeString(&USART2Handler, bufferMsg);

			i2cBuffer = i2c_readSingeRegister(&i2cprueba, WHO_AM_I);
			sprintf(bufferMsg, "dataRead = 0x%x \n", (unsigned int) i2cBuffer);
			writeString(&USART2Handler, bufferMsg);
			usart2DataReceived = 0;
		}
		else if(usart2DataReceived == 'x' || usart2DataReceived == 'X'){
			sprintf(bufferMsg, "Axis X (r)\n");
			writeString(&USART2Handler, bufferMsg);

			i2cBuffer = i2c_readSingeRegister(&i2cprueba, ACCEL_XOUT_H);
			sprintf(bufferMsg, "X = 0x%x \n", (unsigned int) i2cBuffer);
			writeString(&USART2Handler, bufferMsg);
			usart2DataReceived = 0;
		}


	}

	return 0;
}

void initSystem(void){

	configPLL(80);

//	Inicializamos el SysTick (importante para poder utilizar la pantalla)
	config_SysTick_ms(getFreqPLL());

	//Led de usuario usado para el blinking

	handlerUserLedPin.pGPIOx = GPIOA; //Se encuentra en el el GPIOA
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_5; // Es el pin 5.
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_OUT; //Se utiliza como salida
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL; //Salida PushPull
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING; //No se usa ninguna resistencia
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_FAST; //Se usa en velocidad rapida

	GPIO_Config(&handlerUserLedPin); //Se carga la configuración para el Led.

	USART2Handler.ptrUSARTx = USART2;
	USART2Handler.USART_Config.USART_baudrate = USART_BAUDRATE_9600;
	USART2Handler.USART_Config.USART_datasize = USART_DATASIZE_8BIT;
	USART2Handler.USART_Config.USART_mode = USART_MODE_RXTX;
	USART2Handler.USART_Config.USART_parity = USART_PARITY_NONE;
	USART2Handler.USART_Config.USART_stopbits =  USART_STOPBIT_1;
	USART2Handler.USART_Config.USART_RX_Int_Ena = ENABLE;
//	USART2Handler.USART_Config.USART_TX_Int_Ena = ENABLE;

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

	pwm3pin.pGPIOx 								  = GPIOC;
	pwm3pin.GPIO_PinConfig_t.GPIO_PinNumber        = PIN_7;
	pwm3pin.GPIO_PinConfig_t.GPIO_PinMode          = GPIO_MODE_ALTFN;
	pwm3pin.GPIO_PinConfig_t.GPIO_PinOPType		  = GPIO_OTYPE_PUSHPULL;
	pwm3pin.GPIO_PinConfig_t.GPIO_PinPuPdControl   = GPIO_PUPDR_NOTHING;
	pwm3pin.GPIO_PinConfig_t.GPIO_PinSpeed         = GPIO_OSPEED_FAST;
	pwm3pin.GPIO_PinConfig_t.GPIO_PinAltFunMode    = AF2;

	GPIO_Config(&pwm3pin);


	pwmtim3.ptrTIMx = TIM3;
	pwmtim3.config.channel = PWM_CHANNEL_2;
	pwmtim3.config.duttyCicle = duttyValue1;
	pwmtim3.config.periodo = 20000;
	pwmtim3.config.prescaler = 80;

	pwm_Config(&pwmtim3);

	enableOutput(&pwmtim3);
	startPwmSignal(&pwmtim3);


	pwm4pin.pGPIOx 								  = GPIOB;
	pwm4pin.GPIO_PinConfig_t.GPIO_PinNumber        = PIN_6;
	pwm4pin.GPIO_PinConfig_t.GPIO_PinMode          = GPIO_MODE_ALTFN;
	pwm4pin.GPIO_PinConfig_t.GPIO_PinOPType		  = GPIO_OTYPE_PUSHPULL;
	pwm4pin.GPIO_PinConfig_t.GPIO_PinPuPdControl   = GPIO_PUPDR_NOTHING;
	pwm4pin.GPIO_PinConfig_t.GPIO_PinSpeed         = GPIO_OSPEED_FAST;
	pwm4pin.GPIO_PinConfig_t.GPIO_PinAltFunMode    = AF2;

	GPIO_Config(&pwm4pin);


	pwmtim4.ptrTIMx = TIM4;
	pwmtim4.config.channel = PWM_CHANNEL_1;
	pwmtim4.config.duttyCicle = duttyValue2;
	pwmtim4.config.periodo = 20000;
	pwmtim4.config.prescaler = 80;

	pwm_Config(&pwmtim4);

	enableOutput(&pwmtim4);
	startPwmSignal(&pwmtim4);

	pwm5pin.pGPIOx 								  = GPIOA;
	pwm5pin.GPIO_PinConfig_t.GPIO_PinNumber        = PIN_0;
	pwm5pin.GPIO_PinConfig_t.GPIO_PinMode          = GPIO_MODE_ALTFN;
	pwm5pin.GPIO_PinConfig_t.GPIO_PinOPType		  = GPIO_OTYPE_PUSHPULL;
	pwm5pin.GPIO_PinConfig_t.GPIO_PinPuPdControl   = GPIO_PUPDR_NOTHING;
	pwm5pin.GPIO_PinConfig_t.GPIO_PinSpeed         = GPIO_OSPEED_FAST;
	pwm5pin.GPIO_PinConfig_t.GPIO_PinAltFunMode    = AF2;

	GPIO_Config(&pwm5pin);


	pwmtim5.ptrTIMx = TIM5;
	pwmtim5.config.channel = PWM_CHANNEL_1;
	pwmtim5.config.duttyCicle = duttyValue3;
	pwmtim5.config.periodo = 20000;
	pwmtim5.config.prescaler = 80;

	pwm_Config(&pwmtim5);

	enableOutput(&pwmtim5);
	startPwmSignal(&pwmtim5);

	writeString(&USART2Handler,string);
	writeStringInt(&USART2Handler,string);

	I2cSCL.pGPIOx 								  = GPIOB;
	I2cSCL.GPIO_PinConfig_t.GPIO_PinNumber		  = PIN_8;
	I2cSCL.GPIO_PinConfig_t.GPIO_PinMode          = GPIO_MODE_ALTFN;
	I2cSCL.GPIO_PinConfig_t.GPIO_PinOPType		  = GPIO_OTYPE_OPENDRAIN;
	I2cSCL.GPIO_PinConfig_t.GPIO_PinPuPdControl   = GPIO_PUPDR_PULLUP;
	I2cSCL.GPIO_PinConfig_t.GPIO_PinSpeed         = GPIO_OSPEED_HIGH;
	I2cSCL.GPIO_PinConfig_t.GPIO_PinAltFunMode    = AF4;
	GPIO_Config(&I2cSCL);

	I2cSDA.pGPIOx 								  = GPIOB;
	I2cSDA.GPIO_PinConfig_t.GPIO_PinNumber		  = PIN_9;
	I2cSDA.GPIO_PinConfig_t.GPIO_PinMode          = GPIO_MODE_ALTFN;
	I2cSDA.GPIO_PinConfig_t.GPIO_PinOPType		  = GPIO_OTYPE_OPENDRAIN;
	I2cSDA.GPIO_PinConfig_t.GPIO_PinPuPdControl   = GPIO_PUPDR_PULLUP;
	I2cSDA.GPIO_PinConfig_t.GPIO_PinSpeed         = GPIO_OSPEED_HIGH;
	I2cSDA.GPIO_PinConfig_t.GPIO_PinAltFunMode    = AF4;
	GPIO_Config(&I2cSDA);

	i2cprueba.modeI2C = I2C_MODE_SM;
	i2cprueba.slaveAddress = ACCEL_ADDRESS;
	i2cprueba.ptrI2Cx = I2C1;
	i2c_config(&i2cprueba);


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



}

//Calback del timer2 para el blinking
void BasicTimer2_Callback(void){
	banderaLedUsuario = 1;
}
//Recibimos los datos de la imagen y los mandamos a la pantalla, aumentamos el contador para poder
//reiniciarla una vez se llene la imagen.
void USART2Rx_Callback(void){
	usart2DataReceived = getRxData();
}

