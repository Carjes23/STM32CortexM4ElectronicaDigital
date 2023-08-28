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

//Se incluyen las librerias necesarias para el desarrollo del proyecto
#include <stdint.h>
#include <stm32f4xx.h>
#include "GPIOxDriver.h"
#include "USARTxDriver.h"
#include "BasicTimer.h"
#include <stdbool.h>
#include "SysTick.h"
#include <arm_math.h>
#include "PwmDriver.h"
#include "PLLDriver.h"
#include "I2CxDriver.h"
#include "LCDI2C.h"

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

//Registr necesario para comunicarse con el modulo I2C de la LCD

#define LCD_ADDRES 0x21 //O 0x21

/*
 * Variables definidas para la tarea.
 */

//Variables para el tratamento de la aceleración en X
uint8_t AccelX_low = 0;
uint8_t AccelX_high = 0;
int16_t AccelX = 0;
float AccelXEsc = 0.0f;
float xmean = 0.f;
uint8_t ofsX = 0;

//Variables para el tratamento de la aceleración en y
uint8_t AccelY_low = 0;
uint8_t AccelY_high = 0;
int16_t AccelY = 0;
float AccelYEsc = 0.0f;
float ymean = 0.f;
uint8_t ofsY = 0;

//Variables para el tratamento de la aceleración en z
uint8_t AccelZ_low = 0;
uint8_t AccelZ_high = 0;
int16_t AccelZ = 0;
float AccelZEsc = 0.0f;
float zmean = 0.f;
uint8_t ofsZ = 0;

//Banderas para marcar eventos
bool banderaLedUsuario = 0; //Bandera TIM2 el cual nos da el blinking del LD2 periodo 250 ms
bool flagDatos = 0; //Bandera que nos dice que se debe tomar dato periodo 1 ms
bool flag2Segundos = 0;	//Bandera que nos dice si se activo la pregunta de 2Seg, lo que recoge
//datos por 2 segudnos y luego lo imprime por la terminal USART
bool flag2SegundosTer = 0; //BAndera que indica cuando se completaron los 2 segundos.

uint8_t usart6DataReceived = 0; //Dato recibido por el usart 6

char bufferMsg[256] = { 0 }; //Para guardar un formato de texto
char bufferMsg2[30] = { 0 }; //Para guardar un formato de texto
uint16_t i2cBuffer = 0;		 //Buffer para la recepción de datos en el I2C

//Handlers utilizados para manipular los perifericos
BasicTimer_Handler_t handlerTim2 = { 0 }; //Timer para el blinking
GPIO_Handler_t handlerUserLedPin = { 0 }; // Para manipular el led de usuario

PWM_Handler_t pwmtim3x = { 0 }; //Para configurar el PWM en el timer 3 para X
GPIO_Handler_t pwm3xpin = { 0 }; //Pin que nos entrega la señal PWM para X Pin C6

USART_Handler_t USART6Handler = { 0 }; //Para manejar el USART6
GPIO_Handler_t tx6pin = { 0 }; //Para manipular el pin de trasminsión de datos del USART6
GPIO_Handler_t rx6pin = { 0 }; //Para manipular el pin de recepcioón de datos del USART6

PWM_Handler_t pwmtim3y = { 0 }; //Para configurar el PWM enn el timer 3 para y
GPIO_Handler_t pwm3ypin = { 0 }; //Pin que nos entrega la señal PWM para Y Pin c7

PWM_Handler_t pwmtim3z = { 0 }; //Para configurar el PWM enn el timer 3 para z
GPIO_Handler_t pwm3zpin = { 0 }; //Pin que nos entrega la señal PWM para Z Pin c8

uint16_t duttyValuex = 2000; //Valor del dutty inicial
uint16_t duttyValuey = 1000; //Valor del dutty inicial
uint16_t duttyValuez = 20000; //Valor del dutty inicial

//Handler I2Cs
I2C_Handler_t i2cAcelerometro = { 0 }; //I2C encargado de comunicarse con el acelerometro
I2C_Handler_t i2cLCD = { 0 };		   //I2C encargado de comunicarse con la LCD

LCDI2C_handler_t lcdHandler = { 0 };   //Handler para manipular la LCD

GPIO_Handler_t I2cSDA = { 0 }; // Handler que manipula el envio/recepcón de datos del Acelerometro
GPIO_Handler_t I2cSCL = { 0 }; // Handler que manipula la frecuencia de comunicación del Acelerometro

GPIO_Handler_t I2cSDA2 = { 0 }; // Handler que manipula el envio/recepcón de datos de la LCD
GPIO_Handler_t I2cSCL2 = { 0 };	// Handler que manipula la frecuencia de comunicación de la LCD

BasicTimer_Handler_t handlerTim4 = { 0 }; //Timer encargado del muestro del acelerometro.

/*
 * Arreglos para guardar datos cada 10 muestreos
 */
float DatoX10[10] = { 0 };
float DatoY10[10] = { 0 };
float DatoZ10[10] = { 0 };

/*
 * Arreglos para guardar datos cuando se activa la función de los dos segundos
 */
float DatoX2000[2000] = { 0 };
float DatoY2000[2000] = { 0 };
float DatoZ2000[2000] = { 0 };

/*
 * Registros necesario para leer los ejes
 */
uint8_t regDatos[6] = { ACCEL_XOUT_L, ACCEL_XOUT_H, ACCEL_YOUT_L, ACCEL_YOUT_H,
ACCEL_ZOUT_L, ACCEL_ZOUT_H };
/*
 * Respuesta del acelerometro al solicitarle datos.
 */
uint8_t resDatos[6] = { 0 };

/*
 * Contadores que nos permiten cambiar de diferentes opcioens
 */
uint8_t datoPantalla = 0;			//Para cambiar el ultimo dato de pantalla
uint16_t contadorImprimir2000 = 0;//Para imprimir los 2000 datos luego de recibirlos, se realizara un contador global para no
//parar el sistema por tanto tiempo en un solo lugar.
uint8_t contador2seg = 0;//Prra contar 2 segundos, y luego cambiar el ultimo dato por pantalla
uint8_t contador10 = 0;			//Contador para guardar datos cada 10 muestreos
uint16_t contador2000 = 0;//Contador para guardar datos cuando se activa la funcion de los 2 segundos
uint8_t contador1seg = 0;	//Contador para contar 1 segundo usando el blinking.

//Funciones definidas para la tarea
/*
 * Funcion encargada de hallarle el promedio a un arreglo
 */
float meanFunction(float *numbers, int cantidad);
/*
 * Funcion encargada de cambiar el color del led con respecto a la gravedad.
 */
void cambiarLed(void);
void initSystem(void); // Función que inicializa el sistema
void USART6Rx_Callback(void); //Definir el callback
void VerificarUsartOcupado(void);
/*
 * Funcion que se encarga de refresacar la Led cada segundo.
 */
void LCDRefresh(void);

int main(void) {


	//Iniciamos sistma
	initSystem();

	//Parametros que no cambian en el display
	lcdMoveCursorTo(0x00); 	//Posición de memoria de la segunda linea
	sprintf(bufferMsg2, "AccX =          m/s2");
	lcdWriteMessage(bufferMsg2);
	lcdMoveCursorTo(0x40); 	//Posición de memoria de la segunda linea
	sprintf(bufferMsg2, "AccY =          m/s2");
	lcdWriteMessage(bufferMsg2);
	lcdMoveCursorTo(0x14);
	sprintf(bufferMsg2, "AccZ =          m/s2");
	lcdWriteMessage(bufferMsg2);
	lcdCursorOnOff(0);

	while (1) {
		//Se activa cada vez que el timer 2 se llena cada 250 ms y cambiamos el estado del led de usuario
		//Tambien se aprovecha para cambiar los pwm de los leds, logrando el cambio de color
		//Al cambiar los ejes.
		if(banderaLedUsuario){
			banderaLedUsuario = 0; //Se reinicia en 0
			GPIOxTooglePin(&handlerUserLedPin); //cambiamos el valor del led
		}
		if (contador1seg > 3) {
			contador1seg = 0;
			cambiarLed(); //Cambiamos los PWM para que se vea en los leds
			LCDRefresh(); //Invocamos la función que nos permite refrescar la Led.
		}
		/*
		 * Este if se activa siempre y cuando ya tengamos los 2000 datos para imprimir por
		 * Usart
		 */
		if (flag2SegundosTer == 1) {
			/*
			 * Para el primer paso mandamos el formato como se entregando los datos, luego se empiezan
			 * a recorrer los arreglos para que impriman todos los datos
			 */
			if (contadorImprimir2000 == 0) {
				writeStringInt(&USART6Handler, "x;  y;  z\n");
			}
			if (contadorImprimir2000 < 2000) {
				VerificarUsartOcupado();
				sprintf(bufferMsg, " %.2f m/s²; %.2f m/s²; %.2f m/s²; # %u\n",
						DatoX2000[contadorImprimir2000],
						DatoY2000[contadorImprimir2000],
						DatoZ2000[contadorImprimir2000],
						contadorImprimir2000 + 1);
				writeStringInt(&USART6Handler, bufferMsg);
				contadorImprimir2000++;
			} else {
				/*
				 * Cuando se termina se renician las banderas de impimir2000 para una proxima solicitud de dats
				 * y la funcion de 2 segundos terminados para que deje de entrar en este if.
				 */

				flag2SegundosTer = 0;
				contadorImprimir2000 = 0;
			}
		}

		/*
		 * Esta bandera se activa cada 1ms y le pide los datos al acelerometro
		 * También guarda los ultimos 10 datos en un arreglo y si la función
		 * de los 2 segundos se encuentra activada tambien se encarga de guardar
		 * los datos en arreglos.
		 */

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

			//Este if nos ayuda a guarar los primeros 9 datos en el arreglo de 10
			//El else se encarga de guardar el dato 10
			if (contador10 < 9) {

				DatoX10[contador10] = AccelXEsc;
				DatoY10[contador10] = AccelYEsc;
				DatoZ10[contador10] = AccelZEsc;

				contador10++;
			}

			else {
				DatoX10[9] = AccelXEsc;
				DatoY10[9] = AccelYEsc;
				DatoZ10[9] = AccelZEsc;
				contador10 = 0;
			}

			/*
			 * Si la funcion de 2 segundos se encuentra activada
			 * ese if se encarga de guardar los datos, y cuando culmine
			 * reiniciamos las banderas y activaos la bandera
			 * Flag2segundosTerminados, para que empiece el envio de datos
			 * con la funcioón vista antes
			 */

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
			//Se cierra la bandera de toma de datos.
			flagDatos = 0;
		}

		//Hacemos un "eco" con el valor que nos llega por el serial
		/*
		 * Todas las funciones tienen una estructura similar, las que leen tienen
		 * la indicación al igual que las que escriben, se usan las funciones para
		 * leer y escribir un solo registro, junto con el sprintf para mandar el formato
		 * indicado por la terminal serial
		 *
		 */
		if (usart6DataReceived != '\0') {
			//Para preguntar con quien estamos hablando
			if (usart6DataReceived == 'w') {
				i2cBuffer = i2c_readSingleRegister(&i2cAcelerometro, WHO_AM_I);
				VerificarUsartOcupado();
				sprintf(bufferMsg, "Device ID = 0x%x (read)\n", //deberia responder un 0xe5
						(unsigned int) i2cBuffer);
				writeStringInt(&USART6Handler, bufferMsg);
				usart6DataReceived = '\0';
			}
			//Para ajustar la frecuencia
			else if (usart6DataReceived == 'f') {
				VerificarUsartOcupado();
				sprintf(bufferMsg, "Cambiamos la frecuencia a 3200Hz(write)\n");
				writeStringInt(&USART6Handler, bufferMsg);
				//0x0A -> 100 Hz
				i2c_writeSingleRegister(&i2cAcelerometro, ACCEL_BW_RATE, 0x0F);
				usart6DataReceived = '\0';
			}
			//Ajustamos el formato, colocandolo es FullRes -> Maxima resolución.
			else if (usart6DataReceived == 'd') {
				VerificarUsartOcupado();
				sprintf(bufferMsg,
						"Colocamos el rango en +- 2g con maxima resolución(write)\n");
				writeStringInt(&USART6Handler, bufferMsg);

				i2cBuffer = i2c_readSingleRegister(&i2cAcelerometro,
				ACCEL_DATA_FORMAT);
				//Limpiamos el valor del rango actual
				i2cBuffer &= ~0x0F;
				i2cBuffer |= ADXL345_RANGE_2_G;

				//Habilitamos el FullRes
				i2cBuffer |= 0x08;

				i2c_writeSingleRegister(&i2cAcelerometro, ACCEL_DATA_FORMAT,
						i2cBuffer);

				usart6DataReceived = '\0';
			}

			else if (usart6DataReceived == 's') { //Para resetear al equipo
				VerificarUsartOcupado();
				sprintf(bufferMsg, "Empezar Mediciones(write)\n");
				writeStringInt(&USART6Handler, bufferMsg);

				i2c_writeSingleRegister(&i2cAcelerometro, POWER_CTL, 0x08);
				usart6DataReceived = '\0';
			}
			/*
			 * Esta sección nos da los valores actuales en los diferentes ejes.
			 * Se hace un tratamiento de datos igual al mostrado arriba.
			 */
			else if (usart6DataReceived == 'x') { //lecturas en z
				VerificarUsartOcupado();
				sprintf(bufferMsg, "Axis X(read)\n");
				writeStringInt(&USART6Handler, bufferMsg);

				AccelX_low = i2c_readSingleRegister(&i2cAcelerometro,
				ACCEL_XOUT_L);
				AccelX_high = i2c_readSingleRegister(&i2cAcelerometro,
				ACCEL_XOUT_H);
				AccelX = AccelX_high << 8 | AccelX_low;
				AccelXEsc = AccelX * FACTORESCALADO;

				VerificarUsartOcupado();
				sprintf(bufferMsg, "AccelX = %.2f \n", (float) AccelXEsc);
				writeStringInt(&USART6Handler, bufferMsg);
				usart6DataReceived = '\0';

			} else if (usart6DataReceived == 'y') { //lecturas en y

				VerificarUsartOcupado();
				sprintf(bufferMsg, "Axis Y(read)\n");
				writeStringInt(&USART6Handler, bufferMsg);

				AccelY_low = i2c_readSingleRegister(&i2cAcelerometro,
				ACCEL_YOUT_L);
				AccelY_high = i2c_readSingleRegister(&i2cAcelerometro,
				ACCEL_YOUT_H);
				AccelY = AccelY_high << 8 | AccelY_low;
				AccelYEsc = AccelY * FACTORESCALADO;

				VerificarUsartOcupado();
				sprintf(bufferMsg, "AccelY = %.2f \n", (float) AccelYEsc);
				writeStringInt(&USART6Handler, bufferMsg);
				usart6DataReceived = '\0';
			} else if (usart6DataReceived == 'z') { //lecturas en z

				VerificarUsartOcupado();
				sprintf(bufferMsg, "Axis Z(read)\n");
				writeStringInt(&USART6Handler, bufferMsg);

				AccelZ_low = i2c_readSingleRegister(&i2cAcelerometro,
				ACCEL_ZOUT_L);
				AccelZ_high = i2c_readSingleRegister(&i2cAcelerometro,
				ACCEL_ZOUT_H);
				AccelZ = AccelZ_high << 8 | AccelZ_low;
				AccelZEsc = AccelZ * FACTORESCALADO;

				VerificarUsartOcupado();
				sprintf(bufferMsg, "AccelZ = %.2f \n", (float) AccelZEsc);
				writeStringInt(&USART6Handler, bufferMsg);
				usart6DataReceived = '\0';
			} else if (usart6DataReceived == '2') { //Indica que se debe iniciar la toma de 2 segundos de datos.
				writeStringInt(&USART6Handler,
						"Se tomaran datos por 2 segundos y luego se imprimen por pantalla \n");
				flag2Segundos = 1;
				usart6DataReceived = '\0';

			}
			/*
			 * Ajuste para los offesets se basa en sumar lo necesario para que (x,y), en un primer momento sean los mas
			 * cercano posible a 0, y que z sea los más cercano a 9.8 m/s²
			 */
			else if (usart6DataReceived == 'a') {
				writeStringInt(&USART6Handler,
						"Ajustar datos (se agrega offset)\n");
				//Verificamos
				float aux = 0;
				aux = -AccelX / 4;
				ofsX = (int) round(aux);
				i2c_writeSingleRegister(&i2cAcelerometro,
				ACCEL_OFSX, ofsX);

				aux = -AccelY / 4;
				ofsY = (int) round(aux);
				i2c_writeSingleRegister(&i2cAcelerometro,
				ACCEL_OFSY, ofsY);

				aux = ((1 / ADXL345_MG2G_MULTIPLIER) - AccelZ) / 4;
				ofsZ = (int) round(aux);
				i2c_writeSingleRegister(&i2cAcelerometro,
				ACCEL_OFSZ, ofsZ);

				usart6DataReceived = '\0';

			} else { // caso default
				usart6DataReceived = '\0';
			}
		}
	}
	return 0;
}

void initSystem(void) {

	//Configuramos el PLL a 80 MHz

	configPLL(80);

	//Se cobtiene la recuencia para darsela al Systick, los otros drivers llaman esta función
	//Por si mismos auto-ajustandose a la frecuencia que se les asigne.

	uint16_t freq = getFreqPLL();

	//	Inicializamos el SysTick se le entraga el valor de la frecuencia actual del PLL
	config_SysTick();
	/*
	 * Se configuran los pines para el uso de la LCD, tal y como se ha hecho anteriormete
	 * Se utilizan en modo Pull Up la forma de usarse el I2C
	 */
	I2cSCL2.pGPIOx = GPIOA;
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
	/*
	 * Se utiliza la pantalla en FM ya que se noto que se nota menos el cambio en esta configuración
	 * Se le da el address del modulo de I2C, y en este caso usaremos el I2C3
	 */
	i2cLCD.modeI2C = I2C_MODE_FM;
	i2cLCD.slaveAddress = LCD_ADDRES;
	i2cLCD.ptrI2Cx = I2C3;
	i2c_config(&i2cLCD);

	//Se le pasa el puntero del I2C al handler de la LCD
	lcdHandler.ptrHandlerI2C = &i2cLCD;
	lcdi2cconfig(&lcdHandler);
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

	//Led de usuario usado para el blinking

	handlerUserLedPin.pGPIOx = GPIOA; //Se encuentra en el el GPIOA
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinNumber = PIN_5; // Es el pin 5.
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_OUT; //Se utiliza como salida
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL; //Salida PushPull
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING; //No se usa ninguna resistencia
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_FAST; //Se usa en velocidad rapida

	GPIO_Config(&handlerUserLedPin); //Se carga la configuración para el Led.

	//Configuración Timer 2 que controlara el blinking

	/*
	 * En este caso a diferencia de tareas pasadas es necesario usar el 100us, ya que
	 * el otro valor superaria el valor maximo del pre-escaler.
	 */
	handlerTim2.ptrTIMx = TIM2; //El timer que se va a usar
	handlerTim2.TIMx_Config.TIMx_interruptEnable = 1; //Se habilitan las interrupciones
	handlerTim2.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; //Se usara en modo ascendente
	handlerTim2.TIMx_Config.TIMx_period = 2500; //Se define el periodo en este caso el led cambiara cada 250ms
	handlerTim2.TIMx_Config.TIMx_speed = BTIMER_SPEED_100us; //Se define la "velocidad" que se usara

	BasicTimer_Config(&handlerTim2); //Se carga la configuración.

	/*
	 * Se configura los pines ncesarios para manipular el USART6 que me fue correspondido por
	 * estar en la casilla 4 de la segunda columna
	 */
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

	/*
	 * Se configura el USART6 con los requerimientos presentados
	 */

	USART6Handler.ptrUSARTx = USART6;
	USART6Handler.USART_Config.USART_baudrate = USART_BAUDRATE_115200;
	USART6Handler.USART_Config.USART_datasize = USART_DATASIZE_8BIT;
	USART6Handler.USART_Config.USART_mode = USART_MODE_RXTX;
	USART6Handler.USART_Config.USART_parity = USART_PARITY_NONE;
	USART6Handler.USART_Config.USART_stopbits = USART_STOPBIT_1;
	USART6Handler.USART_Config.USART_RX_Int_Ena = ENABLE;

	USART_Config(&USART6Handler);

	/*
	 * Se configuran los pines y los pwm, se usara el TIM3, con sus primeros 3 canales para este
	 * proposito, tal y como se muestra acontinuación
	 */

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

	//Configuración Timer 4 que la toma de datos cada 1ms

	handlerTim4.ptrTIMx = TIM4; //El timer que se va a usar
	handlerTim4.TIMx_Config.TIMx_interruptEnable = 1; //Se habilitan las interrupciones
	handlerTim4.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; //Se usara en modo ascendente
	handlerTim4.TIMx_Config.TIMx_period = 100; //Se define el periodo en este caso el led cambiara cada 250ms
	handlerTim4.TIMx_Config.TIMx_speed = BTIMER_SPEED_10us; //Se define la "velocidad" que se usara

	BasicTimer_Config(&handlerTim4); //Se carga la configuración.

}

/*
 * Función encargada de sacar promedios, usada para promediar los ultimos 10 datos.
 */
float meanFunction(float *numbers, int cantidad) {

	float res;
	float sum = 0;

	for (int i = 0; i < cantidad; i++) {
		sum += numbers[i];
	}

	res = sum / (float) cantidad;

	return res;
}

/*
 * Funcion utilizada para cambiar los pwm, que a su vez cambia los colores del led RGB.
 */
void cambiarLed(void) {

	/*
	 * Realizamos una regresión lineal en donde -2g es 0; 0 es 10000; 2g 2000
	 * Por los que nos queda (16000*ValordeXpromedio)/(2g) + 10000 = 80000/g * ValorXpromedio + 10000
	 * además de un ajuste para que no se pasen de los limites.
	 */
	xmean = meanFunction(DatoX10, 10);
	ymean = meanFunction(DatoY10, 10);
	zmean = meanFunction(DatoZ10, 10);
	float xmeanConv = 8000 * xmean / SENSORS_GRAVITY_EARTH + 10000;
	float ymeanConv = 8000 * ymean / SENSORS_GRAVITY_EARTH + 10000;
	float zmeanConv = 8000 * zmean / SENSORS_GRAVITY_EARTH + 10000;

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
	duttyValuey = (int) ymeanConv; //TODO
	duttyValuez = (int) zmeanConv;

	updateDuttyCycle(&pwmtim3x, duttyValuex);
	updateDuttyCycle(&pwmtim3y, duttyValuey);
	updateDuttyCycle(&pwmtim3z, duttyValuez);

}
/*
 * Funcion que refresca el led, se aprovecha del blinking de 250 ms, por eso tenemos algo que nos
 * iguala a 4, que nos daría un segundo, la parte de contador 2 segundos es para que la ultima linea
 * de la lcd cambia cada 2 segundos.
 */
void LCDRefresh(void) {
	if (contador1seg == 4) {
		contador2seg++;
		if (contador2seg == 2) {
			contador2seg = 0;
			if (datoPantalla < 6) {
				datoPantalla++;
			} else {
				datoPantalla = 0;
			}

		}
		contador1seg = 0;
		/*
		 * Enviar los datos a la lcd
		 */

		/*
		 * Armamos el formato que necesitamos se dejan solo
		 * dos decimales por cuestiones de incertidumbre ya que
		 * la mimima cantidad que puede medir el acelerometro es 0.04 m/s²
		 */
		lcdMoveCursorTo(0x00 + 9); 	//Posición de memoria de la segunda linea
		sprintf(bufferMsg2, "%.2f ", xmean);
		lcdWriteMessage(bufferMsg2);
		lcdMoveCursorTo(0x40 + 9); 	//Posición de memoria de la segunda linea
		sprintf(bufferMsg2, "%.2f ", ymean);
		lcdWriteMessage(bufferMsg2);
		lcdMoveCursorTo(0x14 + 9);
		sprintf(bufferMsg2, "%.2f ", zmean);
		lcdWriteMessage(bufferMsg2);
		lcdMoveCursorTo(0x54);	//Posición de memoria tercera linea
		/*
		 * Lo siguiente se encarga de la administracción de la ultima fila, esta
		 * primero nos indica si el acelerometro esta configurado para medir y nos
		 * indica como inciarlo, luego va saltando entre varias opciones cada 2 segundos
		 * donde se nos muestra el offset de cada eje la sensibildiad y otras opciones de
		 * teclas para usar en la terminal
		 */
		if (xmean == 0.0f && ymean == 0.0f && zmean == 0.0f) {
			sprintf(bufferMsg2, "AccNoActiveKey s");
			lcdWriteMessage(bufferMsg2);
			datoPantalla = 0;
		} else {
			float auxi = 0;
			switch (datoPantalla) {

			case 0:
				sprintf(bufferMsg2, "AdjustOffsetKey a   ");
				lcdWriteMessage(bufferMsg2);
				break;
			case 1:
				i2cBuffer = i2c_readSingleRegister(&i2cAcelerometro,
				ACCEL_OFSX);
				auxi = ((int8_t) i2cBuffer) * 15.6
						/ 1000* SENSORS_GRAVITY_EARTH; //15.6mg es la minima cantidad de offset
				sprintf(bufferMsg2, "OffsetX = %.2f   ", auxi);
				lcdWriteMessage(bufferMsg2);
				break;
			case 2:
				i2cBuffer = i2c_readSingleRegister(&i2cAcelerometro,
				ACCEL_OFSY);
				auxi = ((int8_t) i2cBuffer) * 15.6
						/ 1000* SENSORS_GRAVITY_EARTH;
				sprintf(bufferMsg2, "OffsetY = %.2f ", auxi);
				lcdWriteMessage(bufferMsg2);
				break;
			case 3:
				i2cBuffer = i2c_readSingleRegister(&i2cAcelerometro,
				ACCEL_OFSZ);
				auxi = ((int8_t) i2cBuffer) * 15.6
						/ 1000* SENSORS_GRAVITY_EARTH;
				sprintf(bufferMsg2, "OffsetZ = %.2f ", auxi);
				lcdWriteMessage(bufferMsg2);
				break;
			case 4:
				auxi = (4.0f / 1000.0f) * SENSORS_GRAVITY_EARTH;
				sprintf(bufferMsg2, "Sens = %.2f m/s2 ", auxi);
				lcdWriteMessage(bufferMsg2);
				break;
			case 5:
				sprintf(bufferMsg2, "O.Keys:w,x,y,z,2,f,d");
				lcdWriteMessage(bufferMsg2);
				break;
			case 6:
				sprintf(bufferMsg2, "O.Keys:w,x,y,z,2,f,d");
				break;
			default:
				sprintf(bufferMsg2, "O.Keys:w,x,y,z,2,f,d");
				break;
			}
		}
	}
}
//Calback del timer2 para el blinking
void BasicTimer2_Callback(void) {
	banderaLedUsuario = 1;
	contador1seg++;
}

//Calback del timer4 para la toma de datos
void BasicTimer4_Callback(void) {
	flagDatos = 1;
}

//Callback para leer lo que se mandna por el USART6
void USART6Rx_Callback(void) {
	usart6DataReceived = getRxData();
}

void VerificarUsartOcupado(void) {
	bool flagNewData = getFlagNewData();
	while (flagNewData != 0) { //No deja ingresar nuevos datos hasta que se envien por completo
		__NOP();
		flagNewData = getFlagNewData();
	}
}
