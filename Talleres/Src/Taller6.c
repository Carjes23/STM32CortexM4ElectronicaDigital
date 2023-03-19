/**
 ******************************************************************************
 * @file           : main.c
 * @author         : dacardenasj
 * @brief          : Configuracion basica de un proyecto
 ******************************************************************************
  *
 ******************************************************************************
 */

#include <stdint.h>
/*
 * Funcion principal del programa
 * Esta funcion es el corazon del programa!!
 *
 * */
#define SUMA '+'
#define RESTA '-'
#define MULTIPLICACION '*'
#define DIVISION '/'
#define FACTORIAL '!'
#define FIBONACCI 5


uint32_t Operacion2(uint8_t tipoDeOpereacion, uint16_t numero);
uint16_t Operacion(uint8_t tipoDeOpereacion, uint16_t numero1,uint16_t numero2);


// Funcion de calcular operación.

uint16_t Operacion(uint8_t tipoDeOpereacion, uint16_t numero1,uint16_t numero2){
	switch(tipoDeOpereacion){
		case SUMA:
			return numero1 + numero2;
			break;
		case RESTA:
			return numero1 - numero2;
			break;
		case DIVISION:
			return numero1 / numero2;
			break;
		case MULTIPLICACION:
			return numero1 * numero2;
			break;
		default:
			return 0;
			break;
	}
}




int main(void){
	uint32_t variable = 0;

	while(1){
		variable = Operacion2(FACTORIAL, 5);
		variable = Operacion2(FIBONACCI, 10);
		variable = Operacion2(FACTORIAL, 4);
		variable = Operacion2(FIBONACCI, 23);
		variable = Operacion2(FIBONACCI, 30);
	}

	return 0;
}
uint32_t Operacion2(uint8_t tipoDeOpereacion, uint16_t numero){
	switch(tipoDeOpereacion){
		uint16_t resultado = 0;
		case FACTORIAL:
			resultado = 1;
			while(numero!=0){
				resultado *= numero;
				numero--;
			}
			return resultado;
			break;
		case FIBONACCI:
			resultado = 1;
			uint16_t resultadoa = 1;
			uint16_t temp = 0;
			resultadoa = 1;
			resultado = 1;
			while(numero!=0){
				temp = resultadoa;
				resultadoa = resultado;
				resultado += temp;
				numero--;
			}
			return resultado;
			break;
		default:
			return resultado;
	}
}

