/*
 * PLLDriver.c
 *
 *  Created on: May 21, 2023
 *      Author: daniel
 */

#include <stm32f4xx.h>
#include "PLLDriver.h"

void configPLL(uint16_t freqPLL){
	//Selecionamos HSI como la fuente para el PLL
	RCC -> PLLCFGR &= ~(RCC_PLLCFGR_PLLSRC);

	//Asignamos el divisor para la señal de entrada que sea facil de manipular
	//Por lo que la pondremos en 2MHz colocando en el registro 8.
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLM); // Limpiamos
	RCC->PLLCFGR |= (RCC_PLLCFGR_PLLM_3); // Ponemos un 8 en el PLLM

	//Ahora conifugraremos la multiplicación
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLN); // Limpiamos

	//ponemos el PLLN en 80 para conseguir 160MHz
	RCC->PLLCFGR |= (RCC_PLLCFGR_PLLN_2);
	RCC->PLLCFGR |= (RCC_PLLCFGR_PLLN_5);
	RCC->PLLCFGR |= (RCC_PLLCFGR_PLLN_6);

	//  Borro el posible valor configurado
	RCC->PLLCFGR &= ~(0xFF << RCC_PLLCFGR_PLLN_Pos);

	// Cargamos el valor del multiplicador el cual al final terminaria siendo la frecuencia
	//Esto teniendo en cuenta que se utilizamos el PLLp para dividir entre 2
	if(freqPLL <= 100 && 50 < freqPLL)
		RCC->PLLCFGR |= (freqPLL << RCC_PLLCFGR_PLLN_Pos);
	else
		RCC->PLLCFGR |= (PLL_STANDARD_FREQ << RCC_PLLCFGR_PLLN_Pos);

	// La salida del PLL depende del valor VCO y del preescaler PLLP, el cual
	// su minimo valor es de 2 con esto configuramos siempre la frecuencia que le entreguemos
	// entre 50 a 100
	RCC->PLLCFGR &= ~(0b11 << RCC_PLLCFGR_PLLP_Pos); // Salida DIV2

	// Cambio el valor VOS (regulador de voltaje) para que se tenga el
	// rendimiento adecuado Esto se debe hacer antes de encender el PLL

	if(freqPLL <= 64)
		PWR->CR |= (0x1 << PWR_CR_VOS_Pos);
	else if(freqPLL <= 84)
		PWR->CR |= (0x2 << PWR_CR_VOS_Pos);
	else if(freqPLL <= 100)
		PWR->CR |= (0x3 << PWR_CR_VOS_Pos);

	// Activamos el PLL en el control register del RCC
	RCC->CR |= RCC_CR_PLLON;

	// Esperamos a que el PLL se cierre (se estabilice)
	while (!(RCC->CR & RCC_CR_PLLRDY)){
		__NOP();
	}

	/* ---- Configuración de Preescalers para velocidad de los Buses ---- */
	//  Organizamos los preescalers del hardware, de forma que funcionen
	// a la velocidad maxima permitida
	// Preescaler del hardware del procesador, el valor en 0 significa
	// 	 sin preescaler
	RCC->CFGR &= ~(0xF << RCC_CFGR_HPRE_Pos);

	// Preescaler para el APB2 (high speed), el valor en 0 significa
	// sin preescaler
	RCC->CFGR &= ~(0x7 << RCC_CFGR_PPRE2_Pos);

	// Preescaler para el APB1 (low speed). Se debe poner un preescaler
	// de 2:1, para que la velocidad
	// sea la máxima permitida. Primero lo limpio, luego lo configuro
	RCC->CFGR &= ~(0x7 << RCC_CFGR_PPRE1_Pos); // Limpiar
	RCC->CFGR |= (0x4 << RCC_CFGR_PPRE1_Pos);  // Preescaler 2:1

	// Ahora debemos configurar el periodo de espera para leer la
	// memoria flash (Lantency), tal cual como esta descrito en la seccion
	// 3.4.1, tabla 5, pag 45.

	FLASH->ACR &= ~(0xF << FLASH_ACR_LATENCY_Pos); // Limpiamos la posicion

	if(50 <= freqPLL && freqPLL <= 64)
		FLASH -> ACR |= (0x1 << FLASH_ACR_LATENCY_Pos); // Configurando 1 WS
	else if(90 <= freqPLL && freqPLL <= 100)
		FLASH -> ACR |= (0x3 << FLASH_ACR_LATENCY_Pos); // Configurando 3 WS
	else // Default State: (64 <= freqPLL && freqPLL <= 90)
		FLASH -> ACR |= (0x2 << FLASH_ACR_LATENCY_Pos); // Configurando 2 WS

	//  Ahora falta hacer el cambio del sistema a que funcione en la nueva frecuencia
	RCC->CFGR &= ~RCC_CFGR_SW_1;  // Borramos la posicion
	RCC->CFGR |= RCC_CFGR_SW_1;   // PLL como reloj principal


}

// Obtener  la configuración
uint8_t getConfigPLL(void){

	// Seleccion de los dos valores SWS1 y SWS0, que indican cual es la
	// fuente del reloj principal

	uint32_t auxClockState = (RCC->CFGR & 0b1100) >> 2;

	switch(auxClockState){
	case HSI_CLOCK_CONFIGURED:
		return 0;
		break;

	case HSE_CLOCK_CONFIGURED:
		return 1;
		break;

	case PLL_CLOCK_CONFIGURED:
		return 2;
		break;

	default:
		return 0;
		break;

	}
}

// Obtener la frecuencia función importante para cualquier aplicaciom.
uint16_t getFreqPLL(void){

	// Retorno del valor actual de PLL, discriminación del valor actual
	switch(getConfigPLL()){

	default:
	case HSI_CLOCK_CONFIGURED:
		return 16;
		break;

	case PLL_CLOCK_CONFIGURED:

		return (RCC->PLLCFGR & 0b111111111000000) >> RCC_PLLCFGR_PLLN_Pos;
		break;

	}
}

void configPresMCO1(uint8_t prescaler){
	//limpiamos
	RCC->CFGR &= ~(0b111<<RCC_CFGR_MCO1PRE_Pos);
	//colocamos el valor
	RCC->CFGR |= (prescaler<<RCC_CFGR_MCO1PRE_Pos);
}

void configChannelMCO1(uint8_t channel){
	//limpiamos
	RCC->CFGR &= ~(0b11<<RCC_CFGR_MCO1_Pos);
	//colocamos el valor
	RCC->CFGR |= (channel<<RCC_CFGR_MCO1_Pos);
}

void changeTrim(uint8_t newValue){
	//limpiamos
	RCC-> CR &= ~(0b11111<<RCC_CR_HSITRIM_Pos);
	//colocamos el nuevo valor
	RCC-> CR |= newValue<<RCC_CR_HSITRIM_Pos;
}



