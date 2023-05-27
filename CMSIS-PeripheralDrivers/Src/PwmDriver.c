/*
 * PwmDriver.c
 *
 *  Created on: XXXX , 2022
 *      Author: namontoy
 */
#include "PwmDriver.h"

/**/
void pwm_Config(PWM_Handler_t *ptrPwmHandler){

	/* 1. Activar la señal de reloj del periférico requerido */
	if(ptrPwmHandler->ptrTIMx == TIM2){
		//APB1 primero limpiamos
		// Registro del RCC que nos activa la señal de reloj para el TIM2
		RCC -> APB1ENR |= RCC_APB1ENR_TIM2EN;
	}
	else if(ptrPwmHandler->ptrTIMx == TIM3){
		// Registro del RCC que nos activa la señal de reloj para el TIM3
		RCC -> APB1ENR |= RCC_APB1ENR_TIM3EN;
	}
	else if(ptrPwmHandler->ptrTIMx == TIM4){
		// Registro del RCC que nos activa la señal de reloj para el TIM4
		RCC -> APB1ENR |= RCC_APB1ENR_TIM4EN;
	}
	else if(ptrPwmHandler->ptrTIMx == TIM5){
		// Registro del RCC que nos activa la señal de reloj para el TIM5
		RCC -> APB1ENR |= RCC_APB1ENR_TIM5EN;
	}
	else{
		__NOP();
	}

	/* 1. Cargamos la frecuencia deseada */
	setFrequency(ptrPwmHandler);

	/* 2. Cargamos el valor del dutty-Cycle*/
	setDuttyCycle(ptrPwmHandler);

	/* 2a. Estamos en UP_Mode, el limite se carga en ARR y se comienza en 0 */
	// Configurar el registro que nos controla el modo up or down
	ptrPwmHandler->ptrTIMx->CR1 &= (RESET << TIM_CR1_DIR_Pos);
	ptrPwmHandler->ptrTIMx->CR1 |= TIM_CR1_ARPE;
	/* 2b. Configuramos el Auto-reload. Este es el "limite" hasta donde el CNT va a contar */
	ptrPwmHandler->ptrTIMx->ARR = ptrPwmHandler->config.periodo - 1;

	/* 2c. Reiniciamos el registro counter*/
	ptrPwmHandler->ptrTIMx->CNT = RESET;

	/* 3. Configuramos los bits CCxS del registro TIMy_CCMR1, de forma que sea modo salida
	 * (para cada canal hay un conjunto CCxS)
	 *
	 * 4. Además, en el mismo "case" podemos configurar el modo del PWM, su polaridad...
	 *
	 * 5. Y además activamos el preload bit, para que cada vez que exista un update-event
	 * el valor cargado en el CCRx será recargado en el registro "shadow" del PWM */
	switch(ptrPwmHandler->config.channel){
	case PWM_CHANNEL_1:{
		// Seleccionamos como salida el canal
		ptrPwmHandler->ptrTIMx->CCMR1 &= ~(0b11<<TIM_CCMR1_CC1S_Pos);

		// Configuramos el canal como PWM
		ptrPwmHandler->ptrTIMx->CCMR1 |= (0b110<<TIM_CCMR1_OC1M_Pos);

		// Activamos la funcionalidad de pre-load
		ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC1PE;

		break;
	}

	case PWM_CHANNEL_2:{
		// Seleccionamos como salida el canal
		ptrPwmHandler->ptrTIMx->CCMR1 &= ~(0b11<<TIM_CCMR1_CC2S_Pos);

		// Configuramos el canal como PWM
		ptrPwmHandler->ptrTIMx->CCMR1 |= (0b110<<TIM_CCMR1_OC2M_Pos);

		// Activamos la funcionalidad de pre-load
		ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC2PE;

		break;
	}
	case PWM_CHANNEL_3:{
		// Seleccionamos como salida el canal
		ptrPwmHandler->ptrTIMx->CCMR2 &= ~(0b11<<TIM_CCMR2_CC3S_Pos);

		// Configuramos el canal como PWM
		ptrPwmHandler->ptrTIMx->CCMR2 |= (0b110<<TIM_CCMR2_OC3M_Pos);

		// Activamos la funcionalidad de pre-load
		ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR2_OC3PE;

		break;
	}
	case PWM_CHANNEL_4:{
		// Seleccionamos como salida el canal
		ptrPwmHandler->ptrTIMx->CCMR2 &= ~(0b11<<TIM_CCMR2_CC4S_Pos);

		// Configuramos el canal como PWM
		ptrPwmHandler->ptrTIMx->CCMR2 |= (0b110<<TIM_CCMR2_OC4M_Pos);

		// Activamos la funcionalidad de pre-load
		ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR2_OC4PE;

		break;
	}

	default:{
		break;
	}


	}// fin del switch-case
	/* 6. Activamos la salida seleccionada */
	enableOutput(ptrPwmHandler);
}

/* Función para activar el Timer y activar todos el módulo PWM */
void startPwmSignal(PWM_Handler_t *ptrPwmHandler) {

	//Activamos el timer

	ptrPwmHandler->ptrTIMx->CR1 &= ~(TIM_CR1_CEN);
	ptrPwmHandler->ptrTIMx->CR1 |= TIM_CR1_CEN;

	//Activamos la salida dependiendo del chanel usado

	switch(ptrPwmHandler->config.channel){
	case PWM_CHANNEL_1:{
		ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC1E;

		break;
	}

	case PWM_CHANNEL_2:{
		ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC2E;

		break;
	}
	case PWM_CHANNEL_3:{
		ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC3E;

		break;
	}
	case PWM_CHANNEL_4:{
		ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC4E;

		break;
	}

	default:{
		break;
	}
}
}
/* Función para desactivar el Timer y detener todos el módulo PWM*/
void stopPwmSignal(PWM_Handler_t *ptrPwmHandler) {
	//desactivamos el timer

	ptrPwmHandler->ptrTIMx->CR1 &= ~(TIM_CR1_CEN);

	//Activamos la salida dependiendo del chanel usado

	switch(ptrPwmHandler->config.channel){
	case PWM_CHANNEL_1:{
		ptrPwmHandler->ptrTIMx->CCER &= ~(TIM_CCER_CC1E);

		break;
	}

	case PWM_CHANNEL_2:{
		ptrPwmHandler->ptrTIMx->CCER &= ~(TIM_CCER_CC2E);

		break;
	}
	case PWM_CHANNEL_3:{
		ptrPwmHandler->ptrTIMx->CCER &= ~(TIM_CCER_CC3E);

		break;
	}
	case PWM_CHANNEL_4:{
		ptrPwmHandler->ptrTIMx->CCER &= ~(TIM_CCER_CC4E);

		break;
	}

	default:{
		break;
	}
}
}
/* Función encargada de activar cada uno de los canales con los que cuenta el TimerX */
void enableOutput(PWM_Handler_t *ptrPwmHandler) {

	switch(ptrPwmHandler->config.channel){
	case PWM_CHANNEL_1:{
		// Activamos la salida del canal 1
		/* agregue acá su código */
		ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC1E;

		break;
	}

	case PWM_CHANNEL_2:{
		ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC2E;

		break;
	}
	case PWM_CHANNEL_3:{
		ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC3E;

		break;
	}
	case PWM_CHANNEL_4:{
		ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC4E;

		break;
	}

	default:{
		break;
	}
	}
}

/* 
 * La frecuencia es definida por el conjunto formado por el preescaler (PSC)
 * y el valor límite al que llega el Timer (ARR), con estos dos se establece
 * la frecuencia.
 * */
void setFrequency(PWM_Handler_t *ptrPwmHandler){

	// Cargamos el valor del prescaler, nos define la velocidad (en ns) a la cual
	// se incrementa el Timer
	ptrPwmHandler->ptrTIMx->PSC = ptrPwmHandler->config.prescaler - 1;

	// Cargamos el valor del ARR, el cual es el límite de incrementos del Timer
	// antes de hacer un update y reload.
	ptrPwmHandler->ptrTIMx->ARR = ptrPwmHandler->config.periodo  - 1	;
}


/* Función para actualizar la frecuencia, funciona de la mano con setFrequency */
void updateFrequency(PWM_Handler_t *ptrPwmHandler, uint16_t newFreq){
	// Actualizamos el registro que manipula el periodo
    /* agregue acá su código */
	ptrPwmHandler->config.periodo = newFreq;

	// Llamamos a la fucnión que cambia la frecuencia
	setFrequency(ptrPwmHandler);
}

/* El valor del dutty debe estar dado en valores de %, entre 0% y 100%*/
void setDuttyCycle(PWM_Handler_t *ptrPwmHandler){

	// Seleccionamos el canal para configurar su dutty
	switch(ptrPwmHandler->config.channel){
	case PWM_CHANNEL_1:{
		ptrPwmHandler->ptrTIMx->CCR1 = ptrPwmHandler->config.duttyCicle;

		break;
	}

	case PWM_CHANNEL_2:{
		ptrPwmHandler->ptrTIMx->CCR2 = ptrPwmHandler->config.duttyCicle;

		break;
	}

	case PWM_CHANNEL_3:{
		ptrPwmHandler->ptrTIMx->CCR3 = ptrPwmHandler->config.duttyCicle;

		break;
	}

	case PWM_CHANNEL_4:{
		ptrPwmHandler->ptrTIMx->CCR4 = ptrPwmHandler->config.duttyCicle;

		break;
	}


	default:{
		break;
	}

	}// fin del switch-case

}


/* Función para actualizar el Dutty, funciona de la mano con setDuttyCycle */
void updateDuttyCycle(PWM_Handler_t *ptrPwmHandler, uint16_t newDutty){
	// Actualizamos el registro que manipula el dutty
    ptrPwmHandler->config.duttyCicle = newDutty;

	// Llamamos a la fucnión que cambia el dutty y cargamos el nuevo valor
	setDuttyCycle(ptrPwmHandler);
}




