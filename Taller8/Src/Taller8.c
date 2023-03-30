/*
 * mainExti.c
 *
 *  Created on: Mar 27, 2023
 *      Author: jutoroa
 */

#include <stdint.h>
#include "ExtiDriver.h"
#include "GPIOxDriver.h"

GPIO_Handler_t handlerStateLed = {0};
GPIO_Handler_t handlerEncoder = {0};

EXTI_Config_t handlerExti = {0};

void initSystem(void);

int main(void){

	initSystem();

	while(1){
		__NOP();
	}

}

void initSystem(void){
	handlerStateLed.pGPIOx = GPIOA;
	handlerStateLed.GPIO_PinConfig_t.GPIO_PinNumber = PIN_5;
	handlerStateLed.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_OUT;
	handlerStateLed.GPIO_PinConfig_t.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
	handlerStateLed.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_HIGH;

	GPIO_Config(&handlerStateLed);

	handlerEncoder.pGPIOx = GPIOA;
	handlerEncoder.GPIO_PinConfig_t.GPIO_PinNumber = PIN_0;
	handlerEncoder.GPIO_PinConfig_t.GPIO_PinMode = GPIO_MODE_IN;
	handlerEncoder.GPIO_PinConfig_t.GPIO_PinPuPdControl = GPIO_PUPDR_PULLDOWN;
	handlerEncoder.GPIO_PinConfig_t.GPIO_PinSpeed = GPIO_OSPEED_HIGH;


	handlerExti.edgeType = EXTERNAL_INTERRUPT_RISING_EDGE;
	handlerExti.pGPIOHandler = &handlerEncoder;

	extInt_Config(&handlerExti);
}

void callback_extInt0(void){

}


