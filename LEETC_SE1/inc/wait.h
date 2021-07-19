/**
* @file		wait.h
* @brief	Contains the delay API.
* @version	1.0
* @date		Nov 2020
* @author	PedroG
*
* Copyright(C) 2020-2025, PedroG
* All rights reserved.
*/

#ifndef _WAIT_H_
#define _WAIT_H_

/** @defgroup WAIT WAIT
 * This package provides the core capabilities for wait functions.
 * @{
 */

/** @defgroup WAIT_Public_Functions WAIT Public Functions
 * @{
 */


#include <stdint.h>
#include <stdbool.h>

#ifdef FREERTOS
	#include "FreeRTOS.h"
	#include "task.h"
#endif


/*
 *
 *
 * Constants:
 *
 *
 */


#ifdef FREERTOS
	/**
	 * @brief	Convert Ticks to Milliseconds
	 */
	#define pdTICKS_TO_MS(xTicks) (((TickType_t) (xTicks) * 1000u) / configTICK_RATE_HZ)
#endif

/**
 * @brief	PCLK.
 * @note    We are using PCLK = 1/4*CCLK
 */
#define TIMER_PCLK (SystemCoreClock/4)

/**
 * @brief	Timer resolution (1us).
 */
#define TIMER_RES 0.000001

/**
 * @brief	Interrupt Register Mask.
 * @note 	We are using Match Register 0.
 */
#define TIMER_IR_MR0 0x1

/**
 * @brief	Systick Frequency.
 */
#define SYSTICK_FREQ (SystemCoreClock / 1000)

/**
 * @brief	Type of resource to use in WAIT API.
 */
typedef enum
{
	SYS, /*!< to use the SYSTICK resource. */
	TIM0, /*!< to use the TIMER resource of timer0. */
	IRQ0, /*!< to use the IRQ (Interrupt Request) resource of timer0. */
	TIM1, /*!< to use the TIMER resource of timer1. */
	IRQ1, /*!< to use the IRQ (Interrupt Request) resource of timer1. */
	TIM2, /*!< to use the TIMER resource of timer2. */
	IRQ2, /*!< to use the IRQ (Interrupt Request) resource of timer2. */
	TIM3, /*!< to use the TIMER resource of timer3. */
	IRQ3, /*!< to use the IRQ (Interrupt Request) resource of timer3. */
} WAIT;

/**
 * @brief	Timer Control Register modes.
 */
typedef enum
{
	RESET = 0x3, /*!< Reset TC. */
	DISABLE_RESET = 0x2, /*!< Reset and disable TC. */
	ENABLE = 0x1, /*!< Enable TC. */
	DISABLE = 0x0 /*!< Disable TC. */
} TIMER_TCR;

/**
 * @brief	Enable PCONP bit for all timers.
 */
typedef enum
{
	PCONP_ENABLE_TIM0 = (1<<1), /*!< PCONP Enable bit for timer0. */
	PCONP_ENABLE_TIM1 = (1<<2), /*!< PCONP Enable bit for timer1. */
	PCONP_ENABLE_TIM2 = (1<<22), /*!< PCONP Enable bit for timer2. */
	PCONP_ENABLE_TIM3 = (1<<23) /*!< PCONP Enable bit for timer3. */
} TIMER_ENABLE;


/**
 * @brief	PCLK Mask for all timers.
 */
typedef enum
{
	BITS_PCLK_TIM0 = (0x03 << 2), /*!< PCLK Mask for timer0 -> bits [3:2]. */
	BITS_PCLK_TIM1 = (0x03 << 4), /*!< PCLK Mask for timer1 -> bits [5:4]. */
	BITS_PCLK_TIM2 = (0x03 << 12), /*!< PCLK Mask for timer2 -> bits [13:12]. */
	BITS_PCLK_TIM3 = (0x03 << 14) /*!< PCLK Mask for timer3 -> bits [15:14]. */
} TIMER_PCLK_BITS;


/*
 *
 *
 * Functions:
 *
 *
 */


/**
 * @brief	Initialises the wait API, using the desired resource.
 * @param   mode: -> Which resource to use (see enum WAIT).
 * @return	0 if initialisation successed, -1 if fails.
 * @note	This function must be called prior to any other WAIT functions.
 * @note 	Only 1 type of resource may be initialised for each timer, i.e.
 * 			if TIMi is initialised, trying to initialise IRQi will fail,
 * 			and vice-versa.
 */
int32_t WAIT_Init(WAIT mode);

/**
 * @brief	Waits a number of milliseconds.
 * @note	Requires SYSTICK Initialisation [SYS].
 * @param	millis: -> The whole number of milliseconds to wait.
 */
void WAIT_SYS_Ms(uint32_t millis);

/**
 * @brief	Waits a number of milliseconds.
 * @note	Requires TIMER0 Initialisation [TIM].
 * @param	micros: -> The whole number of microseconds to wait.
 */
void WAIT_TIM0_Us(uint32_t micros);

/**
 * @brief	Interrupts after a certain amount of us and executes a certain function.
 * @note	Requires Interrupt Request Handler Initialisation [IRQ] for timer0.
 * @param	micros: -> The whole number of microseconds to wait.
 * @param	f	  : -> Function to be executed upon interrupt.
 */
void WAIT_IRQ0_Us(uint32_t micros, void (*f)(void));

/**
 * @brief	Waits a number of milliseconds.
 * @note	Requires TIMER1 Initialisation [TIM].
 * @param	micros: -> The whole number of microseconds to wait.
 */
void WAIT_TIM1_Us(uint32_t micros);

/**
 * @brief	Interrupts after a certain amount of us and executes a certain function.
 * @note	Requires Interrupt Request Handler Initialisation [IRQ] for timer1.
 * @param	micros: -> The whole number of microseconds to wait.
 * @param	f	  : -> Function to be executed upon interrupt.
 */
void WAIT_IRQ1_Us(uint32_t micros, void (*f)(void));

/**
 * @brief	Waits a number of milliseconds.
 * @note	Requires TIMER2 Initialisation [TIM].
 * @param	micros: -> The whole number of microseconds to wait.
 */
void WAIT_TIM2_Us(uint32_t micros);

/**
 * @brief	Interrupts after a certain amount of us and executes a certain function.
 * @note	Requires Interrupt Request Handler Initialisation [IRQ] for timer2.
 * @param	micros: -> The whole number of microseconds to wait.
 * @param	f	  : -> Function to be executed upon interrupt.
 */
void WAIT_IRQ2_Us(uint32_t micros, void (*f)(void));

/**
 * @brief	Waits a number of milliseconds.
 * @note	Requires TIMER3 Initialisation [TIM].
 * @param	micros: -> The whole number of microseconds to wait.
 */
void WAIT_TIM3_Us(uint32_t micros);

/**
 * @brief	Interrupts after a certain amount of us and executes a certain function.
 * @note	Requires Interrupt Request Handler Initialisation [IRQ] for timer3.
 * @param	micros: -> The whole number of microseconds to wait.
 * @param	f	  : -> Function to be executed upon interrupt.
 */
void WAIT_IRQ3_Us(uint32_t micros, void (*f)(void));

/**
 * @brief	Ask if a specified timer is busy (waiting to interrupt).
 * @param	timer: -> Timer requested.
 * @return  True if requested timer is busy, false if not.
 */
bool WAIT_IsBusy(WAIT timer);

/**
 * @brief	Get difference in milliseconds from parameter.
 * @note	Requires SYSTICK Initialisation [SYS].
 * @param	start: -> if 0 get current milliseconds.
 * @return	Elapsed ms since start.
 */
uint32_t WAIT_SYS_GetElapsedMs(uint32_t start);


/**
 * @}
 */


/**
 * @}
 */

#endif /* _WAIT_H_ */
