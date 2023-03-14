/*
 * 05-introGPIOx.c
 *
 *  Created on: Mar 9, 2023
 *      Author: daniel
 */

#include "stm32f411xx_hal.h"

int main(void){
	uint32_t variable = 0;
	//configuración inicial MCU.
	//Habilidatndo el clock A y C
	RCC -> AHB1ENR  &= ~(1 << 0);// limpiando
	RCC -> AHB1ENR |= 1 << 0; //asignandole el valor deseado
	RCC -> AHB1ENR  &= ~(1 << 2);// limpiando
	RCC -> AHB1ENR |= 1 << 2; //asignandole el valor deseado

	//configurando el mode
	GPIOA -> MODER &= ~(0b11<<10); //limpiando
	GPIOA -> MODER |= (0b01<<10); //colocando el valor deseado como salida
	GPIOC -> MODER &= ~(0b11<<26); //limpiando

	//configurando el tipo de salida
	GPIOA -> OTYPER &= ~(0b1<<5);	//output push-pull

	//configurar la velocidad.
	GPIOA -> OSPEEDR &= ~(0b11<<10);
	//limpiar
	GPIOA -> ODR &= ~(0b1<<5);


	/*Loop forever */
	while(1){
		//cambiar valores
		variable = GPIOC -> IDR;
		variable &=  (0b1<<13);
		if (variable == 0b1<<13){
			GPIOA -> ODR |= (0b1<<5);
		} else {GPIOA -> ODR &= ~(0b1<<5);};

	}

	return 0;
}
