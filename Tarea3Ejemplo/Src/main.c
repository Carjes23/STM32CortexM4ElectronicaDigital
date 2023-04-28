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
 *
 * A continución se desarrollara la tarea 3 de Taller V que conciste en un encoder
 * rotativo que nos permitira aumentar o disminuir un contador, además de una función
 * adicional donde se nos permite tener una "culebrita" que genera un patron al "avanzar"
 * adicionalmente se le hallade el blinking el cual se econtrara dentro de la board del micro y
 * una funcion extra para cambiar el perido de refresco cuando se tiene el contador, además de un led
 * que nos dice en que estado se encuentra, los filtros usados para el ruido del encoder fueron hechos
 * a traves de software ya que el autor probo ambos metodos software y hardware pero le parecio que el
 * funcionamiento a traves de software fue superior al que se obtuvo por hardware.
 *
 *
 * Explicación funcionamiento filtro por software.
 *
 * 	Se aprovecha la funcion one pulse mode que tienen los timers, la cual solo "cuenta" una vez hasta
 * 	el "objetivo" (ARR -> limite) activando la "bandera"(o su respectivo handler)
 * 	 y se desactiva el registro de contar (CR1_CEN), cuando este se habilita
 * 	de nuevo, el timer vuelve a "contar" hasta el objetivo.
 *
 * 	Aprovechandonos de lo anterior el filtro consiste en que para recibir una nueva "instruccion" el
 * 	programa verificara la bandera tanto de la interrupción(como se haría normalmente) y del filtro
 * 	con esto consguiremos que ordenes sucecivas dadas en periodos de tiempo menores al contador del filtro
 * 	sean completamente despreciadas, dentro de la verificación se limpian ambas banderas y se vuelve a activar
 * 	el registro de contar (CR1_CEN)
 *
 * A continuación veremos el desarrollo de la tarea y se intentara comentar lo que se concidere relevante.
 *
 ******************************************************************************
 */

// Se exportan las librerias que se usaran

#include <stdint.h>
#include "ExtiDriver.h"
#include "GPIOxDriver.h"
#include "BasicTimer.h"
#include <stdbool.h>

//Se definen las constantes que se consideran importantes para el desarrollo de la tarea.

#define CONTADOR		0
#define INFINITO		1

//Se definen en 0 los diferentes handlers para los timers que se usaran, en este caso usaremos 3 timers

BasicTimer_Handler_t handlerTim2 = {0}; //Timer para el blinking
BasicTimer_Handler_t handlerTim3 = {0}; //Timer para hacer el suicheo entre los transistores.
BasicTimer_Handler_t handlerTim4 = {0}; //Timer para el filtro en software

//Se definen en 0 los handlers para los pines que se usaran

//Boton y led de usuario

GPIO_Handler_t handlerUserLedPin = {0};		//Handler del led de usuario en la board LD2
GPIO_Handler_t handlerUserButton = {0};		//Handler boton de usurio usado para cambiar el periodo de refresco.

//Salidas del enconder.

GPIO_Handler_t handlerEncoderA = {0}; 	 	//Salida DT del enconder
GPIO_Handler_t handlerEncoderB = {0}; 	 	//Salida CLK del enconder
GPIO_Handler_t handlerBotEncoder = {0};	 	//Salida SW del enconder, lo que vendría siendo el boton.

//Definicion GPIO que controlan el 7 segmentos

GPIO_Handler_t handlerLedA = {0};			//Segmento A en el 7 Segmentos
GPIO_Handler_t handlerLedB = {0};			//Segmento B en el 7 Segmentos
GPIO_Handler_t handlerLedF = {0};			//Segmento F en el 7 Segmentos
GPIO_Handler_t handlerLedC = {0};			//Segmento C en el 7 Segmentos
GPIO_Handler_t handlerLedE = {0};			//Segmento E en el 7 Segmentos
GPIO_Handler_t handlerLedD = {0};			//Segmento D en el 7 Segmentos
GPIO_Handler_t handlerLedG = {0}; 			//Segmento G en el 7 Segmentos

//Definicion GPIO que controlan el led RGB que determina la frecuencia(periodo)
//de refresco del contador.

GPIO_Handler_t handlerLedFrecA = {0}; //Led indicador de frecuencia A -> 10 ms -> Rojo
GPIO_Handler_t handlerLedFrecB = {0}; //Led indicador de frecuencia B -> 50 ms -> Verde
GPIO_Handler_t handlerLedFrecC = {0}; //Led indicador de frecuencia C -> 100 ms -> Azul

//Definicion GPIO que controla el suicheo de los transistores
/*
 * Acá se debe aclarar que se usaron transistores distintos, uno NPN y otro PNP
 * ya que con esto se consigue un efecto de "negado" entre ellos, logrando
 * realizar la tarea de suicheo solo con un pin.
 *
 */

GPIO_Handler_t handlerTransistor = {0}; // Pin que controla el suicheo.

//Contadores utilizados en el programa.

//	Contadores relacionados a la función contadora que va de 0 a 99.

uint8_t contador = 0; 		//Contador en general
uint8_t unidades = 0; 		//Unidades del contador anterior, se consigue utilizando el modulo 10 del contador.
uint8_t decenas = 0;		//Decenas del contador anterior, se consigue utilizando la divición entre 10 del contador.
//El anterior solo es posible ya que solo trabajamos con numeros enteros, las unidades se pierden.

//	Contador relacionados a la culebrita.
/*
 * Ya que la culebrita tiene 11 posibles estados se puede utilizar un contador ciclico entre 0 y 11,
 * consiguiendo con esto el desarrollo de este.
 *
 */

uint8_t contadorc = 0; // Contador culebrita es ciclico y va de 0-11.

//	Contador del timer 3 relacionado al transistor(suicheo).
/*
 * Ya que se tiene que realizar un suiceo se crean dos posibles estados estan separados
 * temporalmente entre sí(tiempos iguales), por esto se necesita un contador, ya que para el primer periodo
 * se le da una condicion al transistor y para el segundo conteo se le da otra, luego de esto
 * se repite el proceso ciclicamente.
 *
 */

uint8_t contadortim3 = 0;

/*
 * Ya que le vamos a dar 3 posibles estados a la frecuencia de suicheo o al periodo, vamos a estar cambiando
 * el "objetivo" (ARR) del timer 3, por esto usaremos un contador, ya que para el contador 0 se tendra un evento
 * y asi sucesivamente hasta llegar al contador 2 donde se reinicia.
 */

uint8_t contadorFrec = 0;

// Booleanos(Variables de solo dos posibles estados) que se utilizaran en el programa

// Valor de estado, este nos dice si nos encontramos en el modo culebrita o contador.

bool estado = 0;

	//Banderas de los diferentes timers
bool banderaLedUsuario = 0; //Bandera TIM2 el cual nos da el blinking del LD2
bool banderaFiltro = 0; // Bandera TIM4 filtro que nos permite atenuar rebotes en los aparatos mecanicos.
		//	Banderas TIM3 la cual tiene 2 tipos una para cuando se quiere prender las unidades y otro las decenas
		//	estas banderas estan separadas tiempos iguales entre sí y permiten el suicheo.
bool banderaTimerLed1 = 0;
bool banderaTimerLed2 = 0;
	//Banderas de las interrupciones EXTI
bool banderaFrecuencia = 0; //Bandera del boton de usuario para cambiar la frecuencia de refresco del 7 segmentos.
bool banderaContador = 0; 	//Bandera relacionada a los cambios que tiene el encoder.
bool banderaBoton = 0; 		//Bandera relacionada al boton del enconder. SW.

	//Valores de los diferentes leds, se considero que esta era la forma mas eficiente de trabajar.
		//Leds de los segmentos del 7 segmetos. El estado 0 es que el segmento esta encendido.
bool ledA = 0; 			//Estado segmento A
bool ledB = 0; 			//Estado segmento B
bool ledC = 0; 			//Estado segmento C
bool ledD = 0; 			//Estado segmento D
bool ledE = 0; 			//Estado segmento E
bool ledF = 0; 			//Estado segmento F
bool ledG = 0; 			//Estado segmento G
		//Valores para el led rbg que representa la velocidad de refresco del display.
bool ledFrecA = 0; 		//Led indicador de frecuencia A -> 10 ms -> Rojo
bool ledFrecB = 0; 		//Led indicador de frecuencia B -> 50 ms -> Verde
bool ledFrecC = 0; 		//Led indicador de frecuencia C -> 100 ms -> Azul

//Se definen en 0 los diferentes handlers para las interrupciones EXTI.

EXTI_Config_t handlerExtiA = {0}; 			//Interrupcion giro enconder ligada al handlerEncoderA(DT)
EXTI_Config_t handlerExtiBot = {0};			//Interrupcion boton enconder ligada al handlerBotEncoder(SW)
EXTI_Config_t handlerExtiBotonUser = {0};	//Interrupcion boton de usuario ligada al handlerUserButton(B2)

//Cabeceras de funciones que se usara.

void initSystem(void); 				//Funcion que inicia el sistema y configura los pines a usar.
void funcionContadora(void); 		//Funcion que realiza el conteo de numeros(0-99) al recibir la interrupcion del ExtiA.
void funcionCulebrita(void);		//Funcion que realiza el conteo de culebrita(0-11 ciclico) al recibir la interrupcion del ExtiA.
void funcionLeds(uint8_t numero); 	//Funcion que controla los pines que se prenden para mostrar el numero que le ingrese.
void funcionLedsC(void); 			//Funcion que controla los pines y el valor del transistor que se prenden para mostrar la posicion deseada.
void funcionFrecuencia(void); 		//Funcion que controla el cambio en frecuencias
void resetDisplay(void); 			//Funcion que borra los numeros en el 7 segmentos

//Cabeceras de funciones callback se sabe que son redundantes pero no afectan nada al codigo y fueron
//de ayuda en el desarrollo para recordar que faltaba por editar.

void BasicTimer2_Callback(void);
void BasicTimer3_Callback(void);
void callback_extInt13(void);

//Funcion main lo cual representa la funcion más imporante del programa

int main(void){

	initSystem(); //Se corre la funcion inicio de sistema donde se cargan las diferentes configuraciones.

	while(1){
		/*
		 * Si se lanza la interrupcion del boton del encoder y la bandera del filtro esta activada(
		 * para evitar rebotes) se procede a cambiar el estado del programa de culebrita a contador o
		 * viceversa, sí no se tiene la bandera del filtro activada se limpia la bandera del boton
		 * con lo que descartaríamos esa interrupcion ya que sería ruido.
		 *
		 * Sí se cumplen ambas condiciones se limpian ambas banderas y se reinicia el CR1_CEN para
		 * que el TIM4 vuelva a contar.
		 */

		if(banderaBoton == 1 && banderaFiltro ==1){
			banderaBoton = 0;
			banderaFiltro = 0;
			estado ^= 1;
			TIM4->CR1 |= TIM_CR1_CEN;
		}

		else if(banderaBoton == 1){
			banderaBoton = 0;
		}

		/*
		 * Condicional que nos permite realizar la funcion blinking se procura limpiar la bandera dentro.
		 */

		if(banderaLedUsuario == 1){
			banderaLedUsuario = 0;
			GPIOxTooglePin(&handlerUserLedPin);
		}
		/*
		 * Si se encuentra en el modo("estado") contador se entra al siguiente if, en el cual se ingresa a la
		 * funcion contadora para ver si se tiene una interrupcion relacionada al cambio de numero, luego se extran
		 * de contador las decenas y las unidades, luego de esto viene una de las fases más importante donde le
		 * damos una condición al transistor para energizar alguno de los dos 7 segmentos y cargar("imprimir") en este
		 * el valor actual.
		 *
		 * Sí el estado es culebrita, este entrara a la funcion que administra el conteo dentro de esta y ahí se abre la
		 * función que controla los leds que se prenden, ya que en el caso de la culebrita solo se "usa" un solo
		 * 7 segmentos a la vez nos permite llamar esta funcion a dentro de la funcion del conteo
		 */


		if(estado == CONTADOR){
			funcionContadora();
			if(banderaTimerLed1==1){
				resetDisplay(); //se apagan los valores anteriores
				GPIO_WritePin(&handlerTransistor, 1);
				banderaTimerLed1 = 0;
				funcionLeds(unidades);
			}
			else if(banderaTimerLed2==1){
				resetDisplay();//se apagan los valores anteriores
				GPIO_WritePin(&handlerTransistor, 0);
				banderaTimerLed2 =0;
				funcionLeds(decenas);
			}
			else{
				__NOP();
				}
			}
		else { funcionCulebrita();}

		/*
		 * Condicional que verifica si se quiere cambiar la frencuencia, teniendo en cuenta la bandera filtro
		 * para evitar rebotes.
		 */

		if(banderaFrecuencia == 1 && banderaFiltro == 1){
			banderaFrecuencia = 0;
			banderaFiltro = 0;
			TIM4->CR1 |= TIM_CR1_CEN;
			funcionFrecuencia();
		}
		else if(banderaFrecuencia == 1){
			banderaFrecuencia = 0;
		}

	}

}

void initSystem(void){

	//Boton de usuario para el cambio de tasa de resfresco.

	handlerUserButton.pGPIOx = GPIOC; //Se encuentra en el el GPIOC
	handlerUserButton.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_13; //Es el pin numero 13
	handlerUserButton.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_IN;//En este caso el pin se usara como entrada de datos
	handlerUserButton.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING; //No se usara ninguna resistencia pull-up o pull-down

	handlerExtiBotonUser.pGPIOHandler = &handlerUserButton; // Se carga la config del pin al handler del EXTI
	handlerExtiBotonUser.edgeType = EXTERNAL_INTERRUPT_FALLING_EDGE; //Se define que se van a detectar flancos de bajada.

	extInt_Config(&handlerExtiBotonUser); //Se carga la configuracion usando la funcion de la libreria.

	//Led de usuario usado para el blinking

	handlerUserLedPin.pGPIOx = GPIOA; //Se encuentra en el el GPIOA
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_5; // Es el pin 5.
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_OUT; //Se utiliza como salida
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL; //Salida PushPull
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING; //No se usa ninguna resistencia
	handlerUserLedPin.GPIO_PinConfig_t.GPIO_PinSpeed		= GPIO_OSPEED_FAST; //Se usa en velocidad rapida

	GPIO_Config(&handlerUserLedPin); //Se carga la configuración para el Led.


	//Configuración Timer 2 que controlara el blinking

	handlerTim2.ptrTIMx = TIM2; //El timer que se va a usar
	handlerTim2.TIMx_Config.TIMx_interruptEnable = 1; //Se habilitan las interrupciones
	handlerTim2.TIMx_Config.TIMx_mode = BTIMER_MODE_UP; //Se usara en modo ascendente
	handlerTim2.TIMx_Config.TIMx_period = 250; //Se define el periodo en este caso el led cambiara cada 250ms
	handlerTim2.TIMx_Config.TIMx_speed = BTIMER_SPEED_1ms; //Se define la "velocidad" que se usara

	BasicTimer_Config(&handlerTim2); //Se carga la configuración.

	//Configuración Timer 3 que controla el suicheo de los transistores.

	handlerTim3.ptrTIMx = TIM3;//El timer que se va a usar
	handlerTim3.TIMx_Config.TIMx_interruptEnable = 1;//Se habilitan las interrupciones
	handlerTim3.TIMx_Config.TIMx_mode = BTIMER_MODE_UP;//Se usara en modo ascendente
	handlerTim3.TIMx_Config.TIMx_period = 100; //Se define el periodo en este caso el default es 10 ms.
	handlerTim3.TIMx_Config.TIMx_speed = BTIMER_SPEED_100us;//Se define la "velocidad" que se usara

	BasicTimer_Config(&handlerTim3);//Se carga la configuración.

	//Configuración Timer 4 que controla el filtro.
	//En este caso al usarlo a 25ms las interrupciones de rebote que se den en ese intervalo son desestimadas.

	handlerTim4.ptrTIMx = TIM4;//El timer que se va a usar
	handlerTim4.TIMx_Config.TIMx_interruptEnable = 1;//Se habilitan las interrupciones
	handlerTim4.TIMx_Config.TIMx_mode = BTIMER_MODE_UP;//Se usara en modo ascendente
	handlerTim4.TIMx_Config.TIMx_period = 50;//Se define el periodo en este caso el default es 50 ms.
	handlerTim4.TIMx_Config.TIMx_speed = BTIMER_SPEED_1ms;//Se define la "velocidad" que se usara
	handlerTim4.TIMx_Config.TIMx_OPM = ENABLE; //Funcion que nos permite que el timer cuente solo una vez y toque habilitarlo para volver a usarlo.

	BasicTimer_Config(&handlerTim4);//Se carga la configuración.

	//Configuracion de las salida DT del encoder. Se usara el PIN A0 por lo que se usara la interrpción EXTI0.

	handlerEncoderA.pGPIOx = GPIOA; //GPIO al que pertenece
	handlerEncoderA.GPIO_PinConfig_t.GPIO_PinNumber = PIN_0; //El pin a usar
	handlerEncoderA.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_IN; //Modo de entrada para detectar las interrupciones.
	handlerEncoderA.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLUP; //Se usa una resistencia PULLUP


	handlerExtiA.edgeType = EXTERNAL_INTERRUPT_FALLING_EDGE; //Se buscaran flancos de bajada
	handlerExtiA.pGPIOHandler = &handlerEncoderA; // Se carga la config del pin al handler del EXTI

	extInt_Config(&handlerExtiA); //Se carga la configuracion para que funcione la interrupcion

	//Configuracion de la salida CLK del encoder. Esta se usara para "comparar".
	//si a mirar el valor de este pin es bajo se relizara una acción y si es
	//alto se realizara la acción contraria (con accion se refiere a sumar o restar una unidad).
	//se usara el PIN_C12

	handlerEncoderB.pGPIOx = GPIOC;//GPIO al que pertenece
	handlerEncoderB.GPIO_PinConfig_t.GPIO_PinNumber = PIN_12;//El pin a usar
	handlerEncoderB.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_IN;//Modo de entrada para mirar el valor que ingresa.
	handlerEncoderB.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;//No se usa.

	GPIO_Config(&handlerEncoderB);//Se carga la configuración.

	//Configracion de la salida SW(boton del handler). Se usara el PIN D2 por lo que se usara la interrpción EXTI2.

	handlerBotEncoder.pGPIOx = GPIOD;//GPIO al que pertenece
	handlerBotEncoder.GPIO_PinConfig_t.GPIO_PinNumber = PIN_2;//El pin a usar
	handlerBotEncoder.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_IN;//Modo de entrada para detectar las interrupciones.
	handlerBotEncoder.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLUP; //Se usa una resistencia PULLUP


	handlerExtiBot.edgeType = EXTERNAL_INTERRUPT_RISING_EDGE;//Se buscaran flancos de bajada
	handlerExtiBot.pGPIOHandler = &handlerBotEncoder;// Se carga la config del pin al handler del EXTI

	extInt_Config(&handlerExtiBot); //Se carga la configuracion para que funcione la interrupcion

	/*
	 * Definicion de los pines para el control del 7 segmentos.
	 * Ya que todos tienen la misma configuracion solo se cometara el pin que se va a utilizar y se
	 * comentara con más detalle el primero de estos
	 */

	//Segmento A Pin_C3

	handlerLedA.pGPIOx = GPIOC; //GPIO al que pertenece
	handlerLedA.GPIO_PinConfig_t.GPIO_PinNumber = 		PIN_3;//El pin a usar
	handlerLedA.GPIO_PinConfig_t.GPIO_PinMode = 		GPIO_MODE_OUT; //Modo de salida
	handlerLedA.GPIO_PinConfig_t.GPIO_PinOPType		= 	GPIO_OTYPE_PUSHPULL; //Modo PushPull
	handlerLedA.GPIO_PinConfig_t.GPIO_PinPuPdControl= 	GPIO_PUPDR_NOTHING; //No se usara resistencia adicional
	handlerLedA.GPIO_PinConfig_t.GPIO_PinSpeed		= 	GPIO_OSPEED_FAST; //Se usara en velocidad rapida.

	GPIO_Config(&handlerLedA); // Se carga la configuracion

	//Segmento B Pin_C0

	handlerLedB.pGPIOx = GPIOC;
	handlerLedB.GPIO_PinConfig_t.GPIO_PinNumber = 		PIN_0;
	handlerLedB.GPIO_PinConfig_t.GPIO_PinMode = 		GPIO_MODE_OUT;
	handlerLedB.GPIO_PinConfig_t.GPIO_PinOPType		= 	GPIO_OTYPE_PUSHPULL;
	handlerLedB.GPIO_PinConfig_t.GPIO_PinPuPdControl= 	GPIO_PUPDR_NOTHING;
	handlerLedB.GPIO_PinConfig_t.GPIO_PinSpeed		= 	GPIO_OSPEED_FAST;

	GPIO_Config(&handlerLedB);

	//Segmento F Pin_C2

	handlerLedF.pGPIOx = GPIOC;
	handlerLedF.GPIO_PinConfig_t.GPIO_PinNumber = 		PIN_2;
	handlerLedF.GPIO_PinConfig_t.GPIO_PinMode = 		GPIO_MODE_OUT;
	handlerLedF.GPIO_PinConfig_t.GPIO_PinOPType		= 	GPIO_OTYPE_PUSHPULL;
	handlerLedF.GPIO_PinConfig_t.GPIO_PinPuPdControl= 	GPIO_PUPDR_NOTHING;
	handlerLedF.GPIO_PinConfig_t.GPIO_PinSpeed		= 	GPIO_OSPEED_FAST;

	GPIO_Config(&handlerLedF);

	//Segmento C Pin_C1

	handlerLedC.pGPIOx = GPIOC;
	handlerLedC.GPIO_PinConfig_t.GPIO_PinNumber = 		PIN_1;
	handlerLedC.GPIO_PinConfig_t.GPIO_PinMode = 		GPIO_MODE_OUT;
	handlerLedC.GPIO_PinConfig_t.GPIO_PinOPType		= 	GPIO_OTYPE_PUSHPULL;
	handlerLedC.GPIO_PinConfig_t.GPIO_PinPuPdControl= 	GPIO_PUPDR_NOTHING;
	handlerLedC.GPIO_PinConfig_t.GPIO_PinSpeed		= 	GPIO_OSPEED_FAST;

	GPIO_Config(&handlerLedC);

	//Segmento E Pin_A10

	handlerLedE.pGPIOx = GPIOA;
	handlerLedE.GPIO_PinConfig_t.GPIO_PinNumber = 		PIN_10;
	handlerLedE.GPIO_PinConfig_t.GPIO_PinMode = 		GPIO_MODE_OUT;
	handlerLedE.GPIO_PinConfig_t.GPIO_PinOPType		= 	GPIO_OTYPE_PUSHPULL;
	handlerLedE.GPIO_PinConfig_t.GPIO_PinPuPdControl= 	GPIO_PUPDR_NOTHING;
	handlerLedE.GPIO_PinConfig_t.GPIO_PinSpeed		= 	GPIO_OSPEED_FAST;

	GPIO_Config(&handlerLedE);

	//Segmento D Pin_C4

	handlerLedD.pGPIOx = GPIOC;
	handlerLedD.GPIO_PinConfig_t.GPIO_PinNumber = 		PIN_4;
	handlerLedD.GPIO_PinConfig_t.GPIO_PinMode = 		GPIO_MODE_OUT;
	handlerLedD.GPIO_PinConfig_t.GPIO_PinOPType		= 	GPIO_OTYPE_PUSHPULL;
	handlerLedD.GPIO_PinConfig_t.GPIO_PinPuPdControl= 	GPIO_PUPDR_NOTHING;
	handlerLedD.GPIO_PinConfig_t.GPIO_PinSpeed		= 	GPIO_OSPEED_FAST;

	GPIO_Config(&handlerLedD);

	//Segmento G Pin_B3

	handlerLedG.pGPIOx = GPIOB;
	handlerLedG.GPIO_PinConfig_t.GPIO_PinNumber = 		PIN_3;
	handlerLedG.GPIO_PinConfig_t.GPIO_PinMode = 		GPIO_MODE_OUT;
	handlerLedG.GPIO_PinConfig_t.GPIO_PinOPType		= 	GPIO_OTYPE_PUSHPULL;
	handlerLedG.GPIO_PinConfig_t.GPIO_PinPuPdControl= 	GPIO_PUPDR_NOTHING;
	handlerLedG.GPIO_PinConfig_t.GPIO_PinSpeed		= 	GPIO_OSPEED_FAST;

	GPIO_Config(&handlerLedG);

	//Pines relacionados al vializador de frecuencias su configuracion es muy similar a la anteior

	//Componente roja del RGB. PIN_A9

	handlerLedFrecA.pGPIOx = GPIOA;
	handlerLedFrecA.GPIO_PinConfig_t.GPIO_PinNumber = 		PIN_9;
	handlerLedFrecA.GPIO_PinConfig_t.GPIO_PinMode = 		GPIO_MODE_OUT;
	handlerLedFrecA.GPIO_PinConfig_t.GPIO_PinOPType		= 	GPIO_OTYPE_PUSHPULL;
	handlerLedFrecA.GPIO_PinConfig_t.GPIO_PinPuPdControl= 	GPIO_PUPDR_NOTHING;
	handlerLedFrecA.GPIO_PinConfig_t.GPIO_PinSpeed		= 	GPIO_OSPEED_FAST;

	GPIO_Config(&handlerLedFrecA);
	GPIO_WritePin(&handlerLedFrecA, SET); //Estado Inicial encendido a que estamos a 10 ms.

	//Componente verde del RGB. PIN_B2

	handlerLedFrecB.pGPIOx = GPIOB;
	handlerLedFrecB.GPIO_PinConfig_t.GPIO_PinNumber = 		PIN_2;
	handlerLedFrecB.GPIO_PinConfig_t.GPIO_PinMode = 		GPIO_MODE_OUT;
	handlerLedFrecB.GPIO_PinConfig_t.GPIO_PinOPType		= 	GPIO_OTYPE_PUSHPULL;
	handlerLedFrecB.GPIO_PinConfig_t.GPIO_PinPuPdControl= 	GPIO_PUPDR_NOTHING;
	handlerLedFrecB.GPIO_PinConfig_t.GPIO_PinSpeed		= 	GPIO_OSPEED_FAST;

	GPIO_Config(&handlerLedFrecB);

	//Componente acul del RGB. PIN_C7

	handlerLedFrecC.pGPIOx = GPIOC;
	handlerLedFrecC.GPIO_PinConfig_t.GPIO_PinNumber = 		PIN_7;
	handlerLedFrecC.GPIO_PinConfig_t.GPIO_PinMode = 		GPIO_MODE_OUT;
	handlerLedFrecC.GPIO_PinConfig_t.GPIO_PinOPType		= 	GPIO_OTYPE_PUSHPULL;
	handlerLedFrecC.GPIO_PinConfig_t.GPIO_PinPuPdControl= 	GPIO_PUPDR_NOTHING;
	handlerLedFrecC.GPIO_PinConfig_t.GPIO_PinSpeed		= 	GPIO_OSPEED_FAST;

	GPIO_Config(&handlerLedFrecC);

	//Pin salida que le da la condicion al tansistor, muy similar a los anteriores pero en vez
	//de prender un led nos deja conducir en un trasistor o en el otro.
	//Se usa el Pin_C10

	handlerTransistor.pGPIOx = GPIOC;
	handlerTransistor.GPIO_PinConfig_t.GPIO_PinNumber = 		PIN_10;
	handlerTransistor.GPIO_PinConfig_t.GPIO_PinMode = 		GPIO_MODE_OUT;
	handlerTransistor.GPIO_PinConfig_t.GPIO_PinOPType		= 	GPIO_OTYPE_PUSHPULL;
	handlerTransistor.GPIO_PinConfig_t.GPIO_PinPuPdControl= 	GPIO_PUPDR_NOTHING;
	handlerTransistor.GPIO_PinConfig_t.GPIO_PinSpeed		= 	GPIO_OSPEED_FAST;

	GPIO_Config(&handlerTransistor);

}

/*
 * Los callback la descripción general de ellos es que se intenta hacer
 * lo minimo posible dentro de ellos, en la mayría de los casos se levantan banderas, o se hacen pequeños calculos
 * para poder levvantar banderas.
 */

//Calback del timer2 para el blinking
void BasicTimer2_Callback(void){
	banderaLedUsuario = 1;
}

//Callback EXTI 0 para el giro del enconder

void callback_extInt0(void){
	banderaContador = 1;
}

//Callback EXTI 2 para el boton del encodner
void callback_extInt2(void){
	banderaBoton = 1;
}

//Callback del suicheo se tienen dos posibles estados uno para unidades y otro para decenas.
void BasicTimer3_Callback(void){
	contadortim3++;
	if(contadortim3 == 1){
		banderaTimerLed1 = 1;
	}
	else if(contadortim3 == 2){
		banderaTimerLed2 = 1;
		contadortim3 = 0;
	}
}

//Callback del filtro

void BasicTimer4_Callback(void){
	banderaFiltro = 1;
}

//Callback para el boon de usuario que nos cambia la frecuencia de refresco.

void callback_extInt13(void){
	banderaFrecuencia = 1;
}


void funcionContadora(void){
	//condicional que verifica la bandera para realizar cambios en el contador con su respectivo filtro
		if(banderaContador == 1 && banderaFiltro == 1){
			banderaContador = 0; //Se baja la bandera
			banderaFiltro = 0;//Se baja la bandera
			TIM4->CR1 |= TIM_CR1_CEN;//se reinicia el funcionamiento del contador del filtro
			if(GPIO_ReadPin(&handlerEncoderB) == 0){//cambiar a 1 si esta al reves.
				if(contador != 99){//sumar solo si no se encuentra en 99
					contador++;
				}
				else{__NOP();}
			} else{
				if(contador != 0){//restar solo si no se encuentra en 00
					contador--;
				} else{__NOP();}
			}

			decenas = contador / 10;
			unidades = contador % 10;
		}
		//Se limpia la bandera del grio del enconder si la interrupcion se dio en un tiempo que no debía por lo que se considera ruido.
		else if(banderaContador == 1){
			banderaContador = 0;
		}
}

/*
 * Esta funcion es la encargada de prender los segmentos necesarios para cada numero.
 * Se tiene un switch case donde se le dan los valores requeridos a cada segmento
 * teniendo en cuenta que 0 es encendido y 1 es apagado, al final se cargan todos esos valores
 * a los respectivos pines.
 */

void funcionLeds(uint8_t numero){
	switch(numero){
		case 0:
			ledA = 0;
			ledB = 0;
			ledC = 0;
			ledD = 0;
			ledE = 0;
			ledF = 0;
			ledG = 1;
			break;

		case 1:
			ledA = 1;
			ledB = 0;
			ledC = 0;
			ledD = 1;
			ledE = 1;
			ledF = 1;
			ledG = 1;
			break;

		case 2:
			ledA = 0;
			ledB = 0;
			ledC = 1;
			ledD = 0;
			ledE = 0;
			ledF = 1;
			ledG = 0;
			break;

		case 3:
			ledA = 0;
			ledB = 0;
			ledC = 0;
			ledD = 0;
			ledE = 1;
			ledF = 1;
			ledG = 0;
			break;

		case 4:
			ledA = 1;
			ledB = 0;
			ledC = 0;
			ledD = 1;
			ledE = 1;
			ledF = 0;
			ledG = 0;
			break;

		case 5:
			ledA = 0;
			ledB = 1;
			ledC = 0;
			ledD = 0;
			ledE = 1;
			ledF = 0;
			ledG = 0;
			break;

		case 6:
			ledA = 0;
			ledB = 1;
			ledC = 0;
			ledD = 0;
			ledE = 0;
			ledF = 0;
			ledG = 0;
			break;

		case 7:
			ledA = 0;
			ledB = 0;
			ledC = 0;
			ledD = 1;
			ledE = 1;
			ledF = 1;
			ledG = 1;
			break;

		case 8:
			ledA = 0;
			ledB = 0;
			ledC = 0;
			ledD = 0;
			ledE = 0;
			ledF = 0;
			ledG = 0;
			break;

		case 9:
			ledA = 0;
			ledB = 0;
			ledC = 0;
			ledD = 0;
			ledE = 1;
			ledF = 0;
			ledG = 0;
			break;

		default:
			break;
		}

	//Se cargan lso valores a los pines.

	GPIO_WritePin(&handlerLedA, ledA);
	GPIO_WritePin(&handlerLedB, ledB);
	GPIO_WritePin(&handlerLedC, ledC);
	GPIO_WritePin(&handlerLedD, ledD);
	GPIO_WritePin(&handlerLedE, ledE);
	GPIO_WritePin(&handlerLedF, ledF);
	GPIO_WritePin(&handlerLedG, ledG);

}
//Funcion que "cuenta" para el prgorama de la culebrita

void funcionCulebrita(void){
	//condicional que verifica la bandera para realizar cambios en el "contador" con su respectivo filtro
	if(banderaContador == 1 && banderaFiltro ==1){
			banderaFiltro = 0; //Se baja la bandera
			banderaContador = 0;//Se baja la bandera
			TIM4->CR1 |= TIM_CR1_CEN;//Se pone en marcha de nuevo el contador del filtro
			if(GPIO_ReadPin(&handlerEncoderB) == 0){
				if(contadorc != 11){//Se suma siempre y cuando el valor actual no sea once
					contadorc++;
				}
				else{contadorc = 0;}//Se reinicia para comenzar el ciclo de nuevo
			} else{
				if(contadorc != 0){//Se resta siempre y cuadno el valor actual no sea cero
					contadorc--;
				} else{contadorc = 11;}//Se reinicia para comenzar el ciclo de nuevo
			}
		}
	//Se baja la bandera para los casos donde la interrupcion se haya dado en un momento no requerido
	//por lo que se toma como ruido
	else if(banderaContador == 1){
			banderaContador = 0;
		}
	//Se llama la función que prende los led de la culebrita.
	funcionLedsC();
}

/*
 * Funcion que prende los segmentos de la culebrita, funciona muy similar a la otra funcion anterior, pero en esta también se
 * especifica el valor de los transistores para prenderlo en el lugar deseado. Siendo 1 en el transistor unidades y 0 decenas
 */
void funcionLedsC(void){
	switch(contadorc){
		case 0:
			GPIO_WritePin(&handlerTransistor, 1);
			ledA = 0;
			ledB = 1;
			ledC = 1;
			ledD = 1;
			ledE = 1;
			ledF = 1;
			ledG = 1;
			break;

		case 1:
			GPIO_WritePin(&handlerTransistor, 0);
			ledA = 0;
			ledB = 1;
			ledC = 1;
			ledD = 1;
			ledE = 1;
			ledF = 1;
			ledG = 1;
			break;

		case 2:
			GPIO_WritePin(&handlerTransistor, 0);
			ledA = 1;
			ledB = 1;
			ledC = 1;
			ledD = 1;
			ledE = 1;
			ledF = 0;
			ledG = 1;
			break;

		case 3:
			GPIO_WritePin(&handlerTransistor, 0);
			ledA = 1;
			ledB = 1;
			ledC = 1;
			ledD = 1;
			ledE = 0;
			ledF = 1;
			ledG = 1;
			break;

		case 4:
			GPIO_WritePin(&handlerTransistor, 0);
			ledA = 1;
			ledB = 1;
			ledC = 1;
			ledD = 0;
			ledE = 1;
			ledF = 1;
			ledG = 1;
			break;

		case 5:
			GPIO_WritePin(&handlerTransistor, 1);
			ledA = 1;
			ledB = 1;
			ledC = 1;
			ledD = 1;
			ledE = 0;
			ledF = 1;
			ledG = 1;
			break;

		case 6:
			GPIO_WritePin(&handlerTransistor, 1);
			ledA = 1;
			ledB = 1;
			ledC = 1;
			ledD = 1;
			ledE = 1;
			ledF = 0;
			ledG = 1;
			break;

		case 7:
			GPIO_WritePin(&handlerTransistor, 0);
			ledA = 1;
			ledB = 0;
			ledC = 1;
			ledD = 1;
			ledE = 1;
			ledF = 1;
			ledG = 1;
			break;

		case 8:
			GPIO_WritePin(&handlerTransistor, 0);
			ledA = 1;
			ledB = 1;
			ledC = 0;
			ledD = 1;
			ledE = 1;
			ledF = 1;
			ledG = 1;
			break;

		case 9:
			GPIO_WritePin(&handlerTransistor, 1);
			ledA = 1;
			ledB = 1;
			ledC = 1;
			ledD = 0;
			ledE = 1;
			ledF = 1;
			ledG = 1;
			break;

		case 10:
			GPIO_WritePin(&handlerTransistor, 1);
			ledA = 1;
			ledB = 1;
			ledC = 0;
			ledD = 1;
			ledE = 1;
			ledF = 1;
			ledG = 1;
			break;

		case 11:
			GPIO_WritePin(&handlerTransistor, 1);
			ledA = 1;
			ledB = 0;
			ledC = 1;
			ledD = 1;
			ledE = 1;
			ledF = 1;
			ledG = 1;
			break;

		default:
			break;

	}

	//Se cargan los valores a los pines.
	GPIO_WritePin(&handlerLedA, ledA);
	GPIO_WritePin(&handlerLedB, ledB);
	GPIO_WritePin(&handlerLedC, ledC);
	GPIO_WritePin(&handlerLedD, ledD);
	GPIO_WritePin(&handlerLedE, ledE);
	GPIO_WritePin(&handlerLedF, ledF);
	GPIO_WritePin(&handlerLedG, ledG);

}


void funcionFrecuencia(void){

	//Se tienen 3 posibles opciones de frecuencia y cada una prende una de las conmponentes principales del led RGB
	//O rojo o verde o azul.
	switch(contadorFrec){
		case 0:
			handlerTim3.TIMx_Config.TIMx_period = 100;
			ledFrecA = 1;
			ledFrecB = 0;
			ledFrecC = 0;
			break;
		case 1:
			handlerTim3.TIMx_Config.TIMx_period = 500;
			ledFrecA = 0;
			ledFrecB = 1;
			ledFrecC = 0;
			break;
		case 2:
			handlerTim3.TIMx_Config.TIMx_period = 1000;
			ledFrecA = 0;
			ledFrecB = 0;
			ledFrecC = 1;
			break;
		default:
			break;
	}

	if(contadorFrec!=2){
		contadorFrec++;//Suma siempre y cuando no sea 2.
	} else{contadorFrec = 0;}//Se reinicia.

	BasicTimer_Config(&handlerTim3);//Se le da la nueva configuracion al timer

	//Se le cargan los valores a los pines que controlan el RGB
	GPIO_WritePin(&handlerLedFrecA, ledFrecA);
	GPIO_WritePin(&handlerLedFrecB, ledFrecB);
	GPIO_WritePin(&handlerLedFrecC, ledFrecC);

}

//Funcion que borra los numeros en el 7 segmentos
void resetDisplay(void){
	//Se le asigna un valor de 1 a todos ya que asi se apagan.
	ledA = 1;
	ledB = 1;
	ledC = 1;
	ledD = 1;
	ledE = 1;
	ledF = 1;
	ledG = 1;
	//Se cargan los valores a los pines.
	GPIO_WritePin(&handlerLedA, ledA);
	GPIO_WritePin(&handlerLedB, ledB);
	GPIO_WritePin(&handlerLedC, ledC);
	GPIO_WritePin(&handlerLedD, ledD);
	GPIO_WritePin(&handlerLedE, ledE);
	GPIO_WritePin(&handlerLedF, ledF);
	GPIO_WritePin(&handlerLedG, ledG);

}

