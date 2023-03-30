/*
 * mainExti.c
 *
 *  Created on: Mar 27, 2023
 *      Author: jutoroa
 */

#include <stdint.h>
#include "ExtiDriver.h"
#include "GPIOxDriver.h"
#include "BasicTimer.h"
#include <stdbool.h>

#define CONTADOR		0
#define INFINITO		1

//Led de usuario

GPIO_Handler_t handlerUserLedPin = {0};
BasicTimer_Handler_t handlerTim2 = {0};
BasicTimer_Handler_t handlerTim3 = {0};

//Configuracion necesaria tarea

GPIO_Handler_t handlerStateLed = {0};
GPIO_Handler_t handlerEncoderA = {0}; 	 //DT
GPIO_Handler_t handlerEncoderB = {0}; 	 //CLK
GPIO_Handler_t handlerBotEncoder = {0};	 //SW
GPIO_Handler_t handlerLedA = {0};	 //Led A
GPIO_Handler_t handlerLedB = {0}; //Led B
GPIO_Handler_t handlerLedF = {0}; //Led F
GPIO_Handler_t handlerLedC = {0}; //Led C
GPIO_Handler_t handlerLedE = {0}; //Led E
GPIO_Handler_t handlerLedD = {0}; //Led D
GPIO_Handler_t handlerLedG = {0}; //Led G

//Pines transistores.
GPIO_Handler_t handlerTransistor1 = {0}; //Transistor 1
GPIO_Handler_t handlerTransistor2= {0}; //Transistor 2


uint8_t unidades = 0;
uint8_t decenas = 0;
uint8_t contador = 0;

bool banderaContador = 0;
bool banderaBoton = 0;
bool banderaLedUsuario = 0;
bool banderaTimerLed = 0;
bool estado = 0;

//Leds valores.

bool ledA = 0;
bool ledB = 0;
bool ledC = 0;
bool ledD = 0;
bool ledE = 0;
bool ledF = 0;
bool ledG = 0;

EXTI_Config_t handlerExtiA = {0};
EXTI_Config_t handlerExtiBot = {0};

void initSystem(void);
void funcionContadora(void);
void funcionInfinito(void);
void funcionLeds(uint8_t numero);

void BasicTimer2_Callback(void);
void BasicTimer3_Callback(void);

int main(void){

	initSystem();

	while(1){

		if(banderaLedUsuario == 1){
			banderaLedUsuario = 0;
			GPIOxTooglePin(&handlerUserLedPin);
			unidades++;
//			GPIOxTooglePin(&handlerLedG); PRUEBA
		}

		if(estado == CONTADOR){
			funcionContadora();
		} else { funcionInfinito();}

		if(banderaBoton == 1){
			banderaBoton = 0;
			estado ^= 1;
		}

		if(banderaTimerLed==1){
			funcionLeds(unidades);
			GPIOxTooglePin(&handlerTransistor1);
			GPIOxTooglePin(&handlerTransistor2);
		} else{funcionLeds(decenas);}



	}

}

void initSystem(void){
	//Led de usuario

	handlerUserLedPin.pGPIOx = GPIOA; // Aquí se coloca a que GPIO pertenece el pin en este caso es el A
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_5; //El numero del pin en este caso es el
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_OUT; //En que modo se va a utilizar el pin es este caso lo usaremos como salida
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL; //Sí se configura out, en que modo se va a usar, en este caso se usara como push-pull
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING; //Si se le va agregár una resistencia Pull-Up o Pull-down, no  le pondremos ninguna
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_FAST; //A que velocidad se va utilizr la salida del
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinAltFunMode	= AF0; //Ninguna funcion alternativa
	//Carga de la configuración.
	//Se carga la configuración para el Led.
	GPIO_Config(&handlerUserLedPin);


	//Se configura el Handeler.

	handlerTim2.ptrTIMx = TIM2;
	handlerTim2.TIMx_Config.TIMx_interruptEnable = 1;
	handlerTim2.TIMx_Config.TIMx_mode = BTIMER_MODE_UP;
	handlerTim2.TIMx_Config.TIMx_period = 250;
	handlerTim2.TIMx_Config.TIMx_speed = BTIMER_SPEED_1ms;

	BasicTimer_Config(&handlerTim2);

	//Timer 3 transistores.

	handlerTim3.ptrTIMx = TIM2;
	handlerTim3.TIMx_Config.TIMx_interruptEnable = 1;
	handlerTim3.TIMx_Config.TIMx_mode = BTIMER_MODE_UP;
	handlerTim3.TIMx_Config.TIMx_period = 15;
	handlerTim3.TIMx_Config.TIMx_speed = BTIMER_SPEED_1ms;

	BasicTimer_Config(&handlerTim2);

	handlerStateLed.pGPIOx = GPIOA;
	handlerStateLed.GPIO_PinConfig_t.GPIO_PinNumber = PIN_5;
	handlerStateLed.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_OUT;
	handlerStateLed.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	handlerStateLed.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_HIGH;

	GPIO_Config(&handlerStateLed);

	handlerEncoderA.pGPIOx = GPIOA;
	handlerEncoderA.GPIO_PinConfig_t.GPIO_PinNumber = PIN_0;
	handlerEncoderA.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_IN;
	handlerEncoderA.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLDOWN;
	handlerEncoderA.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_HIGH;


	handlerExtiA.edgeType = EXTERNAL_INTERRUPT_RISING_EDGE;
	handlerExtiA.pGPIOHandler = &handlerEncoderA;

	extInt_Config(&handlerExtiA);

	handlerEncoderB.pGPIOx = GPIOH;
	handlerEncoderB.GPIO_PinConfig_t.GPIO_PinNumber = PIN_0;
	handlerEncoderB.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_IN;
	handlerEncoderB.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLDOWN;
	handlerEncoderB.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_HIGH;

	GPIO_Config(&handlerEncoderB);

	handlerBotEncoder.pGPIOx = GPIOH;
	handlerBotEncoder.GPIO_PinConfig_t.GPIO_PinNumber = PIN_1;
	handlerBotEncoder.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_IN;
	handlerBotEncoder.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLDOWN; //revisar
	handlerBotEncoder.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_HIGH;


	handlerExtiBot.edgeType = EXTERNAL_INTERRUPT_RISING_EDGE;
	handlerExtiBot.pGPIOHandler = &handlerEncoderA;

	extInt_Config(&handlerExtiBot);

	handlerLedA.pGPIOx = GPIOC;
	handlerLedA.GPIO_PinConfig_t.GPIO_PinNumber = 		PIN_3;
	handlerLedA.GPIO_PinConfig_t.GPIO_PinMode = 		GPIO_MODE_OUT;
	handlerLedA.GPIO_PinConfig_t.GPIO_PinOPType		= 	GPIO_OTYPE_PUSHPULL; //Sí se configura out, en que modo se va a usar, en este caso se usara como push-pull
	handlerLedA.GPIO_PinConfig_t.GPIO_PinPuPdControl= 	GPIO_PUPDR_NOTHING; //Si se le va agregár una resistencia Pull-Up o Pull-down, no  le pondremos ninguna
	handlerLedA.GPIO_PinConfig_t.GPIO_PinSpeed		= 	GPIO_OSPEED_FAST; //A que velocidad se va utilizr la salida del
	handlerLedA.GPIO_PinConfig_t.GPIO_PinAltFunMode	= 	AF0; //Ninguna funcion alternativa


	GPIO_Config(&handlerLedA);
//	GPIO_WritePin(&handlerLedA, 0); // Estado inicial


	handlerLedB.pGPIOx = GPIOC;
	handlerLedB.GPIO_PinConfig_t.GPIO_PinNumber = 		PIN_0;
	handlerLedB.GPIO_PinConfig_t.GPIO_PinMode = 		GPIO_MODE_OUT;
	handlerLedB.GPIO_PinConfig_t.GPIO_PinOPType		= 	GPIO_OTYPE_PUSHPULL; //Sí se configura out, en que modo se va a usar, en este caso se usara como push-pull
	handlerLedB.GPIO_PinConfig_t.GPIO_PinPuPdControl= 	GPIO_PUPDR_NOTHING; //Si se le va agregár una resistencia Pull-Up o Pull-down, no  le pondremos ninguna
	handlerLedB.GPIO_PinConfig_t.GPIO_PinSpeed		= 	GPIO_OSPEED_FAST; //A que velocidad se va utilizr la salida del
	handlerLedB.GPIO_PinConfig_t.GPIO_PinAltFunMode	= 	AF0; //Ninguna funcion alternativa


	GPIO_Config(&handlerLedB);
//	GPIO_WritePin(&handlerLedB, 0); // Estado inicial

	handlerLedF.pGPIOx = GPIOC;
	handlerLedF.GPIO_PinConfig_t.GPIO_PinNumber = 		PIN_2;
	handlerLedF.GPIO_PinConfig_t.GPIO_PinMode = 		GPIO_MODE_OUT;
	handlerLedF.GPIO_PinConfig_t.GPIO_PinOPType		= 	GPIO_OTYPE_PUSHPULL; //Sí se configura out, en que modo se va a usar, en este caso se usara como push-pull
	handlerLedF.GPIO_PinConfig_t.GPIO_PinPuPdControl= 	GPIO_PUPDR_NOTHING; //Si se le va agregár una resistencia Pull-Up o Pull-down, no  le pondremos ninguna
	handlerLedF.GPIO_PinConfig_t.GPIO_PinSpeed		= 	GPIO_OSPEED_FAST; //A que velocidad se va utilizr la salida del
	handlerLedF.GPIO_PinConfig_t.GPIO_PinAltFunMode	= 	AF0; //Ninguna funcion alternativa


	GPIO_Config(&handlerLedF);
//	GPIO_WritePin(&handlerLedF, 0); // Estado inicial

	handlerLedC.pGPIOx = GPIOC;
	handlerLedC.GPIO_PinConfig_t.GPIO_PinNumber = 		PIN_1;
	handlerLedC.GPIO_PinConfig_t.GPIO_PinMode = 		GPIO_MODE_OUT;
	handlerLedC.GPIO_PinConfig_t.GPIO_PinOPType		= 	GPIO_OTYPE_PUSHPULL; //Sí se configura out, en que modo se va a usar, en este caso se usara como push-pull
	handlerLedC.GPIO_PinConfig_t.GPIO_PinPuPdControl= 	GPIO_PUPDR_NOTHING; //Si se le va agregár una resistencia Pull-Up o Pull-down, no  le pondremos ninguna
	handlerLedC.GPIO_PinConfig_t.GPIO_PinSpeed		= 	GPIO_OSPEED_FAST; //A que velocidad se va utilizr la salida del
	handlerLedC.GPIO_PinConfig_t.GPIO_PinAltFunMode	= 	AF0; //Ninguna funcion alternativa


	GPIO_Config(&handlerLedC);
//	GPIO_WritePin(&handlerLedF, 0); // Estado inicial

	handlerLedE.pGPIOx = GPIOA;
	handlerLedE.GPIO_PinConfig_t.GPIO_PinNumber = 		PIN_10;
	handlerLedE.GPIO_PinConfig_t.GPIO_PinMode = 		GPIO_MODE_OUT;
	handlerLedE.GPIO_PinConfig_t.GPIO_PinOPType		= 	GPIO_OTYPE_PUSHPULL; //Sí se configura out, en que modo se va a usar, en este caso se usara como push-pull
	handlerLedE.GPIO_PinConfig_t.GPIO_PinPuPdControl= 	GPIO_PUPDR_NOTHING; //Si se le va agregár una resistencia Pull-Up o Pull-down, no  le pondremos ninguna
	handlerLedE.GPIO_PinConfig_t.GPIO_PinSpeed		= 	GPIO_OSPEED_FAST; //A que velocidad se va utilizr la salida del
	handlerLedE.GPIO_PinConfig_t.GPIO_PinAltFunMode	= 	AF0; //Ninguna funcion alternativa


	GPIO_Config(&handlerLedE);
//	GPIO_WritePin(&handlerLedE, 0); // Estado inicial

	handlerLedD.pGPIOx = GPIOC;
	handlerLedD.GPIO_PinConfig_t.GPIO_PinNumber = 		PIN_4;
	handlerLedD.GPIO_PinConfig_t.GPIO_PinMode = 		GPIO_MODE_OUT;
	handlerLedD.GPIO_PinConfig_t.GPIO_PinOPType		= 	GPIO_OTYPE_PUSHPULL; //Sí se configura out, en que modo se va a usar, en este caso se usara como push-pull
	handlerLedD.GPIO_PinConfig_t.GPIO_PinPuPdControl= 	GPIO_PUPDR_NOTHING; //Si se le va agregár una resistencia Pull-Up o Pull-down, no  le pondremos ninguna
	handlerLedD.GPIO_PinConfig_t.GPIO_PinSpeed		= 	GPIO_OSPEED_FAST; //A que velocidad se va utilizr la salida del
	handlerLedD.GPIO_PinConfig_t.GPIO_PinAltFunMode	= 	AF0; //Ninguna funcion alternativa


	GPIO_Config(&handlerLedD);
//	GPIO_WritePin(&handlerLedD, 0); // Estado inicial

	handlerLedG.pGPIOx = GPIOB;
	handlerLedG.GPIO_PinConfig_t.GPIO_PinNumber = 		PIN_3;
	handlerLedG.GPIO_PinConfig_t.GPIO_PinMode = 		GPIO_MODE_OUT;
	handlerLedG.GPIO_PinConfig_t.GPIO_PinOPType		= 	GPIO_OTYPE_PUSHPULL; //Sí se configura out, en que modo se va a usar, en este caso se usara como push-pull
	handlerLedG.GPIO_PinConfig_t.GPIO_PinPuPdControl= 	GPIO_PUPDR_NOTHING; //Si se le va agregár una resistencia Pull-Up o Pull-down, no  le pondremos ninguna
	handlerLedG.GPIO_PinConfig_t.GPIO_PinSpeed		= 	GPIO_OSPEED_FAST; //A que velocidad se va utilizr la salida del
	handlerLedG.GPIO_PinConfig_t.GPIO_PinAltFunMode	= 	AF0; //Ninguna funcion alternativa


	GPIO_Config(&handlerLedG);
//	GPIO_WritePin(&handlerLedE, 0); // Estado inicial

	handlerTransistor1.pGPIOx = GPIOB;
	handlerTransistor1.GPIO_PinConfig_t.GPIO_PinNumber = 		PIN_3;
	handlerTransistor1.GPIO_PinConfig_t.GPIO_PinMode = 		GPIO_MODE_OUT;
	handlerTransistor1.GPIO_PinConfig_t.GPIO_PinOPType		= 	GPIO_OTYPE_PUSHPULL; //Sí se configura out, en que modo se va a usar, en este caso se usara como push-pull
	handlerTransistor1.GPIO_PinConfig_t.GPIO_PinPuPdControl= 	GPIO_PUPDR_NOTHING; //Si se le va agregár una resistencia Pull-Up o Pull-down, no  le pondremos ninguna
	handlerTransistor1.GPIO_PinConfig_t.GPIO_PinSpeed		= 	GPIO_OSPEED_FAST; //A que velocidad se va utilizr la salida del
	handlerTransistor1.GPIO_PinConfig_t.GPIO_PinAltFunMode	= 	AF0; //Ninguna funcion alternativa


	GPIO_Config(&handlerTransistor1);
	GPIO_WritePin(&handlerTransistor1, 1); // Estado inicial contrario al 2

	handlerTransistor2.pGPIOx = GPIOB;
	handlerTransistor2.GPIO_PinConfig_t.GPIO_PinNumber = 		PIN_3;
	handlerTransistor2.GPIO_PinConfig_t.GPIO_PinMode = 		GPIO_MODE_OUT;
	handlerTransistor2.GPIO_PinConfig_t.GPIO_PinOPType		= 	GPIO_OTYPE_PUSHPULL; //Sí se configura out, en que modo se va a usar, en este caso se usara como push-pull
	handlerTransistor2.GPIO_PinConfig_t.GPIO_PinPuPdControl= 	GPIO_PUPDR_NOTHING; //Si se le va agregár una resistencia Pull-Up o Pull-down, no  le pondremos ninguna
	handlerTransistor2.GPIO_PinConfig_t.GPIO_PinSpeed		= 	GPIO_OSPEED_FAST; //A que velocidad se va utilizr la salida del
	handlerTransistor2.GPIO_PinConfig_t.GPIO_PinAltFunMode	= 	AF0; //Ninguna funcion alternativa


	GPIO_Config(&handlerTransistor2);
	GPIO_WritePin(&handlerTransistor2, 0); // Estado inicial contrario al 1





}

void BasicTimer2_Callback(void){
	banderaLedUsuario = 1;
}

void callback_extInt0(void){
	banderaContador = 1;
}

void callback_extInt1(void){
	banderaBoton = 1;
}

void funcionContadora(void){
	//Bandera que verifica cambios en el contador
		if(banderaContador == 1){
			banderaContador = 0;
			if(GPIO_ReadPin(&handlerEncoderB) == 0){//cambiar a 1 si esta al reves.
				if(contador != 99){
					contador++;
				}
				else{__NOP();}
			} else{
				if(contador != 0){
					contador--;
				} else{__NOP();}
			}

			decenas = contador / 10;
			unidades = contador % 10;
		}
}

void funcionLeds(uint8_t numero){


	switch(numero){


		case 0:
			ledA = 0;
			ledB = 0;
			ledC = 0;
			ledD = 0;
			ledE = 0;
			ledF = 0;
			ledG = 1;
			break;

		case 1:
			ledA = 1;
			ledB = 0;
			ledC = 0;
			ledD = 1;
			ledE = 1;
			ledF = 1;
			ledG = 1;
			break;

		case 2:
			ledA = 0;
			ledB = 0;
			ledC = 1;
			ledD = 0;
			ledE = 0;
			ledF = 1;
			ledG = 0;
			break;

		case 3:
			ledA = 0;
			ledB = 0;
			ledC = 0;
			ledD = 0;
			ledE = 1;
			ledF = 1;
			ledG = 0;
			break;

		case 4:
			ledA = 1;
			ledB = 0;
			ledC = 0;
			ledD = 1;
			ledE = 1;
			ledF = 0;
			ledG = 0;
			break;

		case 5:
			ledA = 0;
			ledB = 1;
			ledC = 0;
			ledD = 0;
			ledE = 1;
			ledF = 0;
			ledG = 0;
			break;

		case 6:
			ledA = 0;
			ledB = 1;
			ledC = 0;
			ledD = 0;
			ledE = 0;
			ledF = 0;
			ledG = 0;
			break;

		case 7:
			ledA = 0;
			ledB = 0;
			ledC = 0;
			ledD = 1;
			ledE = 1;
			ledF = 1;
			ledG = 1;
			break;

		case 8:
			ledA = 0;
			ledB = 0;
			ledC = 0;
			ledD = 0;
			ledE = 0;
			ledF = 0;
			ledG = 0;
			break;

		case 9:
			ledA = 0;
			ledB = 0;
			ledC = 0;
			ledD = 0;
			ledE = 1;
			ledF = 0;
			ledG = 0;
			break;

		default:
			break;
		}
	GPIO_WritePin(&handlerLedA, ledA);
	GPIO_WritePin(&handlerLedB, ledB);
	GPIO_WritePin(&handlerLedC, ledC);
	GPIO_WritePin(&handlerLedD, ledD);
	GPIO_WritePin(&handlerLedE, ledE);
	GPIO_WritePin(&handlerLedF, ledF);
	GPIO_WritePin(&handlerLedG, ledG);

}

void funcionInfinito(void){
	__NOP();
}

void BasicTimer3_Callback(void){
	banderaTimerLed = 1;
}



