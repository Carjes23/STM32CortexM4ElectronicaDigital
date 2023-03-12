/*
 * Taller5.c
 *
 *  Created on: Mar 9, 2023
 *      Author: dacardenasj
 */

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define CONSTANTE  100
#define UNSIGNED 0
#define SIGNED 1

void clearGlobal(void);
uint8_t getMaxChar(void);
uint32_t getMaxVarValue (uint8_t a, bool b );

bool variableBooleana = true;
uint8_t parametro1 = 180;
uint8_t parametro2 = 200;

void clearGlobal(void){
	parametro1 = 0;
	parametro2 = 0;
	variableBooleana = false;
};

int main(void){

	//cambio
	clearGlobal();

	uint8_t maxChar = getMaxChar();
	uint32_t max = getMaxVarValue (16, 1);

	return 0;

}


int function(uint8_t int1, uint16_t int2){

}

void function2(uint8_t int1, uint16_t int2){

}
uint8_t getMaxChar(void){
	uint8_t maxChar = pow(2,8)-1;
	return maxChar;
}

uint32_t getMaxVarValue (uint8_t a, bool b ){
	uint32_t maxVar= 0;
	if (b==UNSIGNED){
		maxVar = pow(2,a)-1;
	} else {
		maxVar = pow(2,a)/2-1;
	};
	return maxVar;

}


