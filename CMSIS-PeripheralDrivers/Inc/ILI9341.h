/*
 * ILI9341.h
 *
 *  Created on: May 9, 2023
 *      Author: daniel
 */

#ifndef ILI9341_H_
#define ILI9341_H_

//Macros extraidas de los documentos de adafruit.

#define ILI9341_TFTWIDTH 240  ///< ILI9341 max TFT width
#define ILI9341_TFTHEIGHT 320 ///< ILI9341 max TFT height

#define ILI9341_NOP 0x00     ///< No-op register
#define ILI9341_SWRESET 0x01 ///< Software reset register
#define ILI9341_RDDID 0x04   ///< Read display identification information
#define ILI9341_RDDST 0x09   ///< Read Display Status

#define ILI9341_SLPIN 0x10  ///< Enter Sleep Mode
#define ILI9341_SLPOUT 0x11 ///< Sleep Out
#define ILI9341_PTLON 0x12  ///< Partial Mode ON
#define ILI9341_NORON 0x13  ///< Normal Display Mode ON

#define ILI9341_RDMODE 0x0A     ///< Read Display Power Mode
#define ILI9341_RDMADCTL 0x0B   ///< Read Display MADCTL
#define ILI9341_RDPIXFMT 0x0C   ///< Read Display Pixel Format
#define ILI9341_RDIMGFMT 0x0D   ///< Read Display Image Format
#define ILI9341_RDSELFDIAG 0x0F ///< Read Display Self-Diagnostic Result

#define ILI9341_INVOFF 0x20   ///< Display Inversion OFF
#define ILI9341_INVON 0x21    ///< Display Inversion ON
#define ILI9341_GAMMASET 0x26 ///< Gamma Set
#define ILI9341_DISPOFF 0x28  ///< Display OFF
#define ILI9341_DISPON 0x29   ///< Display ON

#define ILI9341_CASET 0x2A ///< Column Address Set
#define ILI9341_PASET 0x2B ///< Page Address Set
#define ILI9341_RAMWR 0x2C ///< Memory Write
#define ILI9341_RAMRD 0x2E ///< Memory Read

#define ILI9341_PTLAR 0x30    ///< Partial Area
#define ILI9341_VSCRDEF 0x33  ///< Vertical Scrolling Definition
#define ILI9341_MADCTL 0x36   ///< Memory Access Control
#define ILI9341_VSCRSADD 0x37 ///< Vertical Scrolling Start Address
#define ILI9341_PIXFMT 0x3A   ///< COLMOD: Pixel Format Set

#define ILI9341_FRMCTR1 0xB1 ///< Frame Rate Control (In Normal Mode/Full Colors)
#define ILI9341_FRMCTR2 0xB2 ///< Frame Rate Control (In Idle Mode/8 colors)
#define ILI9341_FRMCTR3 0xB3 ///< Frame Rate control (In Partial Mode/Full Colors)
#define ILI9341_INVCTR 0xB4  ///< Display Inversion Control
#define ILI9341_DFUNCTR 0xB6 ///< Display Function Control

#define ILI9341_PWCTR1 0xC0 ///< Power Control 1
#define ILI9341_PWCTR2 0xC1 ///< Power Control 2
#define ILI9341_PWCTR3 0xC2 ///< Power Control 3
#define ILI9341_PWCTR4 0xC3 ///< Power Control 4
#define ILI9341_PWCTR5 0xC4 ///< Power Control 5
#define ILI9341_VMCTR1 0xC5 ///< VCOM Control 1
#define ILI9341_VMCTR2 0xC7 ///< VCOM Control 2

#define ILI9341_RDID1 0xDA ///< Read ID 1
#define ILI9341_RDID2 0xDB ///< Read ID 2
#define ILI9341_RDID3 0xDC ///< Read ID 3
#define ILI9341_RDID4 0xDD ///< Read ID 4

#define ILI9341_GMCTRP1 0xE0 ///< Positive Gamma Correction
#define ILI9341_GMCTRN1 0xE1 ///< Negative Gamma Correction
//#define ILI9341_PWCTR6     0xFC

// Color definitions
#define ILI9341_BLACK 0x0000       ///<   0,   0,   0
#define ILI9341_NAVY 0x000F        ///<   0,   0, 123
#define ILI9341_DARKGREEN 0x03E0   ///<   0, 125,   0
#define ILI9341_DARKCYAN 0x03EF    ///<   0, 125, 123
#define ILI9341_MAROON 0x7800      ///< 123,   0,   0
#define ILI9341_PURPLE 0x780F      ///< 123,   0, 123
#define ILI9341_OLIVE 0x7BE0       ///< 123, 125,   0
#define ILI9341_LIGHTGREY 0xC618   ///< 198, 195, 198
#define ILI9341_DARKGREY 0x7BEF    ///< 123, 125, 123
#define ILI9341_BLUE 0x001F        ///<   0,   0, 255
#define ILI9341_GREEN 0x07E0       ///<   0, 255,   0
#define ILI9341_CYAN 0x07FF        ///<   0, 255, 255
#define ILI9341_RED 0xF800         ///< 255,   0,   0
#define ILI9341_MAGENTA 0xF81F     ///< 255,   0, 255
#define ILI9341_YELLOW 0xFFE0      ///< 255, 255,   0
#define ILI9341_WHITE 0xFFFF       ///< 255, 255, 255
#define ILI9341_ORANGE 0xFD20      ///< 255, 165,   0
#define ILI9341_GREENYELLOW 0xAFE5 ///< 173, 255,  41
#define ILI9341_PINK 0xFC18        ///< 255, 130, 198


#include <stdint.h>
#include "SPIxDriver.h"
#include "GPIOxDriver.h"


// ILI9341 Sctruc.
typedef struct
{
    SPI_Handler_t *spi_handler;		//El spi utilizado
    GPIO_Handler_t *cs_pin; 		//Entregarlos configurados como salida, push-pull, pull_down o nada.
    GPIO_Handler_t *dc_pin;		//Entregarlos configurados como salida, push-pull, pull_down o nada.
    GPIO_Handler_t *reset_pin;	//Entregarlos configurados como salida, push-pull, pull_down o nada.
} ILI9341_Handler_t;

// Funciones
void ILI9341_Init(ILI9341_Handler_t *ili9341_handler); //Para iniciar el funcionamiento
void ILI9341_Reset(ILI9341_Handler_t *ili9341_handler); //Para resetear a pantalla
void ILI9341_WriteCommand(ILI9341_Handler_t *ili9341_handler, uint8_t cmd); //Para mandar un comando
void ILI9341_WriteData(ILI9341_Handler_t *ili9341_handler, uint8_t *data, uint16_t len); //Para mandar datos, recibe un puntero y la cantidad de tdatos
void ILI9341_SetAddressWindow(ILI9341_Handler_t *ili9341_handler, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1); //Para ajustar a que región se mandaran los datos
void ILI9341_DrawPixel(ILI9341_Handler_t *ili9341_handler, uint16_t x, uint16_t y, uint16_t color); //Para cambiar un solo pixel de la pantalla
void ILI9341_FillRectangle(ILI9341_Handler_t *ili9341_handler, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color); //Para rellenar un cuadrado de un color en expecifico
void ILI9341_SendImage(ILI9341_Handler_t *ili9341_handler, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t *imagen); //Para mandar una imagen.
void ILI9341_Init_com(ILI9341_Handler_t *ili9341_handler);  //Funcion que manda los comandos necesarios para iniciar la pantalla.
void Ili_setRotation(ILI9341_Handler_t *ili9341_handler, uint8_t m); //Funcion que define la direccion de llenado de la pantalla.
void Ili_starCom(ILI9341_Handler_t *ili9341_handler); //Funcion que inicia la comunicación
void Ili_endCom(ILI9341_Handler_t *ili9341_handler); //Funcion que finaliza la comunicacón
void ILI9341_WriteCharacter(ILI9341_Handler_t *ili9341_handler, uint16_t x, uint16_t y, uint16_t color,uint16_t bg, char c, uint8_t size_x, uint8_t size_y);
void ILI9341_WriteString(ILI9341_Handler_t *ili9341_handler, uint16_t x, uint16_t y, uint16_t color, uint16_t bg, char *str, uint8_t size_x, uint8_t size_y) ;
#endif // ILI9341_H_
