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
	delay_ms(20);  // wait for >4.1ms
	lcd_send_cmd(0x30);
	delay_ms(5);  // wait for >4.1ms
	lcd_send_cmd(0x30);
	delay_100us(3);  // wait for >100us
	lcd_send_cmd(0x30);
	delay_ms(5);
	lcd_send_cmd(0x20);  // 4bit mode
	delay_100us(3);
	lcd_send_cmd(0x20);  // 4bit mode
	lcd_send_cmd(0x08); // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	lcd_send_cmd(0x28); //Display on/off control --> D=0,C=0, B=0  ---> display off
	lcd_send_cmd(0x06);  // clear display
	lcd_send_cmd(0x0F); //Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	lcdClear();
}

void lcdHome(void) {
	lcd_send_cmd(0x02);
	delay_ms(5);
}

void lcdClear(void) {
	lcd_send_cmd(0x01);
	delay_ms(20);
}

void lcd_clear2 (void)
{
	lcdHome();
	for (int i=0; i<100; i++)
	{
		lcd_send_data (' ');
	}
}

void lcdMoveCursorTo(uint16_t posicion) {
	lcd_send_cmd(0x80 + posicion);
	delay_ms(5);
}

void lcdCursorOnOff(LCDI2C_handler_t *ptrLcd, uint8_t onOff) {
	if (onOff == 0) { // OFF
		lcd_send_cmd(0x0C);
	} else {
		lcd_send_cmd(0x0F);
	}
	delay_100us(5);
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
	delay_ms(1);
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
	delay_ms(1);
}

void lcdWriteMessage(const char * message){
	int i = 0;
	while(message[i]){
		lcd_send_data(message[i]);
		i++;
	}
}
