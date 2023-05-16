/*
 * ILI9341.c
 *
 *  Created on: May 9, 2023
 *      Author: daniel
 */


#include "ILI9341.h"
#include <stddef.h>
#include "SysTick.h"

#define MADCTL_MY 0x80  ///< Bottom to top
#define MADCTL_MX 0x40  ///< Right to left
#define MADCTL_MV 0x20  ///< Reverse Mode
#define MADCTL_ML 0x10  ///< LCD refresh Bottom to top
#define MADCTL_RGB 0x00 ///< Red-Green-Blue pixel order
#define MADCTL_BGR 0x08 ///< Blue-Green-Red pixel order
#define MADCTL_MH 0x04  ///< LCD refresh right to left

// Send initialization commands
uint8_t initcmd[] = {
  0xEF, 3, 0x03, 0x80, 0x02,
  0xCF, 3, 0x00, 0xC1, 0x30,
  0xED, 4, 0x64, 0x03, 0x12, 0x81,
  0xE8, 3, 0x85, 0x00, 0x78,
  0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
  0xF7, 1, 0x20,
  0xEA, 2, 0x00, 0x00,
  ILI9341_PWCTR1  , 1, 0x23,
  ILI9341_PWCTR2  , 1, 0x10,
  ILI9341_VMCTR1  , 2, 0x3E, 0x28,
  ILI9341_VMCTR2  , 1, 0x86,
  ILI9341_MADCTL  , 1, 0x48,
  ILI9341_VSCRSADD, 1, 0x00,
  ILI9341_PIXFMT  , 1, 0x55,
  ILI9341_FRMCTR1 , 2, 0x00, 0x18,
  ILI9341_DFUNCTR , 3, 0x08, 0x82, 0x27,
  0xF2, 1, 0x00,
  ILI9341_GAMMASET , 1, 0x01,
  ILI9341_GMCTRP1 , 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
  ILI9341_GMCTRN1 , 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
  ILI9341_SLPOUT, 0x00,
  ILI9341_DISPON,
};

void ILI9341_Init(ILI9341_Handler_t *ili9341_handler)
{
    // Initialize SPI
    SPI_Config(ili9341_handler->spi_handler);
    SPI_WriteChar(ili9341_handler->spi_handler, 0x23);
    SPI_WriteChar(ili9341_handler->spi_handler, 0x11);

    // Initialize GPIO pins for CS, DC, and RESET


	//Initialize Reset

    //Ponemos el reset en su estado "natural" y CS en el estado donde no recibe los datos


	GPIO_WritePin((ili9341_handler->cs_pin), SET);



    // Send initialization commands to ILI9341
   ILI9341_Init_com(ili9341_handler);
}

void ILI9341_Reset(ILI9341_Handler_t *ili9341_handler)
{
    // Se realiza el toogle para que se entre en moodo reset.
    GPIOxTooglePin((ili9341_handler->reset_pin));
    delay_ms(150);
    GPIOxTooglePin((ili9341_handler->reset_pin));

}

void ILI9341_WriteCommand(ILI9341_Handler_t *ili9341_handler, uint8_t cmd)
{
    // Poner DC pin en bajo para recibir comandos
    GPIO_WritePin((ili9341_handler->dc_pin), RESET);

    // Enviar los comandos con el SPI
    SPI_WriteChar(ili9341_handler->spi_handler, cmd);



}

void ILI9341_WriteData(ILI9341_Handler_t *ili9341_handler,  uint8_t *data, uint16_t len)
{
    // Set DC pin high for data mode
    GPIO_WritePin((ili9341_handler->dc_pin), SET);


    // Send data via SPI
    SPI_Transmit(ili9341_handler->spi_handler, data, len);

}

void ILI9341_SetAddressWindow(ILI9341_Handler_t *ili9341_handler, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1){
    uint8_t data[4];


    ILI9341_WriteCommand(ili9341_handler, 0x2A); // Column Address Set
    data[0] = (x0 >> 8) & 0xFF;
    data[1] = x0 & 0xFF;
    data[2] = (x1 >> 8) & 0xFF;
    data[3] = x1 & 0xFF;
    ILI9341_WriteData(ili9341_handler, data, 4);

    ILI9341_WriteCommand(ili9341_handler, 0x2B); // Page Address Set
    data[0] = (y0 >> 8) & 0xFF;
    data[1] = y0 & 0xFF;
    data[2] = (y1 >> 8) & 0xFF;
    data[3] = y1 & 0xFF;
    ILI9341_WriteData(ili9341_handler, data, 4);

    ILI9341_WriteCommand(ili9341_handler, 0x2C); // Memory Write

}

void ILI9341_DrawPixel(ILI9341_Handler_t *ili9341_handler, uint16_t x, uint16_t y, uint16_t color)
{
    ILI9341_SetAddressWindow(ili9341_handler, x, y, x, y);

    uint8_t data[2] = {(color >> 8) & 0xFF, color & 0xFF};
    ILI9341_WriteData(ili9341_handler, data, 2);
}



void ILI9341_FillRectangle(ILI9341_Handler_t *ili9341_handler, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    // Set the address window on the ILI9341 display
	Ili_starCom(ili9341_handler);
    ILI9341_SetAddressWindow(ili9341_handler, x0, y0, x1, y1);

    // Convert the 16-bit color value into an array of two 8-bit values
    uint8_t data[2] = {(color >> 8) & 0xFF, color & 0xFF};

    // Calculate the size of the rectangle in pixels
    uint32_t size = (x1 - x0 + 1) * (y1 - y0 + 1);

    // Loop through each pixel in the rectangle
    for (uint32_t i = 0; i < size; i++)
    {
        // Send the color data for each pixel using the SPI transmit function
        ILI9341_WriteData(ili9341_handler, data, 2);
    }
    Ili_endCom(ili9341_handler);
}


void ILI9341_SendImage(ILI9341_Handler_t *ili9341_handler, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t *imagen)
{
    // Set the address window on the ILI9341 display
	Ili_starCom(ili9341_handler);
    ILI9341_SetAddressWindow(ili9341_handler, x0, y0, x1, y1);

    uint8_t size = sizeof(imagen);

    // Send the color data for each pixel using the SPI transmit function
    ILI9341_WriteData(ili9341_handler, imagen, size);

    Ili_endCom(ili9341_handler);
}

void ILI9341_Init_com(ILI9341_Handler_t *ili9341_handler)
{
    // Reset the display
    ILI9341_Reset(ili9341_handler);

    int i = 0;
    int final = sizeof(initcmd);

    while(i <= final){
    	uint8_t current_cmd = initcmd[i];
    	Ili_starCom(ili9341_handler);
    	ILI9341_WriteCommand(ili9341_handler, current_cmd);
    	i++;
    	uint8_t arg_count = initcmd[i];
    	while (arg_count--) {
    		i++;
			ILI9341_WriteData(ili9341_handler, initcmd+(i), 1);
		}
    	Ili_endCom(ili9341_handler);
    	// Some commands require a delay after execution
		if (current_cmd == ILI9341_SLPOUT || current_cmd == ILI9341_DISPON) {
			delay_ms(150);
		}
		i++;
    }

}

void Ili_setRotation(ILI9341_Handler_t *ili9341_handler, uint8_t m){
  uint8_t rotation = m % 4; // can't be higher than 3
  switch (rotation) {
  case 0:
    m = (MADCTL_MX | MADCTL_BGR);
    int _width = ILI9341_TFTWIDTH;
    (void) _width;
    int _height = ILI9341_TFTHEIGHT;
    (void) _height;
    break;
  case 1:
    m = (MADCTL_MV | MADCTL_BGR);
    _width = ILI9341_TFTHEIGHT;
    _height = ILI9341_TFTWIDTH;
    break;
  case 2:
    m = (MADCTL_MY | MADCTL_BGR);
    _width = ILI9341_TFTWIDTH;
    _height = ILI9341_TFTHEIGHT;
    break;
  case 3:
    m = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
    _width = ILI9341_TFTHEIGHT;
    _height = ILI9341_TFTWIDTH;
    break;
  }

  Ili_starCom(ili9341_handler);
  ILI9341_WriteCommand(ili9341_handler, ILI9341_MADCTL);
  ILI9341_WriteData(ili9341_handler, &m, 1);
  Ili_endCom(ili9341_handler);
}

void Ili_starCom(ILI9341_Handler_t *ili9341_handler){
	// Poner CS pin bajo para empezar la comunicación
    GPIO_WritePin((ili9341_handler->cs_pin), RESET);
}

void Ili_endCom(ILI9341_Handler_t *ili9341_handler){
	// Poner CS pin bajo para empezar la comunicación
    GPIO_WritePin((ili9341_handler->cs_pin), SET);

}


