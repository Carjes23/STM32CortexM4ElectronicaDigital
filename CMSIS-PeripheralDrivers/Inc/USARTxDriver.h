/*
 * USARTxDriver.h
 *
 *  Created on: Apr 6, 2022
 *      Author: namontoy
 */

#include <stdio.h>
#include <stdbool.h>

#ifndef USARTXDRIVER_H_
#define USARTXDRIVER_H_

#define USART_MODE_TX		0
#define USART_MODE_RX		1
#define USART_MODE_RXTX		2
#define USART_MODE_DISABLE	3

#define USART_BAUDRATE_9600		0
#define USART_BAUDRATE_19200	1
#define USART_BAUDRATE_115200	2

#define USART_DATASIZE_8BIT		0
#define USART_DATASIZE_9BIT		1

#define USART_PARITY_NONE	0
#define USART_PARITY_ODD	1
#define USART_PARITY_EVEN	2

#define USART_STOPBIT_1		0
#define USART_STOPBIT_0_5	1
#define USART_STOPBIT_2		2
#define USART_STOPBIT_1_5	3

/* Estructura para la configuración de la comunicacion:
 * Velocidad (baudrate)
 * Tamaño de los datos
 * Control de errores
 * Bit de parada
 */


typedef struct
{
	uint8_t USART_mode;
	uint32_t USART_baudrate;
	uint8_t USART_datasize;
	uint8_t USART_parity;
	uint8_t USART_stopbits;
	uint8_t USART_RX_Int_Ena;
	uint8_t USART_TX_Int_Ena;
}USART_Config_t;

/*
 * Definicion del Handler para un USART:
 * - Estructura que contiene los SFR que controlan el periferico
 * - Estructura que contiene la configuración especifica del objeto
 * - Buffer de recepcion de datos
 * - Elemento que indica cuantos datos se recibieron
 * - Buffer de transmision de datos
 * - Elemento que indica cuantos datos se deben enviar.
 */
typedef struct
{
	USART_TypeDef	*ptrUSARTx;
	USART_Config_t	USART_Config;
	uint8_t			receptionBuffer[64];
	uint8_t			dataInputSize;
	uint8_t			transmisionBuffer[64];
	uint8_t			dataOutputSize;
}USART_Handler_t;



/* Definicion de los prototipos para las funciones del USART */
void USART_Config(USART_Handler_t *ptrUsartHandler);

void USART1Rx_Callback(void); /* Esta función debe ser sobre-escrita en el main para que el sistema funcione*/
void USART2Rx_Callback(void);
void USART6Rx_Callback(void);
void USART1Tx_Char(void);
void USART1Tx_String(void);
void USART2Tx_Char(void);
void USART2Tx_String(void);
void USART6Tx_Char(void);
void USART6Tx_String(void);

int writeChar(USART_Handler_t *ptrUsartHandler, int dataToSend );
int readChar(USART_Handler_t *ptrUsartHandler);
void writeString(USART_Handler_t *ptrUsartHandler, char* string);
int writeCharInt(USART_Handler_t *ptrUsartHandler, int dataToSendI );
void writeStringInt(USART_Handler_t *ptrUsartHandler, char* string);
bool getFlagNewData(void);

uint8_t getRxData(void);

#endif /* USARTXDRIVER_H_ */
