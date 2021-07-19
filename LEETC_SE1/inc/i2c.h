/**
* @file		i2c.h
* @brief	Contains the I2C API.
* @version	1.0
* @date		Mar 2021
* @author	PedroG
*
* Copyright(C) 2020-2025, PedroG
* All rights reserved.
*/

#ifndef I2C_H_
#define I2C_H_

/** @defgroup I2C I2C
 * This package provides the core capabilities for i2c functions.
 * @{
 */

/** @defgroup I2C_Public_Functions SPI Public Functions
 * @{
 */


#include <string.h>
#include <stdlib.h>
#include "wait.h"


/*
 *
 *
 * Constants:
 *
 *
 */


/**
 * @brief	I2C's maximum time (milliseconds) the device driver will wait before timing out.
 */
#define I2C_TIMEOUT_MS 100

/**
 * @brief	I2C's maximum buffer length for writing and reading.
 */
#define I2C_BUFFER_LENGTH 1024

/**
 * @brief	I2C's interface power/clock control bit.
 */
typedef enum {
	I2C0_PCONP_ENABLE = (1 << 7), /*!< Power/clock control bit for I2C0. */
	I2C1_PCONP_ENABLE = (1 << 19), /*!< Power/clock control bit for I2C1. */
	I2C2_PCONP_ENABLE = (1 << 26) /*!< Power/clock control bit for I2C2. */
} I2C_PCONP_ENABLE;

/**
 * @brief	I2C's PINs.
 */
typedef enum {
	SDA1 = 0, /*!< P0[0] -> Pin 9. */
	SCL1 = 1, /*!< P0[1] -> Pin 10. */
	SDA2 = 10, /*!< P0[10] -> Pin 40. */
	SCL2 = 11 /*!< P0[11] -> Pin 41. */
} I2C_PINS;

/**
 * @brief	I2C's pin modes.
 */
typedef enum {
	I2C1_PINSEL = 0x03, /*!< Pin mode for I2C1. */
	I2C2_PINSEL = 0x02 /*!< Pin mode for I2C2. */
} I2C_PINSEL;

/**
 * @brief	I2C's control register bits.
 */
typedef enum {
	AA = (1 << 2), /*!< Assert Acknowledge bit. */
	SI = (1 << 3), /*!< Interrupt bit. */
	STO = (1 << 4), /*!< Stop bit. */
	STA = (1 << 5), /*!< Start bit. */
	I2EN = (1 << 6) /*!< Enable Interrupt bit. */
} I2C_CONTROL;

/**
 * @brief	I2C device driver states.
 */
typedef enum {
	I2C_IDLE = 0,
	I2C_BUSY = 1
} I2C_STATE;

/**
 * @brief	Different modes found in Status Register when an interruption is fired.
 */
typedef enum {
	I2C_START = 0x08,
	I2C_REPEATED_START = 0x10,
	I2C_SLAW_ACK = 0x18,
	I2C_SLAW_NACK = 0x20,
	I2C_DATAW_ACK = 0x28,
	I2C_DATAW_NACK = 0x30,
	I2C_ARBITRATION_LOST = 0x38,
	I2C_SLAR_ACK = 0x40,
	I2C_SLAR_NACK = 0x48,
	I2C_DATAR_ACK = 0x50,
	I2C_DATAR_NACK = 0x58
} I2C_STATUS_MODE;


/*
 *
 *
 * Functions:
 *
 *
 */


/**
 * @brief 	Initialises I2C1 interface.
 * @note 	Must be called prior to all other I2C1 functions.
 */
void I2C1_Init();

/**
 * @brief 	Initialises I2C2 interface.
 * @note 	Must be called prior to all other I2C2 functions.
 */
void I2C2_Init();

/**
 * @brief 	Configures I2C1 interface.
 * @param	frequency: -> Rate of I2C1 communication, in Hz.
 * @param	rSize: -> Size of data (in bytes) to read.
 * @param   wBuffer: -> Data to write.
 * @param	wSize: -> Size of data (in bytes) to write.
 * @return	0 if successful, -1 if timed out.
 */
int I2C1_Configure(int frequency, int rSize, char * wBuffer, int wSize);

/**
 * @brief 	Configures I2C2 interface.
 * @param	frequency: -> Rate of I2C2 communication, in Hz.
 * @param	rSize: -> Size of data (in bytes) to read.
 * @param   wBuffer: -> Data to write.
 * @param	wSize: -> Size of data (in bytes) to write.
 * @return	0 if successful, -1 if timed out.
 */
int I2C2_Configure(int frequency, int rSize, char * wBuffer, int wSize);

/**
 * @brief	Start a transmission in I2C1 interface (read or write).
 * @return	0 if successful, -1 if timed out.
 */
int I2C1_Start();

/**
 * @brief	Start a transmission in I2C2 interface (read or write).
 * @return	0 if successful, -1 if timed out.
 */
int I2C2_Start();

/**
 * @brief	Ends a started transmission on interface I2C1.
 */
void I2C1_Stop();

/**
 * @brief	Ends a started transmission on interface I2C2.
 */
void I2C2_Stop();

/**
 * @brief	Synchronously waits for the transmission on interface I2C1 to end.
 * @note	This is a blocking function.
 */
int I2C1_Engine();

/**
 * @brief	Synchronously waits for the transmission on interface I2C2 to end.
 * @note	This is a blocking function.
 */
int I2C2_Engine();

/**
 * @brief	Copy data in read buffer on IC1 interface to specified pointer.
 * @param	buffer: -> Specified pointer where data should be copied to.
 */
void I2C1_GetBuffer(char * buffer);

/**
 * @brief	Copy data in read buffer on IC2 interface to specified pointer.
 * @param	buffer: -> Specified pointer where data should be copied to.
 */
void I2C2_GetBuffer(char * buffer);


/**
 * @}
 */


/**
 * @}
 */

#endif /* I2C_H_ */
