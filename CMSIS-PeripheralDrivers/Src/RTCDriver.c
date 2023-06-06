/*
 * RTCDriver.c
 *
 *  Created on: Jun 5, 2023
 *      Author: daniel
 */

#include "RTCDriver.h"

void config_RTC(RTC_t *pRTC) {

	// Activamos la señal de reloj para el PWR
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;

	// Desactivamos el modo Backup Protection
	PWR->CR |= PWR_CR_DBP;

	// Activamos el LSE (LOW SPEED EXTERNAL 32KHz)
	RCC->BDCR |= RCC_BDCR_LSEON;

	// Habilitamos la señal proveniente del LSE (LOW SPEED EXTERNAL 32KHz)
	RCC->BDCR |= RCC_BDCR_RTCSEL_0;

	// Habilitamos la compuerta AND para el RTC
	RCC->BDCR |= RCC_BDCR_RTCEN;

	/* Iniciamos rutina de inicialización */

	// Desactivamos el Write Protection register con la clave
	RTC->WPR = 0xCAU;
	RTC->WPR = 0x53U;

	/* 1.0 Setiamos el bit INIT	en el ISR */
	RTC->ISR |= RTC_ISR_INIT;

	/* 2.0 Ponemos en 1 el bit de INITF */
	RTC->ISR |= RTC_ISR_INITF;

	/* 3.0 Configuramos el preescaler */

	RTC->PRER |= RTC_PRER_PREDIV_A;
	RTC->PRER |= (0xFF << RTC_PRER_PREDIV_S_Pos);

	/* 4.0 Cargamos los valores del calendario (DR - Date Register) y de la hora (TR - Time Register)*/

	// Escogemos el modo de 24 horas
	RTC->CR &= ~(RTC_CR_FMT);
	RTC->TR &= ~(RTC_TR_PM);





	/* 5.0 Salimos del modo de inicialización */
	RTC->ISR &= ~RTC_ISR_INIT;

	/* 6.0 Activamos nuevamente el Write Protection */
	RTC->WPR = 0xFFU;

}

// Función para convertir de numeros decimales a código BCD
uint8_t decToBCD(int val) {
	return (uint8_t) ((val / 10 * 16) + (val % 10));
}

// Función para convertir de código BCD a numeros decimales

int BCDToDec(uint8_t val) {
	return (int) ((val / 16 * 10) + (val % 16));
}

void setSegundos(int val) {
	// Escribimos los segundos
	RTC->TR |= (decToBCD(val)) << RTC_TR_MNU_Pos;
}

void setHour(int val) {
	// Escribimos las horas
	RTC->TR |= (decToBCD(val)) << RTC_TR_HU_Pos;
}

void setMinutes(int val) {
	// Escribimos los minutos
	RTC->TR |= (decToBCD(val)) << RTC_TR_MNU_Pos;
}

void setDia(int val){
	// Escribimos el dia
	RTC->DR |= (decToBCD(val)) << RTC_DR_DU_Pos;
}

void setMes(int val){
	// Escribimos el dia
	RTC->DR |= (decToBCD(val)) << RTC_DR_MU_Pos;
}

void setYear(int val){
	// Escribimos el dia
	RTC->DR |= (decToBCD(val)) << RTC_DR_YU_Pos;
}

void setDiaSemana(int val){
	// Escribimos el dia
	RTC->DR |= (decToBCD(val)) << RTC_DR_WDU_Pos;
}



