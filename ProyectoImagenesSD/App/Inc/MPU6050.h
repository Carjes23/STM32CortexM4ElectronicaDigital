/*
 * MPU6050.h
 *
 *  Created on: 20/05/2023
 *      Author: Emmanuel
 */

#ifndef MPU6050_H_
#define MPU6050_H_

// MPU6050 Registro --------- Dirección (decimal)
#define XA_OFFSET_H			  6
#define XA_OFFSET_L			  7
#define YA_OFFSET_H			  8
#define YA_OFFSET_L		  	  9
#define ZA_OFFSET_H			  10
#define ZA_OFFSET_L			  11

#define SELF_TEST_X           13
#define SELF_TEST_Y           14
#define SELF_TEST_Z           15
#define SELF_TEST_A           16

#define XG_OFFSET_H			  19
#define XG_OFFSET_L			  20
#define YG_OFFSET_H			  21
#define YG_OFFSET_L			  22
#define ZG_OFFSET_H			  23
#define ZG_OFFSET_L			  24

#define SMPLRT_DIV            25
#define CONFIG                26
#define GYRO_CONFIG           27
#define ACCEL_CONFIG          28

#define FIFO_EN               35

#define INT_PIN_CFG           55
#define INT_ENABLE            56
#define INT_STATUS            58

#define ACCEL_XOUT_H          59
#define ACCEL_XOUT_L          60
#define ACCEL_YOUT_H          61
#define ACCEL_YOUT_L          62
#define ACCEL_ZOUT_H          63
#define ACCEL_ZOUT_L          64
#define TEMP_OUT_H            65
#define TEMP_OUT_L            66
#define GYRO_XOUT_H           67
#define GYRO_XOUT_L           68
#define GYRO_YOUT_H           69
#define GYRO_YOUT_L           70
#define GYRO_ZOUT_H           71
#define GYRO_ZOUT_L           72

#define PWR_MGMT_1            107
#define PWR_MGMT_2            108

#define WHO_AM_I              117

/* Prototipo de las funciones */
void mpu6050_readAll(I2C_Handler_t *ptrI2C, int16_t *arreglo);		// Todos los Accel y giro
void mpu6050_readAccel(I2C_Handler_t *ptrI2C, int16_t *arreglo);	// Todos los ejes
void mpu6050_readGyro(I2C_Handler_t *ptrI2C, int16_t *arreglo);		// Todos los ejes

int16_t mpu6050_readAccelX(I2C_Handler_t *ptrI2C);
int16_t mpu6050_readAccelY(I2C_Handler_t *ptrI2C);
int16_t mpu6050_readAccelZ(I2C_Handler_t *ptrI2C);

int16_t mpu6050_readGyroX(I2C_Handler_t *ptrI2C);
int16_t mpu6050_readGyroY(I2C_Handler_t *ptrI2C);
int16_t mpu6050_readGyroZ(I2C_Handler_t *ptrI2C);

void mpu6050_getNormData(I2C_Handler_t *ptrI2C, float *arreglo);
void mpu6050_getData(I2C_Handler_t* mpuHandler, float *pData);

void mpu6050_getOffsets(I2C_Handler_t* mpuHandler, int16_t *pOffset);
void mpu6050_setOffsets(I2C_Handler_t* mpuHandler, int16_t *pOffset);

#endif /* MPU6050_H_ */
