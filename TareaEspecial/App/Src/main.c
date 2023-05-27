/*
 ******************************************************************************
 * @file           : main.c
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

/*
 * Tarea Especial.
 *
 * 4		1003174419 -> Se trabaja con el USART6
 *
 * Mi cable Negro tierra, Verde Tx pc -> Conectar en Rx-> PA12, Blando -> Rx pc -> Contectar en Tx PA11
 */
#include <stdint.h>
#include <stm32f4xx.h>
#include "GPIOxDriver.h"
#include "USARTxDriver.h"
#include "BasicTimer.h"
#include <stdbool.h>
#include "SysTick.h"
#include <arm_math.h>
#include "PwmDriver.h"
#include "SPIxDriver.h"
#include "ILI9341.h"
#include "PLLDriver.h"
#include "I2CxDriver.h"
#include "LCDI2C.h"

#define ACCEL_ADDRESS 0x1D; //0xD -> Rireccion del Accel con Logic_1
#define ACCEL_XOUT_L 50
#define ACCEL_XOUT_H 51
#define ACCEL_YOUT_L 52
#define ACCEL_YOUT_H 53
#define ACCEL_ZOUT_L 54
#define ACCEL_ZOUT_H 55
#define ACCEL_DATA_FORMAT 49
#define ACCEL_BW_RATE 0x2C

#define SENSORS_GRAVITY_EARTH 9.80665F
#define ADXL345_MG2G_MULTIPLIER 0.004F
#define FACTORESCALADO SENSORS_GRAVITY_EARTH * ADXL345_MG2G_MULTIPLIER

#define POWER_CTL 0x2D
#define WHO_AM_I 0

#define ADXL345_RANGE_16_G 0b11 ///< +/- 16g
#define ADXL345_RANGE_8_G 0b10  ///< +/- 8g
#define ADXL345_RANGE_4_G 0b01  ///< +/- 4g
#define ADXL345_RANGE_2_G 0b00   ///< +/- 2g (default value)

#define LCD_ADDRES 0x21 //O 0x21

float meanFunction(float *numbers, int cantidad);
void cambiarLed(void);

uint8_t AccelX_low = 0;
uint8_t AccelX_high = 0;
int16_t AccelX = 0;
float AccelXEsc = 0.0f;

uint8_t AccelY_low = 0;
uint8_t AccelY_high = 0;
int16_t AccelY = 0;
float AccelYEsc = 0.0f;

uint8_t AccelZ_low = 0;
uint8_t AccelZ_high = 0;
int16_t AccelZ = 0;
float AccelZEsc = 0.0f;

bool banderaLedUsuario = 0; //Bandera TIM2 el cual nos da el blinking del LD2
bool flagDatos = 0;
bool flag2Segundos = 0;
bool flag2SegundosTer = 0;

uint8_t usart6DataReceived = 0; //Dato recibido por el usart dos

char bufferMsg[256] = { 0 }; //Para guardar l configuracion a mandar por el usart
uint8_t i2cBuffer = 0;

//Handlers utilizados para manipular los perifericos
BasicTimer_Handler_t handlerTim2 = { 0 }; //Timer para el blinking
GPIO_Handler_t handlerUserLedPin = { 0 }; // Para manipular el led de usuario

PWM_Handler_t pwmtim3x = { 0 }; //Para configurar el PWM en el timer 3 para x
GPIO_Handler_t pwm3xpin = { 0 }; //Pin que nos entrega la señal PWM para x pc6

USART_Handler_t USART6Handler = { 0 }; //Para manejar el USART6
GPIO_Handler_t tx6pin = { 0 }; //Para manipular el pin de trasminsión de datos del USART6
GPIO_Handler_t rx6pin = { 0 }; //Para manipular el pin de recepcioón de datos del USART6

PWM_Handler_t pwmtim3y = { 0 }; //Para configurar el PWM enn el timer 3 para y
GPIO_Handler_t pwm3ypin = { 0 }; //Pin que nos entrega la señal PWM para t pc7

PWM_Handler_t pwmtim3z = { 0 }; //Para configurar el PWM enn el timer 3 para z
GPIO_Handler_t pwm3zpin = { 0 }; //Pin que nos entrega la señal PWM para z pc8

uint16_t duttyValuex = 2000; //Valor del dutty inicial
uint16_t duttyValuey = 1000; //Valor del dutty inicial
uint16_t duttyValuez = 20000; //Valor del dutty inicial

//Handler I2Cs
I2C_Handler_t i2cAcelerometro = { 0 };
I2C_Handler_t i2cLCD = { 0 };

LCDI2C_handler_t lcdHandler = { 0 };

GPIO_Handler_t I2cSDA = { 0 };
GPIO_Handler_t I2cSCL = { 0 };

GPIO_Handler_t I2cSDA2 = { 0 }; //para la LCD
GPIO_Handler_t I2cSCL2 = { 0 };

BasicTimer_Handler_t handlerTim4 = { 0 }; //Timer para el blinking

uint8_t contador10 = 0;
uint16_t contador2000 = 0;

float DatoX10[10] = { 0 };
float DatoY10[10] = { 0 };
float DatoZ10[10] = { 0 };

float DatoX2000[2000] = { 0 };
float DatoY2000[2000] = { 0 };
float DatoZ2000[2000] = { 0 };
uint8_t regDatos[6] = { ACCEL_XOUT_L, ACCEL_XOUT_H, ACCEL_YOUT_L, ACCEL_YOUT_H,
		ACCEL_ZOUT_L, ACCEL_ZOUT_H };
uint8_t resDatos[6] = { 0 };

void initSystem(void); // Función quue inicializa el sistema
void USART6Rx_Callback(void); //Definir el callback

int main(void) {
	//Activacion cooprocesador matematico
	SCB->CPACR |= (0xF << 20);

	//Iniciamos sistma
	initSystem();

	while (1) {

		if (banderaLedUsuario == 1) {
			banderaLedUsuario = 0;
			cambiarLed();
			GPIOxTooglePin(&handlerUserLedPin);
		}

		if (flag2SegundosTer == 1) {
			for (int i = 0; i < 2000; i++) {
				if (i == 0) {
					writeStringInt(&USART6Handler, "x , y , z \n");
				}
//				delay_ms(4);
				sprintf(bufferMsg, " %.3f m/s², %.3f m/s², %.3f m/s² \n",
						DatoX2000[i], DatoY2000[i], DatoZ2000[i]);
				writeStringInt(&USART6Handler, bufferMsg);
			}
			flag2SegundosTer = 0;
		}

		if (flagDatos == 1) {

			if (contador10 < 10) {

				i2c_readMulRegister(&i2cAcelerometro, regDatos, 6, resDatos);

				AccelX_low = resDatos[0];
				AccelX_high = resDatos[1];
				AccelX = AccelX_high << 8 | AccelX_low;
				AccelXEsc = AccelX * FACTORESCALADO;

				AccelY_low = resDatos[2];
				AccelY_high = resDatos[3];
				AccelY = AccelY_high << 8 | AccelY_low;
				AccelYEsc = AccelY * FACTORESCALADO;

				AccelZ_low = resDatos[4];
				AccelZ_high = resDatos[5];
				AccelZ = AccelZ_high << 8 | AccelZ_low;
				AccelZEsc = AccelZ * FACTORESCALADO;

				DatoX10[contador10] = AccelXEsc;
				DatoY10[contador10] = AccelYEsc;
				DatoZ10[contador10] = AccelZEsc;

				contador10++;
			}

			else {
				contador10 = 0;
			}

			if (flag2Segundos == 1) {
				if (contador2000 < 2000) {
					DatoX2000[contador2000] = AccelXEsc;
					DatoY2000[contador2000] = AccelYEsc;
					DatoZ2000[contador2000] = AccelZEsc;

					contador2000++;
				} else {
					contador2000 = 0;
					flag2Segundos = 0;
					flag2SegundosTer = 1;
				}
			}

			flagDatos = 0;
		}

		//Hacemos un "eco" con el valor que nos llega por el serial
		if (usart6DataReceived != '\0') {
			writeChar(&USART6Handler, usart6DataReceived);

			if (usart6DataReceived == 'w') {
				sprintf(bufferMsg, "WHO_AM_I? (r)\n");
				writeStringInt(&USART6Handler, bufferMsg);

				i2cBuffer = i2c_readSingleRegister(&i2cAcelerometro, WHO_AM_I);
				sprintf(bufferMsg, "dataRead = 0x%x \n",
						(unsigned int) i2cBuffer);
				writeStringInt(&USART6Handler, bufferMsg);
				usart6DataReceived = '\0';
			} else if (usart6DataReceived == 'p') {
				sprintf(bufferMsg, "PWR_MGMT_1 state (r)\n");
				writeStringInt(&USART6Handler, bufferMsg);

				i2cBuffer = i2c_readSingleRegister(&i2cAcelerometro, POWER_CTL);
				sprintf(bufferMsg, "dataRead = 0x%x \n",
						(unsigned int) i2cBuffer);
				writeStringInt(&USART6Handler, bufferMsg);
				usart6DataReceived = '\0';
			}

			else if (usart6DataReceived == 't') {
				sprintf(bufferMsg, "test(r)\n");
				writeStringInt(&USART6Handler, bufferMsg);

				i2c_writeSingleRegister(&i2cAcelerometro, ACCEL_DATA_FORMAT,
						1 << 7);

				//Verificamos
				i2cBuffer = i2c_readSingleRegister(&i2cAcelerometro,
						ACCEL_DATA_FORMAT);

				sprintf(bufferMsg, "dataFormat = 0x%x \n",
						(unsigned int) i2cBuffer);
				writeStringInt(&USART6Handler, bufferMsg);
				usart6DataReceived = '\0';
			}

			else if (usart6DataReceived == 'f') {
				sprintf(bufferMsg, "frec \n");
				writeStringInt(&USART6Handler, bufferMsg);

				i2c_writeSingleRegister(&i2cAcelerometro, ACCEL_BW_RATE, 0x0A);
				i2c_writeSingleRegister(&i2cAcelerometro, ACCEL_DATA_FORMAT,
						0 << 7);

				//Verificamos
				i2cBuffer = i2c_readSingleRegister(&i2cAcelerometro,
						ACCEL_BW_RATE);

				sprintf(bufferMsg, "registro frecuentia = 0x%x \n",
						(unsigned int) i2cBuffer);
				writeStringInt(&USART6Handler, bufferMsg);
				usart6DataReceived = '\0';
			}

			else if (usart6DataReceived == 'd') {
				sprintf(bufferMsg, "Set and modify range(r)\n");
				writeStringInt(&USART6Handler, bufferMsg);

				i2cBuffer = i2c_readSingleRegister(&i2cAcelerometro,
						ACCEL_DATA_FORMAT);
				//Limpiamos el valor del rango actual
				i2cBuffer &= ~0x0F;
				i2cBuffer |= ADXL345_RANGE_4_G;

				//Habilitamos el FullRes
				i2cBuffer |= 0x08;

				i2c_writeSingleRegister(&i2cAcelerometro, ACCEL_DATA_FORMAT,
						i2cBuffer);

				//Verificamos
				i2cBuffer = i2c_readSingleRegister(&i2cAcelerometro,
						ACCEL_DATA_FORMAT);

				sprintf(bufferMsg, "dataFormat = 0x%x \n",
						(unsigned int) i2cBuffer);
				writeStringInt(&USART6Handler, bufferMsg);
				usart6DataReceived = '\0';
			}

			else if (usart6DataReceived == 's') { //Para resetear al equipo
				sprintf(bufferMsg, "Empezar Mediciones \n");
				writeStringInt(&USART6Handler, bufferMsg);

				i2c_writeSingleRegister(&i2cAcelerometro, POWER_CTL, 0x08);
				usart6DataReceived = '\0';
			} else if (usart6DataReceived == 'x') { //lecturas en x
				sprintf(bufferMsg, "Axis X data (r)\n");
				writeStringInt(&USART6Handler, bufferMsg);

				AccelX_low = i2c_readSingleRegister(&i2cAcelerometro,
						ACCEL_XOUT_L);
				AccelX_high = i2c_readSingleRegister(&i2cAcelerometro,
						ACCEL_XOUT_H);
				AccelX = AccelX_high << 8 | AccelX_low;
				AccelXEsc = AccelX * FACTORESCALADO;

				sprintf(bufferMsg, "AccelX = %f \n", (float) AccelXEsc);
				writeStringInt(&USART6Handler, bufferMsg);
				usart6DataReceived = '\0';

			} else if (usart6DataReceived == 'y') { //lecturas en y
				sprintf(bufferMsg, "Axis Y data (r)\n");
				writeStringInt(&USART6Handler, bufferMsg);

				AccelY_low = i2c_readSingleRegister(&i2cAcelerometro,
						ACCEL_YOUT_L);
				AccelY_high = i2c_readSingleRegister(&i2cAcelerometro,
						ACCEL_YOUT_H);
				AccelY = AccelY_high << 8 | AccelY_low;
				AccelYEsc = AccelY * FACTORESCALADO;

				sprintf(bufferMsg, "AccelY = %f \n", (float) AccelYEsc);
				writeStringInt(&USART6Handler, bufferMsg);
				usart6DataReceived = '\0';
			} else if (usart6DataReceived == 'z') { //lecturas en y
				sprintf(bufferMsg, "Axis Z data (r)\n");
				writeStringInt(&USART6Handler, bufferMsg);

				AccelZ_low = i2c_readSingleRegister(&i2cAcelerometro,
						ACCEL_ZOUT_L);
				AccelZ_high = i2c_readSingleRegister(&i2cAcelerometro,
						ACCEL_ZOUT_H);
				AccelZ = AccelZ_high << 8 | AccelZ_low;
				AccelZEsc = AccelZ * FACTORESCALADO;

				sprintf(bufferMsg, "AccelZ = %f \n", (float) AccelZEsc);
				writeStringInt(&USART6Handler, bufferMsg);
				usart6DataReceived = '\0';
			} else if (usart6DataReceived == '2') {
				writeStringInt(&USART6Handler,
						"Se tomaran datos por 2 segundos y luego se imprimen por pantalla \n");
				flag2Segundos = 1;
				usart6DataReceived = '\0';
			} else {
				usart6DataReceived = '\0';
			}
		}
	}
	return 0;
}

void initSystem(void) {

	//Configuramos el PLL a 80 MHz
	configPLL(80);

	uint16_t freq = getFreqPLL();

	//	Inicializamos el SysTick se le entraga el valor de la frecuencia actual del PLL
	config_SysTick_ms(freq);

	//Led de usuario usado para el blinking

	handlerUserLedPin.pGPIOx = GPIOA; //Se encuentra en el el GPIOA
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinNumber = PIN_5; // Es el pin 5.
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_OUT; //Se utiliza como salida
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL; //Salida PushPull
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING; //No se usa ninguna resistencia
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_FAST; //Se usa en velocidad rapida

	GPIO_Config(&handlerUserLedPin); //Se carga la configuración para el Led.

	//Configuración Timer 2 que controlara el blinking

	handlerTim2.ptrTIMx = TIM2; //El timer que se va a usar
	handlerTim2.TIMx_Config.TIMx_interruptEnable = 1; //Se habilitan las interrupciones
	handlerTim2.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; //Se usara en modo ascendente
	handlerTim2.TIMx_Config.TIMx_period = 2500; //Se define el periodo en este caso el led cambiara cada 250ms
	handlerTim2.TIMx_Config.TIMx_speed = BTIMER_SPEED_100us; //Se define la "velocidad" que se usara

	BasicTimer_Config(&handlerTim2); //Se carga la configuración.

	//PA 11 -> 7 bajando -> Tx
	tx6pin.pGPIOx = GPIOA;
	tx6pin.GPIO_PinConfig_t.GPIO_PinNumber = PIN_11;
	tx6pin.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	tx6pin.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLUP;
	tx6pin.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_FAST; //Se usa en velocidad rapida
	tx6pin.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF8;

	GPIO_Config(&tx6pin);
	//PA 12 -> 6 bajando - Rx
	rx6pin.pGPIOx = GPIOA;
	rx6pin.GPIO_PinConfig_t.GPIO_PinNumber = PIN_12;
	rx6pin.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	rx6pin.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLUP;
	rx6pin.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_FAST;
	rx6pin.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF8;

	GPIO_Config(&rx6pin);

	USART6Handler.ptrUSARTx = USART6;
	USART6Handler.USART_Config.USART_baudrate = USART_BAUDRATE_115200;
	USART6Handler.USART_Config.USART_datasize = USART_DATASIZE_8BIT;
	USART6Handler.USART_Config.USART_mode = USART_MODE_RXTX;
	USART6Handler.USART_Config.USART_parity = USART_PARITY_NONE;
	USART6Handler.USART_Config.USART_stopbits = USART_STOPBIT_1;
	USART6Handler.USART_Config.USART_RX_Int_Ena = ENABLE;

	USART_Config(&USART6Handler);

	pwm3xpin.pGPIOx = GPIOC;
	pwm3xpin.GPIO_PinConfig_t.GPIO_PinNumber = PIN_6;
	pwm3xpin.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	pwm3xpin.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	pwm3xpin.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	pwm3xpin.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_FAST;
	pwm3xpin.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF2;

	GPIO_Config(&pwm3xpin);

	pwmtim3x.ptrTIMx = TIM3;
	pwmtim3x.config.channel = PWM_CHANNEL_1;
	pwmtim3x.config.duttyCicle = duttyValuex;
	pwmtim3x.config.periodo = 20000;
	pwmtim3x.config.prescaler = freq;

	pwm_Config(&pwmtim3x);

	enableOutput(&pwmtim3x);
	startPwmSignal(&pwmtim3x);

	pwm3ypin.pGPIOx = GPIOC;
	pwm3ypin.GPIO_PinConfig_t.GPIO_PinNumber = PIN_7;
	pwm3ypin.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	pwm3ypin.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	pwm3ypin.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	pwm3ypin.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_FAST;
	pwm3ypin.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF2;

	GPIO_Config(&pwm3ypin);

	pwmtim3y.ptrTIMx = TIM3;
	pwmtim3y.config.channel = PWM_CHANNEL_2;
	pwmtim3y.config.duttyCicle = duttyValuey;
	pwmtim3y.config.periodo = 20000;
	pwmtim3y.config.prescaler = freq;

	pwm_Config(&pwmtim3y);

	enableOutput(&pwmtim3y);
	startPwmSignal(&pwmtim3y);

	pwm3zpin.pGPIOx = GPIOC;
	pwm3zpin.GPIO_PinConfig_t.GPIO_PinNumber = PIN_8;
	pwm3zpin.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	pwm3zpin.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	pwm3zpin.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	pwm3zpin.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_FAST;
	pwm3zpin.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF2;

	GPIO_Config(&pwm3zpin);

	pwmtim3z.ptrTIMx = TIM3;
	pwmtim3z.config.channel = PWM_CHANNEL_3;
	pwmtim3z.config.duttyCicle = duttyValuez;
	pwmtim3z.config.periodo = 20000;
	pwmtim3z.config.prescaler = freq;

	pwm_Config(&pwmtim3z);

	enableOutput(&pwmtim3z);
	startPwmSignal(&pwmtim3z);

	I2cSCL.pGPIOx = GPIOB;
	I2cSCL.GPIO_PinConfig_t.GPIO_PinNumber = PIN_8;
	I2cSCL.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	I2cSCL.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_OPENDRAIN;
	I2cSCL.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLUP;
	I2cSCL.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_HIGH;
	I2cSCL.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF4;
	GPIO_Config(&I2cSCL);

	I2cSDA.pGPIOx = GPIOB;
	I2cSDA.GPIO_PinConfig_t.GPIO_PinNumber = PIN_9;
	I2cSDA.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	I2cSDA.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_OPENDRAIN;
	I2cSDA.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLUP;
	I2cSDA.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_HIGH;
	I2cSDA.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF4;
	GPIO_Config(&I2cSDA);

	i2cAcelerometro.modeI2C = I2C_MODE_FM;
	i2cAcelerometro.slaveAddress = ACCEL_ADDRESS
	;
	i2cAcelerometro.ptrI2Cx = I2C1;
	i2c_config(&i2cAcelerometro);

	I2cSCL2.pGPIOx = GPIOA; //8 8 contando
	I2cSCL2.GPIO_PinConfig_t.GPIO_PinNumber = PIN_8;
	I2cSCL2.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	I2cSCL2.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_OPENDRAIN;
	I2cSCL2.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLUP;
	I2cSCL2.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_HIGH;
	I2cSCL2.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF4;
	GPIO_Config(&I2cSCL2);

	I2cSDA2.pGPIOx = GPIOB;
	I2cSDA2.GPIO_PinConfig_t.GPIO_PinNumber = PIN_4;
	I2cSDA2.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	I2cSDA2.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_OPENDRAIN;
	I2cSDA2.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLUP;
	I2cSDA2.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_HIGH;
	I2cSDA2.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF9;
	GPIO_Config(&I2cSDA2);

	i2cLCD.modeI2C = I2C_MODE_SM;
	i2cLCD.slaveAddress = LCD_ADDRES;
	i2cLCD.ptrI2Cx = I2C3;
	i2c_config(&i2cLCD);

	lcdHandler.ptrHandlerI2C = &i2cLCD;

	lcdi2cconfig(&lcdHandler);

	//Configuración Timer 4 que controlara el blinking

	handlerTim4.ptrTIMx = TIM4; //El timer que se va a usar
	handlerTim4.TIMx_Config.TIMx_interruptEnable = 1; //Se habilitan las interrupciones
	handlerTim4.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; //Se usara en modo ascendente
	handlerTim4.TIMx_Config.TIMx_period = 100; //Se define el periodo en este caso el led cambiara cada 250ms
	handlerTim4.TIMx_Config.TIMx_speed = BTIMER_SPEED_10us; //Se define la "velocidad" que se usara

	BasicTimer_Config(&handlerTim4); //Se carga la configuración.

}

float meanFunction(float *numbers, int cantidad) {

	float res;
	float sum = 0;

	for (int i = 0; i < cantidad; i++) {
		sum += numbers[i];
	}

	res = sum / (float) cantidad;

	return res;
}

void cambiarLed(void) {

	/*
	 * Realizamos una regresión lineal en donde -2g es 0; 0 es 10000; 2g 2000
	 * Por los que nos queda (10000*ValordeXpromedio)/(2g) + 10000 = 500/g * ValorXpromedio + 10000
	 */
	float xmeanConv = 8000 * meanFunction(DatoX10, 10) / SENSORS_GRAVITY_EARTH
			+ 10000;
	float ymeanConv = 8000 * meanFunction(DatoY10, 10) / SENSORS_GRAVITY_EARTH
			+ 10000;
	float zmeanConv = 8000 * meanFunction(DatoZ10, 10) / SENSORS_GRAVITY_EARTH
			+ 10000;

	if (xmeanConv < 0) {
		xmeanConv = 0.0f;
	}
	if (ymeanConv < 0) {
		ymeanConv = 0.0f;
	}
	if (zmeanConv < 0) {
		zmeanConv = 0.0f;
	}

	if (xmeanConv > 20000) {
		xmeanConv = 20000.0f;
	}
	if (ymeanConv > 20000) {
		ymeanConv = 20000.0f;
	}
	if (zmeanConv > 20000) {
		zmeanConv = 20000.0f;
	}

	duttyValuex = (int) xmeanConv;
	duttyValuey = (int) ymeanConv;
	duttyValuez = (int) zmeanConv;

	updateDuttyCycle(&pwmtim3x, duttyValuex);
	updateDuttyCycle(&pwmtim3y, duttyValuey);
	updateDuttyCycle(&pwmtim3z, duttyValuez);

}

//Calback del timer2 para el blinking
void BasicTimer2_Callback(void) {
	banderaLedUsuario = 1;
}

//Calback del timer6 para el blinking
void BasicTimer4_Callback(void) {
	flagDatos = 1;
}

//Callback para leer lo que se madna por el USART6
void USART6Rx_Callback(void) {
	usart6DataReceived = getRxData();
}
