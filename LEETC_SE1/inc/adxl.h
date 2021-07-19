/**
* @file		adxl.h
* @brief	Contains the ADXL345 API.
* @version	1.0
* @date		Jan 2021
* @author	PedroG
*
* Copyright(C) 2020-2025, PedroG
* All rights reserved.
*/

#ifndef ADXL_H_
#define ADXL_H_

/** @defgroup ADXL ADXL
 * This package provides the core capabilities for adxl functions.
 * @{
 */

/** @defgroup ADXL_Public_Functions ADXL Public Functions
 * @{
 */

#include <stdbool.h>
#include <stdio.h>
#include "spi.h"


#ifdef FREERTOS
	#include "FreeRTOS.h"
	#include "task.h"
	#include "queue.h"
#endif


/**
 * Structure representing axis.
 */
typedef struct {
	signed short x;
	signed short y;
	signed short z;
} AXIS;


/*
 *
 *
 * Constants:
 *
 *
 */


#ifdef FREERTOS
	/**
	 * @brief	Stack size of ADXL Axis task.
	 * @brief	Only needed in FreeRTOS environment.
	 */
	#define TASK_ADXL_AXIS_STACK_SIZE configMINIMAL_STACK_SIZE*0.5
	/**
	 * @brief	Priority of ADXL Axis task.
	 * @brief	Only needed in FreeRTOS environment.
	 */
	#define TASK_ADXL_AXIS_PRIORITY tskIDLE_PRIORITY + 1
#endif


/**
 * @brief	ADXL's data format resolution bit.
 */
#define ADXL_RESOLUTION_BIT (1 << 3)

/**
 * @brief	ADXL's measurement mode bit.
 */
#define ADXL_MEASURE_BIT (1 << 3)

/**
 * @brief	ADXL's command read bit.
 */
#define ADXL_READ_BIT (1 << 7)

/**
 * @brief	ADXL command's multiple byte bit.
 */
#define ADXL_MB_BIT (1 << 6)

/**
 * @brief   ADXL FIFO mode
 * @note 	0 for bypass, 1 for fifo, 2 for stream, 3 for trigger
 */
#define ADXL_FIFO_MODE (2 << 6)

/**
 * @brief   ADXL FIFO sample bits
 */
#define ADXL_SAMPLE_BITS 0x1f

/**
 * @brief	Maximum transfer length possible.
 */
#define MAX_LENGTH 0x39

/**
 * @brief	Chip Select GPIO0 pin.
 */
#define CS 16

/**
 * @brief	ADXL commands.
 */
typedef enum {
	DEVID = 0x00, /*!< Developer ID */
	BW_RATE = 0x2c, /*!< Data rate */
	POWER_CTL = 0x2d, /*!< Power saving features */
	DATA_FORMAT = 0x31, /*!< Data format control */
	DATAX0 = 0x32, /*!< X-Axis data 0 */
	DATAX1 = 0x33, /*!< X-Axis data 1 */
	DATAY0 = 0x34, /*!< Y-Axis data 0 */
	DATAY1 = 0x35, /*!< Y-Axis data 1 */
	DATAZ0 = 0x36, /*!< Z-Axis data 0 */
	DATAZ1 = 0x37, /*!< Z-Axis data 1 */
	FIFO_CTL = 0x38, /*!< FIFO settings */
} ADXL_Commands;


/*
 *
 *
 * Functions:
 *
 *
 */


/**
 * @brief	Initialise ADXL.
 * @param   frequency: -> SPI transfer rate.
 * @param   dataResolution: -> Resolution of axis data. Should be [0, 3]. The higher this value, the higher the resolution.
 * @return  0 if successful, -1 if failed.
 * @note	This function must be called prior to any ADXL functions.
 */
int32_t ADXL_Init(int frequency, int dataResolution);

/**
 * @brief	Obtain X-Axis, Y-Axis and Z-Axis as short values.
 * @return  Axis values structure.
 */
AXIS ADXL_GetAxis();

/**
 * @brief	Gets Developer Id, for testing connection.
 * @return  Returns DEVID, or -1 if failed.
 */
int ADXL_GetDevId(void);


/**
 * @}
 */


/**
 * @}
 */

#endif /* ADXL_H_ */
