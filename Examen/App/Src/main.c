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

//Drivers usados para el examen
#include "GPIOxDriver.h"
#include "USARTxDriver.h"
#include "BasicTimer.h"
#include "PwmDriver.h"
#include "AdcDriver.h"
#include <stdbool.h>
#include <string.h>
#include "PLLDriver.h"
#include "RTCDriver.h"
#include "I2CxDriver.h"
#include "SysTick.h"

#include "arm_math.h"

//Se definen algunos registros importantes del acelerrometro
#define ACCEL_ADDRESS 0x1D		//0xD -> Rireccion del Accel con Logic_1
#define ACCEL_XOUT_L 50	   		//Prmeros 4 bits de X
#define ACCEL_XOUT_H 51   		//Ultimos 4 bits de X
#define ACCEL_YOUT_L 52   		//Prmeros 4 bits de Y
#define ACCEL_YOUT_H 53   		//Ultimos 4 bits de X
#define ACCEL_ZOUT_L 54   		//Prmeros 4 bits de Z
#define ACCEL_ZOUT_H 55   		//Ultimos 4 bits de X
#define ACCEL_DATA_FORMAT 49	//REgistro del cormato
#define ACCEL_BW_RATE 0x2C		//Registro para la frecuencia
#define ACCEL_OFSX 30 //Offset en X
#define ACCEL_OFSY 31 //Offset en X
#define ACCEL_OFSZ 32 //Offset en X

#define SENSORS_GRAVITY_EARTH 9.80665F		//Gravedad
#define ADXL345_MG2G_MULTIPLIER 0.004F		//Factor de multiplicacion a +-2G o a full resolution
#define FACTORESCALADO SENSORS_GRAVITY_EARTH * ADXL345_MG2G_MULTIPLIER //Factor de conversión de LSB a m/s²

#define POWER_CTL 0x2D			//Registro de power donde se activa el muestreo
#define WHO_AM_I 0				//Registro que responde con el device ID 0xe0

#define ADXL345_RANGE_16_G 0b11 ///< +/- 16g
#define ADXL345_RANGE_8_G 0b10  ///< +/- 8g
#define ADXL345_RANGE_4_G 0b01  ///< +/- 4g
#define ADXL345_RANGE_2_G 0b00   ///< +/- 2g (default value)

//Vector que apunta a los datos input de la FFT.
float *datos;

/*
 * Variables definidas para la tarea.
 */

//Fechas
uint8_t segundos, minutos, horas, dias, meses, years, diaSemana = 0;

//Variables para el tratamento de la aceleración en X
uint8_t AccelX_low = 0;
uint8_t AccelX_high = 0;
int16_t AccelX = 0;
float AccelXEsc = 0.0f;

//Variables para el tratamento de la aceleración en y
uint8_t AccelY_low = 0;
uint8_t AccelY_high = 0;
int16_t AccelY = 0;
float AccelYEsc = 0.0f;

//Variables para el tratamento de la aceleración en z
uint8_t AccelZ_low = 0;
uint8_t AccelZ_high = 0;
int16_t AccelZ = 0;
float AccelZEsc = 0.0f;

//VAriables relacionadas al acelerometro.

uint16_t freqAdc = 20;

float transformedSignal[256];
float transformedAbs[256];

float32_t stopTime = 1.0;
uint32_t ifftFlag = 0;
uint32_t doBitReverse = 1;
arm_rfft_fast_instance_f32 config_Rfft_fast_f32;
arm_cfft_radix4_instance_f32 configRadix4_f32;
arm_status status = ARM_MATH_ARGUMENT_ERROR;
arm_status statusInitFFT = ARM_MATH_ARGUMENT_ERROR;
uint16_t fttSize = 256;

//Handler I2Cs
I2C_Handler_t i2cAcelerometro = { 0 }; //I2C encargado de comunicarse con el acelerometro

GPIO_Handler_t I2cSDA = { 0 }; // Handler que manipula el envio/recepcón de datos del Acelerometro
GPIO_Handler_t I2cSCL = { 0 }; // Handler que manipula la frecuencia de comunicación del Acelerometro

//Generación Handlers
//GPIO

GPIO_Handler_t handlerUserLedPin = { 0 };
GPIO_Handler_t handlerUserLedPin2 = { 0 };
GPIO_Handler_t tx6pin = { 0 };
GPIO_Handler_t rx6pin = { 0 };
GPIO_Handler_t pwprueba = { 0 };
GPIO_Handler_t handlerPinMCO = { 0 };

//Diversos buffer para la recepcion y transmision de datos

char bufferReception[64] = { 0 };
char cmd[64] = { 0 };
unsigned int firstParameter = 256;
unsigned int secondParameter = 256;
char userMsg[64] = { 0 };

//Timers

BasicTimer_Handler_t handlerTimer2 = { 0 }; // Timer para el blinking
BasicTimer_Handler_t handlerTimer5 = { 0 }; //Timer para la toma de datos

//Handler para el control de la terminal
USART_Handler_t handlerTerminal = { 0 };
//Para la recepcion de datos por la terminal
uint8_t rxData = 0;

//Funciones para inciar el sistema
void initSystem(void);
//Funcion para tratar los comandos
void parseCommands(char *ptrBufferReception);
//Funcion que actuailiza la fecha
void getFecha(void);

//Funcion para cuadrar el ADC
ADC_Config_t channnel_0 = { 0 };
//Variables relacionadas al adc
uint8_t ADCISCOMPLETE = 0;
uint16_t adcData[2][256] = { 0 };
float adcData2[2][256] = { 0 };
//Variable para cambiar la sintonizacion del HSI
uint8_t trimValue = 13;
//Ultimos datos de cada ADC
uint16_t adcLastData1 = 0;
uint16_t adcLastData2 = 0;

//Vanderas para manejar estados.
bool banderaADC = 0;
bool adcIsComplete = 0;
bool banderaImprimir = 0;
bool banderaImprimirAct = 0;
bool stringComplete = 0;
bool flagDatos = 0;
bool flag256Listos = 0;
bool pllTrue = 0;
bool flagDatosTer = 0;
//Registros para la lecutra del acelerometro
uint8_t regDatos[6] = { ACCEL_XOUT_L, ACCEL_XOUT_H, ACCEL_YOUT_L, ACCEL_YOUT_H,
ACCEL_ZOUT_L, ACCEL_ZOUT_H };
//Bandera que cuenta el adc
uint8_t adcIsCompleteCount = 0;
uint16_t counterReception = 0;
uint16_t contador256 = 0;
//Arreglos para guardar los datos
float DatoX256[256] = { 0 };
float DatoY256[256] = { 0 };
float DatoZ256[256] = { 0 };

uint16_t contadorImprimir256 = 0; //Para imprimir los 256 datos

uint16_t i2cBuffer = 0;		 //Buffer para la recepción de datos en el I2C

uint32_t count256 = 0;		//Para guardar los d¿256 datos

/*
 * Respuesta del acelerometro al solicitarle datos.
 */
uint8_t resDatos[6] = { 0 };	//Para recibir los resultados del acelerometro

PWM_Handler_t pwmadc = { 0 }; //Para configurar el PWM en el timer 3 para X

char bufferData[128] = { 0 };	//para crear mensajes

uint8_t segundero = 0;		//"Bandera" que nos permite actulalizar el reloj

char dia[10] = { 0 };		//Nimbre del día actual

int main(void) {
	//Se inicia el sistema
	initSystem();
	//Los delays es para que no se solapen en el Acelerometro
	delay_ms(15);
	writeString(&handlerTerminal, "Solución Examen \n");
	i2c_writeSingleRegister(&i2cAcelerometro, POWER_CTL, 0x08);
	delay_ms(1);
	//Colocamos la frecuencia de muestreo del accel a 3600 hz
	i2c_writeSingleRegister(&i2cAcelerometro, ACCEL_BW_RATE, 0x0F);
	/*
	 * Se envian mensajes de inicio
	 */
	writeString(&handlerTerminal,
			"El acelerometro se encuentra listo para su funcionamiento. \n");
	writeString(&handlerTerminal,
			"Para conocer todos los comandos disponibles usar help @ \n");
	while (1) {
		// Cada 500 ms se actualza
		if (segundero > 2) {
			getFecha();
			segundero = 0;
		}
		// El caracter '@' nos indica que es el final de la cadena
		if (rxData != '\0') {
			writeChar(&handlerTerminal, rxData);
			bufferReception[counterReception] = rxData;
			counterReception++;

			// If the incoming character is a newline, set a flag
			// so the main loop can do something about it
			if (rxData == '\r') {
				stringComplete = 1;
				sprintf(bufferData, "\n");
				writeString(&handlerTerminal, bufferData);
				//Agrego esta linea para crear el string con el null al final
				bufferReception[counterReception] = '\0';
				counterReception = 0;
			}
			if(rxData == '\b'){
				counterReception--;
				counterReception--;
			}
			//Para que no vuelva entrar. Solo cambia debido a la interrupcion
			rxData = '\0';
		}

		//Hacemos un analisis de la cadena de datos obtenida
		if (stringComplete) {
			parseCommands(bufferReception);
			stringComplete = 0;
		}
		//Se imrpimen los 256 datos del dc
		if (banderaImprimir) {
			for (int i = 0; i < 256; i++) {
				sprintf(bufferData, "%u\t%u \n", adcData[0][i], adcData[1][i]);
				writeString(&handlerTerminal, bufferData);
			}
			//Inicialización FFT
			statusInitFFT = arm_rfft_fast_init_f32(&config_Rfft_fast_f32,
					fttSize);

			if (statusInitFFT == ARM_MATH_SUCCESS) {
				//Imprimir mensaje de exito
				sprintf(bufferData, "Datos adquiridos \n");
				writeString(&handlerTerminal, bufferData);
			}
			banderaImprimir = 0;
		}

		/*
		 * Este if se activa siempre y cuando ya tengamos los 256 datos para imprimir por
		 * Usart
		 */
		if (flagDatosTer == 1) {
			/*
			 * Para el primer paso mandamos el formato como se entregando los datos, luego se empiezan
			 * a recorrer los arreglos para que impriman todos los datos
			 */
			writeStringInt(&handlerTerminal, "Datos obtenidos\n");
			flagDatosTer = 0;
		}

		if (flagDatos == 1) {
			//Funcion que lee varios registros simultaneamente.
			i2c_readMulRegister(&i2cAcelerometro, regDatos, 6, resDatos);

			/*
			 * En esta seccion se le hace un tratamiento a los diferentes ejes
			 * para conseguir el valor en m/s², primero se adquieren los primeros
			 * 8 bits de la medida, luego los otros 8 bits, los mezclamos y multiplicamos por
			 * un factor de escalado = Gravedad*4mg
			 */
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

			/*
			 * Si la funcion de 2 segundos se encuentra activada
			 * ese if se encarga de guardar los datos, y cuando culmine
			 * reiniciamos las banderas y activaos la bandera
			 * flagDatosTerminados, para que empiece el envio de datos
			 * con la funcioón vista antes
			 */

			if (flag256Listos == 1) {
				if (contador256 < 256) {
					DatoX256[contador256] = AccelXEsc;
					DatoY256[contador256] = AccelYEsc;
					DatoZ256[contador256] = AccelZEsc;

					contador256++;
				} else {
					contador256 = 0;
					flag256Listos = 0;
					flagDatosTer = 1;
					//Inicialización FFT
					statusInitFFT = arm_rfft_fast_init_f32(
							&config_Rfft_fast_f32, fttSize);

					if (statusInitFFT == ARM_MATH_SUCCESS) {
						//Imprimir mensaje de exito
						sprintf(bufferData, "Datos adquiridos \n");
						writeString(&handlerTerminal, bufferData);
					}

					// Imprimir el areglo de 256 datos
					for (int i = 0; i < 256; i++) {
						sprintf(bufferData, "n=%d x=%.2f y=%.2f z=%.2f\n", i,
								DatoX256[i], DatoY256[i], DatoZ256[i]);
						writeString(&handlerTerminal, bufferData);
					}
				}
			}
			//Se cierra la bandera de toma de datos.
			flagDatos = 0;
		}
	}
	return 0;
}

void initSystem(void) {

//Activacion cooprocesador matematico(importante para esta tarea)
	SCB->CPACR |= (0xF << 20);

//Configuramos el PLL a 100 MHz

	configPLL(100);

//Se cobtiene la recuencia para darsela al Systick, los otros drivers llaman esta función
//Por si mismos auto-ajustandose a la frecuencia que se les asigne.

	uint16_t freq = getFreqPLL();

//	Inicializamos el SysTick se le entraga el valor de la frecuencia actual del PLL
	config_SysTick(freq);

	/*
	 * Se configuran los pines necesarios para la comunicación con el acelerometro.
	 */
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

	/*
	 * Se configura el I2C1 en fast mode y se le pasa la direccion del acelerometro
	 */
	i2cAcelerometro.modeI2C = I2C_MODE_FM;
	i2cAcelerometro.slaveAddress = ACCEL_ADDRESS;
	i2cAcelerometro.ptrI2Cx = I2C1;
	i2c_config(&i2cAcelerometro);
	/*
	 * Leds de estado
	 */
	handlerUserLedPin.pGPIOx = GPIOH; //Se encuentra en el el GPIOH
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinNumber = PIN_1; // Es el pin 1
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_OUT; //Se utiliza como salida
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL; //Salida PushPull
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING; //No se usa ninguna resistencia
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_FAST; //Se usa en velocidad rapida

	GPIO_Config(&handlerUserLedPin); //Se carga la configuración para el Led.

	handlerUserLedPin2.pGPIOx = GPIOA; //Se encuentra en el el GPIOA
	handlerUserLedPin2.GPIO_PinConfig_t.GPIO_PinNumber = PIN_5; // Es el pin 5.
	handlerUserLedPin2.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_OUT; //Se utiliza como salida
	handlerUserLedPin2.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL; //Salida PushPull
	handlerUserLedPin2.GPIO_PinConfig_t.GPIO_PinPuPdControl =
	GPIO_PUPDR_NOTHING; //No se usa ninguna resistencia
	handlerUserLedPin2.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_FAST; //Se usa en velocidad rapida

	GPIO_Config(&handlerUserLedPin2); //Se carga la configuración para el Led.

//PA2
	//PA 11 -> 7 bajando -> Tx amarillo
	tx6pin.pGPIOx = GPIOA;
	tx6pin.GPIO_PinConfig_t.GPIO_PinNumber = PIN_11;
	tx6pin.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	tx6pin.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLUP;
	tx6pin.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_FAST; //Se usa en velocidad rapida
	tx6pin.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF8;

	GPIO_Config(&tx6pin);
	//PA 12 -> 6 bajando - Rx marron
	rx6pin.pGPIOx = GPIOA;
	rx6pin.GPIO_PinConfig_t.GPIO_PinNumber = PIN_12;
	rx6pin.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	rx6pin.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLUP;
	rx6pin.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_FAST;
	rx6pin.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF8;

	GPIO_Config(&rx6pin);

	handlerTimer2.ptrTIMx = TIM2; //El timer que se va a usar
	handlerTimer2.TIMx_Config.TIMx_interruptEnable = 1; //Se habilitan las interrupciones
	handlerTimer2.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; //Se usara en modo ascendente
	handlerTimer2.TIMx_Config.TIMx_period = 2500; //Se define el periodo en este caso el led cambiara cada 250ms
	handlerTimer2.TIMx_Config.TIMx_speed = BTIMER_SPEED_100us; //Se define la "velocidad" que se usara

	BasicTimer_Config(&handlerTimer2); //Se carga la configuración.

//Timer para la toma de datos

	handlerTimer2.ptrTIMx = TIM3; //El timer que se va a usar
	handlerTimer2.TIMx_Config.TIMx_interruptEnable = 1; //Se habilitan las interrupciones
	handlerTimer2.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; //Se usara en modo ascendente
	handlerTimer2.TIMx_Config.TIMx_period = 50; //Se define el periodo en este caso el led cambiara cada 250ms
	handlerTimer2.TIMx_Config.TIMx_speed = BTIMER_SPEED_100us; //Se define la "velocidad" que se usara

	BasicTimer_Config(&handlerTimer2); //Se carga la configuración.

	handlerTerminal.ptrUSARTx = USART6;
	handlerTerminal.USART_Config.USART_baudrate = USART_BAUDRATE_115200;
	handlerTerminal.USART_Config.USART_datasize = USART_DATASIZE_8BIT;
	handlerTerminal.USART_Config.USART_mode = USART_MODE_RXTX;
	handlerTerminal.USART_Config.USART_parity = USART_PARITY_NONE;
	handlerTerminal.USART_Config.USART_stopbits = USART_STOPBIT_1;
	handlerTerminal.USART_Config.USART_RX_Int_Ena = ENABLE;

	USART_Config(&handlerTerminal);
//Configuración del pwm del timer 5 que controla el ADC

	pwmadc.ptrTIMx = TIM5;
	pwmadc.config.channel = PWM_CHANNEL_1;
	pwmadc.config.duttyCicle = 100;
	pwmadc.config.periodo = 500;
	pwmadc.config.prescaler = 10;

	pwm_Config(&pwmadc);
	enableOutput(&pwmadc);
	startPwmSignal(&pwmadc);

	//Salida para ver la velocidad de muestreo del ADC
	pwprueba.pGPIOx = GPIOA;
	pwprueba.GPIO_PinConfig_t.GPIO_PinNumber = PIN_0;
	pwprueba.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	pwprueba.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	pwprueba.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	pwprueba.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_FAST;
	pwprueba.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF2;

	GPIO_Config(&pwprueba);

	//COnfiugracion ADC se pones un samplig period de 56 ya que ~= 100 mhz / 56 ~= 1.78 Mhz que es más que suficiente
	uint8_t channels[2] = { ADC_CHANNEL_1, ADC_CHANNEL_4 };
	channnel_0.channels = channels;
	channnel_0.dataAlignment = ADC_ALIGNMENT_RIGHT;
	channnel_0.numberOfChannels = 2;
	uint8_t samplingPeriod[2] = { 0 };
	samplingPeriod[0] = ADC_SAMPLING_PERIOD_56_CYCLES
	;
	samplingPeriod[1] = ADC_SAMPLING_PERIOD_56_CYCLES
	;
	channnel_0.samplingPeriod = samplingPeriod;
	channnel_0.resolution = ADC_RESOLUTION_12_BIT;
	channnel_0.externType = EXTEN_RISING_TIMER5_CC1;

	adc_Config(&channnel_0);

	/*Configuracion del Pin para ver la velocidad */
	handlerPinMCO.pGPIOx = GPIOA;
	handlerPinMCO.GPIO_PinConfig_t.GPIO_PinNumber = PIN_8;
	handlerPinMCO.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	handlerPinMCO.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	handlerPinMCO.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_FAST;
	handlerPinMCO.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	handlerPinMCO.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF0;

	/* Cargamos la configuracion del Pin en los registros*/
	GPIO_Config(&handlerPinMCO);

	configChannelMCO1(MCO1_HSI_CHANNEL);
	configPresMCO1(0);
	config_RTC();

}
//Calback del timer2 para el blinking
void BasicTimer2_Callback(void) {
	GPIOxTooglePin(&handlerUserLedPin);
	GPIOxTooglePin(&handlerUserLedPin2);
	segundero++;
}

//Calback del timer2 para el blinking
void BasicTimer3_Callback(void) {
	GPIOxTooglePin(&handlerUserLedPin2);
	flagDatos = 1;
}

void USART6Rx_Callback(void) {
	rxData = getRxData();
}

void adcComplete_Callback(void) {
	if (adcIsCompleteCount == 0) {
		adcLastData1 = getADC();
		adcIsCompleteCount++;
		if (banderaImprimirAct) {
			adcData[0][count256] = adcLastData1;
			adcData2[0][count256] = adcLastData1/4095.0f;
		}
	} else {
		adcLastData2 = getADC();
		adcIsCompleteCount = 0;
		if (banderaImprimirAct) {
			adcData[1][count256] = adcLastData2;
			adcData2[1][count256] = adcLastData2/4095.0f;
			count256++;
		}
	}
	if (count256 == 256) {
		count256 = 0;
		banderaImprimirAct = 0;
		banderaImprimir = 1;
	}

}

void parseCommands(char *ptrBufferReception) {

	/* Lee la cadena de caracteres a la que apunta el "ptrBufferReception
	 * y almacena en tres elementos diferentes: un string llamado "cmd",
	 * y dos integer llamados "firstParameter" y "secondParameter"
	 * De esta forma podemos introducir informacion al micro desde el puerto
	 */
	sscanf(ptrBufferReception, "%s %u %u %s", cmd, &firstParameter,
			&secondParameter, userMsg);
	if (strcmp(cmd, "help") == 0) {
		writeString(&handlerTerminal, "Help Menu CMDs: \n");
		writeString(&handlerTerminal, "1)  Help -> Print this menu \n");
		writeString(&handlerTerminal, "Default MCO -> HSI div0\n");
		writeString(&handlerTerminal,
				"2)  MCO_prescaler -> #div(0,2,3,4,5); warning PLL > 2\n");
		writeString(&handlerTerminal,
				"3)  MCO_channel #(LSE=0, PLL=1, HSI=2); warning PLL default prescaler 2\n");
		writeString(&handlerTerminal,
				"4) Changetrim #(0-> disminuir, 1-> aumentar) \n");

		writeString(&handlerTerminal,
				"5)  adc_frecuencia #(Valor de la frecuencia entre 10 y 20)khz\n");
		writeString(&handlerTerminal,
				"6)  adc_datos (imprime los siguientes 256 datos)\n");
		writeString(&handlerTerminal,
				"Comandos relacionados al RTC\n"
						"Tener en cuenta que no acepta datos incorrectos, si un dato es valido y el otro no solo se conservara el dato valido\n");
		writeString(&handlerTerminal,
				"7)  RTC_set_fecha1 #(minuto) #(segundo) \n");
		writeString(&handlerTerminal, "8)  RTC_set_fecha2 #(año) #(hora) \n");
		writeString(&handlerTerminal, "9)  RTC_set_fecha3 #(mes) #(dia) \n");
		writeString(&handlerTerminal,
				"10) RTC_set_fecha4 #(diaSemana), Lunes 1 ... Domingo 7 \n");
		writeString(&handlerTerminal, "11) RTC_get_fecha \n");
		writeString(&handlerTerminal,
				"12) Ac_datos: Captura de datos e impresión se dejan listos para Fourier\n");
		writeString(&handlerTerminal,
				"13) Ac_FFT: FFT result (x = 0; y = 1,  z = 2) \n");
		writeString(&handlerTerminal,
				"14) FFT 1: FFT result (ADCdatos1 = 1 ADCdatos2 = 2) default ADCdatos1 \n");

	}

	else if (strcmp(cmd, "MCO_prescaler") == 0) {
		if (firstParameter == 0 && pllTrue == 0) {
			writeString(&handlerTerminal,
					"CMD: MCO_prescaler sin division no permitido en PLL \n");
			configPresMCO1(0);
		} else if (firstParameter == 0 && pllTrue == 1) {
			writeString(&handlerTerminal,
					"CMD: MCO_prescaler sin division no permitido en PLL \n");
		} else if (firstParameter == 2) {
			writeString(&handlerTerminal, "CMD: MCO_prescaler divison 2 \n");
			configPresMCO1(MCO1_PRESCALER_DIV_2);
		} else if (firstParameter == 3) {
			writeString(&handlerTerminal, "CMD: MCO_prescaler divison 3 \n");
			configPresMCO1(MCO1_PRESCALER_DIV_3);
		} else if (firstParameter == 4) {
			writeString(&handlerTerminal, "CMD: MCO_prescaler divison 4 \n");
			configPresMCO1(MCO1_PRESCALER_DIV_4);
		} else if (firstParameter == 5) {
			writeString(&handlerTerminal, "CMD: MCO_prescaler divison 5 \n");
			configPresMCO1(MCO1_PRESCALER_DIV_5);
		} else {
			writeString(&handlerTerminal, "Wrong number \n");
		}
	}

	else if (strcmp(cmd, "MCO_channel") == 0) {

		if (firstParameter == 0) {
			pllTrue = 0;
			writeString(&handlerTerminal, "CMD: MCO_channel LSE \n");
			configChannelMCO1(MCO1_LSE_CHANNEL);
		} else if (firstParameter == 1) {
			pllTrue = 1;
			writeString(&handlerTerminal,
					"CMD: MCO_channel PLL se coloca como prescaler default 2 \n");
			configPresMCO1(MCO1_PRESCALER_DIV_2);
			configChannelMCO1(MCO1_PLL_CHANNEL);

		} else if (firstParameter == 2) {
			pllTrue = 0;
			writeString(&handlerTerminal, "CMD: MCO_channel HSI \n");
			configChannelMCO1(MCO1_HSI_CHANNEL);
		} else {
			pllTrue = 0;
			writeString(&handlerTerminal, "Wrong number \n");
		}
	}

	else if (strcmp(cmd, "Changetrim") == 0) {

		if (firstParameter == 0) {
			trimValue--;
			sprintf(bufferData, "CMD: Disminuir trim, nuevo valor %d\n",
					trimValue);
			writeString(&handlerTerminal, bufferData);
			changeTrim(trimValue);
		} else if (firstParameter == 1) {
			trimValue++;
			sprintf(bufferData, "CMD: Disminuir trim, nuevo valor %d\n",
					trimValue);
			writeString(&handlerTerminal, bufferData);
			changeTrim(trimValue);

		} else {
			writeString(&handlerTerminal, "Wrong number \n");
		}
	} else if (strcmp(cmd, "adc_frecuencia") == 0) {

		if (firstParameter <= 20 && firstParameter >= 10) {
			sprintf(bufferData, "Se cambio la frecuencia de mustreo a %d \n",
					firstParameter);
			writeString(&handlerTerminal, bufferData);
			freqAdc = firstParameter;
			uint16_t auxFreq = (10000 / firstParameter);
			updateFrequency(&pwmadc, auxFreq);

		} else {
			sprintf(bufferData, "Frecuencia no permitida \n");
			writeString(&handlerTerminal, bufferData);
		}
	}

	else if (strcmp(cmd, "adc_datos") == 0) {
		writeString(&handlerTerminal,
				"Se enviaran los siguientes datos de amboas canales \n");
		banderaImprimirAct = 1;
	}

	else if (strcmp(cmd, "RTC_set_fecha1") == 0) {
		enableRTCChange();
		if (firstParameter < 60 && firstParameter >= 0) {
			setMinutes(firstParameter);
		} else {
			writeString(&handlerTerminal,
					"Se ha enviado un minuto invalido \n");
		}
		if (secondParameter < 60 && secondParameter >= 0) {
			setSegundos(secondParameter);
		} else {
			writeString(&handlerTerminal,
					"Se ha enviado un segundo invalida \n");
		}
		disableRTCChange();
		getFecha();
	}

	else if (strcmp(cmd, "RTC_set_fecha2") == 0) {
		enableRTCChange();
		if (firstParameter <= 99 && firstParameter >= 0) {
			setYear(firstParameter);
		} else {
			writeString(&handlerTerminal, "Se ha enviado un año invalido");
		}
		if (secondParameter < 60 && secondParameter >= 0) {
			setHour(secondParameter);
		} else {
			writeString(&handlerTerminal, "Se ha enviado un segundo invalida");
		}
		disableRTCChange();
		getFecha();
	}

	else if (strcmp(cmd, "RTC_set_fecha3") == 0) {
		enableRTCChange();
		if (firstParameter <= 12 && firstParameter >= 1) {
			setMes(firstParameter);
		} else {
			writeString(&handlerTerminal, "Se ha enviado un mes invalido");
		}
		if (firstParameter == 1 || firstParameter == 3 || firstParameter == 5
				|| firstParameter == 7 || firstParameter == 8
				|| firstParameter == 10 || firstParameter == 12) {
			if (secondParameter > 0 && secondParameter <= 31) {
				setDia(secondParameter);
			} else {
				writeString(&handlerTerminal, "Días invalidos");
			}
		}
		if (firstParameter == 2) {
			if (secondParameter > 0 && secondParameter <= 28) {
				setDia(secondParameter);
			} else {
				writeString(&handlerTerminal, "Días invalidos");
			}
		}
		if (firstParameter == 4 || firstParameter == 6 || firstParameter == 9
				|| firstParameter == 11) {
			if (secondParameter > 0 && secondParameter <= 30) {
				setDia(secondParameter);
			} else {
				writeString(&handlerTerminal, "Días invalidos");
			}
		}
		getFecha();
		disableRTCChange();
	}

	else if (strcmp(cmd, "RTC_set_fecha4") == 0) {
		enableRTCChange();
		if (firstParameter <= 7 && firstParameter > 0) {
			setDiaSemana(firstParameter);
		} else {
			writeString(&handlerTerminal, "Día de las semana invalido");
		}
		getFecha();
		disableRTCChange();
	}

	else if (strcmp(cmd, "RTC_get_fecha") == 0) {
		sprintf(bufferData, "Hoy es ");
		writeString(&handlerTerminal, bufferData);
		writeString(&handlerTerminal, dia);
		sprintf(bufferData, " Dia %d, Mes %d, Año %d \nSon las %d:%d:%d \n",
				dias, meses, years, horas, minutos, segundos);
		writeString(&handlerTerminal, bufferData);
	} else if (strcmp(cmd, "Ac_datos") == 0) {
		flag256Listos = 1;

	}

	/*  Presentar los datos de la frecuencia leída por el acelerómetro (CMSIS-FFT) */
	else if (strcmp(cmd, "Ac_FFT") == 0) {
		writeString(&handlerTerminal, bufferData);
		stopTime = 0.0;

		int j = 0;

		sprintf(bufferData, "FFT \n");
		writeString(&handlerTerminal, bufferData);
		//Calcular la transformada de Fourier
		if (firstParameter == 0) {
			datos = DatoX256;
		} else if (firstParameter == 1) {
			datos = DatoY256;
		} else {
			writeString(&handlerTerminal,
					"Parametro invalido, se manda el default Z \n");
			datos = DatoZ256;
		}
		if (statusInitFFT == ARM_MATH_SUCCESS) {
			arm_rfft_fast_f32(&config_Rfft_fast_f32, datos, transformedSignal,
					ifftFlag);
			arm_abs_f32(transformedSignal, transformedAbs, fttSize);

			uint32_t indexMax = 0;
			float FTT_Max = transformedAbs[1];
			for (int i = 1; i < fttSize; i++) {
				if (i % 2) {
					if (transformedAbs[i] > FTT_Max) {
						FTT_Max = transformedAbs[i];
						indexMax = j;
					}
					sprintf(bufferData, "%u ; %#.6f\n", j,
							2 * transformedAbs[i]);
					writeString(&handlerTerminal, bufferData);
					j++;
				}
			}

			float frec = (indexMax * 200) / 256.0f;
			sprintf(bufferData, "frecuency %.2f Hz \n", frec);
			writeString(&handlerTerminal, bufferData);

		} else {
			writeString(&handlerTerminal, "FFT not initialized...");
		}
	}

	/*  Presentar los datos de la frecuencia leída por el acelerómetro (CMSIS-FFT) */
	else if (strcmp(cmd, "FFT") == 0) {
		writeString(&handlerTerminal, bufferData);
		stopTime = 0.0;

		int j = 0;

		sprintf(bufferData, "FFT \n");
		writeString(&handlerTerminal, bufferData);
		//Calcular la transformada de Fourier
		if (firstParameter == 1) {
			datos = adcData2[0];
		} else if (firstParameter == 2) {
			datos = adcData2[1];
		} else {
			writeString(&handlerTerminal,
					"Parametro invalido, se manda el default ADCdata1 \n");
			datos = adcData2[0];
		}
		if (statusInitFFT == ARM_MATH_SUCCESS) {
			arm_rfft_fast_f32(&config_Rfft_fast_f32, datos, transformedSignal,
					ifftFlag);
			arm_abs_f32(transformedSignal, transformedAbs, fttSize);

			uint32_t indexMax = 0;
			float FTT_Max = transformedAbs[1];
			for (int i = 1; i < fttSize; i++) {
				if (i % 2) {
					if (transformedAbs[i] > FTT_Max) {
						FTT_Max = transformedAbs[i];
						indexMax = j;
					}
					sprintf(bufferData, "%u ; %#.6f\n", j,
							2 * transformedAbs[i]);
					writeString(&handlerTerminal, bufferData);
					j++;
				}
			}

			float frec = (indexMax * freqAdc * 1000) / 256.0f;
			sprintf(bufferData, "frecuency %.2f Hz \n", frec);
			writeString(&handlerTerminal, bufferData);

		} else {
			writeString(&handlerTerminal, "FFT not initialized...");
		}
	}

	else {
		// Se imprime el mensaje "Wrong CMD" si la escritura no corresponde a los CMD implementados
		writeString(&handlerTerminal, "Wrong CMD \n");
	}
}

void getFecha(void) {
	segundos = getSegundos();
	minutos = getMinutes();
	horas = getHour();
	dias = getDia();
	meses = getMes();
	years = getYear();
	diaSemana = getDiaSemana();
	if (diaSemana == 1) {
		sprintf(dia, "Lunes");
	} else if (diaSemana == 2) {
		sprintf(dia, "Martes");
	} else if (diaSemana == 3) {
		sprintf(dia, "Miercoles");
	} else if (diaSemana == 4) {
		sprintf(dia, "Jueves");
	} else if (diaSemana == 5) {
		sprintf(dia, "Viernes");
	} else if (diaSemana == 6) {
		sprintf(dia, "Sabado");
	} else if (diaSemana == 7) {
		sprintf(dia, "Domingo");
	}
}
