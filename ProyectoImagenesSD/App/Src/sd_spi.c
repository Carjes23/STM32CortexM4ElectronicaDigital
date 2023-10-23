/**
 *************************************************************************
 * @file           : sd_spi.c
 * @author         : emangelma
 * @brief          : Driver para microSD con SPI
 *************************************************************************
 *
 *************************************************************************
 */
/*-----------------------------------------------------------------------*/
/* SPI controls (Platform dependent)                                     */
/*-----------------------------------------------------------------------*/

#include "sd_spi.h"

/* Handlers para los periféricos */
GPIO_Handler_t	handlerSpiSCLK	= {0};	// PB13
GPIO_Handler_t	handlerSpiMISO	= {0};	// PB14
GPIO_Handler_t	handlerSpiMOSI	= {0};	// PB15
GPIO_Handler_t	handlerSpiSS	= {0};	// PB1

SPI_Handler_t2	handlerSpiPort	= {0};	// SPI2

/* Estado físico de la unidad */
static volatile DSTATUS FATFS_SD_Status = STA_NOINIT;

/* Para el manejo de banderas de la tarjeta */
static BYTE FATFS_SD_CardType;

/*-----------------------------------------------------------------------*/
/* Inicialización de la interfaz MMC 									 */
/*-----------------------------------------------------------------------*/
void spi_sdInit(void) {

	/* Se inicializa el Systick (en ms) */
	config_SysTick();
	
	/* --- Configuración e inicialización del SPI2 --- */
	/* GPIO para el SCLK */
	handlerSpiSCLK.pGPIOx	 							= GPIOB;
	handlerSpiSCLK.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_13;
	handlerSpiSCLK.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_ALTFN;
	handlerSpiSCLK.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerSpiSCLK.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerSpiSCLK.GPIO_PinConfig_t.GPIO_PinSpeed			= GPIO_OSPEED_HIGH;
	handlerSpiSCLK.GPIO_PinConfig_t.GPIO_PinAltFunMode	= AF5;

	GPIO_Config(&handlerSpiSCLK);

	/* GPIO para el MISO /SDO */
	handlerSpiMISO.pGPIOx	 							= GPIOB;
	handlerSpiMISO.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_14;
	handlerSpiMISO.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_ALTFN;
	handlerSpiMISO.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerSpiMISO.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerSpiMISO.GPIO_PinConfig_t.GPIO_PinSpeed			= GPIO_OSPEED_HIGH;
	handlerSpiMISO.GPIO_PinConfig_t.GPIO_PinAltFunMode	= AF5;

	GPIO_Config(&handlerSpiMISO);

	/* GPIO para el MOSI /SDI */
	handlerSpiMOSI.pGPIOx	 							= GPIOB;
	handlerSpiMOSI.GPIO_PinConfig_t.GPIO_PinNumber		= PIN_15;
	handlerSpiMOSI.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_ALTFN;
	handlerSpiMOSI.GPIO_PinConfig_t.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerSpiMOSI.GPIO_PinConfig_t.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerSpiMOSI.GPIO_PinConfig_t.GPIO_PinSpeed			= GPIO_OSPEED_HIGH;
	handlerSpiMOSI.GPIO_PinConfig_t.GPIO_PinAltFunMode	= AF5;

	GPIO_Config(&handlerSpiMOSI);

	/* GPIO para el SS /CS */
	handlerSpiSS.pGPIOx	 								= GPIOB;
	handlerSpiSS.GPIO_PinConfig_t.GPIO_PinNumber			= PIN_1;
	handlerSpiSS.GPIO_PinConfig_t.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerSpiSS.GPIO_PinConfig_t.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
	handlerSpiSS.GPIO_PinConfig_t.GPIO_PinPuPdControl		= GPIO_PUPDR_PULLUP;
	handlerSpiSS.GPIO_PinConfig_t.GPIO_PinSpeed			= GPIO_OSPEED_LOW;
	handlerSpiSS.GPIO_PinConfig_t.GPIO_PinAltFunMode		= AF0;

	GPIO_Config(&handlerSpiSS);

	/* Periférico SPI 2 */
	handlerSpiPort.ptrSPIx 								= SPI2;
	handlerSpiPort.SPI_Config.SPI_mode					= SPI_MODE_0;			 // CPOL = 0, CPHA = 0
	handlerSpiPort.SPI_Config.SPI_fullDupplexEnable		= SPI_FULL_DUPPLEX;
	handlerSpiPort.SPI_Config.SPI_datasize				= SPI_DATASIZE_8_BIT;
	handlerSpiPort.SPI_Config.SPI_baudrate				= SPI_BAUDRATE_FPCLK_2; // ~390kbs
	handlerSpiPort.SPI_slavePin 						= handlerSpiSS;

	spi_config(handlerSpiPort);

	// Se pone en alto el pin del esclavo
	GPIO_WritePin(&handlerSpiSS, SET);
	
	/* Delay mientras se estabiliza */
	delay_ms(10);
}

/*-----------------------------------------------------------------------*/
/* Recibir múltiples bytes 												 */
/*-----------------------------------------------------------------------*/
static void rcvr_spi_multi (
	BYTE *buff,		/* Puntero al data buffer */
	UINT btr		/* Número de bytes a recibir (número par) */
)
{
	/* Lectura de múltiples bytes */
	spi_receive(handlerSpiPort, buff, btr);
}

#if _FS_READONLY == 0
/*-----------------------------------------------------------------------*/
/* Enviar múltiples bytes  	                                             */
/*-----------------------------------------------------------------------*/
static void xmit_spi_multi (
	const BYTE *buff,	/* Puntero a los datos */
	UINT btx			/* Número de bytes a enviar (número par) */
)
{
	/* Se escriben múltiples bytes */
	spi_transmit(handlerSpiPort, (uint8_t *)buff, btx);
}
#endif

/*-----------------------------------------------------------------------*/
/* Esperar a que la tarjeta esté lista	                                 */
/*-----------------------------------------------------------------------*/
static int wait_ready (	/* 1:Ready, 0:Timeout */
	UINT wt				/* Timeout [ms] */
)
{
	BYTE d;

	/* Conteo descendente */
	SYSTICK_setTicksDown(wt);
	
	do {
		d = spi_transmitByte(handlerSpiPort, 0xFF);

	/* Se espera a que la tarjeta esté lista o se supere el timeout */
	}while (d != 0xFF && SYSTICK_ticksDown());
	if (d == 0xFF) {
		// TODO --> printf("wait_ready: OK");
	} else {
		// TODO --> printf("wait_ready: timeout");
	}
	return (d == 0xFF) ? 1 : 0;
}


/*-----------------------------------------------------------------------*/
/* Deseleccionar tarjeta y liberar el SPI                                */
/*-----------------------------------------------------------------------*/
static void deselect (void)
{

	// CS High
	spi_unSelectSlave(&handlerSpiPort);

	/* Dummy clock (forzar DO hi-z para múltiples esclavos SPI */
	spi_transmitByte(handlerSpiPort, 0xFF);
	// TODO --> printf("deselect: ok");
}


/*-----------------------------------------------------------------------*/
/* Seleccionar la tarjeta y esperar que esté lista                       */
/*-----------------------------------------------------------------------*/
static int select (void)	/* 1:OK, 0:Timeout */
{
	// CS Low
	spi_selectSlave(&handlerSpiPort);

	/* Dummy clock (forzar DO habilitado) */
	spi_transmitByte(handlerSpiPort, 0xFF);

	// Se espera 500 ms
	if (wait_ready(500)) {
		// TODO --> printf("select: OK");
		return 1;	/* OK */
	}
	// TODO --> printf("select: no");

	deselect();
	return 0;	/* Timeout */
}


/*-----------------------------------------------------------------------*/
/* Recibir un paquete de datos del MMC 		 	                         */
/*-----------------------------------------------------------------------*/
static int rcvr_datablock (	/* 1:OK, 0:Error */
	BYTE *buff,			/* Data buffer */
	UINT btr			/* Tamaño del bloque de datos (byte) */
)
{
	BYTE token;
	
	// Se inicializa un contador descendente de 200 ms (timeout)
	SYSTICK_setTicksDown(200);

	do {// Esperar por el token de inicio de datos "DataStart" en un tiempo de espera de 200ms.
		token = spi_transmitByte(handlerSpiPort, 0xFF);

	// Este bucle tomará un tiempo. Insertar "rot_rdq()" aquí para un entorno multitarea.
	} while ((token == 0xFF) && SYSTICK_ticksDown());
	if (token != 0xFE) {

		// TODO --> printf("rcvr_datablock: token != 0xFE");

		// La función falla si el token DataStart es inválido o si se supera el timeout.
		return 0;
	}

	// Almacena los datos restantes en el buffer.
	rcvr_spi_multi(buff, btr);

	// Descartar el CRC
	spi_transmitByte(handlerSpiPort, 0xFF);
	spi_transmitByte(handlerSpiPort, 0xFF);

	// La función tuvo éxito
	return 1;
}


/*-----------------------------------------------------------------------*/
/* Enviar un paquete de datos al MMC	                                 */
/*-----------------------------------------------------------------------*/
#if _FS_READONLY == 0
static int xmit_datablock (	/* 1:OK, 0:Failed */
	const BYTE *buff,	/* Puntero a los 512 bytes a enviar */
	BYTE token			/* Token */
)
{
	BYTE resp;
	
	// TODO --> printf("xmit_datablock: inside");

	if (!wait_ready(500)) {

		// TODO --> printf("xmit_datablock: not ready");
		return 0;	/* Esperar a que la tarjeta esté lista */
	}

	// TODO --> printf("xmit_datablock: ready");

	/* Enviar token */
	spi_transmitByte(handlerSpiPort, token);

	/* Enviar datos si el token es diferente a StopTran */
	if (token != 0xFD) {

		/* Datos */
		xmit_spi_multi(buff, 512);

		/* Dummy CRC */
		spi_transmitByte(handlerSpiPort, 0xFF);
		spi_transmitByte(handlerSpiPort, 0xFF);

		/* Recibir los datos resp */
		resp = spi_transmitByte(handlerSpiPort, 0xFF);

		/* La función falla si le paquete de datos no fue aceptado */
		if ((resp & 0x1F) != 0x05) return 0;
	}
	return 1;
}
#endif


/*-----------------------------------------------------------------------*/
/* Enviar un paquete de comando al MMC 			                         */
/*-----------------------------------------------------------------------*/
static BYTE send_cmd (		/* Return value: R1 resp (bit 7 == 1 : Falló el envío) */
	BYTE cmd,				/* Índice del comando */
	DWORD arg				/* Argumento */
)
{
	BYTE n, res;
	
	/* Enviar un comando CMD55 antes de ACMD<n> */
	if (cmd & 0x80) {
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);
		if (res > 1) return res;
	}

	/* Selecciona la tarjeta y espera a que esté lista, excepto
	 * para detener la lectura de múltiples bloques. */
	if (cmd != CMD12) {
		deselect();
		if (!select()) return 0xFF;
	}

	/* Enviar paquete de comando */
	spi_transmitByte(handlerSpiPort, 0x40 | cmd);				/* Start + índice de comando */
	spi_transmitByte(handlerSpiPort, (BYTE)(arg >> 24));		/* Argumento[31..24] */
	spi_transmitByte(handlerSpiPort, (BYTE)(arg >> 16));		/* Argumento[23..16] */
	spi_transmitByte(handlerSpiPort, (BYTE)(arg >> 8));			/* Argumento[15..8] */
	spi_transmitByte(handlerSpiPort, (BYTE)arg);				/* Argumento[7..0] */
	n = 0x01;													/* Dummy CRC + Stop */
	if (cmd == CMD0) n = 0x95;									/* CRC válido para CMD0(0) */
	if (cmd == CMD8) n = 0x87;									/* CRC válido para CMD8(0x1AA) */
	spi_transmitByte(handlerSpiPort, n);

	/* Recibir comando resp */
	if (cmd == CMD12) {
		spi_transmitByte(handlerSpiPort, 0xFF);					/* Descartar el siguiente byte cuando se usa CMD12 */
	}
	
	n = 10;													/* Esperar la respuesta (10 bytes max) */
	do {
		res = spi_transmitByte(handlerSpiPort, 0xFF);
	} while ((res & 0x80) && --n);

	return res;												/* Retornar la respuesta recibida */
}

// Esto es para los pines adicionales y opcionales (detección SD y bloqueo de escritura)
uint8_t fatfs_detect(void) {
#if FATFS_USE_DETECT_PIN > 0
	return !TM_GPIO_GetInputPinValue(FATFS_USE_DETECT_PIN_PORT, FATFS_USE_DETECT_PIN_PIN);
#else
	return 1;
#endif
}

uint8_t fatfs_writeEnabled(void) {
#if FATFS_USE_WRITEPROTECT_PIN > 0
	return // TODO --> !TM_GPIO_GetInputPinValue(FATFS_USE_WRITEPROTECT_PIN_PORT, FATFS_USE_WRITEPROTECT_PIN_PIN);
#else
	return 1;
#endif
}

/*-----------------------------------------------------------------------*/
/* Inicialización del disco (Acá puede ser USB, SD, disco duro, etc)     */
/*-----------------------------------------------------------------------*/
DSTATUS fatfs_SD_diskInitialize(void) {
	BYTE n, cmd, ty, ocr[4];
	
	// Inicialización SPI de la SD
	spi_sdInit();
	
	if(FATFS_SD_Status & STA_NODISK){
		return FATFS_SD_Status;
	}

	if (!fatfs_detect()){
		return STA_NODISK;
	}

	// TODO --> ¿poner la velocidad spi lenta? escaler de 64
	// TODO --> El modo rápido es de 2

	// Se pone la velocidad en 390.6 kbs
	spi_setBaudrate(handlerSpiPort, (uint8_t)SPI_BAUDRATE_FPCLK_128);

	// Enviar 80 ciclos dummy de reloj
	for (n = 10; n; n--){
		spi_transmitByte(handlerSpiPort, 0xFF);
	}

	ty = 0;

	/* Se pone la tarjeta en estado SPI o "idle" */
	if (send_cmd(CMD0, 0) == 1) {

		/* Timeout de inicialización = 1 s */
		SYSTICK_setTicksDown(1000);
		if (send_cmd(CMD8, 0x1AA) == 1) {		/* SDv2? */
			for (n = 0; n < 4; n++) {

				/* Obtener el valor de retorno de 32 bits de la resp R7 */
				ocr[n] = (BYTE)spi_transmitByte(handlerSpiPort, 0xFF);
			}

			/* Revisar si la tarjeta soporta VCC de 2.7-3.6V */
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) {

				/* Esperar el final de la inicialización con ACMD41(HCS) */
				while (SYSTICK_ticksDown() && send_cmd(ACMD41, 1UL << 30)) ;

				/* Verificar el bit CCS en el OCR */
				if (SYSTICK_ticksDown() && send_cmd(CMD58, 0) == 0) {
					for (n = 0; n < 4; n++) {
						ocr[n] = (BYTE)spi_transmitByte(handlerSpiPort, 0xFF);
					}

					/* La tarjeta es SDv2 */
					ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
				}
			}

		/* La tarjeta no es SDv2 */
		} else {

			/* Revisar si es SDv1 o MMC */
			if (send_cmd(ACMD41, 0) <= 1){

				/* SDv1 (ACMD41(0)) */
				ty = CT_SD1; cmd = ACMD41;
			} else {

				/* MMCv3 (CMD1(0)) */
				ty = CT_MMC; cmd = CMD1;
			}

			/* Esperar que termine la inicialización */
			while (SYSTICK_ticksDown() && send_cmd(cmd, 0));

			/* Establecer el tamaño de bloque como: 512 */
			if (SYSTICK_ticksDown() || send_cmd(CMD16, 512) != 0) {	// TODO --> ¿Negar o no negar?
				ty = 0;
			}
		}
	}

	/* Tipo de tarjeta */
	FATFS_SD_CardType = ty;
	deselect();

	if (ty) {							/* OK */

		// TODO --> Acá se pone la velocidad rápida
		// Se pone la velocidad en ~ 6.2Mbs (SPI 2 -> Max 50 MHz)
		spi_setBaudrate(handlerSpiPort, (uint8_t)SPI_BAUDRATE_FPCLK_8);

		FATFS_SD_Status &= ~STA_NOINIT;	/* Limpiar la bandera STA_NOINIT */

	} else {							/* Failed */
		FATFS_SD_Status = STA_NOINIT;
	}

	// Revisar si está activo el pin que bloquea la escritura
	if (!fatfs_writeEnabled()) {
		FATFS_SD_Status |= STA_PROTECT;
	} else {
		FATFS_SD_Status &= ~STA_PROTECT;
	}
	
	return FATFS_SD_Status;
}


/*-----------------------------------------------------------------------*/
/* Obtener estado del disco                                              */
/*-----------------------------------------------------------------------*/
DSTATUS fatfs_SD_diskStatus(void) {
	
	/* Revisar si el pin de detección está habilitado */
	if (!fatfs_detect()) {
		return STA_NOINIT;
	}
	
	/* Revisar si el pin de bloqueo de escritura está habilitado */
	if (!fatfs_writeEnabled()) {
		FATFS_SD_Status |= STA_PROTECT;
	} else {
		FATFS_SD_Status &= ~STA_PROTECT;
	}
	
	return FATFS_SD_Status;	/* Retornar el disk status */
}


/*-----------------------------------------------------------------------*/
/* Leer sector(es) del disco                                             */
/*-----------------------------------------------------------------------*/
DRESULT fatfs_SD_diskRead(
	BYTE *buff,		/* Data buffer para almacenar los datos leídos */
	LBA_t sector,	/* Dirección del sector (LBA) */
	UINT count		/* Número de sectores a leer (1..128) */
)
{

	DWORD sect = (DWORD)sector;

	// TODO --> printf("disk_read: inside");
	if (!fatfs_detect() || (FATFS_SD_Status & STA_NOINIT)) {
		return RES_NOTRDY;
	}

	if (!(FATFS_SD_CardType & CT_BLOCK)) {
		sect *= 512;		/* LBA ot BA conversion (byte addressing cards) */
	}

	if (count == 1) {		/* Leer un solo sector */
		if ((send_cmd(CMD17, sect) == 0)	/* READ_SINGLE_BLOCK */
			&& rcvr_datablock(buff, 512))
			count = 0;
	}
	else {					/* Leer múltiples sectores */
		if (send_cmd(CMD18, sect) == 0) {	/* READ_MULTIPLE_BLOCK */
			do {
				if (!rcvr_datablock(buff, 512)) {
					break;
				}
				buff += 512;
			} while (--count);
			send_cmd(CMD12, 0);				/* STOP_TRANSMISSION */
		}
	}
	deselect();

	return count ? RES_ERROR : RES_OK;		/* Return result */
}


/*-----------------------------------------------------------------------*/
/* Escribir sector(es) en el disco                                       */
/*-----------------------------------------------------------------------*/
#if FF_FS_READONLY == 0
DRESULT fatfs_SD_diskWrite(
	const BYTE *buff,	/* Información a ser escrita */
	LBA_t sector,		/* Dirección del sector (LBA) */
	UINT count			/* Número de sectores a escribir (1..128) */
)
{
	DWORD sect = (DWORD)sector;

	// TODO --> printf("disk_write: inside");
	if (!fatfs_detect()) {
		return RES_ERROR;
	}
	if (!fatfs_writeEnabled()) {
		// TODO --> printf("disk_write: Write protected!!! \n---------------------------------------------");
		return RES_WRPRT;
	}
	if (FATFS_SD_Status & STA_NOINIT) {
		return RES_NOTRDY;	/* Revisar el estado de la unidad */
	}
	if (FATFS_SD_Status & STA_PROTECT) {
		return RES_WRPRT;	/* Revisar protección de escritura */
	}

	if (!(FATFS_SD_CardType & CT_BLOCK)) {
		sect *= 512;		/* Conversión LBA ==> BA (byte addressing cards) */
	}

	if (count == 1) {		/* Escritura de un solo sector */
		if ((send_cmd(CMD24, sect) == 0)	/* WRITE_BLOCK */
			&& xmit_datablock(buff, 0xFE)){
			count = 0;
		}
	}
	else {					/* Escritura de múltiples sectores */
		if (FATFS_SD_CardType & CT_SDC) send_cmd(ACMD23, count);	/* Predefinir número de sectores */
		if (send_cmd(CMD25, sect) == 0) {	/* WRITE_MULTIPLE_BLOCK */
			do {
				if (!xmit_datablock(buff, 0xFC)) {
					break;
				}
				buff += 512;
			} while (--count);
			if (!xmit_datablock(0, 0xFD)) {	/* STOP_TRAN token */
				count = 1;
			}
		}
	}
	deselect();

	return count ? RES_ERROR : RES_OK;	/* Return result */
}
#endif


/*-----------------------------------------------------------------------*/
/* Función adicional 	                                                 */
/*-----------------------------------------------------------------------*/
#if _USE_IOCTL
DRESULT fatfs_SD_diskIoctl(
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	BYTE n, csd[16];
	DWORD st, ed, csize;
	LBA_t *dp;

	if (FATFS_SD_Status & STA_NOINIT) {
		return RES_NOTRDY;	/* Check if drive is ready */
	}
	if (!fatfs_detect()) {
		return RES_NOTRDY;
	}

	res = RES_ERROR;

	switch (cmd) {
	case CTRL_SYNC :		/* Wait for end of internal write process of the drive */
		if (select()) res = RES_OK;
		break;

	case GET_SECTOR_COUNT :	/* Get drive capacity in unit of sector (DWORD) */
		if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {
			if ((csd[0] >> 6) == 1) {	/* SDC ver 2.00 */
				csize = csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1;
				*(LBA_t*)buff = csize << 10;
			} else {					/* SDC ver 1.XX or MMC ver 3 */
				n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
				csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
				*(DWORD*)buff = csize << (n - 9);
			}
			res = RES_OK;
		}
		break;

	case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
		if (FATFS_SD_CardType & CT_SD2) {	/* SDC ver 2.00 */
			if (send_cmd(ACMD13, 0) == 0) {	/* Read SD status */
				spi_transmitByte(handlerSpiPort, 0xFF);
				if (rcvr_datablock(csd, 16)) {				/* Read partial block */
					for (n = 64 - 16; n; n--) spi_transmitByte(handlerSpiPort, 0xFF);	/* Purge trailing data */
					*(DWORD*)buff = 16UL << (csd[10] >> 4);
					res = RES_OK;
				}
			}
		} else {					/* SDC ver 1.XX or MMC */
			if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {	/* Read CSD */
				if (FATFS_SD_CardType & CT_SD1) {	/* SDC ver 1.XX */
					*(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
				} else {					/* MMC */
					*(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
				}
				res = RES_OK;
			}
		}
		break;

	case CTRL_TRIM :	/* Erase a block of sectors (used when _USE_ERASE == 1) */
		if (!(FATFS_SD_CardType & CT_SDC)) break;				/* Check if the card is SDC */
		if (fatfs_SD_diskIoctl(MMC_GET_CSD, csd)) break;	/* Get CSD */
		if (!(csd[0] >> 6) && !(csd[10] & 0x40)) break;		/* Check if sector erase can be applied to the card */
		dp = buff; st = (DWORD)dp[0]; ed = (DWORD)dp[1];					/* Load sector block */
		if (!(FATFS_SD_CardType & CT_BLOCK)) {
			st *= 512; ed *= 512;
		}
		if (send_cmd(CMD32, st) == 0 && send_cmd(CMD33, ed) == 0 && send_cmd(CMD38, 0) == 0 && wait_ready(30000))	/* Erase sector block */
			res = RES_OK;	/* FatFs does not check result of this command */
		break;

	default:
		res = RES_PARERR;
	}

	deselect();

	return res;
}
#endif

