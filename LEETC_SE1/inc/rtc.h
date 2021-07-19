/**
* @file		rtc.h
* @brief	Contains the RTC API.
* @version	1.0
* @date		Dec 2020
* @author	PedroG
*
* Copyright(C) 2020-2025, PedroG
* All rights reserved.
*/

#ifndef _RTC_H_
#define _RTC_H_

/** @defgroup RTC RTC
 * This package provides the core capabilities for rtc functions.
 * @{
 */

/** @defgroup RTC_Public_Functions RTC Public Functions
 * @{
 */


#include <stdarg.h>
#include <stdio.h>
#include <time.h>


/*
 *
 *
 * Constants:
 *
 *
 */


/**
 * @brief	Enable PCONP bit for RTC.
 */
#define RTC_PCONP_ENABLE (1 << 9)

/**
 * @brief	Enable RTC counters.
 */
#define RTC_ENABLE 0x01

/**
 * @brief	Disable RTC counters.
 */
#define RTC_DISABLE 0x00

/**
 * @brief	RTC interrupt Location bit.
 */
#define RTC_IL_BIT 0x01

/**
 * @brief	Field that will interrupt.
 */
typedef enum
{
	INONE = 0x00, /*!< to never interrupt. */
	ISEC = 0x01, /*!< to interrupt every second. */
	IMIN = 0x02, /*!< to interrupt every minute. */
	IHOUR = 0x04, /*!< to interrupt every hour. */
	IDOM = 0x08, /*!< to interrupt every day of month. */
	IDOW = 0x10, /*!< to interrupt every day of week. */
	IDOY = 0x20, /*!< to interrupt every day of year. */
	IMONTH = 0x40, /*!< to interrupt every month. */
	IYEAR = 0x080, /*!< to interrupt every year. */
} RTC_INTERRUPT_FIELD;

/**
 * @brief	Time field.
 */
typedef enum
{
	NONE, /*!< None. */
	YEAR, /*!< Year field. */
	MONTH, /*!< Month field. */
	DOM, /*!< Day of Month field. */
	DOW, /*!< Day of Week field. */
	HOUR, /*!< Hour field. */
	MIN, /*!< Minute field. */
	SEC, /*!< Second field. */
} RTC_TIME_FIELD;


/*
 *
 *
 * Functions:
 *
 *
 */


/**
 * @brief	Initialises the RTC API, with the desired time.
 * @param   seconds: -> Seconds since 01/01/1970.
 * @note    If parameter type is INONE, no handler is needed. Only the first 2 parameters will be read.
 * @note    Only the first 3 parameters can be read. Any other parameters will be ignored.
 * @note	This function must be called prior to any other RTC functions.
 */
void RTC_Init(time_t seconds);

/**
 * @brief	Sends current RTC time to desired location.
 * @param   dateTime: -> Desired location.
 */
void RTC_GetValue(struct tm *dateTime);

/**
 * @brief	Sets RTC time value with the desired time.
 * @param   dateTime: -> Desired time.
 */
void RTC_SetValue(struct tm *dateTime);

/**
 * @brief	Gets the RTC time value.
 * @return	Seconds since Epoh (01/01/1970).
 */
time_t RTC_GetSeconds(void);

/**
 * @brief	Sets RTC time value with the desired seconds, since Epoh (01/01/1970).
 * @param   seconds: -> Desired seconds.
 */
void RTC_SetSeconds(time_t seconds);

/**
 * @brief	Enable RTC counters.
 */
void RTC_Enable(void);

/**
 * @brief	Disable RTC counters.
 * @note    You can enable it back using RTC_Enable().
 */
void RTC_Disable(void);

/**
 * @brief	Increment field specified.
 * @note    Incrementing the highest possible value, will set it back to the lowest value.
 */
void RTC_IncrementField(RTC_TIME_FIELD field);

/**
 * @brief	Decrement field specified.
 * @note    Decrementing the lowest possible value, will set it back to the highest value.
 */
void RTC_DecrementField(RTC_TIME_FIELD field);


/**
 * @}
 */


/**
 * @}
 */

#endif /* RTC_H_ */
