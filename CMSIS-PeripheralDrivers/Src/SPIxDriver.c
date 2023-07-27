/*
 ******************************************************************************
 * @file           : SPIxDriver.c
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

#include <stm32f4xx.h>
#include "SPIxDriver.h"
#include "PLLDriver.h"
/**
 * Configurando el SPI.
 * Recordar que siempre se debe comenzar con activar la señal de reloj
 * del periferico que se está utilizando.
 */
void SPI_Config(SPI_Handler_t *ptrSPI_Handler){
	/* 1. Activamos la señal de reloj que viene desde el BUS al que pertenece el periferico */
	/* Lo debemos hacer para cada uno de las pisbles opciones que tengamos (SPI1, SPI2, SPI3, SPI4, SPI5) */
	if(ptrSPI_Handler->ptrSPIx == SPI1){
		RCC -> APB2ENR |= RCC_APB2ENR_SPI1EN;
	}

	else if(ptrSPI_Handler->ptrSPIx == SPI2){
		RCC -> APB1ENR |= RCC_APB1ENR_SPI2EN;
	}

	else if(ptrSPI_Handler->ptrSPIx == SPI3){
		RCC -> APB1ENR |= RCC_APB1ENR_SPI3EN;
	}

	else if(ptrSPI_Handler->ptrSPIx == SPI4){
		RCC -> APB2ENR |= RCC_APB2ENR_SPI4EN;
	}

	else if(ptrSPI_Handler->ptrSPIx == SPI5){
		RCC -> APB2ENR |= RCC_APB2ENR_SPI5EN;
	}

	// 2. COnfiguraciones de los registros CR1

	ptrSPI_Handler->ptrSPIx->CR1 = 0;
	ptrSPI_Handler->ptrSPIx->CR2 = 0;

	// configuramos la polaridad

	if(ptrSPI_Handler->SPI_Config.CPOLCPHA == SPI_CPOLCPHA_MODE_00){
		ptrSPI_Handler->ptrSPIx->CR1 &= ~(SPI_CR1_CPHA);
		ptrSPI_Handler->ptrSPIx->CR1 &= ~(SPI_CR1_CPOL);
	}

	else if(ptrSPI_Handler->SPI_Config.CPOLCPHA == SPI_CPOLCPHA_MODE_01){
		ptrSPI_Handler->ptrSPIx->CR1 |= (SPI_CR1_CPHA);
		ptrSPI_Handler->ptrSPIx->CR1 &= ~(SPI_CR1_CPOL);
	}

	else if(ptrSPI_Handler->SPI_Config.CPOLCPHA == SPI_CPOLCPHA_MODE_01){
		ptrSPI_Handler->ptrSPIx->CR1 &= ~(SPI_CR1_CPHA);
		ptrSPI_Handler->ptrSPIx->CR1 |= (SPI_CR1_CPOL);
	}

	else if(ptrSPI_Handler->SPI_Config.CPOLCPHA == SPI_CPOLCPHA_MODE_11){
		ptrSPI_Handler->ptrSPIx->CR1 |= (SPI_CR1_CPHA);
		ptrSPI_Handler->ptrSPIx->CR1 |= (SPI_CR1_CPOL);
	}

	//Selecionamos el modo Slave o master

	//Limpiamos que es lo mismo que ser slave
	ptrSPI_Handler->ptrSPIx->CR1 &= ~(SPI_CR1_MSTR);
	//preguntamos si es master y si asi lo es ponemos lo necesrio
	if(ptrSPI_Handler->SPI_Config.Mode){
		ptrSPI_Handler->ptrSPIx->CR1 |= (SPI_CR1_MSTR);
	}

	//Seleccionamos el baud rate

	//limpiamos

	ptrSPI_Handler->ptrSPIx->CR1 &= ~(0b111 << SPI_CR1_BR_Pos);

	//ponemos el valor
	uint8_t aux = ptrSPI_Handler->SPI_Config.BaudRatePrescaler;

	if(aux<=7 && aux >= 0){
		ptrSPI_Handler->ptrSPIx->CR1 |= (aux << SPI_CR1_BR_Pos);
	}
	//Selecionar Formato o MSB priomero o al reves

	//Limpiamos que es lo mismo que primero trasmitir el MSB
	ptrSPI_Handler->ptrSPIx->CR1 &= ~(SPI_CR1_LSBFIRST);
	//preguntamos si es primero el LSB y si asi lo es, ponemos lo necesrio
	if(ptrSPI_Handler->SPI_Config.FirstBit){
		ptrSPI_Handler->ptrSPIx->CR1 |= (SPI_CR1_LSBFIRST);
	}

	//DataFormat 8 o 16 bits

	//Limpiamos que es lo mismo que una trasmision de 8 bits
	ptrSPI_Handler->ptrSPIx->CR1 &= ~(SPI_CR1_DFF);
	//preguntamos si es de 16 bits y si asi lo es, ponemos lo necesrio
	if(ptrSPI_Handler->SPI_Config.DataSize){
		ptrSPI_Handler->ptrSPIx->CR1 |= (SPI_CR1_DFF);
	}
	//Seleccionamos el modo
	switch (ptrSPI_Handler->SPI_Config.UseCase) {
        case SPI_USE_TX_ONLY:
            ptrSPI_Handler->ptrSPIx->CR1 &= ~(1 << 10); // Clear RXONLY bit
            break;
        case SPI_USE_RX_ONLY:
            ptrSPI_Handler->ptrSPIx->CR1 |= (1 << 10); // Set RXONLY bit
            break;
        case SPI_USE_TX_RX:
            ptrSPI_Handler->ptrSPIx->CR1 &= ~(1 << 10); // Clear RXONLY bit
            break;
        default:
            break;
    }

	/* 9. Configuramos para que el control del pin SS (selección del slave
	 *    sea controlado por software (nosotros debemos hacer ese control),
	 *    de la otra forma, será el hardware el que controla la selección del slave */
	ptrSPI_Handler->ptrSPIx->CR1 |= SPI_CR1_SSM;
	ptrSPI_Handler->ptrSPIx->CR1 |= SPI_CR1_SSI;

	/* 10. Activamos el periférico SPI */
	ptrSPI_Handler->ptrSPIx->CR1 |= SPI_CR1_SPE;


}

/* funcion para escribir un solo char */
int SPI_WriteChar(SPI_Handler_t *ptrSPI_Handler, uint8_t dataToSend){

	uint8_t auxData;
	(void) auxData;

    // Esperamos a que el buffer TXE este vacio
    while (!(ptrSPI_Handler->ptrSPIx->SR & SPI_SR_TXE)){
        __NOP();
    }

    // Enviamos el dato por el SPI DR
    ptrSPI_Handler->ptrSPIx->DR = dataToSend;

    //Esperamos a que el buffer este vacio
    while (!(ptrSPI_Handler->ptrSPIx->SR & SPI_SR_TXE)){
        __NOP();
    }


    // Esperamos hasta que deje de estar ocuapdo para que los datos no se sobrepongan
    while (ptrSPI_Handler->ptrSPIx->SR & SPI_SR_BSY){
        __NOP();
    }

//    //Con esto limpiamos los datos.



    auxData = ptrSPI_Handler->ptrSPIx->DR;
	auxData = ptrSPI_Handler->ptrSPIx->SR;

    return dataToSend;
}


void SPI_Transmit(SPI_Handler_t *ptrSPI_Handler, uint8_t *data, uint16_t size){
	//Para transmitir mas de un dato nos apoyamos de la función antes creada.
    for (uint16_t i = 0; i < size; i++) {
        // Send the data
        SPI_WriteChar(ptrSPI_Handler,*(data+i));
    }
}
