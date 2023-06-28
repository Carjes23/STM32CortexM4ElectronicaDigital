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
	/*
	 * Comandos necesarios para iniciar la LCD
	 */
	ptr = *ptrLcd;
	delay_ms(50);  // Esperar más de 15 ms al iniciar
	lcd_send_cmd(0x30);
	delay_ms(5);  // esperar más de 5 ms
	lcd_send_cmd(0x30);
	delay_100us(50);  //esperar más de 150 us
	lcd_send_cmd(0x30);
	delay_ms(50);		//Esperar más de 5 ms
	lcd_send_cmd(0x20);  // modo de 4 bits
	delay_ms(50);			//Esperar más de 5 ms
	lcd_send_cmd(0x28); //Apagamos el display
	delay_ms(50);		//Esperar más de 5 ms
	lcd_send_cmd(0x08); // Function set -> Colocamos lo necesario en cuestion de lineas y modo de uso
	delay_ms(50);
	lcd_send_cmd(0x01); //Coocamos el modo de entrada y que el cursor se mueva a la derecha con cada dato.
	delay_ms(50);		//Esperar más de 5 ms
	lcd_send_cmd(0x06);  // Limpiamos el display
	delay_ms(50);		//Esperar más de 5 ms
	lcd_send_cmd(0x0C); //Coocamos el modo de entrada y que el cursor se mueva a la derecha con cada dato.

}
/*
 * Función para ir a la primera posición
 */
void lcdHome(void) {
	lcd_send_cmd(0x02);
	delay_ms(5);
}
/*
 * Función para borrar lo de pantalla
 */
void lcdClear(void) {
	lcd_send_cmd(0x01);
	delay_ms(20);
}

void lcdMoveCursorTo(uint16_t posicion) {
	lcd_send_cmd(0x80 + posicion);
//	delay_ms(5);
}
/*
 * Apagar el cursor
 */
void lcdCursorOnOff(uint8_t onOff) {
	if (onOff == 0) { // OFF
		lcd_send_cmd(0x0C);
	} else {
		lcd_send_cmd(0x0F);
	}
	delay_100us(5);
}
/*
 * Funciones para mandar datos, donde se parten los datos en dos
 * estos se envian primero con el enable apagado y luego se envian
 * los mismos datos con el enable encendido para que se escriban en la pantalla
 * Esto funciona igual en enviar datos, solo que el RS se encuentra en alto.
 */
void lcd_send_cmd(char cmd) {
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (cmd & 0xf0);
	data_l = ((cmd << 4) & 0xf0);
	data_t[0] = data_u | 0x0C;
	data_t[1] = data_u | 0x08;
	data_t[2] = data_l | 0x0C;
	data_t[3] = data_l | 0x08;
	i2c_writeMultTimeSameRegister(ptr.ptrHandlerI2C, 0x00, data_t, 4);
}

void lcd_send_data(char data) {
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (data & 0xf0);
	data_l = ((data << 4) & 0xf0);
	data_t[0] = data_u | 0x0D;
	data_t[1] = data_u | 0x09;
	data_t[2] = data_l | 0x0D;
	data_t[3] = data_l | 0x09;


	i2c_writeMultTimeSameRegister(ptr.ptrHandlerI2C, 0x00, data_t, 4);
}
/**
 * Para enviar mensajes uso repetido de enviar data.
 */
void lcdWriteMessage(const char *message) {
	int i = 0;
	while (message[i]) {
		lcd_send_data(message[i]);
		i++;
	}
}
