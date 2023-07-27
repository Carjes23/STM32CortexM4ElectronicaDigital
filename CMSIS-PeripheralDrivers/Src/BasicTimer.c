/*
 * BasicTimer.c
 *
 *  Created on: Apr 18, 2022
 *      Author: namontoy
 */

#include "BasicTimer.h"
#include "PLLDriver.h"

void BasicTimer_Callback(void);
void TIM2_IRQHandler(void);

/* Variable que guarda la referencia del periférico que se esta utilizando*/
TIM_TypeDef *ptrTimerUsed;

/* Función en la que cargamos la configuración del Timer
 * Recordar que siempre se debe comenzar con activar la señal de reloj
 * del periférico que se está utilizando.
 * Además, en este caso, debemos ser cuidadosos al momento de utilizar las interrupciones.
 * Los Timer están conectados directamente al elemento NVIC del Cortex-Mx
 * Debemos configurar y/o utilizar:
 *  - TIMx_CR1  (control Register 1)
 *  - TIMx_SMCR ( slave mode control register) -> mantener en 0 para modo Timer Básico
 *  - TIMx_DIER (DMA and Interrupt enable register)
 *  - TIMx_SR (Status register)
 *  - TIMx_CNT (Counter)
 *  - TIMx_PSC (Pre-scaler)
 *  - TIMx_ARR  (Auto-reload register)
 *
 *  Como vamos a trabajar con interrupciones, antes de configurar una nueva, debemos desactivar
 *  el sistema global de interrupciones, activar la IRQ específica y luego volver a encender
 *  el sistema.
 */
void BasicTimer_Config(BasicTimer_Handler_t *ptrBTimerHandler) {
	// Guardamos una referencia al periferico que estamos utilizando...
	ptrTimerUsed = ptrBTimerHandler->ptrTIMx;

	/* 0. Desactivamos las interrupciones globales mientras configuramos el sistema.*/
	__disable_irq();
	/* 1. Activar la señal de reloj del periférico requerido */
	if (ptrBTimerHandler->ptrTIMx == TIM2) {
		//APB1 primero limpiamos
		// Registro del RCC que nos activa la señal de reloj para el TIM2
		//limpia
		RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	} else if (ptrBTimerHandler->ptrTIMx == TIM3) {
		// Registro del RCC que nos activa la señal de reloj para el TIM3
		RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	} else if (ptrBTimerHandler->ptrTIMx == TIM4) {
		// Registro del RCC que nos activa la señal de reloj para el TIM4
		RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	} else if (ptrBTimerHandler->ptrTIMx == TIM5) {
		// Registro del RCC que nos activa la señal de reloj para el TIM5
		RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
	} else {
		__NOP();
	}

	/* 2. Configuramos el Pre-scaler
	 * Recordar que el prescaler nos indica la velocidad a la que se incrementa el counter, de forma que
	 * periodo_incremento * veces_incremento_counter = periodo_update
	 * Modificar el valor del registro PSC en el TIM utilizado
	 */

	/* Escriba codigo aca */
	//Se halla el valor requerido usando el PLL con esto ajustamos a cualquiera dato
	//Tener cuidado con sobrepasar el maximo valor del PLL.
	uint32_t auxData = ptrBTimerHandler->TIMx_Config.TIMx_speed * getFreqPLL();
	ptrBTimerHandler->ptrTIMx->PSC = auxData - 1;

	/* 3. Configuramos la dirección del counter (up/down)*/
	if (ptrBTimerHandler->TIMx_Config.TIMx_mode == BTIMER_MODE_UP) {

		/* 3a. Estamos en UP_Mode, el limite se carga en ARR y se comienza en 0 */
		// Configurar el registro que nos controla el modo up or down
		/* Escriba codigo aca */
		ptrBTimerHandler->ptrTIMx->CR1 &= (RESET << TIM_CR1_DIR_Pos);

		/* 3b. Configuramos el Auto-reload. Este es el "limite" hasta donde el CNT va a contar */
		ptrBTimerHandler->ptrTIMx->ARR =
				ptrBTimerHandler->TIMx_Config.TIMx_period - 1;

		/* 3c. Reiniciamos el registro counter*/
		/* Escriba codigo aca */
		ptrBTimerHandler->ptrTIMx->CNT = RESET;

	} else {
		/* 3a. Estamos en DOWN_Mode, el limite se carga en ARR (0) y se comienza en un valor alto
		 * Trabaja contando en direccion descendente*/
		/* Escriba codigo aca */
		ptrBTimerHandler->ptrTIMx->CR1 &= (RESET << TIM_CR1_DIR_Pos);
		ptrBTimerHandler->ptrTIMx->CR1 |= (SET << TIM_CR1_DIR_Pos);

		/* 3b. Configuramos el Auto-reload. Este es el "limite" hasta donde el CNT va a contar
		 * En modo descendente, con numero positivos, cual es el minimi valor al que ARR puede llegar*/
		/* Escriba codigo aca */
		ptrBTimerHandler->ptrTIMx->ARR =
				ptrBTimerHandler->TIMx_Config.TIMx_period - 1;
		/* 3c. Reiniciamos el registro counter
		 * Este es el valor con el que el counter comienza */
		ptrBTimerHandler->ptrTIMx->CNT = 0;
	}

	/* 4. Activamos el Timer (el CNT debe comenzar a contar*/
	//limpiamos
	ptrBTimerHandler->ptrTIMx->CR1 &= ~(TIM_CR1_CEN);
	ptrBTimerHandler->ptrTIMx->CR1 |= TIM_CR1_CEN;

	/*
	 * x. Activamos la funcion one pulse mode
	 */
	if (ptrBTimerHandler->TIMx_Config.TIMx_OPM == ENABLE) {
		ptrBTimerHandler->ptrTIMx->CR1 |= TIM_CR1_OPM;
	}

	/* 5. Activamos la interrupción debida al Timerx Utilizado
	 * Modificar el registro encargado de activar la interrupcion generada por el TIMx*/
	ptrBTimerHandler->ptrTIMx->DIER |=
			ptrBTimerHandler->TIMx_Config.TIMx_interruptEnable;

	/* 6. Activamos el canal del sistema NVIC para que lea la interrupción*/
	if (ptrBTimerHandler->ptrTIMx == TIM2) {
		// Activando en NVIC para la interrupción del TIM2
		NVIC_EnableIRQ(TIM2_IRQn);
	} else if (ptrBTimerHandler->ptrTIMx == TIM3) {
		// Activando en NVIC para la interrupción del TIM3
		NVIC_EnableIRQ(TIM3_IRQn);
	} else if (ptrBTimerHandler->ptrTIMx == TIM4) {
		// Activando en NVIC para la interrupción del TIM3
		NVIC_EnableIRQ(TIM4_IRQn);
	} else if (ptrBTimerHandler->ptrTIMx == TIM5) {
		// Activando en NVIC para la interrupción del TIM3
		NVIC_EnableIRQ(TIM5_IRQn);
	} else {
		__NOP();
	}

	/* 7. Volvemos a activar las interrupciones del sistema */
	__enable_irq();
}

__attribute__((weak)) void BasicTimer2_Callback(void) {
	/* NOTE : This function should not be modified, when the callback is needed,
	 the BasicTimerX_Callback could be implemented in the main file
	 */
	__NOP();
}
__attribute__((weak)) void BasicTimer3_Callback(void) {
	/* NOTE : This function should not be modified, when the callback is needed,
	 the BasicTimerX_Callback could be implemented in the main file
	 */
	__NOP();
}
__attribute__((weak)) void BasicTimer4_Callback(void) {
	/* NOTE : This function should not be modified, when the callback is needed,
	 the BasicTimerX_Callback could be implemented in the main file
	 */
	__NOP();
}
__attribute__((weak)) void BasicTimer5_Callback(void) {
	/* NOTE : This function should not be modified, when the callback is needed,
	 the BasicTimerX_Callback could be implemented in the main file
	 */
	__NOP();
}

/* Esta es la función a la que apunta el sistema en el vector de interrupciones.
 * Se debe utilizar usando exactamente el mismo nombre definido en el vector de interrupciones,
 * Al hacerlo correctamente, el sistema apunta a esta función y cuando la interrupción se lanza
 * el sistema inmediatamente salta a este lugar en la memoria*/
void TIM2_IRQHandler(void) {
	/* Limpiamos la bandera que indica que la interrupción se ha generado */
	TIM2->SR &= ~TIM_SR_UIF;

	/* LLamamos a la función que se debe encargar de hacer algo con esta interrupción*/
	BasicTimer2_Callback();

}

void TIM3_IRQHandler(void) {
	/* Limpiamos la bandera que indica que la interrupción se ha generado */
	TIM3->SR &= ~TIM_SR_UIF;

	/* LLamamos a la función que se debe encargar de hacer algo con esta interrupción*/
	BasicTimer3_Callback();

}

void TIM4_IRQHandler(void) {
	/* Limpiamos la bandera que indica que la interrupción se ha generado */
	TIM4->SR &= ~TIM_SR_UIF;

	/* LLamamos a la función que se debe encargar de hacer algo con esta interrupción*/
	BasicTimer4_Callback();

}

void TIM5_IRQHandler(void) {
	/* Limpiamos la bandera que indica que la interrupción se ha generado */
	TIM5->SR &= ~TIM_SR_UIF;

	/* LLamamos a la función que se debe encargar de hacer algo con esta interrupción*/
	BasicTimer5_Callback();

}

void TurnOffTimer(BasicTimer_Handler_t *ptrBTimerHandler) {
	//Apagamos el contador
	ptrBTimerHandler->ptrTIMx->CR1 &= ~(TIM_CR1_CEN);
	/* Reiniciamos el registro counter*/
	/* Escriba codigo aca */
	ptrBTimerHandler->ptrTIMx->CNT = 0;

}

void TurnOnTimer(BasicTimer_Handler_t *ptrBTimerHandler) {
	/* Reiniciamos el registro counter*/
	/* Escriba codigo aca */
	ptrBTimerHandler->ptrTIMx->CNT = 0;
	//Apagamos el contador
	ptrBTimerHandler->ptrTIMx->CR1 |= TIM_CR1_CEN;


}

void updateTimerPeriod(BasicTimer_Handler_t *ptrBTimerHandler, uint32_t newPeriod){
	// Cargamos el valor del ARR, el cual es el límite de incrementos del Timer
	// antes de hacer un update y reload.
	ptrBTimerHandler->ptrTIMx->ARR = newPeriod  - 1;
}
