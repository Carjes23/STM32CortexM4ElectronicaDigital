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
#include "BasicTimer.h"
#include "ExtiDriver.h"
#include <stdbool.h>

uint16_t counterExti13 = 0;
uint16_t counterTIM2 = 0;
uint16_t counterTIM3 = 0;
uint16_t counterTIM4 = 0;
uint16_t counterTIM5 = 0;

//void BasicTimer3_Callback(void);
void callback_extInt13(void);

void BasicTimer2_Callback(void){
	GPIO_Handler_t handlerUserLedPin = {0};
	/*
	* Se define el Handler para el pin LD2
	* Con este pin se pretende probar los dos primeros puntos.
	*/


	handlerUserLedPin.pGPIOx = GPIOA; // Aquí se coloca a que GPIO pertenece el pin en este caso es el A
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_5; //El numero del pin en este caso es el

	GPIOxTooglePin(&handlerUserLedPin);

	counterTIM2++;
}

void BasicTimer3_Callback(void){
	counterTIM3++;
}

void BasicTimer4_Callback(void){
	counterTIM4++;
}

void BasicTimer5_Callback(void){
	counterTIM5++;
}

//Callback
void callback_extInt13(void){
	counterExti13++;
}
/*
 * Funcion principal del programa
 * Esta funcion es el corazon del programa!!
 *
 * */





int main(void){
	GPIO_Handler_t handlerUserLedPin = {0};
	/*
	* Se define el Handler para el pin LD2
	* Con este pin se pretende probar los dos primeros puntos.
	*/


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


	//se creal el Handler

	BasicTimer_Handler_t handlerTim2 = {0};

	//Se configura el Handeler.

	handlerTim2.ptrTIMx = TIM2;
	handlerTim2.TIMx_Config.TIMx_interruptEnable = 1;
	handlerTim2.TIMx_Config.TIMx_mode = BTIMER_MODE_UP;
	handlerTim2.TIMx_Config.TIMx_period = 250;
	handlerTim2.TIMx_Config.TIMx_speed = BTIMER_SPEED_1ms;

	BasicTimer_Config(&handlerTim2);

	//se creal el Handler

	BasicTimer_Handler_t handlerTim3 = {0};

	//Se configura el Handeler.

	handlerTim3.ptrTIMx = TIM3;
	handlerTim3.TIMx_Config.TIMx_interruptEnable = 1;
	handlerTim3.TIMx_Config.TIMx_mode = BTIMER_MODE_UP;
	handlerTim3.TIMx_Config.TIMx_period = 500;
	handlerTim3.TIMx_Config.TIMx_speed = BTIMER_SPEED_1ms;

	BasicTimer_Config(&handlerTim3);

	//se creal el Handler

	BasicTimer_Handler_t handlerTim4 = {0};

	//Se configura el Handeler.

	handlerTim4.ptrTIMx = TIM4;
	handlerTim4.TIMx_Config.TIMx_interruptEnable = 1;
	handlerTim4.TIMx_Config.TIMx_mode = BTIMER_MODE_UP;
	handlerTim4.TIMx_Config.TIMx_period = 750;
	handlerTim4.TIMx_Config.TIMx_speed = BTIMER_SPEED_1ms;

	BasicTimer_Config(&handlerTim4);

	//se creal el Handler

	BasicTimer_Handler_t handlerTim5 = {0};

	//Se configura el Handeler.

	handlerTim5.ptrTIMx = TIM5;
	handlerTim5.TIMx_Config.TIMx_interruptEnable = 1;
	handlerTim5.TIMx_Config.TIMx_mode = BTIMER_MODE_UP;
	handlerTim5.TIMx_Config.TIMx_period = 1000;
	handlerTim5.TIMx_Config.TIMx_speed = BTIMER_SPEED_1ms;

	BasicTimer_Config(&handlerTim5);

	GPIO_Handler_t handlerUserButton = {0};


	//Desde ahora se dara una descripción menos detallada, pero especificando cada uno
	handlerUserButton.pGPIOx = GPIOC; //Se encuentra en el el GPIOC
	handlerUserButton.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_13; //El registro que este modifica es el numero 13
	handlerUserButton.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_IN;//En este caso el pin se usara como entrada de datos
	handlerUserButton.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING; //No se usara ninguna resistencia pull-up o pull-down
	handlerUserButton.GPIO_PinConfig_t.GPIO_PinAltFunMode	= AF0; //Ninguna funcion alternativa

	GPIO_Config(&handlerUserButton);

	/*
	 * Implementacion manual
	 */

//	/*
//	 * 1. Activar la señal de reloj para el SYSCFG
//	 */
//	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
//
//	// 2.Configurar el multiplexor 13, asignando el puerto GPIOC
//
//	SYSCFG -> EXTICR[3] &= ~(0xF << SYSCFG_EXTICR4_EXTI13_Pos);
//	SYSCFG -> EXTICR[3] |= SYSCFG_EXTICR4_EXTI13_PC;
//
//	//3. COnfigurar el EXTIC
//	EXTI -> FTSR = 0; //desactivamos TOdas las posibles detecciones de flancos de bajada.
//	EXTI -> RTSR = 0; //llevamos un valor coonocido al registro
//	EXTI -> RTSR |= EXTI_RTSR_TR13; //Activamos la detención del flanco de subida.
//	EXTI -> IMR = 0;
//	EXTI -> IMR |= EXTI_IMR_IM13; //Activamos la interrupcion EXTI_13
//
//	//4 Desactivamos las interrupciones gobbales
//	__disable_irq();
//
//	// Incluir la interrupcion en el NVIC
//	NVIC_EnableIRQ(EXTI15_10_IRQn);
//	//Crear ISR
//	//callback
//	//Enable interruciones globales
//	__enable_irq();
//
//
//
	/*
	 * Implementación con driver
	 */
	EXTI_Config_t handlerExtiBoton = {0};
	handlerExtiBoton.pGPIOHandler = &handlerUserButton;
	handlerExtiBoton.edgeType = EXTERNAL_INTERRUPT_FALLING_EDGE;

	extInt_Config(&handlerExtiBoton);
	while(1){

	}


	return 0;
}

//Verifica la interrupcion bajar la bandera y llamar el callback.
//void EXTI15_10_IRQHandler(void){
//	/*VErificamos que sea la bandera ya que se va lenvantar muchos pines.
//	 */
//	if((EXTI -> PR & EXTI_PR_PR13) !=0){
//		EXTI -> PR |= EXTI_PR_PR13; //Bajando la vandera
//		callback_exti13();
//	};
//}


