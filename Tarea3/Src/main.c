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
BasicTimer_Handler_t handlerTim4 = {0};

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
GPIO_Handler_t handlerTransistor = {0}; //Transistor 1


uint8_t unidades = 0;
uint8_t decenas = 0;
uint8_t contador = 0;
uint8_t contadorc = 0;
uint8_t contadortim3 = 0;

bool banderaContador = 0;
bool banderaBoton = 0;
bool banderaLedUsuario = 0;
bool banderaTimerLed1 = 0;
bool banderaTimerLed2 = 0;
bool estado = 0;
bool banderaFiltro = 0;

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
void funcionLedsC(void);

void BasicTimer2_Callback(void);
void BasicTimer3_Callback(void);

int main(void){

	initSystem();

	while(1){

		if(banderaBoton == 1 && banderaFiltro ==1){
			banderaBoton = 0;
			banderaFiltro = 0;
			estado ^= 1;
			TIM4->CR1 |= TIM_CR1_CEN;
		}

		else if(banderaBoton == 1){
			banderaBoton = 0;
		}

		if(banderaLedUsuario == 1){
			banderaLedUsuario = 0;
			GPIOxTooglePin(&handlerUserLedPin);
		}

		decenas = contador / 10;
		unidades = contador % 10;

		if(estado == CONTADOR){
			funcionContadora();
			if(banderaTimerLed1==1){
				GPIO_WritePin(&handlerTransistor, 1);
				banderaTimerLed1 = 0;
				funcionLeds(unidades);
			}
			else if(banderaTimerLed2==1){
				GPIO_WritePin(&handlerTransistor, 0); //Cambiar si es al contrario
				banderaTimerLed2 =0;
				funcionLeds(decenas);
			}
			else{
				__NOP();
				}
			}
		else { funcionInfinito();}

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

	handlerTim3.ptrTIMx = TIM3;
	handlerTim3.TIMx_Config.TIMx_interruptEnable = 1;
	handlerTim3.TIMx_Config.TIMx_mode = BTIMER_MODE_UP;
	handlerTim3.TIMx_Config.TIMx_period = 10;
	handlerTim3.TIMx_Config.TIMx_speed = BTIMER_SPEED_1ms;

	BasicTimer_Config(&handlerTim3);

	handlerTim4.ptrTIMx = TIM4;
	handlerTim4.TIMx_Config.TIMx_interruptEnable = 1;
	handlerTim4.TIMx_Config.TIMx_mode = BTIMER_MODE_UP;
	handlerTim4.TIMx_Config.TIMx_period = 250;
	handlerTim4.TIMx_Config.TIMx_speed = BTIMER_SPEED_100us;
	handlerTim4.TIMx_Config.TIMx_OPM = ENABLE;

	BasicTimer_Config(&handlerTim4);


	handlerEncoderA.pGPIOx = GPIOA;
	handlerEncoderA.GPIO_PinConfig_t.GPIO_PinNumber = PIN_0;
	handlerEncoderA.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_IN;
	handlerEncoderA.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLDOWN;


	handlerExtiA.edgeType = EXTERNAL_INTERRUPT_FALLING_EDGE;
	handlerExtiA.pGPIOHandler = &handlerEncoderA;

	extInt_Config(&handlerExtiA);

	handlerEncoderB.pGPIOx = GPIOC;
	handlerEncoderB.GPIO_PinConfig_t.GPIO_PinNumber = PIN_12;
	handlerEncoderB.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_IN;
	handlerEncoderB.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLDOWN;

	GPIO_Config(&handlerEncoderB);

	handlerBotEncoder.pGPIOx = GPIOD;
	handlerBotEncoder.GPIO_PinConfig_t.GPIO_PinNumber = PIN_2;
	handlerBotEncoder.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_IN;
	handlerBotEncoder.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLUP; //revisar


	handlerExtiBot.edgeType = EXTERNAL_INTERRUPT_RISING_EDGE;
	handlerExtiBot.pGPIOHandler = &handlerBotEncoder;

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

	handlerTransistor.pGPIOx = GPIOC;
	handlerTransistor.GPIO_PinConfig_t.GPIO_PinNumber = 		PIN_10;
	handlerTransistor.GPIO_PinConfig_t.GPIO_PinMode = 		GPIO_MODE_OUT;
	handlerTransistor.GPIO_PinConfig_t.GPIO_PinOPType		= 	GPIO_OTYPE_PUSHPULL; //Sí se configura out, en que modo se va a usar, en este caso se usara como push-pull
	handlerTransistor.GPIO_PinConfig_t.GPIO_PinPuPdControl= 	GPIO_PUPDR_NOTHING; //Si se le va agregár una resistencia Pull-Up o Pull-down, no  le pondremos ninguna
	handlerTransistor.GPIO_PinConfig_t.GPIO_PinSpeed		= 	GPIO_OSPEED_FAST; //A que velocidad se va utilizr la salida del
	handlerTransistor.GPIO_PinConfig_t.GPIO_PinAltFunMode	= 	AF0; //Ninguna funcion alternativa


	GPIO_Config(&handlerTransistor);




}

void BasicTimer2_Callback(void){
	banderaLedUsuario = 1;
}

void callback_extInt0(void){
	banderaContador = 1;
}

void callback_extInt2(void){
	banderaBoton = 1;
}

void funcionContadora(void){
	//Bandera que verifica cambios en el contador
		if(banderaContador == 1 && banderaFiltro == 1){
			banderaContador = 0;
			banderaFiltro = 0;
			TIM4->CR1 |= TIM_CR1_CEN;
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
		else if(banderaContador == 1){
			banderaContador = 0;
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
	if(banderaContador == 1 && banderaFiltro ==1){
			banderaFiltro = 0;
			banderaContador = 0;
			TIM4->CR1 |= TIM_CR1_CEN;
			if(GPIO_ReadPin(&handlerEncoderB) == 0){//cambiar a 1 si esta al reves.
				if(contadorc != 11){
					contadorc++;
				}
				else{contadorc = 0;}
			} else{
				if(contadorc != 0){
					contadorc--;
				} else{contadorc = 11;}
			}
		}
	else if(banderaContador == 1){
			banderaContador = 0;
		}
	funcionLedsC();
}

void BasicTimer3_Callback(void){
	contadortim3++;
	if(contadortim3 == 1){
		banderaTimerLed1 = 1;
	}
	else if(contadortim3 == 2){
		banderaTimerLed2 = 1;
		contadortim3 = 0;
	}
}

void funcionLedsC(void){
	switch(contadorc){
		case 0:
			GPIO_WritePin(&handlerTransistor, 1);
			ledA = 0;
			ledB = 1;
			ledC = 1;
			ledD = 1;
			ledE = 1;
			ledF = 1;
			ledG = 1;
			break;
		case 1:
			GPIO_WritePin(&handlerTransistor, 0);
			ledA = 0;
			ledB = 1;
			ledC = 1;
			ledD = 1;
			ledE = 1;
			ledF = 1;
			ledG = 1;
			break;
		case 2:
			GPIO_WritePin(&handlerTransistor, 0);
			ledA = 1;
			ledB = 1;
			ledC = 1;
			ledD = 1;
			ledE = 1;
			ledF = 0;
			ledG = 1;
			break;
		case 3:
			GPIO_WritePin(&handlerTransistor, 0);
			ledA = 1;
			ledB = 1;
			ledC = 1;
			ledD = 1;
			ledE = 0;
			ledF = 1;
			ledG = 1;
			break;
		case 4:
			GPIO_WritePin(&handlerTransistor, 0);
			ledA = 1;
			ledB = 1;
			ledC = 1;
			ledD = 0;
			ledE = 1;
			ledF = 1;
			ledG = 1;
			break;
		case 5:
			GPIO_WritePin(&handlerTransistor, 1);
			ledA = 1;
			ledB = 1;
			ledC = 1;
			ledD = 1;
			ledE = 0;
			ledF = 1;
			ledG = 1;
			break;
		case 6:
			GPIO_WritePin(&handlerTransistor, 1);
			ledA = 1;
			ledB = 1;
			ledC = 1;
			ledD = 1;
			ledE = 1;
			ledF = 0;
			ledG = 1;
			break;
		case 7:
			GPIO_WritePin(&handlerTransistor, 0);
			ledA = 1;
			ledB = 0;
			ledC = 1;
			ledD = 1;
			ledE = 1;
			ledF = 1;
			ledG = 1;
			break;
		case 8:
			GPIO_WritePin(&handlerTransistor, 0);
			ledA = 1;
			ledB = 1;
			ledC = 0;
			ledD = 1;
			ledE = 1;
			ledF = 1;
			ledG = 1;
			break;
		case 9:
			GPIO_WritePin(&handlerTransistor, 1);
			ledA = 1;
			ledB = 1;
			ledC = 1;
			ledD = 0;
			ledE = 1;
			ledF = 1;
			ledG = 1;
			break;
		case 10:
			GPIO_WritePin(&handlerTransistor, 1);
			ledA = 1;
			ledB = 1;
			ledC = 0;
			ledD = 1;
			ledE = 1;
			ledF = 1;
			ledG = 1;
			break;
		case 11:
			GPIO_WritePin(&handlerTransistor, 1);
			ledA = 1;
			ledB = 0;
			ledC = 1;
			ledD = 1;
			ledE = 1;
			ledF = 1;
			ledG = 1;
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

void BasicTimer4_Callback(void){
	banderaFiltro = 1;
}

