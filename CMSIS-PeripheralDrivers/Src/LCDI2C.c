/*
 * LCDI2C.c
 *
 *  Created on: May 26, 2023
 *      Author: daniel
 */

#include "LCDI2C.h"
#include "I2CxDriver.h"
#include "SysTick.h"
#include <stdint.h>

LCDI2C_handler_t ptr = { 0 };

void lcdi2cconfig(LCDI2C_handler_t *ptrLcd) {
	ptr = *ptrLcd;
//	delay_ms(20);
//	lcd_send_cmd(0b11<<4);
//	delay_ms(5);
//	lcd_send_cmd(0b11<<4);
//	delay_100us(2);
//	lcd_send_cmd(0b11<<4);
//	delay_ms(5);
//	lcd_send_cmd(0b11<<4);
//	lcd_send_cmd(0b1);
//	delay_100us(2);
//	lcd_send_cmd(0x8);
//	lcd_send_cmd(0x28);
//	lcd_send_cmd(0x06);
//	lcd_send_cmd(0x28);
//	lcd_send_cmd(0xF);
//	lcdClear(ptrLcd);
//	lcdHome(ptrLcd);
//
//	lcd_send_cmd(0b11<<4);
//	lcd_send_cmd(0b111<<2);
//	lcd_send_cmd(0b11<<2);
//	lcd_send_cmd('1');

	lcd_send_cmd(0x30);
	delay_ms(5);  // wait for >4.1ms
	lcd_send_cmd(0x30);
	delay_ms(1);  // wait for >100us
	lcd_send_cmd(0x30);
	delay_ms(10);
	lcd_send_cmd(0x20);  // 4bit mode
	delay_ms(10);
	lcd_send_cmd(0x28); // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	delay_ms(1);
	lcd_send_cmd(0x08); //Display on/off control --> D=0,C=0, B=0  ---> display off
	delay_ms(1);
	lcd_send_cmd(0x01);  // clear display
	delay_ms(1);
	lcd_send_cmd(0x06); //Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	delay_ms(1);
	lcd_send_cmd(0x0C); //Display on/off control --> D = 1, C and B = 0. (Cursor and blink, last two bits)
	delay_ms(1);
	lcd_send_data('H');

}

void lcdHome(LCDI2C_handler_t *ptrLcd) {
	lcd_send_cmd(0x02);
	delay_ms(4);
}

void lcdClear(LCDI2C_handler_t *ptrLcd) {
	lcd_send_cmd(0x01);
	delay_ms(4);
}

void lcdMoveCursorTo(LCDI2C_handler_t *ptrLcd, uint16_t posicion) {
	lcd_send_cmd(0x80 | posicion);
}

void lcdCursorOnOff(LCDI2C_handler_t *ptrLcd, uint8_t onOff) {
	if (onOff == 0) { // OFF
		lcd_send_cmd(0x0C);
	} else {
		lcd_send_cmd(0x0F);
	}
}


void lcd_send_cmd (char cmd)
{
  char data_u, data_l;
	uint8_t data_t[4];
	data_u = (cmd&0xf0);
	data_l = ((cmd<<4)&0xf0);
	data_t[0] = data_u|0x0C;  //en=1, rs=0
	data_t[1] = data_u|0x08;  //en=0, rs=0
	data_t[2] = data_l|0x0C;  //en=1, rs=0
	data_t[3] = data_l|0x08;  //en=0, rs=0
	i2c_writeMultTimeSameRegister(ptr.ptrHandlerI2C, 0x00, data_t, 4);

}

void lcd_send_data (char data)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (data&0xf0);
	data_l = ((data<<4)&0xf0);
	data_t[0] = data_u|0x0D;  //en=1, rs=1
	data_t[1] = data_u|0x09;  //en=0, rs=1
	data_t[2] = data_l|0x0D;  //en=1, rs=1
	data_t[3] = data_l|0x09;  //en=0, rs=1
	i2c_writeMultTimeSameRegister(ptr.ptrHandlerI2C, 0x00, data_t, 4);

}

void lcdWriteMessage(const char * message){
	int i = 0;
	if(message[i] != 0){
		lcd_send_data(message[i]);
		i++;
	}
}
