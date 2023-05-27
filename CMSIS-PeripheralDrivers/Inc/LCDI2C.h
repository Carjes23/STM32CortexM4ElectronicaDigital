/*
 * LCDI2C.h
 *
 *  Created on: May 26, 2023
 *      Author: daniel
 */

#ifndef LCDI2C_H_
#define LCDI2C_H_

#include "I2CxDriver.h"




// ILI9341 Sctruc.
typedef struct
{
    I2C_Handler_t *ptrHandlerI2C;		//El I2C utilizado
} LCDI2C_handler_t;


void lcdi2cconfig(LCDI2C_handler_t *ptrLcd);
void lcdHome(LCDI2C_handler_t *ptrLcd);
void lcdClear(LCDI2C_handler_t *ptrLcd);
void lcdMoveCursorTo(LCDI2C_handler_t *ptrLcd,uint16_t posicion);
void lcdCursorOnOff(LCDI2C_handler_t *ptrLcd, uint8_t onOff);
void lcd_send_cmd (char cmd);
void lcd_send_data (char data);

#endif /* LCDI2C_H_ */
