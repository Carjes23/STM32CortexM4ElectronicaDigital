/*
 * Taller8_template.c
 *
 *  Created on: 28/03/2023
 *      Author: alvelezp - jutoroa
 */

// Taller 8 - Paso por valor y paso por referencia

#include <stdint.h>

// Creacion de una funcion que duplique el valor de un numero

// 1.0 Paso por valor Básico

void duplicarNumero(uint8_t numero){
	// No modifica numero, sino que le hace las operaciones a una variable local
	numero *= 2;
}

// 1.1 Paso por referencia

void duplicarNumeroRef(uint8_t *numero){
	*numero *= 2;//Modifica el numero pero usando punteros.
}

// 1.2 Paso por valor reasignando variables.

uint8_t duplicarNumeroReturn(uint8_t numero){
	return(2*numero);
}

// 1.3 Arreglos por Referencia

void abonarDeudas(uint16_t misDeudas[], uint8_t cantidadDeudas){

	for(uint8_t i = 0; i < cantidadDeudas; i++){
		misDeudas[i] /= 2;
	}

}

// ***** // SOLUCION EJERCICIO // ***** //

void stringCaseConverter(char *string){
	uint8_t j = 00;
	while(string[j] != 00){
		if (string[j] < 91 && string[j] > 64){
			string[j] += 32;
		}
		else if (string[j] < 123 && string[j] > 96){
			string[j] -= 32;
	}
		j++;
	}
}

int main(void){

	// Crear una variable-
	uint8_t n = 10;

	duplicarNumero(n);

	duplicarNumeroRef(&n);


	n = duplicarNumeroReturn(n);

	uint16_t deudadMensuales[5] = {10, 20, 30, 40, 50};

	abonarDeudas(deudadMensuales, 5);


	char string[] = "Hola como estas?";

	stringCaseConverter(string);


	/* 1.5 EJERCICIO:

	Crear una función llamada stringCaseConverter que no retorne ningún
	valor, y reciba una string.

	Esta función deberá cambiar las minúsculas por mayúsculas y viceversa
	del string original. */
	return 0;

}
