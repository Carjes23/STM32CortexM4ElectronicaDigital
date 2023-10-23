/*
 * SysTickDriver.h
 *
 *  Created on: May 2, 2022
 *      Author: namontoy
 *
 *  Modificado
 *      	  : emangelma
 */

#ifndef INC_SYSTICKDRIVER2_H_
#define INC_SYSTICKDRIVER2_H_

#include <stm32f4xx.h>

extern volatile uint64_t ticks;
extern volatile uint32_t ticksDown;

void config_SysTickMs(void);
uint64_t getTicksMs(void);
void systick_delayMs(uint32_t msToWait);

/**
 * Retorna el valor de ticksDown
 */
#define SYSTICK_ticksDown()				(ticksDown)

/**
 * Establece el valor de ticksDown como time [ms]
 */
#define SYSTICK_setTicksDown(time)		(ticksDown = (time))

#endif /* INC_SYSTICKDRIVER2_H_ */
