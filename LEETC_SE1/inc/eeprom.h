/*
 * eeprom.h
 *
 *  Created on: 30 Mar 2021
 *      Author: pedro
 */

#ifndef EEPROM_H_
#define EEPROM_H_

/** @defgroup EEPROM EEPROM
 * This package provides the core capabilities for eeprom functions.
 * @{
 */

/** @defgroup EEPROM_Public_Functions EEPROM Public Functions
 * @{
 */

#include "i2c.h"


/*
 *
 *
 * Constants:
 *
 *
 */

/**
 * @brief	 Max length (in bytes) of a single page to write to EEPROM.
 */
#define EEPROM_PAGE_LENGTH 32

/**
 * @brief 	EEPROM address.
 */
#define EEPROM_ADDRESS 0x50

/**
 * @brief	 EEPROM operating frequency.
 */
#define EEPROM_FREQUENCY 400000


/*
 *
 *
 * Functions:
 *
 *
 */

/*
 * @brief 	Initialise EEPROM driver.
 */
void EEPROM_Init();

/*
 * @brief 	Read size bytes to given buffer from EEPROM.
 * @param 	buffer: -> Pointer to buffer of bytes, where data is to be read to.
 * @param	size: -> Number of bytes to read.
 * @return 	0 if successful, -1 otherwise.
 * @note	This function is blocking.
 */
int EEPROM_Read(char * buffer, int size);

/*
 * @brief 	Write size bytes to EEPROM from buffer.
 * @param 	buffer: -> Pointer to buffer of bytes, where data is to be written from.
 * @param	size: -> Number of bytes to write.
 * @return	0 if successful, -1 otherwise.
 * @note	This function is blocking.
 */
int EEPROM_Write(char * buffer, int size);


/**
 * @}
 */


/**
 * @}
 */

#endif /* EEPROM_H_ */
