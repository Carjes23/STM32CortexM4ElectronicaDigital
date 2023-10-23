/**
 ******************************************************************************
 * @file           : main_accelFatFs.c
 * @author         : dacardenasj
 * @brief          : Uso de librerías FatFs y microSD con adaptador SPI.
 ******************************************************************************
 *
 ******************************************************************************
 */

#include "stdint.h"
#include "string.h"
#include "stm32f4xx.h"

/* Librerías propias de drivers y demás utilidades */
#include "GPIOxDriver.h"
#include "BasicTimer.h"
#include "USARTxDriver.h"
#include "ExtiDriver.h"
#include "ff.h"
#include "sd_spi.h"
#include "SysTick.h"
#include "PLLDriver.h"
#include <stdlib.h>
#include <strings.h>
#include "arm_math.h"
#include "ILI9341.h"
#include "I2CDriver.h"
#include "MPU6050.h"

/* Definiciones para el tamaño de FFT */
#define 		FFT_LEN_128		128

// MACROS para identificar direcciones
#define RIGHT 	1
#define UP 		2
#define LEFT 	3
#define DOWN 	4
#define PAUSE 	5

/*Cambiables*/

uint16_t limitImage = 3; //Cambiar dependiendo la cantidad de iamgenes
uint16_t timChangeIm = 3; //multiplo de 5 sg cambiar dependiendo cuanto
//quiere que se demore en default

// Tamaño de los bloques a procesar
uint16_t chunk_size = 512;

// Handler del MPU6050 (sensor) usando I2C
I2C_Handler_t i2c1_mpu6050 = { 0 };

// Definición de handlers para los periféricos a utilizar
// (En esta sección se definen los distintos handlers para gestionar el hardware)
GPIO_Handler_t handlerLedState = { 0 };     // Handler para LED de estado
GPIO_Handler_t handlerUsartRx = { 0 };      // Handler para USART en modo RX
GPIO_Handler_t handlerUsartTx = { 0 };      // Handler para USART en modo TX
GPIO_Handler_t handlerUserButtonPin = { 0 }; // Handler para el botón del usuario
GPIO_Handler_t mpu6050_sdaPin = { 0 };      // Pin SDA del MPU6050
GPIO_Handler_t mpu6050_sclPin = { 0 };      // Pin SCL del MPU6050

// Timers utilizados
BasicTimer_Handler_t handlerTimerOkState = { 0 };

// USART para comunicación con la terminal
USART_Handler_t commTerm = { 0 };

// Configuración para el botón del usuario con interrupciones externas
EXTI_Config_t handlerUserButton = { 0 };

// Objetos relacionados con el sistema de archivos y microSD
FATFS FatFs;   // Objeto para manejo del sistema de archivos
FIL fil;       // Objeto para manejar un archivo específico
FRESULT res;   // Variable de resultados para detectar errores

// Variables para el manejo y consulta del espacio en la SD
DWORD free_clusters, free_sectors, total_sectors;
FATFS *getFreeFs;

// Buffer para lectura de datos
BYTE readBuf[64];

// Variable para almacenar los bytes escritos y poder verificarlos
UINT bytesWrote;

// Arreglos de datos para el MPU6050
int16_t mpu6050_data[6] = { 0 };
float mpuNormData[6] = { 0 };

/* Para los promedios de los sensores */
int32_t sumaSensores[6] = { 0 };
int16_t promedioSensores[6] = { 0 };

/* Variables auxiliares */
volatile uint8_t usart2_rxData = '\0';
volatile uint8_t counter = 0;
volatile DSTATUS stat = 2;
volatile uint8_t buttonFlag = RESET;
volatile uint8_t measure = RESET;
volatile uint16_t spellCount = 0;
volatile uint8_t mpuErr;
/*
 * Pines y handlers necesarios para manipular la pantalla
 * CS -> Chip Select
 * RST -> Reset
 * DC -> Data / Command (Seleccionamos que estamos mandando si datos o comandos)
 * SIPI -> Que SPI se va a utilizar en este caso el 2
 * MOSI -> Master Output Slave Input -> El micro envia datos que la pantalla recibe
 * SCK -> Clock del SPI.
 * ILI -> Handler de la pantalla
 */

GPIO_Handler_t CSPrueba = { 0 };
GPIO_Handler_t RSTPrueba = { 0 };
GPIO_Handler_t DCPrueba = { 0 };
SPI_Handler_t SPIPrueba = { 0 };
GPIO_Handler_t MOSIPrueba = { 0 };
GPIO_Handler_t SCKPrueba = { 0 };
ILI9341_Handler_t ili = { 0 };

//Variables para el cambio de imagen
uint16_t timer5Count = 0;
bool flagChange = 0;
uint16_t currentImage = 1;
BasicTimer_Handler_t handlerTim5 = { 0 };
uint8_t newDirVal = 0;
bool enableChange = 0;
uint16_t color = ILI9341_PURPLE;
bool manualChange = 0;
uint8_t accelSample = 0;
uint16_t umbralAccel = 3276;

//Variables relacionadas al muestreo
BasicTimer_Handler_t handlerTim4 = { 0 };
//Timer 4
bool flagTim4Calibration = 0;

//Filtro de cambio
BasicTimer_Handler_t handlerTim3 = { 0 };

// Buffer para envio de datos
char bufferData[128] = { 0 };

char fileName[23] = { 0 };
void printImage(char *fileName, uint16_t currentImageN);

/* Prototipos de las funciones */
void initSystems(void);

void Calibrate(I2C_Handler_t *mpuHandler, int16_t *pProm);

/* Para rediccionar el printf() por USART2 */
int __io_putchar(int ch) {

	// Se envía el caracter por USART2
	writeChar(&commTerm, (char) ch);
	return ch;
}

/* Función principal */
int main(void) {

	/* Reloj a 100 MHz */
	configPLL(100);

	/* Inicialización de los periféricos */
	initSystems();
	delay_ms(10);
	//Realizamos la calibración.
	i2c_writeSingleRegister(&i2c1_mpu6050, PWR_MGMT_1, 0x00);
	mpuErr = i2c_readSingleRegister(&i2c1_mpu6050, PWR_MGMT_1);
	if (mpuErr) {
		printf("Error al inicializar el MPU. Revisar la conexion\n\r");
		while (1)
			;
	} else {
		printf("MPU6050 Inicializado correctamente\n\r");
	}
	usart2_rxData = 'c';
	// Definimos rotación & Fondo de pantalla
	Ili_setRotation(&ili, 3);

	//Inicializamos tod0 en 0
	timer5Count = 0;
	flagChange = 0;
	printImage(fileName, currentImage);
	handlerTim5.ptrTIMx = TIM5; //El timer que se va a usar
	handlerTim5.TIMx_Config.TIMx_interruptEnable = 1; //Se habilitan las interrupciones
	handlerTim5.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; //Se usara en modo ascendente
	handlerTim5.TIMx_Config.TIMx_period = 50000; //Se define el periodo en este caso el led cambiara cada 250ms
	handlerTim5.TIMx_Config.TIMx_speed = BTIMER_SPEED_100us; //Se define la "velocidad" que se usara

	BasicTimer_Config(&handlerTim5); //Se carga la configuración.

	// Se monta la microSD --> 1 para montarla inmediatamente
	//					   --> 0 para montarla luego
	res = f_mount(&FatFs, "", 1);
	if (res != FR_OK) {
		// Se presenta el mensaje de error
		printf("f_mount error (%i)\r\n", res);
		while (1)
			;
	} else {
		printf("SD montada\n\r");
	}

	/* Main loop */
	while (1) {
		//sprintf(fileName, "%d.txt", 1);
		//printImage(fileName);
		if (newDirVal && enableChange) {
			enableChange = 0;
			TurnOffTimer(&handlerTim4);
			if (newDirVal == LEFT) {
				TurnOffTimer(&handlerTim5);

				timer5Count = 0;
				if (currentImage == 1) {
					currentImage = limitImage;
				} else {
					currentImage--;
				}
				printImage(fileName, currentImage);
				TurnOnTimer(&handlerTim5);

			} else if (newDirVal == RIGHT) {
				TurnOffTimer(&handlerTim5);
				timer5Count = 0;
				if (currentImage < limitImage) {
					currentImage++;
				} else {
					currentImage = 1;
				}
				printImage(fileName, currentImage);
				TurnOnTimer(&handlerTim5);
			}
			if (newDirVal == UP) {
				timChangeIm++;

				sprintf(bufferData, "%d", timChangeIm);
				ILI9341_WriteString(&ili, 0, 0, color, 0, bufferData, 3, 3);

			} else if (newDirVal == DOWN) {
				if (timChangeIm != 2) {
					timChangeIm--;
					ILI9341_WriteString(&ili, 0, 0, color, 0, bufferData, 3, 3);
				}
			}
			newDirVal = 0;
			TurnOnTimer(&handlerTim3);
			TurnOnTimer(&handlerTim4);
		}
		if (flagChange) {
			flagChange = 0;
			newDirVal = RIGHT;
		}
		/* Para la lectura de todos los sensores */
		if (usart2_rxData != 0) {
			if (usart2_rxData == 'a') {

				// Se leen los registros
				mpu6050_readAll(&i2c1_mpu6050, mpu6050_data);

				// Informar en pantalla la lectura del Accel Z
				printf(
						"AccX: %i, AccY: %i, AccZ: %i, GyrX: %i, GyrY: %i, GyrZ: %i\n\r",
						mpu6050_data[0], mpu6050_data[1], mpu6050_data[2],
						mpu6050_data[3], mpu6050_data[4], mpu6050_data[5]);

				// Se limpia el caracter leído
				usart2_rxData = '\0';
			} // Si el dato recibido por USART2 es 'c', se inicia el proceso de calibración.
			else if (usart2_rxData == 'c') {

				// Se activa la bandera para indicar que se está en proceso de calibración.
				flagTim4Calibration = 1;

				// Se toman 32 muestras para cada sensor con el objetivo de realizar un promedio.
				while (counter < 32) {
					// Si la medida está lista para ser tomada...
					if (measure) {
						// Se leen todos los valores del acelerómetro y del giroscopio.
						mpu6050_readAll(&i2c1_mpu6050, mpu6050_data);

						// Se suman los valores de cada sensor para posterior cálculo de promedio.
						sumaSensores[0] += mpu6050_data[0];  // AccX
						sumaSensores[1] += mpu6050_data[1];  // AccY
						sumaSensores[2] += mpu6050_data[2];  // AccZ
						sumaSensores[3] += mpu6050_data[3];  // GyrX
						sumaSensores[4] += mpu6050_data[4];  // GyrY
						sumaSensores[5] += mpu6050_data[5];  // GyrZ

						// Se reinicia la bandera de medida.
						measure = RESET;
					} else {
						// Si la medida no está lista, se ejecuta una instrucción sin operación
						// (esperando al siguiente muestreo o interrupción que cambie el estado de 'measure').
						__NOP();
					}
				}

				// Se reinicia el contador de muestras.
				counter = 0;

				// Se desactiva la bandera de calibración.
				flagTim4Calibration = 0;

				for (uint8_t i = 0; i < 6; i++) {
					// Se calcula el promedio de los 32 datos recolectados para cada sensor.
					promedioSensores[i] = (sumaSensores[i] >> 5); // División por 32 usando desplazamiento a la derecha.

					// Se reinicia el acumulador de la suma para futuras calibraciones.
					sumaSensores[i] = 0;
				}

				// Se invoca la función de calibración con los valores promedio calculados.
				Calibrate(&i2c1_mpu6050, promedioSensores);
			}

		}
	}
	return 0;
}

/* Inicialización de todos los periféricos que componen el proyecto */
void initSystems(void) {
	//Activacion cooprocesador matematico
	SCB->CPACR |= (0xF << 20);

	config_SysTick();

	/* Configuración de los pines sobre los que funciona el I2C1 */
	// SCL
	mpu6050_sclPin.pGPIOx = GPIOB;
	mpu6050_sclPin.GPIO_PinConfig_t.GPIO_PinNumber = PIN_8;
	mpu6050_sclPin.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	mpu6050_sclPin.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_OPENDRAIN;
	mpu6050_sclPin.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	mpu6050_sclPin.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_FAST;
	mpu6050_sclPin.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF4;

	GPIO_Config(&mpu6050_sclPin);

	// SDA
	mpu6050_sdaPin.pGPIOx = GPIOB;
	mpu6050_sdaPin.GPIO_PinConfig_t.GPIO_PinNumber = PIN_9;
	mpu6050_sdaPin.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	mpu6050_sdaPin.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_OPENDRAIN;
	mpu6050_sdaPin.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	mpu6050_sdaPin.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_FAST;
	mpu6050_sdaPin.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF4;

	/* Configuración I2C1 */
	i2c1_mpu6050.ptrI2Cx = I2C1;
	i2c1_mpu6050.modeI2C = I2C_MODE_FM;
	i2c1_mpu6050.slaveAddress = 0b1101000; 	// Dirección del slave. AD0 --> GND

	i2c_config(&i2c1_mpu6050);

	/* LED de estado */
	handlerLedState.pGPIOx = GPIOB;
	handlerLedState.GPIO_PinConfig_t.GPIO_PinNumber = PIN_7;
	handlerLedState.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_OUT;
	handlerLedState.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	handlerLedState.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	handlerLedState.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_MEDIUM;
	handlerLedState.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF14;

	// Cargamos la configuración del pin LedState.
	GPIO_Config(&handlerLedState);

	/* USART Rx */
	handlerUsartRx.pGPIOx = GPIOA;
	handlerUsartRx.GPIO_PinConfig_t.GPIO_PinNumber = PIN_3;
	handlerUsartRx.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	handlerUsartRx.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	handlerUsartRx.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	handlerUsartRx.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_HIGH;
	handlerUsartRx.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF7;

	GPIO_Config(&handlerUsartRx);

	/* USART Tx */
	handlerUsartTx.pGPIOx = GPIOA;
	handlerUsartTx.GPIO_PinConfig_t.GPIO_PinNumber = PIN_2;
	handlerUsartTx.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	handlerUsartTx.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	handlerUsartTx.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	handlerUsartTx.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_HIGH;
	handlerUsartTx.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF7;

	GPIO_Config(&handlerUsartTx);

	/* Ahora configuramos el periférico */
	commTerm.ptrUSARTx = USART2;
	commTerm.USART_Config.USART_mode = USART_MODE_RXTX;
	commTerm.USART_Config.USART_baudrate = USART_BAUDRATE_115200;
	commTerm.USART_Config.USART_parity = USART_PARITY_NONE;
	commTerm.USART_Config.USART_stopbits = USART_STOPBIT_1;
	commTerm.USART_Config.USART_RX_Int_Ena = ENABLE;
	commTerm.USART_Config.USART_TX_Int_Ena = DISABLE;

	USART_Config(&commTerm);

	/* Configuración del pin para el botón de usuario */
	handlerUserButtonPin.pGPIOx = GPIOA;
	handlerUserButtonPin.GPIO_PinConfig_t.GPIO_PinNumber = PIN_0;
	handlerUserButtonPin.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_IN;
	handlerUserButtonPin.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	handlerUserButtonPin.GPIO_PinConfig_t.GPIO_PinPuPdControl =
	GPIO_PUPDR_PULLUP;
	handlerUserButtonPin.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_HIGH;
	handlerUserButtonPin.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF0;

	/* Configuración EXTI0 */
	handlerUserButton.pGPIOHandler = &handlerUserButtonPin;
	handlerUserButton.edgeType = EXTERNAL_INTERRUPT_RISING_EDGE;

	extInt_Config(&handlerUserButton);

	GPIO_Config(&mpu6050_sdaPin);

	// Configuramos el TimerOK, para que genere una interrupción periódica.
	handlerTimerOkState.ptrTIMx = TIM2;
	handlerTimerOkState.TIMx_Config.TIMx_mode = BTIMER_MODE_UP;
	handlerTimerOkState.TIMx_Config.TIMx_period = 5000;	// Me debe generar una int cada 500 ms
	handlerTimerOkState.TIMx_Config.TIMx_speed = 10000;	//
	handlerTimerOkState.TIMx_Config.TIMx_interruptEnable = ENABLE;

	// Se carga la configuración del timer 2
	BasicTimer_Config(&handlerTimerOkState);

	//Timer para el periodo de descanso
	handlerTim3.ptrTIMx = TIM3; //El timer que se va a usar
	handlerTim3.TIMx_Config.TIMx_interruptEnable = 1; //Se habilitan las interrupciones
	handlerTim3.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; //Se usara en modo ascendente
	handlerTim3.TIMx_Config.TIMx_period = 2500 * 2; //Se define el periodo en este caso el default es 50 ms.
	handlerTim3.TIMx_Config.TIMx_speed = BTIMER_SPEED_100us; //Se define la "velocidad" que se usara
	handlerTim3.TIMx_Config.TIMx_OPM = ENABLE; //Funcion que nos permite que el timer cuente solo una vez y toque habilitarlo para volver a usarlo.

	BasicTimer_Config(&handlerTim3); //Se carga la configuración.

	//Pines necesarios para el uso de la pantalla

	CSPrueba.pGPIOx = GPIOA;
	CSPrueba.GPIO_PinConfig_t.GPIO_PinNumber = PIN_8;
	CSPrueba.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_OUT;
	CSPrueba.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	CSPrueba.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	CSPrueba.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_HIGH;

	GPIO_Config(&CSPrueba);
	GPIO_WritePin(&CSPrueba, SET);

	RSTPrueba.pGPIOx = GPIOB;
	RSTPrueba.GPIO_PinConfig_t.GPIO_PinNumber = PIN_10;
	RSTPrueba.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_OUT;
	RSTPrueba.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	RSTPrueba.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	RSTPrueba.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_HIGH;

	GPIO_Config(&RSTPrueba);
	GPIO_WritePin(&RSTPrueba, SET);

	DCPrueba.pGPIOx = GPIOB;
	DCPrueba.GPIO_PinConfig_t.GPIO_PinNumber = PIN_4;
	DCPrueba.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_OUT;
	DCPrueba.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	DCPrueba.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	DCPrueba.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_HIGH;

	GPIO_Config(&DCPrueba);
	GPIO_WritePin(&DCPrueba, SET);

	MOSIPrueba.pGPIOx = GPIOB;
	MOSIPrueba.GPIO_PinConfig_t.GPIO_PinNumber = PIN_5;
	MOSIPrueba.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	MOSIPrueba.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	MOSIPrueba.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	MOSIPrueba.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_HIGH;
	MOSIPrueba.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF5;
	GPIO_Config(&MOSIPrueba);
	GPIO_WritePin(&MOSIPrueba, SET);

	SCKPrueba.pGPIOx = GPIOB;
	SCKPrueba.GPIO_PinConfig_t.GPIO_PinNumber = PIN_3;
	SCKPrueba.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_ALTFN;
	SCKPrueba.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	SCKPrueba.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	SCKPrueba.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_HIGH;
	SCKPrueba.GPIO_PinConfig_t.GPIO_PinAltFunMode = AF5;

	GPIO_Config(&SCKPrueba);
	GPIO_WritePin(&SCKPrueba, SET);

	SPIPrueba.ptrSPIx = SPI1;
	SPIPrueba.SPI_Config.BaudRatePrescaler = SPI_BAUDRATE_DIV8;
	SPIPrueba.SPI_Config.CPOLCPHA = SPI_CPOLCPHA_MODE_00;
	SPIPrueba.SPI_Config.DataSize = SPI_DATASIZE_8BIT;
	SPIPrueba.SPI_Config.FirstBit = SPI_FIRSTBIT_MSB;
	SPIPrueba.SPI_Config.Mode = SPI_MODE_MASTER;

	//Handler de la pantalla

	ili.spi_handler = &SPIPrueba;
	ili.cs_pin = &CSPrueba;
	ili.dc_pin = &DCPrueba;
	ili.reset_pin = &RSTPrueba;

	ILI9341_Init(&ili);

	//Para el muestreo
	handlerTim4.ptrTIMx = TIM4; //El timer que se va a usar
	handlerTim4.TIMx_Config.TIMx_interruptEnable = 1; //Se habilitan las interrupciones
	handlerTim4.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; //Se usará en modo ascendente
	handlerTim4.TIMx_Config.TIMx_period = 1000; // 10 ms considerando una base de tiempo de 10 µs
	handlerTim4.TIMx_Config.TIMx_speed = BTIMER_SPEED_10us;

	BasicTimer_Config(&handlerTim4);
	TurnOnTimer(&handlerTim4);

}

void printImage(char *fileName, uint16_t currentImageN) {

	// Formatea el nombre del archivo basado en el número de imagen actual. Ejemplo: "5.txt"
	sprintf(fileName, "%d.txt", currentImageN);

	// Intenta abrir el archivo con el nombre formateado en modo lectura
	if (f_open(&fil, fileName, FA_READ) == FR_OK) {

		// Inicia la comunicación con el display ILI9341
		Ili_starCom(&ili);

		// Establece la ventana de direcciones para el área completa del display
		ILI9341_SetAddressWindow(&ili, 0, 0, 320 - 1, 240 - 1);

		// Buffer para leer chunks del archivo
		char buffer[chunk_size + 1];
		UINT bytesRead;

		int index = 0; // Índice para llevar el control de los datos de la imagen procesados

		// Lee el archivo por chunks hasta que se procesen 153600 valores o se alcance el final del archivo
		while (index < 153600
				&& f_read(&fil, buffer, chunk_size, &bytesRead) == FR_OK
				&& bytesRead > 0) {

			// Busca la última coma en el chunk leído
			for (int i = bytesRead - 1; i >= 0; i--) {
				if (buffer[i] == ',') {
					// Si se encuentra una coma, retrocede el puntero del archivo para asegurar que el siguiente
					// chunk comience con un número completo, no un número truncado.
					f_lseek(&fil, f_tell(&fil) - (bytesRead - i) + 1);
					buffer[i] = '\0'; // Termina la cadena en la última coma encontrada
					break;
				}
			}

			// Divide el buffer en tokens basados en comas (","), que se asume que separan valores de pixel en el archivo
			char *token = strtok(buffer, ",");
			while (token != NULL) {
				uint8_t value = atoi(token); // Convierte el token a un número entero

				// Asegura que el valor esté dentro del rango válido (0-255) para un byte
				if (value >= 0 && value <= 255) {
					// Escribe el valor del pixel en el display
					ILI9341_WriteData(&ili, &value, 1);
					index++;  // Incrementa el índice tras procesar un valor
				}

				// Verifica que no excedamos el tamaño esperado de la imagen
				if (index >= 153600) {
					break;
				}

				// Obtiene el siguiente token en el buffer
				token = strtok(NULL, ",");
			}
		}

		// Cierra el archivo de imagen después de leerlo
		f_close(&fil);

		// Termina la comunicación con el display ILI9341
		Ili_endCom(&ili);
	}
}

/* Callback de la interrupción del TIM2*/
void BasicTimer2_Callback(void) {
	GPIOxTooglePin(&handlerLedState);
}

/* Callback de la interrupción del TIM3*/
void BasicTimer3_Callback(void) {
	enableChange = 1;
}

/* Callback que se ejecuta cada vez que se recibe un carácter */
void USART2Rx_Callback(void) {
	usart2_rxData = getRxData();
}

/**
 * Esta función se invoca como un callback para el timer básico 4.
 * Se usa para detectar el movimiento en el acelerómetro y tomar medidas de calibración.
 */
void BasicTimer4_Callback(void) {
	// Si se produce un cambio manual, incrementa la muestra del acelerómetro.
	if (manualChange) {
		accelSample++;
	}

	// Si la bandera de calibración está establecida, incrementa el contador de medidas.
	if (flagTim4Calibration) {
		measure = SET;
		counter++;
	}

	// Si la muestra del acelerómetro excede un cierto umbral, se resetea y se leen los datos del MPU6050.
	if (accelSample > 7) {
		accelSample = 0;
		mpu6050_readAccel(&i2c1_mpu6050, mpu6050_data);

		// Recupera las coordenadas X e Y del acelerómetro.
		int16_t AccX = mpu6050_data[0];
		int16_t AccY = mpu6050_data[1];

		// Determina la dirección basada en los valores de X e Y comparados con un umbral.
		if (AccY > umbralAccel) {
			newDirVal = RIGHT;
		} else if (AccY < -umbralAccel) {
			newDirVal = LEFT;
		} else if (AccX > umbralAccel) {
			newDirVal = UP;
		} else if (AccX < -umbralAccel) {
			newDirVal = DOWN;
		}
	}
}

/**
 * Esta función calibra el MPU6050 usando promedios de valores anteriores.
 *
 * @param mpuHandler: Controlador del MPU6050.
 * @param pProm: Puntero a un arreglo que contiene los promedios a usar para la calibración.
 */
void Calibrate(I2C_Handler_t *mpuHandler, int16_t *pProm) {
	// Arreglo para almacenar los offsets actuales del MPU6050.
	int16_t offsets[6];

	// Obtiene los offsets actuales del MPU6050.
	mpu6050_getOffsets(mpuHandler, offsets);

	// Ajusta los offsets basados en los promedios proporcionados.
	for (int i = 0; i < 6; i++) {
		offsets[i] -= *(pProm + i) / 20;
	}

	// Establece los nuevos offsets para el MPU6050.
	mpu6050_setOffsets(mpuHandler, offsets);

	// Comprueba si cualquier valor promedio excede un umbral (20 en este caso).
	bool needCalibration = false;
	for (int i = 0; i < 6; i++) {
		if (fabs(*(pProm + i)) > 20) {
			needCalibration = true;
			break;
		}
	}

	if (needCalibration) {
		// Si algún valor promedio excede el umbral, indica que la calibración es necesaria.
		printf("\nCalibrando el MPU\n");

		// Indica que se debe repetir el ciclo de calibración.
		usart2_rxData = 'c';
		flagTim4Calibration = 1;
	} else {
		// Si todos los valores promedio están dentro de los límites, indica que la calibración está completa.
		usart2_rxData = '\0';
		printf("\n~ Calibracion completa MPU ~\n");
	}
}

/* Función Callback para el EXTI0 (PA0) (User button) */
void callback_extInt0(void) {
	//Activamos la transición manual.
	manualChange ^= 1;
}

