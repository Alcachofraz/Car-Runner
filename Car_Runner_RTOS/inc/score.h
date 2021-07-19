/*
 * score.h
 *
 *  Created on: 9 Jan 2021
 *      Author: pedro
 */

#ifndef SCORE_H_
#define SCORE_H_

/** @defgroup SCORE SCORE
 * This package provides the core capabilities for score management functions.
 * @{
 */

/** @defgroup SCORE_Public_Functions SCORE Public Functions
 * @{
 */

/*
 * 	Memory will be used to store the best
 * 	scores, and respective user name.
 *
 * ----------------------------------------
 * |         |         |         |
 * | score 1 | score 2 | score 3 |   ...
 * |         |         |         |
 * ----------------------------------------
 * \________/\________/\________/\________/
 *     16        16        16        208
 * \______________________________________/
 * 				     256
 *
 * 	The program will ask for user name.
 * 	There will be only one score registered per user
 * name. The best one.
 */


#include "flash.h"
#include "eeprom.h"
#include "network.h"

#include <stdbool.h>

// Kernel includes.
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


/**
 *
 *
 * CONSTANTS:
 *
 *
 */

/**
 * @note	Score Task Priority.
 */
#define TASK_SCORE_PRIORITY tskIDLE_PRIORITY + 1

/**
 * @note	Score Task Stack Size.
 */
#define TASK_SCORE_STACK_SIZE configMINIMAL_STACK_SIZE*1

/**
 * @brief	Number of scores to be sequentially displayed in Idle Mode.
 */
#define SCORE_NUM 3

/**
 * @brief	Flash sector to be used.
 */
#define FLASH_SECTOR 29

/**
 * @brief	Maximum possible score, due to LCD fitting reasons.
 */
#define MAX_SCORE 999

/**
 * @brief	User name maximum length.
 */
#define NAME_LENGTH 11

/**
 * @brief	Structure to hold a score, and respective user name.
 * @note    This the Data Type (16 bytes) that will be stored in the Flash Memory.
 */
typedef struct Score {
	char name[NAME_LENGTH+1]; /*!< Name of user (12 bytes) */
	uint32_t score; /*!< His best score (4 bytes) */
} Score;

/**
 * @brief	Type of memory device to be used.
 */
typedef enum {
	FLASH, /*!< Use LPC1769 FLASH memory. */
	EEPROM /*!< Use expansion EEPROM memory. */
} MEMORY_DEVICE;


/**
 *
 *
 * Functions:
 *
 *
 */


/**
 * @brief	Initialises Score Saver and Publisher, in the desired memory device.
 * @param   device: -> Local memory device.
 * @return  If initialisation succeeded.
 */
bool SCORE_Init(MEMORY_DEVICE device);

/**
 * @brief	Writes the requested Score to given pointer.
 * @param   score: -> Pointer where Score shall be written.
 * @param   n: -> Index of Score requested [0, SCORE_NUM-1].
 */
int SCORE_Get(Score * score, int n);

/**
 * @brief	Sorts given array of Scores downwardly.
 * @param   scores: -> Array of scores.
 * @param   size: -> Size of the array.
 */
void SCORE_Sort(Score * scores, int size);

/**
 * @brief	If given score is one of the best 3, saves it to flash memory.
 * @param   score: -> User's score.
 * @param   username: -> String username.
 */
void SCORE_Save(uint32_t score, char * username);

/**
 * @brief	Erases all scores from flash memory.
 */
void SCORE_Erase(void);

/**
 * @brief	Task to save score locally, and publish in server (if record).
 */
void SCORE_SaveAndPublishTask(void *pvParameters);


/**
 * @}
 */


/**
 * @}
 */

#endif /* SCORE_H_ */
