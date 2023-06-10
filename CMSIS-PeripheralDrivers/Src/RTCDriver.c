/*
 * RTCDriver.c
 *
 *  Created on: Jun 5, 2023
 *      Author: daniel
 */

#include "RTCDriver.h"

void config_RTC(void) {

	// Activamos la señal de reloj para el PWR
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;

	// Desactivamos el modo Backup Protection
	PWR->CR |= PWR_CR_DBP;

	// Habilitamos la compuerta AND para el RTC
	RCC->BDCR |= RCC_BDCR_RTCEN;

	// Activamos el LSE (LOW SPEED EXTERNAL 32KHz)
	RCC->BDCR |= RCC_BDCR_LSEON;

	while((RCC->BDCR & RCC_BDCR_LSERDY_Pos) == 0){
		__NOP();
	}


	// Habilitamos la señal proveniente del LSE (LOW SPEED EXTERNAL 32KHz)
	RCC->BDCR |= RCC_BDCR_RTCSEL_0;



	/* Iniciamos rutina de inicialización */

	// Desactivamos el Write Protection register con la clave
	RTC->WPR |= 0xCA;
	RTC->WPR |= 0x53;

	/* 1.0 Setiamos el bit INIT	en el ISR */
	RTC->ISR |= RTC_ISR_INIT;

	/* 2.0 Ponemos en 1 el bit de INITF */
	while((RTC->ISR & RTC_ISR_INITF)==0){
		__NOP();
	}

	/* 3.0 Configuramos el preescaler */

	RTC->PRER |= RTC_PRER_PREDIV_A;
	RTC->PRER |= (0xFF << RTC_PRER_PREDIV_S_Pos);


	RTC->CR |= RTC_CR_BYPSHAD;
	/* 4.0 Cargamos los valores del calendario (DR - Date Register) y de la hora (TR - Time Register)*/
	RTC->DR = 0;
	RTC->TR = 0;
	// Escogemos el modo de 24 horas
	RTC->CR &= ~(RTC_CR_FMT);
	RTC->TR &= ~(RTC_TR_PM);

	RTC->TR |= (decToBCD(11)) << RTC_TR_SU_Pos;
	RTC->TR |= (decToBCD(11)) << RTC_TR_HU_Pos;
	RTC->TR |= (decToBCD(11)) << RTC_TR_MNU_Pos;
	RTC->DR |= (decToBCD(23)) << RTC_DR_DU_Pos;
	RTC->DR |= (decToBCD(11)) << RTC_DR_MU_Pos;
	RTC->DR |= (decToBCD(01)) << RTC_DR_YU_Pos;
	RTC->DR |= (decToBCD(RTC_WEEKDAY_FRIDAY)) << RTC_DR_WDU_Pos;

	/* 5.0 Salimos del modo de inicialización */
	RTC->ISR &= ~RTC_ISR_INIT;



	/* 6.0 Activamos nuevamente el Write Protection */
	RTC->WPR = 0xFFU;

}

// Función para convertir de numeros decimales a código BCD
uint8_t decToBCD(int val) {
	uint8_t aux = (uint8_t) ((val / 10 * 16) + (val % 10));
	return aux;
}

// Función para convertir de código BCD a numeros decimales

int BCDToDec(uint8_t val) {
	int aux = (int) ((val / 16 * 10) + (val % 16));
	return aux;
}

void setSegundos(int val) {
	// Escribimos los segundos
	RTC->TR &= ~(RTC_TR_SU|RTC_TR_ST);
	RTC->TR |= (decToBCD(val)) << RTC_TR_SU_Pos;
}

uint8_t getSegundos(void) {
	// Escribimos los segundos
	uint8_t aux = (int) ((RTC->TR) & (RTC_TR_SU|RTC_TR_ST))>>RTC_TR_SU_Pos;
	return BCDToDec(aux);
}

void setHour(int val) {
	RTC->TR &= ~(RTC_TR_HU|RTC_TR_HT);
	// Escribimos las horas
	RTC->TR |= (decToBCD(val)) << RTC_TR_HU_Pos;
}

uint8_t getHour(void) {
	// Escribimos los segundos
	uint8_t aux = (int) ((RTC->TR) & (RTC_TR_HT|RTC_TR_HU))>>RTC_TR_HU_Pos;
	return BCDToDec(aux);
}


void setMinutes(int val) {
	// Escribimos los minutos
	RTC->TR &= ~(RTC_TR_MNU|RTC_TR_MNT);
	RTC->TR |= (decToBCD(val)) << RTC_TR_MNU_Pos;
}

uint8_t getMinutes(void) {
	// Escribimos los segundos
	uint8_t aux = (int) ((RTC->TR) & (RTC_TR_MNT|RTC_TR_MNU))>>RTC_TR_MNU_Pos;
	return BCDToDec(aux);
}

void setDia(int val){
	// Escribimos el dia
	RTC->DR &= ~(RTC_DR_DT|RTC_DR_DU);
	RTC->DR |= (decToBCD(val)) << RTC_DR_DU_Pos;
}

uint8_t getDia(void) {
	// Escribimos los segundos
	uint8_t aux = (int) ((RTC->DR) & (RTC_DR_DT|RTC_DR_DU))>>RTC_DR_DU_Pos;
	return BCDToDec(aux);
}

void setMes(int val){
	// Escribimos el dia
	RTC->DR &= ~(RTC_DR_MU|RTC_DR_MT);
	RTC->DR |= (decToBCD(val)) << RTC_DR_MU_Pos;
}
uint8_t getMes(void) {
	// Escribimos los segundos
	uint8_t aux = (int) ((RTC->DR) & (RTC_DR_MT|RTC_DR_MU))>>RTC_DR_MU_Pos;
	return BCDToDec(aux);
}

void setYear(int val){
	// Escribimos el dia
	RTC->DR &= ~(RTC_DR_YT|RTC_DR_YU);
	RTC->DR |= (decToBCD(val)) << RTC_DR_YU_Pos;
}

uint8_t getYear(void) {
	// Escribimos los segundos
	uint8_t aux = (int) ((RTC->DR) & (RTC_DR_YT|RTC_DR_YU))>>RTC_DR_YU_Pos;
	return BCDToDec(aux);
}

void setDiaSemana(int val){
	// Escribimos el dia
	RTC->DR &= ~(RTC_DR_WDU);
	RTC->DR |= (decToBCD(val)) << RTC_DR_WDU_Pos;
}

uint8_t getDiaSemana(void) {
	// Escribimos los segundos
	uint8_t aux = (int) ((RTC->DR) & (RTC_DR_WDU))>>RTC_DR_WDU_Pos;
	return BCDToDec(aux);
}

void enableRTCChange(void){
	// Desactivamos el Write Protection register con la clave
	RTC->WPR |= 0xCA;
	RTC->WPR |= 0x53;

	/* 1.0 Setiamos el bit INIT	en el ISR */
	RTC->ISR |= RTC_ISR_INIT;

	/* 2.0 Ponemos en 1 el bit de INITF */
	while((RTC->ISR & RTC_ISR_INITF)==0){
		__NOP();
	}

}

void disableRTCChange(void){
	/* 5.0 Salimos del modo de inicialización */
	RTC->ISR &= ~RTC_ISR_INIT;

	/* 6.0 Activamos nuevamente el Write Protection */
	RTC->WPR = 0xFFU;
}

