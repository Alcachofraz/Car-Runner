/*
* @file		flash.h
* @brief	Contains the flash interface API.
* @version	1.0
* @date		Dec 2020
* @author	PedroG
*
* Copyright(C) 2020-2025, PedroG
* All rights reserved.
 */

#ifndef FLASH_H_
#define FLASH_H_

/** @defgroup FLASH FLASH
 * This package provides the core capabilities for flash output functions.
 * @{
 */

/** @defgroup FLASH_Public_Functions FLASH Public Functions
 * @{
 */

#include <string.h>


/*
 *
 *
 * Constants:
 *
 *
 */


/**
 * @brief	Flash's IAP routine location.
 */
#define FLASH_IAP_LOCATION 0x1FFF1FF1

/**
 * @brief	Flash's sector 29 start address.
 */
#define FLASH_START_ADDRESS_29 0x00078000

/**
 * @brief	Flash's sector 29 end address.
 */
#define FLASH_END_ADDRESS_29 0x0007FFFF

/**
 * @brief	Flash's Minimal write size.
 */
#define FLASH_MINIMAL_WRITE_SIZE 256

/**
 * @brief	Flash's Minimal compare size.
 */
#define FLASH_MINIMAL_COMPARE_SIZE 4

/**
 * @brief	Flash's IAP routine function.
 */
typedef void (*IAP)(unsigned int [],unsigned int[]);

/**
 * @brief	Flash related functions addresses.
 */
typedef enum {
	PREPARE_SECTORS = 50, /*!< Command address to prepare sector(s). */
	COPY_RAM_TO_FLASH = 51, /*!< Command address to copy RAM data to a flash memory address. */
	ERASE_SECTORS = 52, /*!< Command address to erase sector(s). */
	BLANK_CHECK_SECTORS = 53, /*!< Command address to blank check sector(s). */
	COMPARE_ADDRESSES = 56 /*!< Command address to compare addresses. */
} FLASH_COMMANDS;

/**
 * @brief	Status return of flash related functions.
 */
typedef enum {
	CMD_SUCCESS = 0x00, /*!< Command is executed successfully. */
	INVALID_COMMAND = 0x01, /*!< Invalid command. */
	SRC_ADDR_ERROR = 0x02, /*!< Source address is not on a word boundary */
	DST_ADDR_ERROR = 0x03, /*!< Destination address is not on a correct boundary. */
	SRC_ADDR_NOT_MAPPED = 0x04, /*!< Source address is not mapped in the memory map.  */
	DST_ADDR_NOT_MAPPED = 0x05, /*!< Destination address is not mapped in the memory map. */
	COUNT_ERROR = 0x06, /*!<Byte count is not multiple of 4 or is not a permitted value */
	INVALID_SECTOR = 0x07, /*!< Sector number is invalid. */
	SECTOR_NOT_BLANK = 0x08, /*!< Sector is not blank */
	SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION = 0x09, /*!< Command to prepare sector for write operation was not executed. */
	COMPARE_ERROR = 0x0A, /*!< Source and destination data is not same. */
	BUSY = 0x0B /*!< Flash programming hardware interface is busy. */
} FLASH_STATUS_CODES;


/*
 *
 *
 *
 * Functions:
 *
 *
 *
 */


/**
 * @brief	Erases all data from sector 'startSector' to 'endSector'.
 * @param   startSector: -> First sector.
 * @param   endSector: -> Last sector.
 * @return  CMD_SUCCESS: if sector was successfully erased.
 * @return  INVALID_SECTOR: if sector specified is not valid.
 * @note	To erase a single sector, the 2 parameters should hold the same value.
 */
unsigned int FLASH_EraseSectors(unsigned int startSector, unsigned int endSector);

/**
 * @brief	Write data in 'srcAddr' into 'dstAddr'.
 * @param   sector: -> Sector containing address 'dstAddr'.
 * @param   dstAddr: -> Destination address.
 * @param   srcAddr: -> Source address, where data to be written is.
 * @param   size: -> Number of bytes to be written. Should be 256, 512, 1024 or 4096.
 * @return  CMD_SUCCESS: if data was successfully written.
 * @return  INVALID_SECTOR: if sector specified is not valid.
 * @return  SRC_ADDR_ERROR: if source address is not a word boundary.
 * @return  DST_ADDR_ERROR: if destination address is not on correct boundary.
 * @return  SRC_ADDR_NOT_MAPPED: Source address is not mapped.
 * @return  DST_ADDR_NOT_MAPPED: Destination address is not mapped.
 * @return  COUNT_ERROR: Byte count is not 256, 512, 1024 or 4096.
 * @note	Address 'dstAddr' should be contained in sector 'sector'.
 */
unsigned int FLASH_WriteData(unsigned int sector, void *dstAddr, void *srcAddr, unsigned int size);

/**
 * @brief	Compares data from 2 addresses.
 * @param   dstAddr: -> First address of data.
 * @param   srcAddr: -> Second address of data.
 * @param   size: -> Byte count to be compared. Should be a multiple of 4.
 * @return  CMD_SUCCESS: if data coincides.
 * @return  COMPARE_ERROR: if data does not coincide.
 * @return  INVALID_SECTOR: if sector specified is not valid.
 * @return  COUNT_ERROR: if byte count is not a multiple of 4.
 * @return  ADDR_ERROR: if address is not on word boundary.
 * @return  ADDR_NOT_MAPPED: if address is not mapped.
 */
unsigned int FLASH_VerifyData(void *dstAddr, void *srcAddr, unsigned int size);


/**
 * @}
 */


/**
 * @}
 */

#endif /* FLASH_H_ */
