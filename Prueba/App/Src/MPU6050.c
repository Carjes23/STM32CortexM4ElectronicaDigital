/*
 * MPU6050.c
 *
 *  Created on: 20/05/2023
 *      Author: Emmanuel
 */

#include "I2CxDriver.h"
#include "MPU6050.h"

/* Función para almacenar las medidas de todos los acelerómetros
 * y giroscopios en un arreglo:
 * Primero los acelerómetros X, Y y Z
 * Luego los giroscopios X, Y y Z*/
void mpu6050_readAll(I2C_Handler_t *ptrI2C, int16_t *arreglo){

	uint8_t auxArray[14];

	// Se leen los 14 registros correspondiendes a los acelerómetros, temperatura y giroscopios
	i2c_readMultipleRegisters(ptrI2C, auxArray, ACCEL_XOUT_H, 14);

	// Se organizan las medidas en el arreglo ingresado
	arreglo[0] = (auxArray[0] << 8) | auxArray[1];		// AccX
	arreglo[1] = (auxArray[2] << 8) | auxArray[3];		// AccY
	arreglo[2] = (auxArray[4] << 8) | auxArray[5];		// AccZ
	arreglo[3] = (auxArray[8] << 8) | auxArray[9];		// GyrX
	arreglo[4] = (auxArray[10] << 8) | auxArray[11];	// GyrY
	arreglo[5] = (auxArray[12] << 8) | auxArray[13];	// GyrZ
}

/* Función que organiza un arreglo con las medidas de los Accel. */
void mpu6050_readAccel(I2C_Handler_t *ptrI2C, int16_t *arreglo){

	uint8_t auxArray[6] = {0};

	// Se leen los registros correspondientes a los acelerómetros
	i2c_readMultipleRegisters(ptrI2C, auxArray, ACCEL_XOUT_H, 6);

	// Se organizan las medidas en el arreglo ingresado
	arreglo[0] = (uint16_t)(auxArray[0] << 8) | auxArray[1];	// X
	arreglo[1] = (uint16_t)(auxArray[2] << 8) | auxArray[3];	// Y
	arreglo[2] = (uint16_t)(auxArray[4] << 8) | auxArray[5];	// Z
}

/* Función que organiza un arreglo con las medidas de los Gyros. */
void mpu6050_readGyro(I2C_Handler_t *ptrI2C, int16_t *arreglo){

	uint8_t auxArray[6] = {0};

	// Se leen los registros correspondientes a los giroscopios
	i2c_readMultipleRegisters(ptrI2C, auxArray, GYRO_XOUT_H, 6);

	// Se organizan las medidas en el arreglo ingresado
	arreglo[0] = (auxArray[0] << 8) | auxArray[1];	// X
	arreglo[1] = (auxArray[2] << 8) | auxArray[3];	// Y
	arreglo[2] = (auxArray[4] << 8) | auxArray[5];	// Z
}

/* Función que retorna la medida del Accel X */
int16_t mpu6050_readAccelX(I2C_Handler_t *ptrI2C){

	uint8_t high = i2c_readSingleRegister(ptrI2C, ACCEL_XOUT_H);
	uint8_t low = i2c_readSingleRegister(ptrI2C, ACCEL_XOUT_L);

	// Se organiza medida
	return (int16_t)((high << 8) | low);
}

/* Función que retorna la medida del Accel Y */
int16_t mpu6050_readAccelY(I2C_Handler_t *ptrI2C){

	uint8_t high = i2c_readSingleRegister(ptrI2C, ACCEL_YOUT_H);
	uint8_t low = i2c_readSingleRegister(ptrI2C, ACCEL_YOUT_L);

	// Se organiza medida
	return (int16_t)((high << 8) | low);
}

/* Función que retorna la medida del Accel Z */
int16_t mpu6050_readAccelZ(I2C_Handler_t *ptrI2C){

	uint8_t high = i2c_readSingleRegister(ptrI2C, ACCEL_ZOUT_H);
	uint8_t low = i2c_readSingleRegister(ptrI2C, ACCEL_ZOUT_L);

	// Se organiza medida
	return (int16_t)((high << 8) | low);
}

/* Función que retorna la medida del Gyro X */
int16_t mpu6050_readGyroX(I2C_Handler_t *ptrI2C){

	uint8_t high = i2c_readSingleRegister(ptrI2C, GYRO_XOUT_H);
	uint8_t low = i2c_readSingleRegister(ptrI2C, GYRO_XOUT_L);

	// Se organiza medida
	return (int16_t)((high << 8) | low);
}

/* Función que retorna la medida del Gyro Y */
int16_t mpu6050_readGyroY(I2C_Handler_t *ptrI2C){

	uint8_t high = i2c_readSingleRegister(ptrI2C, GYRO_YOUT_H);
	uint8_t low = i2c_readSingleRegister(ptrI2C, GYRO_YOUT_L);

	// Se organiza medida
	return (int16_t)((high << 8) | low);
}

/* Función que retorna la medida del Gyro Z */
int16_t mpu6050_readGyroZ(I2C_Handler_t *ptrI2C){

	uint8_t high = i2c_readSingleRegister(ptrI2C, GYRO_ZOUT_H);
	uint8_t low = i2c_readSingleRegister(ptrI2C, GYRO_ZOUT_L);

	// Se organiza medida
	return (int16_t)((high << 8) | low);
}

/* Función para obtener los valores normalizados */
void mpu6050_getNormData(I2C_Handler_t *ptrI2C, float *arreglo){

	uint8_t auxArray[14] = {0};

	// Se leen todos los registros
	i2c_readMultipleRegisters(ptrI2C, auxArray, ACCEL_XOUT_H, 14);

	arreglo[0] = (int16_t)((auxArray[0] << 8) | auxArray[1]) * 2.0/65535.0;
	arreglo[1] = (int16_t)((auxArray[2] << 8) | auxArray[3]) * 2.0/65535.0;
	arreglo[2] = (int16_t)((auxArray[4] << 8) | auxArray[5]) * 2.0/65535.0;
	arreglo[3] = (int16_t)((auxArray[8] << 8) | auxArray[9]) * 2.0/65535.0;
	arreglo[4] = (int16_t)((auxArray[10] << 8) | auxArray[11]) * 2.0/65535.0;
	arreglo[5] = (int16_t)((auxArray[12] << 8) | auxArray[13]) * 2.0/65535.0;
}

/* Para obtener las medidas con sus unidades */
void mpu6050_getData(I2C_Handler_t* mpuHandler, float *pData){

	uint8_t dataArray[14] = {0};

	i2c_readMultipleRegisters(mpuHandler, dataArray, 14, ACCEL_XOUT_H);

	int16_t _accelX = dataArray[0] << 8 | dataArray[1];
	float AccelX = _accelX*4.0/65535.0;

	int16_t _accelY = dataArray[2] << 8 | dataArray[3];
	float AccelY = _accelY*4.0/65535.0;

	int16_t _accelZ = dataArray[4] << 8 | dataArray[5];
	float AccelZ = _accelZ*4.0/65535.0;

	int16_t _gyrosX = dataArray[8] << 8 | dataArray[9];
	float GyrosX = _gyrosX*500.0/65535.0;

	int16_t _gyrosY = dataArray[10] << 8 | dataArray[11];
	float GyrosY = _gyrosY*500.0/65535.0;

	int16_t _gyrosZ = dataArray[12] << 8 | dataArray[13];
	float GyrosZ = _gyrosZ*500.0/65535.0;

	*(pData+0) = AccelX;
	*(pData+1) = AccelY;
	*(pData+2) = AccelZ;
	*(pData+3) = GyrosX;
	*(pData+4) = GyrosY;
	*(pData+5) = GyrosZ;
}

/* - Funciones relacionadas con la calibración del instrumento - */
/* Para saber los offset actuales */
void mpu6050_getOffsets(I2C_Handler_t* mpuHandler, int16_t *pOffset){

	// Accel X
	uint8_t xa_offs_h = i2c_readSingleRegister(mpuHandler, XA_OFFSET_H);
	uint8_t xa_offs_l = i2c_readSingleRegister(mpuHandler, XA_OFFSET_L);
	int16_t xa_offs = xa_offs_h << 8 | xa_offs_l;
	*(pOffset+0) = xa_offs;

	// Accel Y
	uint8_t ya_offs_h = i2c_readSingleRegister(mpuHandler, YA_OFFSET_H);
	uint8_t ya_offs_l = i2c_readSingleRegister(mpuHandler, YA_OFFSET_L);
	int16_t ya_offs = ya_offs_h << 8 | ya_offs_l;
	*(pOffset+1) = ya_offs;

	// Accel Z
	uint8_t za_offs_h = i2c_readSingleRegister(mpuHandler, ZA_OFFSET_H);
	uint8_t za_offs_l = i2c_readSingleRegister(mpuHandler, ZA_OFFSET_L);
	int16_t za_offs = za_offs_h << 8 | za_offs_l;
	*(pOffset+2) = za_offs;

	// Gyro X
	uint8_t xg_offs_h = i2c_readSingleRegister(mpuHandler, XG_OFFSET_H);
	uint8_t xg_offs_l = i2c_readSingleRegister(mpuHandler, XG_OFFSET_L);
	int16_t xg_offs = xg_offs_h << 8 | xg_offs_l;
	*(pOffset+3) = xg_offs;

	// Gyro Y
	uint8_t yg_offs_h = i2c_readSingleRegister(mpuHandler, YG_OFFSET_H);
	uint8_t yg_offs_l = i2c_readSingleRegister(mpuHandler, YG_OFFSET_L);
	int16_t yg_offs = yg_offs_h << 8 | yg_offs_l;
	*(pOffset+4) = yg_offs;

	// Gyro Z
	uint8_t zg_offs_h = i2c_readSingleRegister(mpuHandler, ZG_OFFSET_H);
	uint8_t zg_offs_l = i2c_readSingleRegister(mpuHandler, ZG_OFFSET_L);
	int16_t zg_offs = zg_offs_h << 8 | zg_offs_l;
	*(pOffset+5) = zg_offs;
}

/* Función para establecer los offset */
void mpu6050_setOffsets(I2C_Handler_t* mpuHandler, int16_t *pOffset){

	// Se preparan los valores para ser enviados por I2C
	uint8_t ax_offs_h = *(pOffset+0) >> 8;
	uint8_t ax_offs_l = *(pOffset+0);
	uint8_t ay_offs_h = *(pOffset+1) >> 8;
	uint8_t ay_offs_l = *(pOffset+1);
	uint8_t az_offs_h = *(pOffset+2) >> 8;
	uint8_t az_offs_l = *(pOffset+2);
	uint8_t gx_offs_h = *(pOffset+3) >> 8;
	uint8_t gx_offs_l = *(pOffset+3);
	uint8_t gy_offs_h = *(pOffset+4) >> 8;
	uint8_t gy_offs_l = *(pOffset+4);
	uint8_t gz_offs_h = *(pOffset+5) >> 8;
	uint8_t gz_offs_l = *(pOffset+5);

	// Se escriben los valores en los registros correspondientes
	i2c_writeSingleRegister(mpuHandler, XA_OFFSET_H, ax_offs_h);
	i2c_writeSingleRegister(mpuHandler, XA_OFFSET_L, ax_offs_l);
	i2c_writeSingleRegister(mpuHandler, YA_OFFSET_H, ay_offs_h);
	i2c_writeSingleRegister(mpuHandler, YA_OFFSET_L, ay_offs_l);
	i2c_writeSingleRegister(mpuHandler, ZA_OFFSET_H, az_offs_h);
	i2c_writeSingleRegister(mpuHandler, ZA_OFFSET_L, az_offs_l);
	i2c_writeSingleRegister(mpuHandler, XG_OFFSET_H, gx_offs_h);
	i2c_writeSingleRegister(mpuHandler, XG_OFFSET_L, gx_offs_l);
	i2c_writeSingleRegister(mpuHandler, YG_OFFSET_H, gy_offs_h);
	i2c_writeSingleRegister(mpuHandler, YG_OFFSET_L, gy_offs_l);
	i2c_writeSingleRegister(mpuHandler, ZG_OFFSET_H, gz_offs_h);
	i2c_writeSingleRegister(mpuHandler, ZG_OFFSET_L, gz_offs_l);
}
