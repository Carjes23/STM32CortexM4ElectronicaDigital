/*
 * I2CxDriver.h
 *
 *  Created on: May 22, 2023
 *      Author: daniel
 */

#ifndef I2CXDRIVER_H_
#define I2CXDRIVER_H_

#include "stm32f4xx.h"
#include "stdint.h"

#define I2C_WRITE_DATA		0
#define I2C_READ_DATA		1

#define MAIN_CLOCK_4_MHz_I2C	4
#define MAIN_CLOCK_16_MHz_I2C	16
#define MAIN_CLOCK_20_MHz_I2C	20

#define  I2C_MODE_SM		0
#define  I2C_MODE_FM		1

#define I2C_MODE_SM_SPEED_100KHz	80
#define I2C_MODE_FM_SPEED_400Khz	13

#define I2C_MAX_RISE_TIME_SM		17
#define I2C_MAX_RISE_TIME_FM		5

/*
#define		I2C_1	0
#define 	I2C_2	1
#define 	I2C_3	2
*/

typedef struct{
	I2C_TypeDef	*ptrI2Cx;
	uint8_t		slaveAddress;
	uint8_t		modeI2C;
	uint8_t		dataI2C;
}I2C_Handler_t;

/* Protiopos de las funcioes publicas */
void i2c_config(I2C_Handler_t *ptrHandlerI2C);
void i2c_startTransaction(I2C_Handler_t *ptrHandlerI2C);
void i2c_reStartTransaction(I2C_Handler_t *ptrHandlerI2C);
void i2c_sendSlaveAddresRW(I2C_Handler_t *ptrHandlerI2C, uint8_t slaveAddress, uint8_t readOrWrite);
void i2c_sendMemoryAddress(I2C_Handler_t *ptrHandlerI2C, uint8_t memAddr);
void i2c_sendDataByte(I2C_Handler_t *ptrHandlerI2C, uint8_t dataToWrite);
uint8_t i2c_readDataBye(I2C_Handler_t *ptrHandlerI2C);
void i2c_stopTransaction(I2C_Handler_t *ptrHandlerI2C);
void i2c_sendAck(I2C_Handler_t *ptrHandlerI2C);
void i2c_sendNoAck(I2C_Handler_t *ptrHandlerI2C);
void i2c_readMultipleRegisters(I2C_Handler_t *pHandlerI2C, uint8_t *rxData, uint8_t initialReg, uint8_t regsToRead);

uint8_t i2c_readSingleRegister(I2C_Handler_t *ptrHandlerI2C, uint8_t regToRead);
void i2c_writeSingleRegister(I2C_Handler_t *ptrHandlerI2C, uint8_t regToRead, uint8_t newValue);

void i2c_readMulRegister(I2C_Handler_t *ptrHandlerI2C, uint8_t * regsToRead, uint8_t cantidad, uint8_t * ress);
void i2c_writeMultTimeSameRegister(I2C_Handler_t *ptrHandlerI2C, uint8_t regToRead,  uint8_t * dataWrite, uint8_t cantidad);

void i2c_readMulRegister2(I2C_Handler_t *ptrHandlerI2C, uint8_t * regsToRead, uint8_t cantidad, uint8_t * ress);


#endif /* I2CXDRIVER_H_ */
