/**
 ******************************************************************************
 * @file           : main_accelFatFs.c
 * @author         : emangelma
 * @brief          : Uso de librerías FatFs y microSD con adaptador SPI.
 ******************************************************************************
 *
 ******************************************************************************
 */

#include "stdint.h"
#include "string.h"

#include "stm32f4xx.h"

/* Includes */
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

/* Defines */
#define 		FFT_LEN_128		128
uint16_t chunk_size = 512;
I2C_Handler_t i2c1_mpu6050 = { 0 };
/* Definición de handlers para los periféricos a utilizar */
GPIO_Handler_t handlerLedState = { 0 }; 	// PB7 ---- GPIO
GPIO_Handler_t pinAuxLED = { 0 };	// ?
GPIO_Handler_t handlerUsartRx = { 0 };
GPIO_Handler_t handlerUsartTx = { 0 };
GPIO_Handler_t handlerUserButtonPin = { 0 };	// PA0
GPIO_Handler_t mpu6050_sdaPin = { 0 };	//
GPIO_Handler_t mpu6050_sclPin = { 0 };	//

BasicTimer_Handler_t handlerTimerOkState = { 0 }; //---- TIM
BasicTimer_Handler_t samplingTimer = { 0 };

USART_Handler_t commTerm = { 0 }; //-------------------- USART

EXTI_Config_t handlerUserButton = { 0 }; //------------- EXTI

// SD->SPI2 -- [MOSI->PB15] [MISO->PB14] [SCK->PB13] [CS->PB1]

/* Para el trabajo con la SD y los archivos */
FATFS FatFs;	// Objeto FatFs para manejo del disco
FIL fil;	// Objeto Fil para manejo de archivos
FRESULT res;	// Para la búsqueda de errores

// Para determinar el espacio libre de la SD
DWORD free_clusters, free_sectors, total_sectors;
FATFS *getFreeFs;

// Arreglo donde se almacenará la información leída
BYTE readBuf[64];

// Para comparar los bytes escritos
UINT bytesWrote;

int16_t mpu6050_data[6] = { 0 };
float mpuNormData[6] = { 0 };
/* Para el archivo .csv */
char csv[6144] = { 0 };

/* Arreglo para los sensores */
int accelX[128] = { 0 };
int accelY[128] = { 0 };
int accelZ[128] = { 0 };
int gyroX[128] = { 0 };
int gyroY[128] = { 0 };
int gyroZ[128] = { 0 };

/* Para los promedios de los sensores */
int32_t sumaSensores[6] = { 0 };
int16_t promedioSensores[6] = { 0 };

/* Variables auxiliares */
volatile uint8_t usart2_rxData = '\0';
volatile uint8_t counter = 0;
volatile DSTATUS stat = 2;
volatile uint8_t buttonFlag = RESET;
volatile uint8_t measure = RESET;
volatile uint8_t createCSV = SET;
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

char fileName[23] = { 0 };
void printImage(char *fileName);

/* Prototipos de las funciones */
void initSystems(void);

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
	// Definimos rotación & Fondo de pantalla
	Ili_setRotation(&ili, 3);

	/* Inicialización de la fft */

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

	sprintf(fileName, "%d.txt", 1);
	printImage(fileName);
//	/* Revisión del espacio de la memoria */
//	res = f_getfree("", &free_clusters, &getFreeFs);
//	if(res != FR_OK){
//		// Se presenta el mensaje de error
//		printf("f_getfree error (%i)\r\n", res);
//		while(1);
//	}
//
//	// La fórmula viene de la documentación de ChaN
//	total_sectors = (getFreeFs->n_fatent - 2) * getFreeFs->csize;
//	free_sectors = free_clusters * getFreeFs->csize;
//
//	// Se muestra en pantalla la información de la microSD.
//	printf("SD card stats:\r\n%10lu KiB total drive space.\r\n%10lu KiB available.\r\n",
//			total_sectors / 2, free_sectors / 2);

//	// Se desmonta la microSD *NOTA: ¡Es muy importante desmontar la microSD!
//	f_mount(NULL, "", 1);
//
//	// Informar en pantalla
//	printf("Se desmonta la SD\n\r");

	// Prueba de la comunicación con el acelerómetro
//	i2c_writeSingleRegister(&i2c1_mpu6050, PWR_MGMT_1, 0x00);
//	mpuErr = i2c_readSingleRegister(&i2c1_mpu6050, PWR_MGMT_1);
//	if(mpuErr){
//		printf("Error al inicializar el MPU. Revisar la conexion\n\r");
//		while(1);
//	}else{
//		printf("MPU6050 Inicializado correctamente\n\r");
//	}

	/* Main loop */
	while (SET) {

		/* Para desmontar la SD */
		if (usart2_rxData == 'd') {

			// Se desmonta la microSD *NOTA: ¡Es muy importante desmontar la microSD!
			f_mount(NULL, "", 1);

			// Informar en pantalla
			printf("Se desmonta la SD\n\r");

			// Se limpia el caracter leído
			usart2_rxData = '\0';
		}

		/* Para la lectura de todos los sensores */
		else if (usart2_rxData == 'a') {

			// Se leen los registros
			mpu6050_readAll(&i2c1_mpu6050, mpu6050_data);

			// Informar en pantalla la lectura del Accel Z
			printf(
					"AccX: %i, AccY: %i, AccZ: %i, GyrX: %i, GyrY: %i, GyrZ: %i\n\r",
					mpu6050_data[0], mpu6050_data[1], mpu6050_data[2],
					mpu6050_data[3], mpu6050_data[4], mpu6050_data[5]);

			// Se limpia el caracter leído
			usart2_rxData = '\0';
		}

		/* Para la lectura de todos los sensores en float */
		else if (usart2_rxData == 'f') {

			// Se leen los registros

			// Informar en pantalla la lectura del Accel Z
			printf(
					"AccX: %.2f, AccY: %.2f, AccZ: %.2f, GyrX: %.2f, GyrY: %.2f, GyrZ: %.2f\n\r",
					mpuNormData[0], mpuNormData[1], mpuNormData[2],
					mpuNormData[3], mpuNormData[4], mpuNormData[5]);

			// Se limpia el caracter leído
			usart2_rxData = '\0';
		}

		/* --- Calibración del MPU --- */
		else if (usart2_rxData == 'c') {

			// Activación del timer 3

			// Se toman 32 datos por cada sensor para realizar un promedio
			while (counter < 32) {
				if (measure) {
					// Lectura de todos los acelerómetros y giroscopios

					sumaSensores[0] += mpu6050_data[0];	// AccX
					sumaSensores[1] += mpu6050_data[1];	// AccY
					sumaSensores[2] += mpu6050_data[2];	// AccZ
					sumaSensores[3] += mpu6050_data[3];	// GyrX
					sumaSensores[4] += mpu6050_data[4];	// GyrY
					sumaSensores[5] += mpu6050_data[5];	// GyrZ

					measure = RESET;
				} else {
					__NOP(); 				// Se espera al siguiente muestreo
				}
			}
			counter = 0;

			for (uint8_t i = 0; i < 6; i++) {
				// Se realiza el promedio de los 32 datos por cada sensor
				promedioSensores[i] = (sumaSensores[i] >> 5);

				// Se reinicia el arreglo de la suma
				sumaSensores[i] = 0;
			}

		}

		// Se revisa si se levanta la bandera del botón
		else if (usart2_rxData == 'm') {

			printf("Prueba que no eres un muggle! . . .\n\r");

			// Encender LED
			GPIO_WritePin(&pinAuxLED, SET);

			// Activación del timer 3

			// Se reinicia el contador y se activa la medición
			counter = 0;
			measure = SET;

			// Se llenan los arreglos correspondientes a los sensores
			while (counter < 128) {
				if (measure) {
					// Lectura de todos los acelerómetros y giroscopios

					accelX[counter] = mpu6050_data[0];
					accelY[counter] = mpu6050_data[1];
					accelZ[counter] = mpu6050_data[2];
					gyroX[counter] = mpu6050_data[3];
					gyroY[counter] = mpu6050_data[4];
					gyroZ[counter] = mpu6050_data[5];

					measure = RESET;
				} else {
					__NOP(); 				// Se espera al siguiente muestreo
				}
			}

			// Mensaje que indica que ya no se están tomando datos
			printf("Organizando el archivo...");

			// Se abre el archivo en append mode
			res = f_open(&fil, "noxx.txt",
			FA_OPEN_ALWAYS | FA_WRITE | FA_OPEN_APPEND);
			if (res != FR_OK) {
				printf("Error (%i) al abrir el archivo", res);
				while (1)
					;
			}

			// Se escriben los datos en el archivo
			res = f_write(&fil, csv, strlen(csv), &bytesWrote);
			if (res != FR_OK) {
				printf("Error (%i) al escribir en el archivo", res);
				while (1)
					;
			}

			// Se cierra el archivo
			res = f_close(&fil);

			// Informar la escritura en pantalla
			printf("Se escribieron los datos en la SD\tSpell Count: %u\n\r",
					spellCount);
			spellCount++;		// Para ver cuántos hechizos van

			// Se ubica el puntero al final del archivo para la escritura de nuevos datos
			f_lseek(&fil, f_size(&fil));

			// Apagar LED
//			GPIO_WritePin(&pinAuxLED, RESET);

			// Se baja la bandera
//			buttonFlag = RESET;
//			if(spellCount == 50){usart2_rxData = '\0'; spellCount = 0;}
			usart2_rxData = '\0';
		}

//		/* Para la lectura del Accel X */
//		else if(usart2_rxData == 'x'){
//
//			// Informar en pantalla la lectura del Accel X
//			printf("AccelX = %i\n\r", mpu6050_readAccelX(&i2c1_mpu6050));
//
//			// Informar en pantalla la lectura del Gyro X
//			printf("GyroX = %i\n\r", mpu6050_readGyroX(&i2c1_mpu6050));
//
//			// Se limpia el caracter leído
//			usart2_rxData = '\0';
//		}
//
//		/* Para la lectura del Accel Y */
//		else if(usart2_rxData == 'y'){
//
//			// Informar en pantalla la lectura del Accel Y
//			printf("AccelY = %i\n\r", mpu6050_readAccelY(&i2c1_mpu6050));
//
//			// Informar en pantalla la lectura del Gyro Y
//			printf("GyroY = %i\n\r", mpu6050_readGyroY(&i2c1_mpu6050));
//
//			// Se limpia el caracter leído
//			usart2_rxData = '\0';
//		}
//
//		/* Para la lectura del Accel Z */
//		else if(usart2_rxData == 'z'){
//
//			// Informar en pantalla la lectura del Accel Z
//			printf("AccelZ = %i\n\r", mpu6050_readAccelZ(&i2c1_mpu6050));
//
//			// Informar en pantalla la lectura del Gyro Z
//			printf("GyroZ = %i\n\r", mpu6050_readGyroZ(&i2c1_mpu6050));
//
//			// Se limpia el caracter leído
//			usart2_rxData = '\0';
//		}
	}
	return 0;
}

/* Inicialización de todos los periféricos que componen el proyecto */
void initSystems(void) {
	//Activacion cooprocesador matematico
	SCB->CPACR |= (0xF << 20);

	config_SysTick();

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

//	/* LED auxiliar PA10*/
//	pinAuxLED.pGPIOx										= GPIOA;
//	pinAuxLED.GPIO_PinConfig_t.GPIO_PinNumber					= PIN_10;
//	pinAuxLED.GPIO_PinConfig_t.GPIO_PinMode					= GPIO_MODE_OUT;
//	pinAuxLED.GPIO_PinConfig_t.GPIO_PinOPType					= GPIO_OTYPE_PUSHPULL;
//	pinAuxLED.GPIO_PinConfig_t.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
//	pinAuxLED.GPIO_PinConfig_t.GPIO_PinSpeed					= GPIO_OSPEED_MEDIUM;
//	pinAuxLED.GPIO_PinConfig_t.GPIO_PinAltFunMode				= AF14;
//
//	// Cargamos la configuración del pin AuxLED.
//	GPIO_Config(&pinAuxLED);

	// Apagar LED
//	GPIO_WritePin(&pinAuxLED, RESET);

	// Configuramos los pines relacionados al puerto serial
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

	GPIO_Config(&mpu6050_sdaPin);

	// Configuramos el TimerOK, para que genere una interrupción periódica.
	handlerTimerOkState.ptrTIMx = TIM2;
	handlerTimerOkState.TIMx_Config.TIMx_mode = BTIMER_MODE_UP;
	handlerTimerOkState.TIMx_Config.TIMx_period = 5000;	// Me debe generar una int cada 500 ms
	handlerTimerOkState.TIMx_Config.TIMx_speed = 10000;	//
	handlerTimerOkState.TIMx_Config.TIMx_interruptEnable = ENABLE;

	// Se carga la configuración del timer 2
	BasicTimer_Config(&handlerTimerOkState);

	// Timer para el muestreo de los sensores
	samplingTimer.ptrTIMx = TIM3;
	samplingTimer.TIMx_Config.TIMx_mode = BTIMER_MODE_UP;
	samplingTimer.TIMx_Config.TIMx_period = 100;// Me debe generar una int cada 10 ms
	samplingTimer.TIMx_Config.TIMx_speed = 10000;		// 100 us
	samplingTimer.TIMx_Config.TIMx_interruptEnable = ENABLE;

	// Se carga la configuración del timer 3
	BasicTimer_Config(&samplingTimer);

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
	/* Configuración I2C1 */
	i2c1_mpu6050.ptrI2Cx = I2C1;
	i2c1_mpu6050.modeI2C = I2C_MODE_FM;
	i2c1_mpu6050.slaveAddress = 0b1101000; 	// Dirección del slave. AD0 --> GND

	i2c_config(&i2c1_mpu6050);

}

void printImage(char *fileName) {

	if (f_open(&fil, fileName, FA_READ) == FR_OK) { // Cambio el nombre del archivo a "1.txt"

		Ili_starCom(&ili);
		ILI9341_SetAddressWindow(&ili, 0, 0, 320 - 1, 240 - 1);

		char buffer[chunk_size + 1];
		UINT bytesRead;
		int index = 0;  // Índice para llenar partImage
		while (index < 153600
				&& f_read(&fil, buffer, chunk_size, &bytesRead) == FR_OK
				&& bytesRead > 0) {

			// Busca la última coma
			for (int i = bytesRead - 1; i >= 0; i--) {
				if (buffer[i] == ',') {
					// Retrocede el puntero de archivo para comenzar desde el número completo en el siguiente chunk
					f_lseek(&fil, f_tell(&fil) - (bytesRead - i) + 1);
					buffer[i] = '\0'; // Termina la cadena en la última coma encontrada
					break;
				}
			}

			// Tokenizamos el buffer
			char *token = strtok(buffer, ",");
			while (token != NULL) {
				uint8_t value = atoi(token);
				if (value >= 0 && value <= 255) {
					ILI9341_WriteData(&ili, &value, 1);
					index++;  // Aumenta el índice después de procesar un valor
				}

				// Asegurar que no excedemos el tamaño del arreglo
				if (index >= 153600) {
					break;
				}

				token = strtok(NULL, ",");
			}
		}

		f_close(&fil);
		Ili_endCom(&ili);
	}
}

/* Callback de la interrupción del TIM2*/
void BasicTimer2_Callback(void) {
	GPIOxTooglePin(&handlerLedState);
}

/* Callback de la interrupción del TIM3*/
void BasicTimer3_Callback(void) {
	measure = SET;
	counter++;
}

/* Callback que se ejecuta cada vez que se recibe un carácter */
void USART2Rx_Callback(void) {
	usart2_rxData = getRxData();
}

/* Función Callback para el EXTI0 (PA0) (User button) */
void callback_extInt0(void) {
	buttonFlag = SET;
	printf("--- User button presionado ---\n\r");
}
