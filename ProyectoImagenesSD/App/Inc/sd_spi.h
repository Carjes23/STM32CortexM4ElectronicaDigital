/*-----------------------------------------------------------------------/
/  Low level disk interface modlue include file   (C)ChaN, 2013          /
/-----------------------------------------------------------------------*/

#ifndef _DISKIO_DEFINED_SD
#define _DISKIO_DEFINED_SD

#define _USE_WRITE	1	/* 1: Enable disk_write function */
#define _USE_IOCTL	1	/* 1: Enable disk_ioctl fucntion */

#include "diskio.h"

#include "stm32f4xx.h"

#include "SpiDriver.h"
#include "SysTick.h"

/* Comandos MMC/SD */
#define CMD0	(0)			/* GO_IDLE_STATE */
#define CMD1	(1)			/* SEND_OP_COND (MMC) */
#define	ACMD41	(0x80+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(8)			/* SEND_IF_COND */
#define CMD9	(9)			/* SEND_CSD */
#define CMD10	(10)		/* SEND_CID */
#define CMD12	(12)		/* STOP_TRANSMISSION */
#define ACMD13	(0x80+13)	/* SD_STATUS (SDC) */
#define CMD16	(16)		/* SET_BLOCKLEN */
#define CMD17	(17)		/* READ_SINGLE_BLOCK */
#define CMD18	(18)		/* READ_MULTIPLE_BLOCK */
#define CMD23	(23)		/* SET_BLOCK_COUNT (MMC) */
#define	ACMD23	(0x80+23)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24	(24)		/* WRITE_BLOCK */
#define CMD25	(25)		/* WRITE_MULTIPLE_BLOCK */
#define CMD32	(32)		/* ERASE_ER_BLK_START */
#define CMD33	(33)		/* ERASE_ER_BLK_END */
#define CMD38	(38)		/* ERASE */
#define CMD55	(55)		/* APP_CMD */
#define CMD58	(58)		/* READ_OCR */

/* Headers */
void spi_sdInit(void);
DSTATUS fatfs_SD_diskInitialize(void);
DSTATUS fatfs_SD_diskStatus(void);
DRESULT fatfs_SD_diskRead(BYTE *buff, DWORD sector, UINT count);
DRESULT fatfs_SD_diskWrite(const BYTE *buff, DWORD sector, UINT count);
DRESULT fatfs_SD_diskIoctl(BYTE cmd, void *buff);

/* 0  -> Sin pin de protección de escritura
 * 1+ -> Con pin de protección de escritura
 */
#define FATFS_USE_WRITEPROTECT_PIN	0

/* 0  -> Sin pin de detección
 * 1+ -> Con pin de detección
 */
#define FATFS_USE_DETECT_PIN		0

#endif

