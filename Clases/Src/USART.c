/**
 ******************************************************************************
 * @file           : main.c
 * @author         : dacardenasj
 * @brief          : Configuracion basica de un proyecto
 ******************************************************************************
  *
  * Notas de clase.
  *
  * USART o UART.
  *
  * Qué es un protocolo de comunicación.
  *
  * 	Es un conjunto de reglas que describen como se trasmite o se intercambia datos,
  * 	especialmente dentro de la red.
  *
  * 	Se compone de;
  * 		Emisor -> Mensaje -> Canar (Siempre analogo).
  * 		Receptor -> Respuesta.
  * 	Para que la comunicación sea exitosa ambos elementos deben conocer las reglas.
  *
  *	Por qué se llaman seriales?
  *		Ya que los elementos van llegando en "orden" uno detras del otro.El pápa es el
  *		RS232.
  *
  *	Asíncronos vs Síncronos.
  *		Como su nombre lo indica lso asíncronos son exporadicos no tenemos tiempo fijo
  *		de trasmisión los tiempos donde no se realiza nada se les conoce como
  *		trasmission gaps. Mientras que los Síncronos son constantes en el tiempo.
  *
  *		Los asíncronos siempre deben tener donde inicia un dato y donde se termina un dato.
  *
  *	Comunicación en paralelo vs serial.
  *		Si se inban a mandar 8 datos se tenian 8 trasmisores y 8 receptores
  *		Mientras que la seríal solo tiene y se va trasmitiendo uno por uno.
  *
  *	Importancia del RS-232
  *		Ansicrono.
  *		Empaquetamiento.
  *		Modular.
  *		Nuevas aplicaciones.
  *		Verificación de errores
  *		Velodidad.
  *		Tamaño.
  *		Consumo de energía.
  *	RS-232 Data Frame. (Recommended Standar - 2323)
  *		Establece.
  *			1 sbit de inicio (siempre va a ser un 0)
  *			1 o 2 bits de parada
  *			Se establece ciclo ancho de datos. (5 bits, 6bits, 7buts, 8bits de datos).
  *			Bit de paridad(verificación) antes del final, ver si el numero de unos es par
  *			o impar (Parte de los stops bits). -> Baud Rate (velocidad de verificación) también
  *			es el tiempo de un bit, valores tipicos (9600, 19200 , 570600, 115200 bits/s)
  *			configuración más tipicas 9600_8N1; 9600_8B2; 52600_8O1, el bitde paridad es opcional.
  *			El bit menos significativo va primero.
  *
  *
  *
  *
  *
  *
 ******************************************************************************
 */


#include <stdint.h>
/*
 * Funcion principal del programa
 * Esta funcion es el corazon del programa!!
 *
 * */



int main(void){
	/*Loop forever */
	while(1){

	}

	return 0;
}

