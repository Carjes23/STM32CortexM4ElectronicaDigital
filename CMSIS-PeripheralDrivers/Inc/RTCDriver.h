/*
 * RTCDriver.h
 *
 *  Created on: Mayo, 2023
 *      Author: dacardenasj
 */

#ifndef RTCDRIVER_H_
#define RTCDRIVER_H_

#include "stm32f4xx.h"

#define RTC_WEEKDAY_MONDAY		0b001
#define RTC_WEEKDAY_TUESDAY		0b010
#define RTC_WEEKDAY_WEDNESDAY	0b011
#define RTC_WEEKDAY_THURSDAY	0b100
#define RTC_WEEKDAY_FRIDAY		0b101
#define RTC_WEEKDAY_SATURDAY	0b110
#define RTC_WEEKDAY_SUNDAY		0b111


// Función de configuración del RTC
void config_RTC();

// Función para convertir de decimal a BCD
uint8_t decToBCD(int val);
// Función para convertir de BCD a decimal
int BCDToDec(uint8_t val);

void setSegundos(int val);
uint8_t getSegundos(void) ;

void setHour(int val) ;
uint8_t getHour(void) ;

void setMinutes(int val);
uint8_t getMinutes(void) ;

void setDia(int val);
uint8_t getDia(void) ;

void setMes(int val);
uint8_t getMes(void) ;

void setYear(int val);
uint8_t getYear(void) ;

void setDiaSemana(int val);
uint8_t getDiaSemana(void) ;

void enableRTCChange(void);
void disableRTCChange(void);


#endif /* RTCDRIVER_H_ */
