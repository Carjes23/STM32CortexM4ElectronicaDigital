/*
 * stm32f411xx_hal.h
 *
 *  Created on: Mar 2, 2023
 *      Author: dacardenasj
 *
 *  Este archivo contiene:
 *  -	valores de reloj principales
 *  -	Distribucion de la memoria (descrito en la figura
 *  	14 del datasheet del micro
 *  -	Posiciones de memoria de los periféricos tabla 1
 *  	(Memory Map)
 *  -	Los demas registros de los periferifcos
 *  -	definiciones de las constantes mas basicas.
 *
 *  Nota: La definicion del NVIC sera realizada al momento de
 *  describir el uso de las interrupciones
 */

#ifndef STM32F411XX_HAL_H_
#define STM32F411XX_HAL_H_

#include <stdio.h>
#include <stddef.h>

#define HSI_CLOCK_SPEED 16000000 //16Mhz
#define HSE_CLOCK_SPEED 4000000 //4Mhz

#define NOP()	asm("NOP")
#define __weak	__attribute__((weak))


/*
 * Base addresse of Flash and SRAM memories
 * Datasheet, memorymap, Figure 14
 * (remember, 1KByte = 1024 bytes
 */

#define FLASH_BASE_ADDR		0X08000000U //Esta es la memoria del programa, 512KB
#define SRAM_BASE_ADDR		0x20000000U //Esta es la memoria RAM, 128KB

/*
 * NOTA:: Obeservar que existen unos registros especificos del
 * cortex  M4 en la region 0xE000000U
 */

/*
 * AHBx and APBx BUs Peripherals base addresses.
 */
#define APB1_BASE_ADDR		0x40000000U
#define APB2_BASE_ADDR		0x40010000U
#define AHB1_BASE_ADDR		0x40020000U
#define AHB2_BASE_ADDR		0x50000000U

//Posiciones de memoria para periferiocos del AHB2
#define USB_OTG_FS_BASE_ADDR		(AHB2_BASE_ADDR + 0x000U)

//Posiciones de moemoria prerifericos AHB1
//Observar que NO esta completa
#define RCC_BASE_ADDR		(AHB1_BASE_ADDR + 0x3800U)
#define GPIOH_BASE_ADDR		(AHB1_BASE_ADDR + 0x1C00U)
#define GPIOE_BASE_ADDR		(AHB1_BASE_ADDR + 0x1000U)
#define GPIOD_BASE_ADDR		(AHB1_BASE_ADDR + 0x0C00U)
#define GPIOC_BASE_ADDR		(AHB1_BASE_ADDR + 0x0800U)
#define GPIOB_BASE_ADDR		(AHB1_BASE_ADDR + 0x0400U)
#define GPIOA_BASE_ADDR		(AHB1_BASE_ADDR + 0x0000U)

/*
 * MACROS GENERICOs
 */

#define ENABLE		1
#define DISABLE		0
#define SET		ENABLE
#define CLEAR		DISABLE
#define RESET		DISABLE
#define GPIO_PIN_SET		SET
#define GPIO_PI_RESET		RESET
#define FLAG_SET		SET
#define	FLAG_RESET		RESET
#define	I2C_WRITE		0
#define I2C_READ		1

/**
 * @param
 * a cada una de los registros que componen el
 * periferico RCC.
 */

typedef struct
{
	volatile uint32_t CR; //ADDR_OFFSET: 0X00
	volatile uint32_t PLLCFGR; //ADDR_OFSET: 0x04
	volatile uint32_t CFGR; //ADDR_OFSET: 0x08
	volatile uint32_t CIR; //ADDR_OFSET: anterior +4
	volatile uint32_t AHB1RSTR; //ADDR_OFSET: anterior +4
	volatile uint32_t AHB2RSTR; //ADDR_OFSET: anterior +4
	volatile uint32_t reserved0; //ADDR_OFSET: anterior +4
	volatile uint32_t reserved1; //ADDR_OFSET: anterior +4
	volatile uint32_t APB1RSTR; //ADDR_OFSET: anterior +4
	volatile uint32_t APB2RSTR; //ADDR_OFSET: anterior +4
	volatile uint32_t reserved2; //ADDR_OFSET: anterior +4
	volatile uint32_t reserved3; //ADDR_OFSET: anterior +4
	volatile uint32_t AHB1ENR; /** ADDR_OFSET: anterior +4 */
	volatile uint32_t AHB2ENR; //ADDR_OFSET: anterior +4
	volatile uint32_t reserved4; //ADDR_OFSET: anterior +4
	volatile uint32_t reserved5; //ADDR_OFSET: anterior +4
	volatile uint32_t APB1ENR; //ADDR_OFSET: anterior +4
	volatile uint32_t APB2ENR; //ADDR_OFSET: anterior +4
	volatile uint32_t reserved6; //ADDR_OFSET: anterior +4
	volatile uint32_t reserved7; //ADDR_OFSET: anterior +4
	volatile uint32_t AHB1LPENR; //ADDR_OFSET: anterior +4
	volatile uint32_t AHB2LPENR; //ADDR_OFSET: anterior +4
	volatile uint32_t reserved8; //ADDR_OFSET: anterior +4
	volatile uint32_t reserved9; //ADDR_OFSET: anterior +4
	volatile uint32_t APB1LPENR; //ADDR_OFSET: anterior +4
	volatile uint32_t APB2LPENR; //ADDR_OFSET: anterior +4
	volatile uint32_t reserved10; //ADDR_OFSET: anterior +4
	volatile uint32_t reserved11; //ADDR_OFSET: anterior +4
	volatile uint32_t BDCR; //ADDR_OFSET: anterior +4
	volatile uint32_t CSR; //ADDR_OFSET: anterior +4
	volatile uint32_t reserved12; //ADDR_OFSET: anterior +4
	volatile uint32_t reserved13; //ADDR_OFSET: anterior +4
	volatile uint32_t SSCGR; //ADDR_OFSET: anterior +4
	volatile uint32_t PLLI2SCFGR; //ADDR_OFSET: anterior +4
	volatile uint32_t reserved14; //ADDR_OFSET: anterior +4
	volatile uint32_t DCKCFGR; //ADDR_OFSET: anterior +4
} RCC_RegDef_t;

#define RCC		((RCC_RegDef_t *) RCC_BASE_ADDR)

//6.3.9 RCC AHB1ENR
#define RCC_AHB1ENR_GPIOA_EN		0
#define RCC_AHB1ENR_GPIOB_EN		1
#define RCC_AHB1ENR_GPIOC_EN		2
#define RCC_AHB1ENR_GPIOD_EN		3
#define RCC_AHB1ENR_GPIOE_EN		4
#define RCC_AHB1ENR_GPIOH_EN		7
#define RCC_AHB1ENR_CRCEN			12
#define RCC_AHB1ENR_DMA1_EN			21
#define RCC_AHB1ENR_DMA2_EN			22

typedef struct
{
	volatile uint32_t MODER; //ADDR_OFFSET: 0X00
	volatile uint32_t OTYPER; //ADDR_OFSET: anterior +4
	volatile uint32_t OSPEEDR; //ADDR_OFSET: anterior +4
	volatile uint32_t PUPDR; //ADDR_OFSET: anterior +4
	volatile uint32_t IDR; //ADDR_OFSET: anterior +4
	volatile uint32_t ODR; //ADDR_OFSET: anterior +4
	volatile uint32_t BSRR; //ADDR_OFSET: anterior +4
	volatile uint32_t LCKR; //ADDR_OFSET: anterior +4
	volatile uint32_t AFRL; //ADDR_OFSET: anterior +4
	volatile uint32_t AFRH; //ADDR_OFSET: anterior +4
} GPIOx_RegDef_t;

//Definicion periferios GPIOx
#define GPIOA		((GPIOx_RegDef_t *) GPIOA_BASE_ADDR)
#define GPIOB		((GPIOx_RegDef_t *) GPIOB_BASE_ADDR)
#define GPIOC		((GPIOx_RegDef_t *) GPIOC_BASE_ADDR)
#define GPIOD		((GPIOx_RegDef_t *) GPIOD_BASE_ADDR)
#define GPIOE		((GPIOx_RegDef_t *) GPIOE_BASE_ADDR)
#define GPIOH		((GPIOx_RegDef_t *) GPIOH_BASE_ADDR)

//Valores estandar para ls configuracioenes  configuración del pin
//8.4.1 GPIOx MODDER (dos bit por cada pin)
#define GPIO_MODE_IN			0
#define GPIO_MODE_OUT			1
#define GPIO_MODE_ALTFN			2
#define GPIO_MODE_ANALOG		3 //ver imagen pag 156 reference manual.

//8.4.2 GPIOx OTYPER (un bit por cada pin) solo cuando se usiliza como salida
#define GPIO_OTYPE_PUSHPULL			0 //saca 0 y 1
#define GPIO_OTYPE_OPENDRAIN		1 //valor de alta impedancia

//8.4.3 GPIOx OSPEEDR (dos bit por cada pin)
//Controla la velocidad de operacion
#define GPIO_OSPEED_LOW				0
#define GPIO_OSPEED_MEDIUM			1
#define GPIO_OSPEED_FAST			2
#define GPIO_OSPEED_HIGH			3

//8.4.3 GPIOx PUPDR (dos bit por cada pin)
//
#define GPIO_PUPDR_NOTHING			0
#define GPIO_PUPDR_PULLUP			1
#define GPIO_PUPDR_PULLDOWN			2
#define GPIO_PUPDR_RESERVED			3

//	Definicion de los nombre de los pines
#define PIN_0			0
#define PIN_1			1
#define PIN_2			2
#define PIN_3			3
#define PIN_4			4
#define PIN_5			5
#define PIN_6			6
#define PIN_7			7
#define PIN_8			8
#define PIN_9			9
#define PIN_10			10
#define PIN_11			11
#define PIN_12			12
#define PIN_13			13
#define PIN_14			14
#define PIN_15			15

//	Definicion de las funciones alternativas

#define AF0   		 0b0000
#define AF1   		 0b0001
#define AF2   		 0b0010
#define AF3   		 0b0011
#define AF4   		 0b0100
#define AF5   		 0b0101
#define AF6   		 0b0110
#define AF7   		 0b0111
#define AF8   		 0b1000
#define AF9   		 0b1001
#define AF10   		 0b1010
#define AF11   		 0b1011
#define AF12   		 0b1100
#define AF13   		 0b1101
#define AF14   		 0b1110
#define AF15   		 0b1111





#endif /* STM32F411XX_HAL_H_ */
