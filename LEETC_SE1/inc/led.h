/*
* @file		led.h
* @brief	Contains the input API.
* @version	1.0
* @date		Nov 2020
* @author	PedroG
*
* Copyright(C) 2020-2025, PedroG
* All rights reserved.
 */

#ifndef LED_H_
#define LED_H_

/** @defgroup LED LED
 * This package provides the core capabilities for led output functions.
 * @{
 */

/** @defgroup LED_Public_Functions LED Public Functions
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
 * @brief	Internal LED -> PO[22].
 */
#define LED2 (1 << 22)


/*
 *
 *
 * Functions:
 *
 *
 */


/**
 * @brief	Initialises the LED API.
 * @param   state: -> If true, leaves LED on. If false, leaves LED off.
 * @note	This function must be called prior to any other LED functions.
 */
void LED_Init(bool state);

/**
 * @brief	Checks LED state
 * @return  Returns true if LED is on, false if LED is off.
 */
bool LED_GetState(void);

/**
 * @brief	Turns LED on.
 */
void LED_On(void);

/**
 * @brief	Turns LED off.
 */
void LED_Off(void);

/**
 * @brief	Invert LED state.
 */
void LED_Invert(void);


/**
 * @}
 */


/**
 * @}
 */

#endif /* LED_H_ */
