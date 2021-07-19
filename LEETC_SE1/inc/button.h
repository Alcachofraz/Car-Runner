/*
* @file		button.h
* @brief	Contains the input button API.
* @version	1.0
* @date		Nov 2020
* @author	PedroG
*
* Copyright(C) 2020-2025, PedroG
* All rights reserved.
 */

#ifndef BUTTON_H_
#define BUTTON_H_

/** @defgroup BUTTON BUTTON
 * This package provides the core capabilities for input buttons functions.
 * @{
 */

/** @defgroup BUTTON_Public_Functions BUTTON Public Functions
 * @{
 */


#include "wait.h"
#include <stdint.h>
#include <stdbool.h>


/*
 *
 *
 * Constants:
 *
 *
 */


/**
 * @brief	Debounce time.
 */
#define DEBOUNCE_TIME 50

/**
 * @brief	Mask for all 3 buttons.
 */
#define BUTTONS_MASK (B1 | B2 | B3)

/**
 * @brief	Input buttons.
 */
typedef enum
{
	B1 = (1 << 1), /*!< PO[1] -> Pin 43. */
	B2 = (1 << 2), /*!< PO[2] -> Pin 44. */
	B3 = (1 << 3) /*!< PO[3] -> Pin 45. */
} BUTTON;


/*
 *
 *
 * Functions:
 *
 *
 */


/**
 * @brief	Initialises the button API.
 * @note	This function must be called prior to any other BUTTON functions.
 */
void BUTTON_Init(void);

/**
 * @brief	Gets input buttons bitmap.
 * @note	This is a non-blocking function.
 * @return  Returns input bitmap, i.e., 0 if no active input.
 */
int32_t BUTTON_Hit(void);

/**
 * @brief	Waits for any input buttons.
 * @note	This is a blocking function.
 * @return  Returns input bitmap, i.e., 0 if no active input.
 */
int32_t BUTTON_Read(void);

/**
 * @brief	Gets bitmap of button events.
 * @note	This is a non-blocking function.
 * @param	current: -> Pointer to int, where current buttons state will be stored.
 * @note	If current buttons state is not wanted, current should be NULL (0).
 * @return  If a certain button is in a different state comparing to the previous call to this function, the corresponding bit is returned as 1.
 */
uint32_t BUTTON_GetButtonsEvents(uint32_t * current);

/**
 * @brief	Gets bitmap of button push events.
 * @note	This is a non-blocking function.
 * @param	current: -> Pointer to int, where current buttons state will be stored.
 * @note	If current buttons state is not wanted, current should be NULL (0).
 * @return  If a certain button is in a different state comparing to the previous call to this function and is pressed now, the corresponding bit is returned as 1.
 */
uint32_t BUTTON_GetButtonsPushEvents(uint32_t * current);

/**
 * @brief	Gets bitmap of button release events.
 * @note	This is a non-blocking function.
 * @param	current: -> Pointer to int, where current buttons state will be stored.
 * @note	If current buttons state is not wanted, current should be NULL (0).
 * @return  If a certain button is in a different state comparing to the previous call to this function and is not pressed now, the corresponding bit is returned as 1.
 */
uint32_t BUTTON_GetButtonsReleaseEvents(uint32_t * current);

/**
 * @brief	Waits until all buttons are released.
 * @param	f: -> Pointer to function to repeatedly execute while waiting. Write '0' if no function is needed. Should be 0 if no interest in such.
 * @note	This is a blocking function.
 */
void BUTTON_WaitRelease(void (*f)(void));


/**
 * @}
 */


/**
 * @}
 */

#endif /* BUTTON_H_ */
