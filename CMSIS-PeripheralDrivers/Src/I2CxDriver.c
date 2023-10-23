/*
 * I2CxDriver.c
 *
 *  Created on: May 22, 2023
 *      Author: daniel
 */

#include <stdint.h>
#include "I2CxDriver.h"
#include "PLLDriver.h"
#include <math.h>

/*
 * Recordar que se debe configurar los pines para el I2C (SDA y SCL),
 * para lo cual se necesita el modulo GPIO y los pines configurados en el modo Alternate Function.
 * Además, estos pines deben ser configurados como salidas open-drain
 * y con la resistencia en modo pull - up.
 * */

// Función de configuración para el I2C
void i2c_config(I2C_Handler_t *ptrHandlerI2C) {

	/*  Activar la señal de reloj para el I2C y los PGIOs */

	if (ptrHandlerI2C->ptrI2Cx == I2C1) {
		// Activación RCC para el I2C1
		RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	} else if (ptrHandlerI2C->ptrI2Cx == I2C2) {
		// Activación RCC para el I2C2
		RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
	} else if (ptrHandlerI2C->ptrI2Cx == I2C3) {
		// Activación RCC para el I2C3
		RCC->APB1ENR |= RCC_APB1ENR_I2C3EN;
	}

	/*  Limpiamos los registros del I2C. Reiniciamos el periferico
	 *  de forma que inicia en un estado conocido */

	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_SWRST;
	__NOP();
	ptrHandlerI2C->ptrI2Cx->CR1 &= ~I2C_CR1_SWRST;

	/*  Programación de la señal de reloj (peripheral input clock) para generar el timing correcto
	 * Indicamos cual es la velocidad del reloj principal, que es la señal utilizada por el periférico
	 * para generar la señal de reloj para el bus I2C
	 * */
	// PLCK1 FRECUENCY in MHz se divide entre dos si es mayor a 50 mhz
	uint16_t freq = getFreqPLL();
	if (freq > 50) {
		freq = 16;
	}

	ptrHandlerI2C->ptrI2Cx->CR2 &= ~(0b111111 << I2C_CR2_FREQ_Pos);	// Borramos la configuración actual
	ptrHandlerI2C->ptrI2Cx->CR2 |= (freq << I2C_CR2_FREQ_Pos); //todo

	/*  Configurar el Clock Control Register
	 * Configuramos el modo I2C en el que el sistema funciona
	 * En esta configuración también se incluye la velocidad del reloj
	 * y el tiempo máximo para el cambio de la señal (T-Rise).
	 * Todos comienza con los dos registros en 0
	 * */

	ptrHandlerI2C->ptrI2Cx->CCR = 0;
	ptrHandlerI2C->ptrI2Cx->TRISE = 0;

	/*
	 * Se hacen todos los calculos a travez de formulas logrando que el I2C se ajuste al PLL
	 */
	float TPCLK1 = 1000.f / freq;
	(void) TPCLK1;

	if (ptrHandlerI2C->modeI2C == I2C_MODE_SM) {

		// Estamos en modo "standar" (SM Mode)
		// Seleccionamos el modo estandar
		ptrHandlerI2C->ptrI2Cx->CCR &= ~I2C_CCR_FS;

		uint16_t auxSpeed = 5000 / TPCLK1;

		// Configuramos el registro que se encarga de generar la señal del reloj
		ptrHandlerI2C->ptrI2Cx->CCR |= (auxSpeed << I2C_CCR_CCR_Pos);

		uint16_t auxTrise = 1000 / TPCLK1 + 1;

		// Configuramos el registro que controla el tiempo T - Rise máximo
		ptrHandlerI2C->ptrI2Cx->TRISE |= auxTrise;
	} else if (ptrHandlerI2C->modeI2C == I2C_MODE_FM) {
		// Estamos en modo "Fast" (FM Mode)
		// Seleccionamos el modo Fast
		ptrHandlerI2C->ptrI2Cx->CCR |= I2C_CCR_FS;

		uint16_t auxSpeed = round(2500.f / (TPCLK1 * 3));

		// COnfiguramos el registro que se encarga de generar la señal del reloj
		ptrHandlerI2C->ptrI2Cx->CCR |= (auxSpeed << I2C_CCR_CCR_Pos);

		uint16_t auxTrise = round((300.f / TPCLK1) + 1);

		// Configuramos el registro que controla el tiempo T - Rise máximo
		ptrHandlerI2C->ptrI2Cx->TRISE |= auxTrise;
	}

	/*  Activamos el I2C Peripheral (Módulo I2C).*/

	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_PE;
}

void i2c_stopTransaction(I2C_Handler_t *ptrHandlerI2C) {

	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_STOP;
}

void i2c_startTransaction(I2C_Handler_t *ptrHandlerI2C) {
	/* Definimos una variable auxiliar */
	uint8_t auxByte = 0;
	(void) auxByte;	// Para no generar warning

	/* Verificamos que la línea no está ocupada - bit "busy" en I2C_CR2 */
	while (ptrHandlerI2C->ptrI2Cx->SR2 & I2C_SR2_BUSY) {
		__NOP();
	}

	/* Generamos la señal "start" */
	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_START;

	/* Esperamos a que la bandera del evento "Start" se levante */
	/* Mientras esperamos, el valor de SB es 0, entonces la negación (!) es 1 */
	while (!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_SB)) {
		__NOP();
	}

}

void i2c_reStartTransaction(I2C_Handler_t *ptrHandlerI2C) {
	/* Generamos la señal "start" */
	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_START;

	/* Esperamos a que la bandera del evento "Start" se levante */
	/* Mientras esperamos, el valor de SB es 0, entonces la negación (!) es 1 */
	while (!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_SB)) {
		__NOP();
	}
}

void i2c_sendNoAck(I2C_Handler_t *ptrHandlerI2C) {
	/* Activamos la indicación para no ACK (indicación para el Slave de terminar)
	 * (Debemos escribir cero en la posición ACK del registro de control 1)
	 */
	ptrHandlerI2C->ptrI2Cx->CR1 &= ~I2C_CR1_ACK;
}

void i2c_sendAck(I2C_Handler_t *ptrHandlerI2C) {
	/* Activamos la indicación para  ACK (indicación para el Slave continue)
	 * (Debemos escribir 1 en la posición ACK del registro de control 1)
	 */
	ptrHandlerI2C->ptrI2Cx->CR1 |= I2C_CR1_ACK;
}

void i2c_sendSlaveAddresRW(I2C_Handler_t *ptrHandlerI2C, uint8_t slaveAddress,
		uint8_t readOrWrite) {
	/* 0. Definimos una variable auxiliar */
	uint8_t auxByte = 0;
	(void) auxByte;	// Para no generar warning

	/* 3. Enviamos la dirección del Slave y el bit que indica que deseamos escribir (0)
	 * (en el siguiente paso se envía la dirección de memoria que se desea leer */

	ptrHandlerI2C->ptrI2Cx->DR = (slaveAddress << 1) | readOrWrite;

	/* 3.1 Esperemos hasta que la bandera del evento "addr" se levante
	 * (esto nos indica que la dirección fue enviada satisfactoriamente */
	while (!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_ADDR)) {
		__NOP();
	}

	/* 3.2 Debemos limpiar la bandera de la recepción de ACK de la "addr", para lo cual
	 * debemos leer en secuencia primero el I2C_SR1 y luego I2C_SR2 */
	auxByte = ptrHandlerI2C->ptrI2Cx->SR1;
	auxByte = ptrHandlerI2C->ptrI2Cx->SR2;
}

void i2c_sendMemoryAddress(I2C_Handler_t *ptrHandlerI2C, uint8_t memAddr) {
	/* 4. Enviamos la dirección de memoria que deseamos leer */
	ptrHandlerI2C->ptrI2Cx->DR = memAddr;

	/* 4.1 Esperamos hasta que el byte sea transmitido */
	while (!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_TXE)) {
		__NOP();
	}
}

void i2c_sendDataByte(I2C_Handler_t *ptrHandlerI2C, uint8_t dataToWrite) {
	/* 5. Cargamos el valor que deseamos escribir */

	ptrHandlerI2C->ptrI2Cx->DR = dataToWrite;

	/* 6. Esperamos hasta que el bit sea transmitido */

	while (!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_BTF)) {
		__NOP();
	}
}

uint8_t i2c_readDataBye(I2C_Handler_t *ptrHandlerI2C) {
	/* 9. Esperamos hasta que el byte entrante sea recibido */
	while (!(ptrHandlerI2C->ptrI2Cx->SR1 & I2C_SR1_RXNE)) {
		__NOP();
	}

	ptrHandlerI2C->dataI2C = ptrHandlerI2C->ptrI2Cx->DR;

	return ptrHandlerI2C->dataI2C;
}

uint8_t i2c_readSingleRegister(I2C_Handler_t *ptrHandlerI2C, uint8_t regToRead) {
	//Creamos variable auiliar
	uint8_t auxRead = 0;
	(void) auxRead;

	//1. Generamos la condicion Start
	i2c_startTransaction(ptrHandlerI2C);

	//2. Enviamos la direccion del esclavo y la indicación de escribir
	i2c_sendSlaveAddresRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress,
			I2C_WRITE_DATA);

	//3. Enviamos la direccion de memoria que deseamos leer
	i2c_sendMemoryAddress(ptrHandlerI2C, regToRead);

	//4 Creamos la condicion de reStar
	i2c_reStartTransaction(ptrHandlerI2C);

	//5 Enviamos la dirrecion del escclavo y la indicacion de leer.
	i2c_sendSlaveAddresRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress,
			I2C_READ_DATA);

	//6.Leemos el dato que envia el esclavo.
	auxRead = i2c_readDataBye(ptrHandlerI2C);

	//7. Generamos la condicion de NoAck, para que el master no responda y el slave solo envie un bye
	i2c_sendNoAck(ptrHandlerI2C);

	//8. generamos la condicion de stop
	i2c_stopTransaction(ptrHandlerI2C);

	return auxRead;
}

void i2c_writeSingleRegister(I2C_Handler_t *ptrHandlerI2C, uint8_t regToRead,
		uint8_t newValue) {

	//generamos la condicion start
	i2c_startTransaction(ptrHandlerI2C);

	//2. Enviamos la direccion del esclavo y la indicación de escribir
	i2c_sendSlaveAddresRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress,
			I2C_WRITE_DATA);

	//3. Enviamos la direccion de memoria que deseamos escribir
	i2c_sendMemoryAddress(ptrHandlerI2C, regToRead);

	//4. enviamos el dato que deseamos escribir
	i2c_sendDataByte(ptrHandlerI2C, newValue);

	//5 generamos la condicion stop
	i2c_stopTransaction(ptrHandlerI2C);
}

void i2c_readMulRegister(I2C_Handler_t *ptrHandlerI2C, uint8_t *regsToRead,
		uint8_t cantidad, uint8_t *ress) {
	//Creamos variable auiliar
	uint8_t auxRead = 0;
	(void) auxRead;

	for (int i = 0; i < cantidad; i++) {

		if (i == 0) {
			//1. Generamos la condicion Start
			i2c_startTransaction(ptrHandlerI2C);


		} else {
			//4 Creamos la condicion de reStar
			i2c_reStartTransaction(ptrHandlerI2C);
		}

		//2. Enviamos la direccion del esclavo y la indicación de escribir
		i2c_sendSlaveAddresRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress,
				I2C_WRITE_DATA);

		//3. Enviamos la direccion de memoria que deseamos leer
		i2c_sendMemoryAddress(ptrHandlerI2C, regsToRead[i]);
		//4 Creamos la condicion de reStar
		i2c_reStartTransaction(ptrHandlerI2C);

		//5 Enviamos la dirrecion del escclavo y la indicacion de leer.
		i2c_sendSlaveAddresRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress,
				I2C_READ_DATA);

		//6.Leemos el dato que envia el esclavo.
		auxRead = i2c_readDataBye(ptrHandlerI2C);

		//Lo guardamos en el arreglo
		ress[i] = auxRead;
	}

	//7. Generamos la condicion de NoAck, para que el master no responda y el slave solo envie un bye
	i2c_sendNoAck(ptrHandlerI2C);

	//8. generamos la condicion de stop
	i2c_stopTransaction(ptrHandlerI2C);

}

void i2c_writeMultTimeSameRegister(I2C_Handler_t *ptrHandlerI2C,
		uint8_t regToRead, uint8_t *dataWrite, uint8_t cantidad) {
	//generamos la condicion start
	i2c_startTransaction(ptrHandlerI2C);




	for (int i = 0; i < cantidad; i++) {
		if (i != 0) {
			//4 Creamos la condicion de reStar
			i2c_reStartTransaction(ptrHandlerI2C);
		}
		//2. Enviamos la direccion del esclavo y la indicación de escribir
		i2c_sendSlaveAddresRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress,I2C_WRITE_DATA);
		//3. Enviamos la direccion de memoria que deseamos escribir
		i2c_sendMemoryAddress(ptrHandlerI2C, regToRead);

		//4. enviamos el dato que deseamos escribir
		i2c_sendDataByte(ptrHandlerI2C, dataWrite[i]);

		__NOP();
	}

	//5 generamos la condicion stop
	i2c_stopTransaction(ptrHandlerI2C);
}

void i2c_readMulRegister2(I2C_Handler_t *ptrHandlerI2C, uint8_t *regsToRead,
		uint8_t cantidad, uint8_t *ress) {
	//Creamos variable auiliar
	uint8_t auxRead = 0;
	(void) auxRead;

	for (int i = 0; i < cantidad; i++) {

		if (i == 0) {
			//1. Generamos la condicion Start
			i2c_startTransaction(ptrHandlerI2C);


		} else {
			//4 Creamos la condicion de reStar
			i2c_reStartTransaction(ptrHandlerI2C);
		}

		//2. Enviamos la direccion del esclavo y la indicación de escribir
		i2c_sendSlaveAddresRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress,
				I2C_WRITE_DATA);

		//3. Enviamos la direccion de memoria que deseamos leer
		i2c_sendMemoryAddress(ptrHandlerI2C, regsToRead[i]);
		//4 Creamos la condicion de reStar
		i2c_reStartTransaction(ptrHandlerI2C);

		//5 Enviamos la dirrecion del escclavo y la indicacion de leer.
		i2c_sendSlaveAddresRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress,
				I2C_READ_DATA);

		//6.Leemos el dato que envia el esclavo.
		auxRead = i2c_readDataBye(ptrHandlerI2C);

		//Lo guardamos en el arreglo
		ress[i] = auxRead;
	}

	//7. Generamos la condicion de NoAck, para que el master no responda y el slave solo envie un bye
	i2c_sendNoAck(ptrHandlerI2C);

	//8. generamos la condicion de stop
	i2c_stopTransaction(ptrHandlerI2C);

}

/* Función para leer registros sucesivos
 * NOTA: No funciona si las direcciones no incrementan en 1 */
void i2c_readMultipleRegisters(I2C_Handler_t *pHandlerI2C, uint8_t *rxData, uint8_t initialReg, uint8_t regsToRead){

	/* 1. Generar un ACK */
	i2c_sendAck(pHandlerI2C);

	/* 2. Generamos la condición Start */
	i2c_startTransaction(pHandlerI2C);

	/* 3. Enviamos la dirección del esclavo y la indicación de ESCRIBIR */
	i2c_sendSlaveAddresRW(pHandlerI2C, pHandlerI2C->slaveAddress, I2C_WRITE_DATA);

	/* 4. Enviamos la dirección de memoria que deseamos leer */
	i2c_sendMemoryAddress(pHandlerI2C, initialReg);

	/* 5. Creamos una condición re reStart */
	i2c_reStartTransaction(pHandlerI2C);

	/* 6. Enviamos la dirección del esclavo y la indicación de LEER */
	i2c_sendSlaveAddresRW(pHandlerI2C, pHandlerI2C->slaveAddress, I2C_READ_DATA);

	/* 6. Almacenar la información en el arreglo ingresado */
	for(uint8_t i = 0; i < (regsToRead); i++){
		rxData[i] = i2c_readDataBye(pHandlerI2C);
	}

	/* 7. Generamos la condición de NoAck, para que el Master no responda y envíe el último byte*/
	i2c_sendNoAck(pHandlerI2C);

	/* 8. Generamos la condición Stop, para que el slave se detenga */
	i2c_stopTransaction(pHandlerI2C);
}
