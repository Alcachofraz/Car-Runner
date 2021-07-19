/*
 * score.c
 *
 *  Created on: Jan 2021
 *      Author: pedro
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif


#include "score.h"


/**
 * Holds a Score value.
 */
static QueueHandle_t queueSCORE = NULL;


static MEMORY_DEVICE dev;


bool SCORE_SaveLocally(int score, char * username);


bool SCORE_Init(MEMORY_DEVICE device) {
	dev = device;

	if (xTaskCreate(SCORE_SaveAndPublishTask, (const char * const) "SCORE_SaveAndPublishTask", TASK_SCORE_STACK_SIZE, NULL, TASK_SCORE_PRIORITY, NULL) != pdPASS) {
		printf("SCORE_SaveAndPublishTask could not be created.\n");
		return false;
	}

	if ((queueSCORE = xQueueCreate(8, sizeof(Score))) == NULL) {
		printf("Queue for SCORE could not be created.\n");
		return false;
	}

	return true;
}

static void SCORE_FLASH_GetAll(Score * scores) {
	memcpy(scores, (Score *)FLASH_START_ADDRESS_29, SCORE_NUM * sizeof(Score));
}

static void SCORE_EEPROM_GetAll(Score * scores) {
	EEPROM_Read((char *)scores, SCORE_NUM * sizeof(Score));
}

static void SCORE_FLASH_SetAll(Score * scores) {
	// Array of new scores to be written (FLASH):
	Score toWrite[FLASH_MINIMAL_WRITE_SIZE/sizeof(Score)] = {0};

	memcpy(toWrite, scores, SCORE_NUM * sizeof(Score));

	// Erase sector before write:
	FLASH_EraseSectors(FLASH_SECTOR, FLASH_SECTOR);

	// Write updated scores:
	FLASH_WriteData(FLASH_SECTOR, (void*) FLASH_START_ADDRESS_29, toWrite, FLASH_MINIMAL_WRITE_SIZE);
	FLASH_VerifyData((void*) FLASH_START_ADDRESS_29, toWrite, FLASH_MINIMAL_WRITE_SIZE);
}

static void SCORE_EEPROM_SetAll(Score * scores) {
	EEPROM_Write((char *)scores, SCORE_NUM * sizeof(Score));
}

int SCORE_Get(Score * score, int n) {
	if ((n < 0) || (n > SCORE_NUM-1)) return -1;

	Score scores[SCORE_NUM] = {0};

	switch(dev) {
		case FLASH:
			SCORE_FLASH_GetAll(scores);
			break;
		case EEPROM:
			SCORE_EEPROM_GetAll(scores);
			break;
	}

	*score = scores[n];

	return 0;
}

void SCORE_Sort(Score * scores, int size) {
	Score temp;
	for (int i = 0; i < size-1; i++) {
		for (int j = 0; j < size-i-1; j++) {
			if (scores[j].score < scores[j+1].score) {
				temp = scores[j];
				scores[j] = scores[j+1];
				scores[j+1] = temp;
			}
		}
	}
}

void SCORE_Erase(void) {
	// Array of empty scores:
	Score scores[SCORE_NUM] = {0};

	switch(dev) {
		case FLASH:
			SCORE_FLASH_SetAll(scores);
			break;
		case EEPROM:
			SCORE_EEPROM_SetAll(scores);
			break;
	}
}

void SCORE_Save(uint32_t score, char * username) {
	Score scoreStruct;
	strncpy(scoreStruct.name, username, NAME_LENGTH+1);
	scoreStruct.score = score;
	xQueueSend(queueSCORE, &scoreStruct, portMAX_DELAY);
}

bool SCORE_SaveLocally(int score, char * username) {
	// Array of old scores to be read:
	Score oldScores[SCORE_NUM] = {0};

	// Array of new scores to be written:
	Score newScores[SCORE_NUM] = {0};

	switch(dev) {
		case FLASH:
			SCORE_FLASH_GetAll(oldScores);
			break;
		case EEPROM:
			SCORE_EEPROM_GetAll(oldScores);
			break;
	}

	bool done = false;
	int lowestScore = 0;

	for (int i = 0; i < SCORE_NUM; i++) {
		newScores[i] = oldScores[i];
		if ((strncmp(newScores[i].name, username, NAME_LENGTH+1) == 0) && !done) { // If user name coincides...
			if (newScores[i].score < score) { // If the new score is better...
				newScores[i].score = score;
				done = true;
			}
			else return false; // If not, return (no need to update flash)
		}
		else if ((newScores[i].score == 0) && !done) { // If there is space available...
			strncpy(newScores[i].name, username, NAME_LENGTH+1);
			newScores[i].score = score;
			done = true;
		}
		if (newScores[i].score < newScores[lowestScore].score) {
			lowestScore = i;
		}
	}

	if (!done) { // If user name was not found, and there is no space available...
		if (score > newScores[lowestScore].score) { // If score is higher than the lowest registered...
			strncpy(newScores[lowestScore].name, username, NAME_LENGTH+1);
			newScores[lowestScore].score = score;
		}
	}

	SCORE_Sort(newScores, SCORE_NUM);

	switch(dev) {
		case FLASH:
			SCORE_FLASH_SetAll(newScores);
			break;
		case EEPROM:
			SCORE_EEPROM_SetAll(newScores);
			break;
	}

	return true;
}

void SCORE_SaveAndPublishTask(void *pvParameters) {
	if (dev == EEPROM) {
		EEPROM_Init();
	}

	Score score;

	for (;;) {
		xQueueReceive(queueSCORE, &score, portMAX_DELAY);

		if (SCORE_SaveLocally(score.score, score.name)) { // If score is on top 3:
			NETWORK_PublishScore(score.score);
		}
	}

	vTaskDelete(NULL);
}
