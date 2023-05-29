/*
 * USARTxDriver.c
 *
 *  Created on: Apr 6, 2022
 *      Author: namontoy
 */

#include <stm32f4xx.h>
#include "USARTxDriver.h"
#include <math.h>
#include "float.h"
#include "PLLDriver.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

uint8_t auxRxData;

bool flagNewData = 0;

//Interrupciones 1
int dataToSend1 = 0;
char *stringToSend1;
bool tipo1 = 0; //Si es 0 se envia un chart si es un 1 un string
int posicionActual1 = 0;

//Interrupciones 2
int dataToSend2 = 0;
char *stringToSend2;
bool tipo2 = 0; //Si es 0 se envia un chart si es un 1 un string
int posicionActual2 = 0;

//Interrupciones 6
int dataToSend6 = 0;
char *stringToSend6;
bool tipo6 = 0; //Si es 0 se envia un chart si es un 1 un string
int posicionActual6 = 0;

/**
 * Configurando el puerto Serial...
 * Recordar que siempre se debe comenzar con activar la señal de reloj
 * del periferico que se está utilizando.
 */
void USART_Config(USART_Handler_t *ptrUsartHandler) {

	/* 0. Desactivamos las interrupciones globales mientras configuramos el sistema.*/
	__disable_irq();

	/* 1. Activamos la señal de reloj que viene desde el BUS al que pertenece el periferico */
	/* Lo debemos hacer para cada uno de las pisbles opciones que tengamos (USART1, USART2, USART6) */
	/* 1.1 Configuramos el USART1 */
	if (ptrUsartHandler->ptrUSARTx == USART1) {
		RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	}

	/* 1.2 Configuramos el USART2 */
	else if (ptrUsartHandler->ptrUSARTx == USART2) {
		RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	}

	/* 1.3 Configuramos el USART6 */
	else if (ptrUsartHandler->ptrUSARTx == USART6) {
		RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
	}

	/* 2. Configuramos el tamaño del dato, la paridad y los bit de parada */
	/* En el CR1 estan parity (PCE y PS) y tamaño del dato (M) */
	/* Mientras que en CR2 estan los stopbit (STOP)*/
	/* Configuracion del Baudrate (registro BRR) */
	/* Configuramos el modo: only TX, only RX, o RXTX */
	/* Por ultimo activamos el modulo USART cuando todos esta correctamente configurado */

	// 2.1 Comienzo por limpiar los registros, para cargar la configuración desde cero
	ptrUsartHandler->ptrUSARTx->CR1 = 0;
	ptrUsartHandler->ptrUSARTx->CR2 = 0;

	// 2.2 Configuracion del Parity:
	// Verificamos si el parity esta activado o no
	// Tenga cuidado, el parity hace parte del tamaño de los datos...
	if (ptrUsartHandler->USART_Config.USART_parity != USART_PARITY_NONE) {
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_PCE; // Activamos la seleccion de paridad
		// Verificamos si se ha seleccionado ODD or EVEN
		if (ptrUsartHandler->USART_Config.USART_parity == USART_PARITY_EVEN) {
			// Es even, entonces cargamos la configuracion adecuada
			// Escriba acá su código
			ptrUsartHandler->ptrUSARTx->CR1 &= ~(USART_CR1_PS); //Queremos que sea 0
		} else {
			// Si es "else" significa que la paridad seleccionada es ODD, y cargamos esta configuracion
			// Escriba acá su código
			ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_PS; //Queremos que sea 1
		}
	} else {
		// Si llegamos aca, es porque no deseamos tener el parity-check
		ptrUsartHandler->ptrUSARTx->CR1 &= ~(USART_CR1_PCE); // Activamos la seleccion de paridad

	}

	// 2.3 Configuramos el tamaño del dato
	if (ptrUsartHandler->USART_Config.USART_datasize == USART_DATASIZE_8BIT) {
		ptrUsartHandler->ptrUSARTx->CR1 &= ~(USART_CR1_M); // Lo ponemos en 8 bits
	} else {
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_M; // Lo ponemos en 9 bits
	}

	// 2.4 Configuramos los stop bits (SFR USART_CR2)
	switch (ptrUsartHandler->USART_Config.USART_stopbits) {
	case USART_STOPBIT_1: {
		// Debemoscargar el valor 0b00 en los dos bits de STOP
		ptrUsartHandler->ptrUSARTx->CR2 |= (0b00 < USART_CR2_STOP_Pos);
		break;
	}
	case USART_STOPBIT_0_5: {
		// Debemoscargar el valor 0b01 en los dos bits de STOP
		ptrUsartHandler->ptrUSARTx->CR2 |= (0b01 < USART_CR2_STOP_Pos);
		break;
	}
	case USART_STOPBIT_2: {
		// Debemoscargar el valor 0b10 en los dos bits de STOP
		ptrUsartHandler->ptrUSARTx->CR2 |= (0b10 < USART_CR2_STOP_Pos);
		break;
	}
	case USART_STOPBIT_1_5: {
		// Debemoscargar el valor 0b11 en los dos bits de STOP
		ptrUsartHandler->ptrUSARTx->CR2 |= (0b11 < USART_CR2_STOP_Pos);
		break;
	}
	default: {
		// En el casopor defecto seleccionamos 1 bit de parada
		ptrUsartHandler->ptrUSARTx->CR2 |= (0b00 < USART_CR2_STOP_Pos);
		break;
	}
	}
	//Obtenes la frecuencia actual desde el PLL, si estamos en usart2 toca dividirla entre 2
	uint16_t freckClock = getFreqPLL();
	if (ptrUsartHandler->ptrUSARTx == USART2 && freckClock > 50) {
		freckClock = getFreqPLL() / 2;
	}
	// 2.5 Configuracion del Baudrate (SFR USART_BRR)
	// Ver tabla de valores (Tabla 73), Frec = 16MHz, overr = 0;
	//Se dejan los comentarios antiguos pero ahora se utilizan formulas
	//matematicas para calcular los resultados automaticamente.
	if (ptrUsartHandler->USART_Config.USART_baudrate == USART_BAUDRATE_9600) {
		float div = (freckClock * 1E6) / (16 * 9600);
		uint16_t mantissa = (int) div;
		uint16_t fraction = (int) round((div - mantissa) * 16);
		uint16_t result = mantissa << 4 | fraction;
		ptrUsartHandler->ptrUSARTx->BRR = result;

		//example for 16Mhz
		// El valor a cargar es 104.1875 -> Mantiza = 104,fraction = 0.1875
		// Mantiza = 104 = 0x68, fraction = 16 * 0.1875 = 3
		// Valor a cargar 0x0683
		// Configurando el Baudrate generator para una velocidad de 9600bps

	}

	else if (ptrUsartHandler->USART_Config.USART_baudrate
			== USART_BAUDRATE_19200) {
		float div = (freckClock * 1E6) / (16 * 19200);
		uint16_t mantissa = (int) div;
		uint16_t fraction = (int) round((div - mantissa) * 16);
		uint16_t result = mantissa << 4 | fraction;
		ptrUsartHandler->ptrUSARTx->BRR = result;
		//example for 16Mhz
		// El valor a cargar es 52.0625 -> Mantiza = 52,fraction = 0.0625
		// Mantiza = 52 = 0x34, fraction = 16 * 0.1875 = 1
		// Valor a cargar 0x0341
		// Escriba acá su código y los comentarios que faltan
	}

	else if (ptrUsartHandler->USART_Config.USART_baudrate
			== USART_BAUDRATE_115200) {
		float div = (freckClock * 1E6) / (16 * 115200);
		uint16_t mantissa = (int) div;
		uint16_t fraction = (int) round((div - mantissa) * 16);
		uint16_t result = mantissa << 4 | fraction;
		ptrUsartHandler->ptrUSARTx->BRR = result;
		//example for 16Mhz
		// El valor a cargar es 8.6875 -> Mantiza = 8,fraction = 0.6875
		// Mantiza = 8 = 0x8, fraction = 16 * 0.6875 = 11 = 0xB
		// Valor a cargar 0x008B
		// Escriba acá su código y los comentarios que faltan
	}

	// 2.6 Configuramos el modo: TX only, RX only, RXTX, disable
	switch (ptrUsartHandler->USART_Config.USART_mode) {
	case USART_MODE_TX: {
		// Activamos la parte del sistema encargada de enviar
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_TE;
		break;
	}
	case USART_MODE_RX: {
		// Activamos la parte del sistema encargada de recibir
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_RE;
		break;
	}
	case USART_MODE_RXTX: {
		// Activamos ambas partes, tanto transmision como recepcion
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_TE;
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_RE;
		break;
	}
	case USART_MODE_DISABLE: {
		// Desactivamos ambos canales
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_UE;
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_TE;
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_RE;
		break;
	}

	default: {
		// Actuando por defecto, desactivamos ambos canales
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_UE;
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_TE;
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_RE;
		break;
	}
	}

	// 2.7 Activamos el modulo serial.
	if (ptrUsartHandler->USART_Config.USART_mode != USART_MODE_DISABLE) {
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_UE;
	}

	//3.Activamos la interrupcion para el rx USART_RX_Int_Ena y el TX
	ptrUsartHandler->ptrUSARTx->CR1 |=
			(ptrUsartHandler->USART_Config.USART_RX_Int_Ena
					<< USART_CR1_RXNEIE_Pos);

	/* 4.. Activamos el canal del sistema NVIC para que lea la interrupción*/
	if (ptrUsartHandler->ptrUSARTx == USART1) {
		// Activando en NVIC para la interrupción del USART1 USART1_IRQHandler
		NVIC_EnableIRQ(USART1_IRQn);
	} else if (ptrUsartHandler->ptrUSARTx == USART2) {
		// Activando en NVIC para la interrupción del USART2 USART1_IRQHandler
		NVIC_EnableIRQ(USART2_IRQn);
	} else if (ptrUsartHandler->ptrUSARTx == USART6) {
		// Activando en NVIC para la interrupción del USART6 USART1_IRQHandler
		NVIC_EnableIRQ(USART6_IRQn);
	}

	else {
		__NOP();
	}

	/* 5. Volvemos a activar las interrupciones del sistema */
	__enable_irq();
}

__attribute__((weak)) void USART1Rx_Callback(void) {
	/* NOTE : This function should not be modified, when the callback is needed,
	 the USART1_Callback could be implemented in the main file
	 */
	__NOP();
}

/*
 * Se crean este par de funciones en cada usart para el envio de datos
 * donde se utilizan las interrupciones.
 */
void USART1Tx_Char(void) {
	USART1->DR = dataToSend1;
	USART1->CR1 &= ~(USART_CR1_TXEIE);

}

void USART1Tx_String(void) {
	char auxData = stringToSend1[posicionActual1];
	if (auxData != 0) {
		USART1->DR = auxData;
		posicionActual1++;
	} else {
		USART6->CR1 &= ~(USART_CR1_TXEIE);
		posicionActual6 = 0;
		flagNewData = 0;
	}

}

__attribute__((weak)) void USART2Rx_Callback(void) {
	/* NOTE : This function should not be modified, when the callback is needed,
	 the USART1_Callback could be implemented in the main file
	 */
	__NOP();
}

/*
 * Se crean este par de funciones en cada usart para el envio de datos
 * donde se utilizan las interrupciones.
 */
void USART2Tx_Char(void) {
	USART2->DR = dataToSend2;
	USART2->CR1 &= ~(USART_CR1_TXEIE);

}

void USART2Tx_String(void) {
	char auxData = stringToSend2[posicionActual2];
	if (auxData != 0) {
		USART2->DR = auxData;
		posicionActual2++;
	} else {
		USART2->CR1 &= ~(USART_CR1_TXEIE);
		posicionActual2 = 0;
		flagNewData = 0;
	}

}
__attribute__((weak)) void USART6Rx_Callback(void) {
	/* NOTE : This function should not be modified, when the callback is needed,
	 the USART1_Callback could be implemented in the main file
	 */
	__NOP();
}
/*
 * Se crean este par de funciones en cada usart para el envio de datos
 * donde se utilizan las interrupciones.
 */
void USART6Tx_Char(void) {
	USART6->DR = dataToSend6;
	USART6->CR1 &= ~(USART_CR1_TXEIE);

}

void USART6Tx_String(void) {
	char auxData = stringToSend6[posicionActual6];//se apagan las interrupciones por transmision
	if (auxData != 0) {
		USART6->DR = auxData;
		posicionActual6++;
	} else {
		USART6->CR1 &= ~(USART_CR1_TXEIE); //se apagan las interrupciones por transmision
		posicionActual6 = 0; //Se reinicia el contador global
		flagNewData = 0;	//Se permite el ingreso de nueva data.
	}

}

uint8_t getRxData(void) {
	return auxRxData;
}

/* Esta es la función a la que apunta el sistema en el vector de interrupciones.
 * Se debe utilizar usando exactamente el mismo nombre definido en el vector de interrupciones,
 * Al hacerlo correctamente, el sistema apunta a esta función y cuando la interrupción se lanza
 * el sistema inmediatamente salta a este lugar en la memoria
 *
 * Además de esto ahora se agrega una parte donde se verifica si la interrupcion es de transmisión
 * donde se envia a la función correspondiente para que se haga el envio
 */
void USART1_IRQHandler(void) {
	if (USART1->SR & USART_SR_RXNE) {
		/* Limpiamos la bandera que indica que la interrupción se ha generado */
		USART1->SR &= ~USART_SR_RXNE;
		//Auxiliar
		auxRxData = (uint8_t) USART1->DR;

		/* LLamamos a la función que se debe encargar de hacer algo con esta interrupción*/
		USART1Rx_Callback();
	}

	else if (USART1->SR & USART_SR_TXE) {
		/* Limpiamos la bandera que indica que la interrupción se ha generado */
		USART1->SR &= ~USART_SR_TXE;

		/* LLamamos a la función que se debe encargar de hacer algo con esta interrupción*/
		if (tipo1 == 0) {
			USART1Tx_Char();
		} else {
			USART1Tx_String();
		}

	}

}

void USART2_IRQHandler(void) {
	if (USART2->SR & USART_SR_RXNE) {
		/* Limpiamos la bandera que indica que la interrupción se ha generado */
		USART2->SR &= ~USART_SR_RXNE;
		//Auxiliar
		auxRxData = (uint8_t) USART2->DR;
		/* LLamamos a la función que se debe encargar de hacer algo con esta interrupción*/
		USART2Rx_Callback();
	}

	else if (USART2->SR & USART_SR_TXE) {
		/* Limpiamos la bandera que indica que la interrupción se ha generado */
		USART2->SR &= ~USART_SR_TXE;

		/* LLamamos a la función que se debe encargar de hacer algo con esta interrupción*/
		if (tipo2 == 0) {
			USART2Tx_Char();
		} else {
			USART2Tx_String();
		}

	}

}

void USART6_IRQHandler(void) {
	if (USART6->SR & USART_SR_RXNE) {
		/* Limpiamos la bandera que indica que la interrupción se ha generado */
		USART6->SR &= ~USART_SR_RXNE;
		//Auxiliar
		auxRxData = (uint8_t) USART6->DR;
		/* LLamamos a la función que se debe encargar de hacer algo con esta interrupción*/
		USART6Rx_Callback();
	}

	else if (USART6->SR & USART_SR_TXE) {
		/* Limpiamos la bandera que indica que la interrupción se ha generado */
		USART6->SR &= ~USART_SR_TXE;

		/* LLamamos a la función que se debe encargar de hacer algo con esta interrupción*/
		if (tipo6 == 0) {
			USART6Tx_Char();
		} else {
			USART6Tx_String();
		}

	}

}

/* funcion para escribir un solo char */
int writeChar(USART_Handler_t *ptrUsartHandler, int dataToSend) {
	while (!(ptrUsartHandler->ptrUSARTx->SR & USART_SR_TXE)) {
		__NOP();
	}

	ptrUsartHandler->ptrUSARTx->DR = dataToSend;

	return dataToSend;
}

int readChar(USART_Handler_t *ptrUsartHandler) {
	return ptrUsartHandler->ptrUSARTx->DR;
}

void writeString(USART_Handler_t *ptrUsartHandler, char *string) {
	int i = 0;
	while (string[i] != 0) {
		writeChar(ptrUsartHandler, string[i]);
		i++;
	}
}

/* funcion para escribir un solo char */
int writeCharInt(USART_Handler_t *ptrUsartHandler, int dataToSendI) {

	if (ptrUsartHandler->ptrUSARTx == USART2) {
		dataToSend2 = dataToSendI;
		tipo2 = 0;
	} else if (ptrUsartHandler->ptrUSARTx == USART1) {
		dataToSend1 = dataToSendI;
		tipo1 = 0;
	} else if (ptrUsartHandler->ptrUSARTx == USART6) {
		dataToSend6 = dataToSendI;
		tipo6 = 0;
	}

	ptrUsartHandler->ptrUSARTx->CR1 |= (USART_CR1_TXEIE);
	return dataToSendI;
}
//FUncion para mandar u String por medio de interrupciones
void writeStringInt(USART_Handler_t *ptrUsartHandler, char *string) {

	while (flagNewData != 0) { //No deja ingresar nuevos datos hasta que se envien por completo
		__NOP();
	}
	if (ptrUsartHandler->ptrUSARTx == USART2) {
		stringToSend2 = string;
		tipo2 = 1;
	} else if (ptrUsartHandler->ptrUSARTx == USART1) {
		stringToSend1 = string;
		tipo1 = 1;
	} else if (ptrUsartHandler->ptrUSARTx == USART6) {
		stringToSend6 = string;
		tipo6 = 1;
	}
	flagNewData = 1;	//Para que no se envien un string mientras se envia uno anterior
	ptrUsartHandler->ptrUSARTx->CR1 |= (USART_CR1_TXEIE);	//Se inicia la bandera para las interrupcioens de transmisión

}

bool getFlagNewData(void){
	return flagNewData;
}

