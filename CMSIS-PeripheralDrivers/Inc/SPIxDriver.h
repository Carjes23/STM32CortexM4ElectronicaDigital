/*
 ******************************************************************************
 * @file           : SPIxDriver.h
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

#include <stdio.h>
#include <stm32f4xx.h>

#ifndef INC_SPIXDRIVER_H_
#define INC_SPIXDRIVER_H_

#define SPI_MODE_SLAVE 				0
#define SPI_MODE_MASTER 			1

#define SPI_CPOLCPHA_MODE_00		0 //CPOL 0, CPHA 0
#define SPI_CPOLCPHA_MODE_01		1 //CPOL 0, CPHA 1
#define SPI_CPOLCPHA_MODE_10		2 //CPOL 1, CPHA 0
#define SPI_CPOLCPHA_MODE_11		3 //CPOL 1, CPHA 1

#define SPI_BAUDRATE_DIV2 			0
#define SPI_BAUDRATE_DIV4 			1
#define SPI_BAUDRATE_DIV8 			2
#define SPI_BAUDRATE_DIV16 			3
#define SPI_BAUDRATE_DIV32 			4
#define SPI_BAUDRATE_DIV64 			5
#define SPI_BAUDRATE_DIV128 		6
#define SPI_BAUDRATE_DIV256 		7

#define SPI_DATASIZE_8BIT			0
#define SPI_DATASIZE_16BIT 			1

#define SPI_FIRSTBIT_MSB 			0
#define SPI_FIRSTBIT_LSB 			1

#define SPI_USE_TX_ONLY 			0
#define SPI_USE_RX_ONLY 			1
#define SPI_USE_TX_RX   			2
#define SPI_USE_DISABLE 			3



/*
 * Estructura con la configuración necesaria para el spi
 */
typedef struct
{
    uint8_t Mode;
    uint8_t BaudRatePrescaler;
    uint8_t DataSize;
    uint8_t CPOLCPHA;
    uint8_t FirstBit;
    uint8_t NSS;
    uint8_t UseCase;
} SPI_Config_t;

/* Handler SPI*/

typedef struct
{
    SPI_TypeDef *ptrSPIx;
    SPI_Config_t SPI_Config;
} SPI_Handler_t;

// Function prototypes
void SPI_Config(SPI_Handler_t *ptrSPI_Handler);
int SPI_WriteChar(SPI_Handler_t *ptrSPI_Handler, uint8_t dataToSend);
void SPI_Transmit(SPI_Handler_t *ptrSPI_Handler, uint8_t *data, uint16_t size);

#endif /* INC_SPIXDRIVER_H_ */
