/*
 * flash.c
 *
 *  Created on: Dec 2020
 *      Author: PedroG
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif


#include <stdio.h>
#include "flash.h"


static unsigned int command[5];
static unsigned int output[5];
static IAP iap_entry =  (IAP) FLASH_IAP_LOCATION;

static unsigned int FLASH_PrepareSectors(unsigned int startSector, unsigned int endSector) {
	// Command parameters:
	command[0] = PREPARE_SECTORS;
	command[1] = startSector;
	command[2] = endSector;

	// Execute command:
	do {
		iap_entry(command, output);
	} while (output[0] == BUSY);
	return output[0];
}

unsigned int FLASH_EraseSectors(unsigned int startSector, unsigned int endSector) {
	// Verify sectors:
	if (endSector < startSector) return INVALID_SECTOR;

	// Prepare sectors:
	if (FLASH_PrepareSectors(startSector, endSector) == INVALID_SECTOR) return INVALID_SECTOR;

	// Command parameters:
	command[0] = ERASE_SECTORS;
	command[1] = startSector;
	command[2] = endSector;
	command[3] = SystemCoreClock/1000;

	// Execute command:
	do {
		iap_entry(command, output);
	} while (output[0] == BUSY);
	return output[0];
}

unsigned int FLASH_WriteData(unsigned int sector, void *dstAddr, void *srcAddr, unsigned int size) {
	// Prepare sectors:
	if (FLASH_PrepareSectors(sector, sector) == INVALID_SECTOR) return INVALID_SECTOR;

	// Command parameters:
	command[0] = COPY_RAM_TO_FLASH;
	command[1] = (unsigned int) dstAddr;
	command[2] = (unsigned int) srcAddr;
	command[3] = size;
	command[4] = SystemCoreClock/1000;

	// Execute command:
	do {
		iap_entry(command, output);
	} while (output[0] == BUSY);
	return output[0];
}

unsigned int FLASH_VerifyData(void *dstAddr, void *srcAddr, unsigned int size) {
	// Command parameters:
	command[0] = COMPARE_ADDRESSES;
	command[1] = (unsigned int) dstAddr;
	command[2] = (unsigned int) srcAddr;
	command[3] = size;

	// Execute command:
	do {
		iap_entry(command, output);
	} while (output[0] == BUSY);
	return output[0];
}
