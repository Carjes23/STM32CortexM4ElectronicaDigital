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
/*Se exportan las librerias del lenguaje que se van a usar
 * En este caso se exportará estdint para el manejo de enteros
 * stdbool para el manejo de booleanos *
 */



#include <stdint.h>
#include <stdbool.h>

/*Se exportan las librerías creadas por nosotros
 * Se exporta el hal a pesar de que ya se utiliza en
 * el GPIOxDriver.h, también se debe tener en cuenta
 * que se agregaron dos funciones adicionales con su header en
 *  GPIOxDriver.h y su respectivo codigo en GPIOxDriver.c, que serían
 *  la funcion GPIOxTooglePin y la función delay
 */

#include "stm32f411xx_hal.h"
#include "GPIOxDriver.h"

/*
 * En la función main se agrega el codigo relacionado con
 * el desarrollo de la tarea
 */


int main(void){
	//Programación GPIO para los dos primeros puntos.

	/*
	 * Se define el Handler para el pin LD2
	 * Con este pin se pretende probar los dos primeros puntos.
	 */
	GPIO_Handler_t handlerUserLedPin = {0};

	/* Se define el handler para el el boton de usuario
	 * este su utilizara para el punto tres para que el contador
	 * funcine en forma inversa.
	 */

	GPIO_Handler_t handlerUserButton = {0};



	/*
	 * Ahora agregamos la configuración para el el pin de usuario
	 * led, que al mirar las referencias se puede notar que, que
	 * pertenece al GPIOA, su pin es el numero, lo vamos a utilizar
	 * como salida en su forma de push-pull, no agregaremos ninguna
	 * resistencia pull-up o pull-down utilizaremos una velocidad media
	 * y no se usara ninguna función especial.
	 * Esta aclaración se hara en cada uno de los pines a usar a frente
	 * de su respectiva configuración.
	 */
	handlerUserLedPin.pGPIOx = GPIOA; // Aquí se coloca a que GPIO pertenece el pin en este caso es el A
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_5; //El numero del pin en este caso es el
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_OUT; //En que modo se va a utilizar el pin es este caso lo usaremos como salida
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL; //Sí se configura out, en que modo se va a usar, en este caso se usara como push-pull
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING; //Si se le va agregár una resistencia Pull-Up o Pull-down, no  le pondremos ninguna
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_MEDIUM; //A que velocidad se va utilizr la salida del
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinAltFunMode	= AF0; //Ninguna funcion alternativa

	//Desde ahora se dara una descripción menos detallada, pero especificando cada uno
	handlerUserButton.pGPIOx = GPIOC; //Se encuentra en el el GPIOC
	handlerUserButton.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_13; //El registro que este modifica es el numero 13
	handlerUserButton.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_IN;//En este caso el pin se usara como entrada de datos
	handlerUserButton.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL; //todo averiguar si se tiene que poner
	handlerUserButton.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING; //No se usara ninguna resistencia pull-up o pull-down
	handlerUserButton.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_MEDIUM; //todo averiguar si se tiene que poner
	handlerUserButton.GPIO_PinConfig_t.GPIO_PinAltFunMode	= AF0; //Ninguna funcion alternativa

	//Carga de la configuración.
	//Se carga la configuración para el Led.
	GPIO_Config(&handlerUserLedPin);
	//Se carga la configuración para el boton de usuario.
	GPIO_Config(&handlerUserButton);

	//DEFINICION PINES PUNTO 3.
	/*
	 * Se utilizaran los pines descritos en la tarea
	 * los cuales son PC9, PC6, PB8, PA6, PC7, PC8, PA7
	 * a cada uno de estos se le debe asignar su respectivo
	 * handler y agregarle la configuración. Ese proceso se hara
	 * a continuación
	 */

	//Configuración PIN PC9
	GPIO_Handler_t handlerPC9= {0}; // Se define el Handler y se le da valor de 0 en todas sus componentes
	handlerPC9.pGPIOx = GPIOC; //El pin pertenece al GPIOC
	handlerPC9.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_9; //Es el pin numero 9
	handlerPC9.GPIO_PinConfig_t.GPIO_PinMode		= GPIO_MODE_OUT;// Se utilizara como salida
	handlerPC9.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;//Se usara en su forma Push-Pull
	handlerPC9.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;//No se agregan resistencias adicionales.
	handlerPC9.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_MEDIUM; //se usara en velocidad media
	handlerPC9.GPIO_PinConfig_t.GPIO_PinAltFunMode	= AF0; //Ninguna funcion alternativa
	GPIO_Config(&handlerPC9); //Se le carga la configuración al pin PC9

	//Configuración PIN PC6
	GPIO_Handler_t handlerPC6= {0};//Se define el Handler y se le da valor de 0 en todas sus componentes
	handlerPC6.pGPIOx = GPIOC;//El pin pertenece al GPIOC
	handlerPC6.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_6; //Es el pin numero 6
	handlerPC6.GPIO_PinConfig_t.GPIO_PinMode		= GPIO_MODE_OUT; // Se utilizara como salida
	handlerPC6.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL; //Se usara en su forma Push-Pull
	handlerPC6.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING; //No se agregan resistencias adicionales.
	handlerPC6.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_MEDIUM; //se usara en velocidad media
	handlerPC6.GPIO_PinConfig_t.GPIO_PinAltFunMode	= AF0; //Ninguna funcion.
	GPIO_Config(&handlerPC6); //Se le carga la configuración al pin PC6

	//Configuración PIN PB8
	GPIO_Handler_t handlerPB8= {0}; // Se define el Handler y se le da valor de 0 en todas sus componentes
	handlerPB8.pGPIOx = GPIOB; //El pin pertenece al GPIOB
	handlerPB8.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_8; //Es el pin numero 8
	handlerPB8.GPIO_PinConfig_t.GPIO_PinMode		= GPIO_MODE_OUT; //Se utilizara como salida
	handlerPB8.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL; //Se usara en forma Push-Pull
	handlerPB8.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING; //No se le agregan resistencia adicionales
	handlerPB8.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_MEDIUM; //se usara en velocidad media
	handlerPB8.GPIO_PinConfig_t.GPIO_PinAltFunMode	= AF0; //Ninguna funcion.
	GPIO_Config(&handlerPB8);

	GPIO_Handler_t handlerPA6= {0}; // Se define el Handler y se le da valor de 0 en todas sus componentes
	handlerPA6.pGPIOx = GPIOA; //El pin pertenece al GPIOA
	handlerPA6.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_6; //Es el pin numero 6
	handlerPA6.GPIO_PinConfig_t.GPIO_PinMode		= GPIO_MODE_OUT;//Se utilizara como salida
	handlerPA6.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;//Se usara en forma Push-Pull
	handlerPA6.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;//No se le agregan resistencia adicionales
	handlerPA6.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_MEDIUM; //se usara en velocidad media
	handlerPA6.GPIO_PinConfig_t.GPIO_PinAltFunMode	= AF0; //Ninguna funcion.
	GPIO_Config(&handlerPA6);

	GPIO_Handler_t handlerPC7= {0}; // Se define el Handler y se le da valor de 0 en todas sus componentes
	handlerPC7.pGPIOx = GPIOC; //El pin pertenece al GPIOC
	handlerPC7.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_7; //Es el pin numero 7
	handlerPC7.GPIO_PinConfig_t.GPIO_PinMode		= GPIO_MODE_OUT;//Se utilizara como salida
	handlerPC7.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;//Se usara en forma Push-Pull
	handlerPC7.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;//No se le agregan resistencia adicionales
	handlerPC7.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_MEDIUM;//se usara en velocidad media
	handlerPC7.GPIO_PinConfig_t.GPIO_PinAltFunMode	= AF0; //Ninguna funcion.
	GPIO_Config(&handlerPC7);

	GPIO_Handler_t handlerPC8= {0}; // Se define el Handler y se le da valor de 0 en todas sus componentes
	handlerPC8.pGPIOx = GPIOC;//El pin pertenece al GPIOC
	handlerPC8.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_8; //Es el pin numero 8
	handlerPC8.GPIO_PinConfig_t.GPIO_PinMode		= GPIO_MODE_OUT;//Se utilizara como salida
	handlerPC8.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;//Se usara en forma Push-Pull
	handlerPC8.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;//No se le agregan resistencia adicionales
	handlerPC8.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_MEDIUM;//se usara en velocidad media
	handlerPC8.GPIO_PinConfig_t.GPIO_PinAltFunMode	= AF0; //Ninguna funcion.
	GPIO_Config(&handlerPC8);

	GPIO_Handler_t handlerPA7= {0};// Se define el Handler y se le da valor de 0 en todas sus componentes
	handlerPA7.pGPIOx = GPIOA;//El pin pertenece al GPIOA
	handlerPA7.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_7; //Es el pin numero 7
	handlerPA7.GPIO_PinConfig_t.GPIO_PinMode		= GPIO_MODE_OUT;//Se utilizara como salida
	handlerPA7.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;//Se usara en forma Push-Pull
	handlerPA7.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;//No se le agregan resistencia adicionales
	handlerPA7.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_MEDIUM;//se usara en velocidad media
	handlerPA7.GPIO_PinConfig_t.GPIO_PinAltFunMode	= AF0; //Ninguna funcion.
	GPIO_Config(&handlerPA7);

	/*
	 * 1)
	 * El error en GPIO_ReadPin venia dado porque no se tenian en cuenta los valores
	 * que estaban por delante del numero de pin, por lo que si se tenian valores diferentes
	 * a 0 delante del pin objetivo (el que queremos leer) se entregaria un valor
	 * erroneo, por esto se tiene que realizar un limpiado previo, este se realiza con
	 * un and sobre el registro junto con una mascara con 0 en todas las posiciones
	 * excepto en la poisicion objetivo en el metodo GPIO_ReadPin en el archivo GPIOxDrive.c
	 * se pueden ver los cambios que se realizaron(luego mientras se hacia la tarea se descubrio
	 * que bastaba solo con un agragarle un and con SET para  consevar solo el primer valor, de todos modos se deja
	 * con la implementacion anterior en el GPIOxDriver.c)
	 * 2)
	 * Para el segundo punto siguiendo la pista, se implementa de forma sencilla
	 * ya que al aplicar un XOR entre el valor que se tiene el PIN y un SET, se lograra
	 * cambiar su estado, esto se puede evidenciar al notar la tabla y solo observar los
	 * casos cuando la segunda variable es 1.
	 * 	Valor pin | SET | XOR entre los 2
	 * 		1	  |  1	|		 0
	 * 		0	  |  1	| 	 	 1
	 */
	uint8_t contador = 1;
	bool PC9 = 0, PC6 = 0, PB8 = 0, PA6 = 0, PC7 = 0, PC8 = 0, PA7 = 0;
	while(1){
//		CODIGO PARA LOS 2 PRIMEROS PUNTOS

		GPIOxTooglePin(&handlerUserLedPin);
		/*
		 * En la parte anterior se prueba que se puede leer un pin ya que
		 * esto es necesario para poder cambiarle el valor al pin además
		 * a continuación se le agrega a una variable el valor de un pin
		 *
		 * */


		//CODIGO PARA EL punto 3
		//La siguiente variable se lee ya que se tiene que avanzar siempre que esta
		//variable tenga un valor de uno y decrecer si tiene un valor de 0.
		bool variable = GPIO_ReadPin(&handlerUserButton);

		/*
		 * Se trata el contador como si fuera un registro de 7 bits, por lo que se
		 * procede de forma muy similiar a lo que sería el lector de pines, en este caso
		 * tiene una implementación diferente al GPIOxDriver.c ya que solo se le hace
		 * un desplazamiento para que en el bit menos significativo se encuentre el valor
		 * que se quiere guardar y luego se le aplica una operación and(&) con unas mascara
		 * con un 1 para solo guardar el valor que se encuentra en esa posición.
		 * Esto se realiza para cada uno de los pines teniendo en cuenta el valor que cada
		 * uno de estos representa.
		 */


		PC9 = (contador >> 6) & SET; //Bit 6 del "registro" contrador
		PC6 = (contador >> 5) & SET; //Bit 5 del "registro" contrador
		PB8 = (contador >> 4) & SET; //Bit 4 del "registro" contrador
		PA6 = (contador >> 3) & SET; //Bit 3 del "registro" contrador
		PC7 = (contador >> 2) & SET; //Bit 2 del "registro" contrador
		PC8 = (contador >> 1) & SET; //Bit 1 del "registro" contrador
		PA7 = (contador) & SET; //Bit 0 del "registro" contrador
		/*
		 * Luego se procede a suscribir el valor que se encuentra en el pin
		 * con el valor antes guardado, esto se realiza
		 * utilizando la función que se encuentra en el GPIOxDriver.c.
		 * Esto se realiza para cada uno de los pines que se van a utilizar.
		 */

		GPIO_WritePin(&handlerPC9,PC9); //Cambiar el valor del bit 6
		GPIO_WritePin(&handlerPC6,PC6);	//Cambiar el valor del bit 5
		GPIO_WritePin(&handlerPB8,PB8); //Cambiar el valor del bit 4
		GPIO_WritePin(&handlerPA6,PA6); //Cambiar el valor del bit 3
		GPIO_WritePin(&handlerPC7,PC7); //Cambiar el valor del bit 2
		GPIO_WritePin(&handlerPC8,PC8); //Cambiar el valor del bit 1
		GPIO_WritePin(&handlerPA7,PA7); //Cambiar el valor del bit 0
		delay(1); //Se le da un delay de 1 segundo para que se pueda ver el cambio está funcion se encuentra en el driver.c
		if (variable == SET){ //Ya que el boton de usuario tiene un valor de 1 como default, cuando este se encuentre en 1
			//vamos a sumar de 1 en 1 y teniendo en cuenta que si el contador se encuentra en el valor de 0 este se reinciia a 1 y si no
			if(contador == 60){ //Verificar si se tiene 60
				contador = 1; //reinciiar en 1
			} else {contador++;}// Si no se tiene 60 seguir aumentando de 1 en 1
		} else{ //Si el boton se encunetra en otro estado diferente a 1 vamos a restar de 1 en 1 verificando que si el valor es 1
			//este se pase a el valor de 60
			if(contador == 1){ //verificar si el valor es 1
				contador = 60; //reiniciar el contador en 60
			} else{contador--;} //Disminuir el valor de 1 en 1
		}
	}
}

//Las funciones delay



