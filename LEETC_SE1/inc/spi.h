/**
* @file		spi.h
* @brief	Contains the SPI API.
* @version	1.0
* @date		Dec 2020
* @author	PedroG
*
* Copyright(C) 2020-2025, PedroG
* All rights reserved.
*/

#ifndef SPI_H_
#define SPI_H_

/** @defgroup SPI SPI
 * This package provides the core capabilities for spi functions.
 * @{
 */

/** @defgroup SPI_Public_Functions SPI Public Functions
 * @{
 */


#include <stdbool.h>


/*
 *
 *
 * Constants:
 *
 *
 */


/**
 * @brief	SPI's interface power/clock control bit.
 */
#define SPI_PCONP_ENABLE (1 << 8)

/**
 * @brief	SPI's CCR counter setting mask.
 */
#define SPI_CCR_MASK 0xFF

/**
 * @brief	SPI's CR transfer bits mask.
 */
#define SPI_CR_BITS_MASK (0x0F << 8)

/**
 * @brief	SPI's transfer complete flag mask.
 */
#define SPIF_MASK (1 << 7)

/**
 * @brief	SPI's pin function for SPI mode.
 */
#define SPI_PIN_FUNTION 0x03

/**
 * @brief	Mask for all possible SPI R/W error flags.
 */
#define SPI_STATUS_ERROR_MASK (15 << 2)

/**
 * @brief	SPI GPIO pins.
 */
typedef enum {
	SCK = 15, /*!< P0[15] -> Pin 13. */
	MISO = 17, /*!< P0[17] -> Pin 12. */
	MOSI = 18 /*!< P0[18] -> Pin 11. */
} SPI_PINS;

/**
 * @brief	SPI CR fields.
 */
typedef enum {
	BIT_ENABLE = (1 << 2), /*!< Set custom bit transfer. */
	CPHA = (1 << 3), /*!< Set clock phase to sample data on the first clock edge of SCK.  */
	CPOL = (1 << 4), /*!< Set clock polarity to active low. */
	MSTR = (1 << 5) /*!< Select master mode. */
} SPI_CR_VALUES;


/*
 *
 *
 * Functions:
 *
 *
 */


/**
 * @brief	Initialise SPI API.
 * @note	This function must be called prior to any other SPI functions.
 */
void SPI_Init(void);

/**
 * @brief	Configure transfer settings.
 * @param   frequency: -> Transfer SPI rate.
 * @param   bitData: -> Bits per transfer. Must be a value between 8 and 16.
 * @param   mode: -> CPHA and CPOL information. Only the 2 lsb's will be read. Should be [0, 3].
 * @return  0 if successfully configured, or -1 if bitData is out of range.
 * @note	This function must be called before 'SPI_Transfer()'.
 */
int32_t SPI_ConfigTransfer(int frequency, int bitData, int mode);

/**
 * @brief	Transfer and receive data.
 * @param   txBuffer: -> Pointer to buffer holding data to be sent.
 * @param   rxBuffer: -> Pointer to buffer where received data should be written.
 * @param   length: -> Length of data to be sent (measured in words).
 * @return  0 if successful, or -1 if parameters are not valid.
 */
int32_t SPI_Transfer(unsigned short *txBuffer, unsigned short *rxBuffer, int length);

/**
 * @brief	Verify user frequency.
 * @param   frequency: -> 0 if pclk/frequency is even and not lower than 8. -1 if not.
 */
int SPI_VerifyFrequency(int frequency);


/**
 * @}
 */


/**
 * @}
 */

#endif /* SPI_H_ */
