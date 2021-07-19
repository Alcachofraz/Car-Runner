/*
===============================================================================
===============================================================================
 Name        : car_runner_rtos.c
 Author      : PedroG
 Version     : 1.0
 Copyright   : PedroG
 Description : LCD Car game using FreeRTOS
===============================================================================
===============================================================================
*/


/*
===========================================================================================================================================================
===========================================================================================================================================================

	INCLUDES:

===========================================================================================================================================================
===========================================================================================================================================================
*/



#include "car_runner_rtos.h"



/*
===========================================================================================================================================================
===========================================================================================================================================================

	GLOBAL VARIABLES:

===========================================================================================================================================================
===========================================================================================================================================================
*/

static STATE state = STATE_IDLE;

static uint32_t bitmap = 0;
static uint32_t points = 1;
static uint32_t fuel = 0;
static uint32_t scoreCount = 0;

static char username[NAME_LENGTH+1]; // Name of current user.

static bool blinked = false;

static bool gameEnd = false;



/*
===========================================================================================================================================================
===========================================================================================================================================================

	SEMAPHORES:

===========================================================================================================================================================
===========================================================================================================================================================
*/

/**
 * Binary semaphore to control access to global state.
 */
SemaphoreHandle_t semGAME_END = NULL;

/**
 * Binary semaphore to control access to global state.
 */
SemaphoreHandle_t semPOINTS = NULL;

/**
 * Binary semaphore to control access to global state.
 */
SemaphoreHandle_t semFUEL = NULL;



/*
===========================================================================================================================================================
===========================================================================================================================================================

	QUEUE:

===========================================================================================================================================================
===========================================================================================================================================================
*/

/**
 * Holds a signed short representing current inclination.
 */
QueueHandle_t queueACC = NULL;

/**
 * Holds a GAME_INFO structure.
 */
QueueHandle_t queueUPDATE_GAME = NULL;

/**
 * Holds a uint8_t dummy value.
 */
QueueHandle_t queueDATE = NULL;



/*
===========================================================================================================================================================
===========================================================================================================================================================

	MAIN:

===========================================================================================================================================================
===========================================================================================================================================================
*/

int main(void) {

	SystemInit();

	if (LCDText_Init() < 0) {
		printf("LCD initialisation failed.");
		return 0;
	}

	if (WAIT_Init(SYS) < 0) {
		printf("WAIT failed");
		return 0;
	}

	ADXL_Init(0, 0);

	if (!SCORE_Init(EEPROM)) {
		printf("SCORE initialisation failed");
		return 0;
	}

	RTC_Init(0);

	NETWORK_Init();

	LCDText_CreateChar(CAR_BACK_CHAR, car_back_charmap);
	LCDText_CreateChar(CAR_FRONT_CHAR, car_front_charmap);
	LCDText_CreateChar(BARRIER_CHAR, barrier_charmap);
	LCDText_CreateChar(FUEL_CHAR, fuel_charmap);
	LCDText_CreateChar(FUEL1_CHAR, fuel1_charmap);
	LCDText_CreateChar(FUEL2_CHAR, fuel2_charmap);
	LCDText_CreateChar(FUEL3_CHAR, fuel3_charmap);
	LCDText_CreateChar(FUEL4_CHAR, fuel4_charmap);

	/**
	 * Semaphores:
	 */

	if ((semGAME_END = xSemaphoreCreateMutex()) == NULL) {
		printf("Semaphore GAME_END could not be created.\n");
		return 0;
	}

	if ((semPOINTS = xSemaphoreCreateMutex()) == NULL) {
		printf("Semaphore POINTS could not be created.\n");
		return 0;
	}

	if ((semFUEL = xSemaphoreCreateMutex()) == NULL) {
		printf("Semaphore FUEL could not be created.\n");
		return 0;
	}

	/**
	 * Queues:
	 */

	if ((queueUPDATE_GAME = xQueueCreate(8, sizeof(GAME_INFO))) == NULL) {
		printf("Queue for UPDATE_GAME could not be created.\n");
		return 0;
	}

	if ((queueDATE = xQueueCreate(1, sizeof(DATE_TIME))) == NULL) {
		printf("Queue for DATE could not be created.\n");
		return 0;
	}

	/**
	 * Tasks:
	 */

	if (xTaskCreate(taskSTATE_MACHINE, (const char * const) "TaskSTATE_MACHINE", TASK_STATE_MACHINE_STACK_SIZE, NULL, TASK_STATE_MACHINE_PRIORITY, NULL) != pdPASS) {
		printf("TaskStateMachine could not be created.\n");
		return 0;
	}

	if (xTaskCreate(taskPOINTS, (const char * const) "TaskPOINTS", TASK_POINTS_STACK_SIZE, NULL, TASK_POINTS_PRIORITY, NULL) != pdPASS) {
		printf("TaskPOINTS could not be created.\n");
		return 0;
	}

	if (xTaskCreate(taskDATE, (const char * const) "TaskDATE", TASK_DATE_STACK_SIZE, NULL, TASK_DATE_PRIORITY, NULL) != pdPASS) {
		printf("TaskDATE could not be created.\n");
		return 0;
	}

	if (xTaskCreate(taskSCORE_COUNT, (const char * const) "TaskSCORE_COUNT", TASK_SCORE_COUNT_STACK_SIZE, NULL, TASK_SCORE_COUNT_PRIORITY, NULL) != pdPASS) {
		printf("TaskSCORE_COUNT could not be created.\n");
		return 0;
	}

	if (xTaskCreate(taskUPDATE_GAME, (const char * const) "TaskUPDATE_GAME", TASK_UPDATE_GAME_STACK_SIZE, NULL, TASK_UPDATE_GAME_PRIORITY, NULL) != pdPASS) {
		printf("TaskUPDATE_GAME could not be created.\n");
		return 0;
	}

	if (xTaskCreate(taskFUEL, (const char * const) "TaskFUEL", TASK_FUEL_STACK_SIZE, NULL, TASK_FUEL_PRIORITY, NULL) != pdPASS) {
		printf("TaskFUEL could not be created.\n");
		return 0;
	}

	if (xTaskCreate(taskBLINK, (const char * const) "TaskBLINK", TASK_BLINK_STACK_SIZE, NULL, TASK_BLINK_PRIORITY, NULL) != pdPASS) {
		printf("TaskBLINK could not be created.\n");
		return 0;
	}

	if (xTaskCreate(taskNETWORK_MANAGER, (const char * const) "TaskNETWORK_MANAGER", TASK_NETWORK_MANAGER_STACK_SIZE, NULL, TASK_NETWORK_MANAGER_PRIORITY, NULL) != pdPASS) {
		printf("TaskNETWORK_MANAGER could not be created.\n");
		return 0;
	}

	/**
	 * Start
	 */

	vTaskStartScheduler();

	while(1);

	return 0;
}



/*
===========================================================================================================================================================
===========================================================================================================================================================

	TASKS:

===========================================================================================================================================================
===========================================================================================================================================================
*/

void taskSTATE_MACHINE(void *pvParameters) {
	for (int i = 0; i < NAME_LENGTH; i++) {
		username[i] = 'a';
	}
	username[NAME_LENGTH] = '\0';
	getUsername();

	for (;;) {
		LCDText_Clear();
		switch (state) {
			case STATE_IDLE:
				idle();
				break;
			case STATE_CONFIG:
				config();
				break;
			case STATE_CONFIG_DATE:
				timeConfig();
				state = STATE_CONFIG;
				break;
			case STATE_CONFIG_ERASE_SCORES:
				state = (yesOrNo(SCORE_Erase) ? STATE_SCORES_ERASED : STATE_CONFIG);
				break;
			case STATE_CONFIG_NAME:
				getUsername();
				state = STATE_CONFIG;
				break;
			case STATE_SCORES_ERASED:
				scoresErased();
				state = STATE_CONFIG;
				break;
			case STATE_PREGAME:
				pregame();
				state = STATE_INGAME;
				break;
			case STATE_INGAME:
				game();
				state = STATE_POSTGAME;
				break;
			case STATE_POSTGAME:
				postgame();
				state = STATE_IDLE;
				break;
		}
	}

	vTaskDelete(NULL);
}

/**
 * Increment variable points each POINTS_RATE ms, all the time.
 */
void taskPOINTS(void *pvParameters) {
	for (;;) {
		WAIT_SYS_Ms(POINTS_RATE); // Delay of 1 second
		xSemaphoreTake(semPOINTS, portMAX_DELAY);
		points++; // Increment points.
		xSemaphoreGive(semPOINTS);
	}

	vTaskDelete(NULL);
}

/**
 * Decrement variable fuel each FUEL_RATE ms, all the time.
 */
void taskFUEL(void *pvParameters) {
	for (;;) {
		WAIT_SYS_Ms(FUEL_RATE); // Delay of 1 second
		xSemaphoreTake(semFUEL, portMAX_DELAY);
		fuel--; // Increment points.
		xSemaphoreGive(semFUEL);
	}

	vTaskDelete(NULL);
}

/**
 * Put the most recent Date Time Strings Structure in queueDATE
 */
void taskDATE(void *pvParameters) {
	DATE_TIME dateTimeStr;
	struct tm dateTime; // Structure in which RTC time will be written

	for (;;) {
		RTC_GetValue(&dateTime); // Write RTC time to structure

		char dayOfWeek[4];
		switch(dateTime.tm_wday) {
			case 0:
				strcpy(dayOfWeek, "Sun");
				break;
			case 1:
				strcpy(dayOfWeek, "Mon");
				break;
			case 2:
				strcpy(dayOfWeek, "Tue");
				break;
			case 3:
				strcpy(dayOfWeek, "Wed");
				break;
			case 4:
				strcpy(dayOfWeek, "Thu");
				break;
			case 5:
				strcpy(dayOfWeek, "Fri");
				break;
			case 6:
				strcpy(dayOfWeek, "Sat");
				break;
		}

		sprintf(dateTimeStr.date, "%s %02d/%02d/%04d", dayOfWeek, dateTime.tm_mday, dateTime.tm_mon + 1, dateTime.tm_year + 1900);
		//strftime(dateTimeStr.date, LCD_DISPLAY_COLUMNS, "%a %d/%m/%Y", &dateTime); // Send structure information to the buffer

		sprintf(dateTimeStr.time, "%02d:%02d:%02d", dateTime.tm_hour, dateTime.tm_min, dateTime.tm_sec);
		//strftime(dateTimeStr.time, LCD_DISPLAY_COLUMNS, "%H:%M:%S", &dateTime);
		xQueueOverwrite(queueDATE, &dateTimeStr); // Block if not in Idle state
	}

	vTaskDelete(NULL);
}

/**
 * Increment at current showing score SCORE_SHOW_TIME rate, all the time.
 */
void taskSCORE_COUNT(void * pvParameters) {
	for (;;) {
		for (int i = 0; i < SCORE_NUM; i++) { // Show scores
			scoreCount = i;
			WAIT_SYS_Ms(SCORE_SHOW_TIME);
		}
	}

	vTaskDelete(NULL);
}

/**
 * Increment at current showing score SCORE_SHOW_TIME rate, all the time.
 */
void taskBLINK(void * pvParameters) {
	for (;;) {
		WAIT_SYS_Ms(HIGHLIGHT_BLINK_TIME);
		blinked = !blinked;
	}

	vTaskDelete(NULL);
}

/**
 * Update game. This is called every GAME_RATE ms.
 */
void taskUPDATE_GAME(void * pvParameters) {
	/*
	 * Car:
	 */
	CAR car;

	/*
	 * Array that represents all current obstacles in LCD DDRAM
	 */
	int map[LCD_DISPLAY_ROWS][LCD_DDRAM_LENGTH];

	int currentFuelCol;

	GAME_INFO info;

	for (;;) {
		// Wait for Game Rate and player input:
		xQueueReceive(queueUPDATE_GAME, &info, portMAX_DELAY);

		if (info.init) {
			/*
			 * Reset points:
			 */
			xSemaphoreTake(semPOINTS, portMAX_DELAY);
			points = 1;
			xSemaphoreGive(semPOINTS);

			/*
			 * Reset fuel:
			 */
			xSemaphoreTake(semFUEL, portMAX_DELAY);
			fuel = MAX_FUEL;
			xSemaphoreGive(semFUEL);

			car.row = 1;
			car.last_row = 1;
			car.front_column = CAR_POSITION + 1;
			car.back_column = CAR_POSITION;
			memset(map, 0, sizeof(map));
			currentFuelCol = 1;

			randomiseObstacles(INITIAL_OBSTACLE_GAP, LCD_DDRAM_LENGTH, map); // Randomise obstacles
			randomiseFuel(INITIAL_OBSTACLE_GAP, LCD_DDRAM_LENGTH, map); // Randomise fuel gallons
		}

		// Update row:
		car.last_row = car.row;
		car.row = info.row;

		// Refresh obstacles and fuel galleons:
		if (car.back_column == CAR_POSITION + (LCD_DDRAM_LENGTH/2)) { // If display is showing [21, 36]
			// Refresh first 20:
			randomiseObstacles(1, LCD_DDRAM_LENGTH/2, map);
			randomiseFuel(1, LCD_DDRAM_LENGTH/2, map);
		}
		if (car.back_column == CAR_POSITION) { // If display is showing [1, 16]
			// Refresh last 20:
			randomiseObstacles((LCD_DDRAM_LENGTH/2) + 1, LCD_DDRAM_LENGTH, map);
			randomiseFuel((LCD_DDRAM_LENGTH/2) + 1, LCD_DDRAM_LENGTH, map);
		}

		// Shift car:
		shiftCar(&car);

		// Shift fuel indicator:
		shiftFuelIndicator(&currentFuelCol);

		// Shift display:
		LCDText_ShiftDisplay(LEFT);

		// Check if a fuel gallon was grabed:
		checkForFuelGrab(&car, map);

		// Verify if car hit an obstacle:
		if (checkForLoss(&car, map)) {
			xSemaphoreTake(semGAME_END, portMAX_DELAY);
			gameEnd = true;
			xSemaphoreGive(semGAME_END);
			SCORE_Save(points, username);
			explodeCar(&car);
		}
	}

	vTaskDelete(NULL);
}


/*
===========================================================================================================================================================
===========================================================================================================================================================

	NETWORK MANAGER:

===========================================================================================================================================================
===========================================================================================================================================================
*/

void taskNETWORK_MANAGER(void * pvParameters) {
	NETWORK_ConnectToAP(SSID, PASS);
	RTC_SetSeconds(NETWORK_GetSeconds()); // Get real seconds since EPOH
	vTaskDelete(NULL);
}



/*
===========================================================================================================================================================
===========================================================================================================================================================

	USERNAME:

===========================================================================================================================================================
===========================================================================================================================================================
*/

void getUsername(void) {

	LCDText_Locate(1, 1);
	LCDText_Printf("User name:");

	int volatile push_events = 0; // Buttons events
	int ch = 0; // [0, 10] -> Character selected, 11 is the special character '\0'
	char aux[NAME_LENGTH+1]; // Auxiliary variable for blinking

	while (1) {
		LCDText_Locate(2, 1);
		if (blinked) {
			strcpy(aux, username);
			aux[ch] = ' ';
			LCDText_Printf(aux);
		}
		else LCDText_Printf(username);

		if ((push_events = BUTTON_GetButtonsReleaseEvents(&bitmap)) != 0) {
			if (push_events & B1) { // Increment
				switch (username[ch]) {
					case 32:
						username[ch] = 97;
						break;
					case 122:
						username[ch] = 32;
						break;
					default:
						username[ch]++;
						break;
				}
			}
			else if (push_events & B2) { // Decrement
				switch (username[ch]) {
					case 32:
						username[ch] = 122;
						break;
					case 97:
						username[ch] = 32;
						break;
					default:
						username[ch]--;
						break;
				}
			}
			else if (push_events & B3) { // Confirm value
				if (ch == 0) username[0] -= 32; // First character must be upper case
				if (++ch > (NAME_LENGTH-1)) { // If it's the last character
					break; // Get out
				}
			}
		}
	}
}



/*
===========================================================================================================================================================
===========================================================================================================================================================

	DATE TIME:

===========================================================================================================================================================
===========================================================================================================================================================
*/

void printDateTime(RTC_TIME_FIELD without) {
	DATE_TIME dateTime;
	xQueuePeek(queueDATE, &dateTime, portMAX_DELAY);

	// Add desired blank field, if 'without' != 'NONE':
	switch(without) {
		case DOW:
			dateTime.date[0] = ' ';
			dateTime.date[1] = ' ';
			dateTime.date[2] = ' ';
			break;
		case DOM:
			dateTime.date[4] = ' ';
			dateTime.date[5] = ' ';
			break;
		case MONTH:
			dateTime.date[7] = ' ';
			dateTime.date[8] = ' ';
			break;
		case YEAR:
			dateTime.date[10] = ' ';
			dateTime.date[11] = ' ';
			dateTime.date[12] = ' ';
			dateTime.date[13] = ' ';
			break;
		default:
			break;
	}

	LCDText_Locate(1, 1);
	LCDText_Printf(dateTime.date); // Write first half to LCD

	switch(without) {
		case HOUR:
			dateTime.time[0] = ' ';
			dateTime.time[1] = ' ';
			break;
		case MIN:
			dateTime.time[3] = ' ';
			dateTime.time[4] = ' ';
			break;
		case SEC:
			dateTime.time[6] = ' ';
			dateTime.time[7] = ' ';
			break;
		default:
			break;
	}

	LCDText_Locate(2, 1);
	LCDText_Printf(dateTime.time); // Write second half to LCD
}



/*
===========================================================================================================================================================
===========================================================================================================================================================

	SCORE:

===========================================================================================================================================================
===========================================================================================================================================================
*/

void printScore(Score *score, int n) {
	char card[3];
	switch (n) {
		case 0:
			strcpy(card, "st");
			break;
		case 1:
			strcpy(card, "nd");
			break;
		case 2:
			strcpy(card, "rd");
			break;
	}
	if (score->score == 0) {
		LCDText_Locate(1, 1);
		LCDText_Printf("%d%s place:", n+1, card);
		LCDText_Locate(2, 1);
		LCDText_Printf("Not defined.    ");
	}
	else {
		LCDText_Locate(1, 1);
		LCDText_Printf("%d%s place:", n+1, card);
		LCDText_Locate(2, 1);
		LCDText_Printf("%dP %s", score->score, score->name);
	}
}



/*
===========================================================================================================================================================
===========================================================================================================================================================

	IDLE MODE:

===========================================================================================================================================================
===========================================================================================================================================================
*/

/*
 * 	Function to run in 'BUTTON_WaitRelease()'.
 */
static void idleWait(void) {
	printDateTime(NONE);
}

void idle(void) {

   	int volatile side = 0; // 0 to show time, 1 to show best score
   	int volatile push_events = 0; // Buttons events

   	BUTTON_WaitRelease(idleWait);

   	while (1) {
   		Score score;

   		// State Machine:
   		switch (side) {
   			case 0: // Showing time
   				printDateTime(NONE);
				break;
   			case 1: // Showing best score
   				SCORE_Get(&score, scoreCount);
				printScore(&score, scoreCount);
				break;
   		}

   		// Input Handler:
   		if ((push_events = BUTTON_GetButtonsPushEvents(&bitmap)) != 0) {
			if (push_events & B1) { // Show time
				if (side != 0) {
					side = 0;
					LCDText_Clear();
				}
			}
			else if (push_events & B2) { // Show best score
				if (side != 1) {
					side = 1;
					LCDText_Clear();
				}
			}
			else if (push_events & B3) { // Start a game
				LCDText_Clear();
				state = STATE_PREGAME;
				return;
			}
   		}
   		if ((bitmap & B1) && (bitmap & B2)) { // Check for buttons hold, and enter Configuration Mode
			LCDText_Clear();
			LCDText_Locate(1, 1);
			LCDText_Printf("Entering config");
			LCDText_Locate(2, 1);
			LCDText_Printf("mode...");

			uint32_t start = WAIT_SYS_GetElapsedMs(0);
			bool exit = false;
			while (WAIT_SYS_GetElapsedMs(start) < BUTTONS_HOLD_TIME) {
				if (!(BUTTON_Hit() & B1) || !(BUTTON_Hit() & B2)) { // If any of the buttons were released
					exit = true;
					break;
				}
			}

			if (exit) continue;

			state = STATE_CONFIG; // If user held the buttons until this point, enter Configuration Mode
			return;
		}
	}
}



/*
===========================================================================================================================================================
===========================================================================================================================================================

	GAME MODE:

===========================================================================================================================================================
===========================================================================================================================================================
*/

void printToCol(int col, unsigned char c1, unsigned char c2) {
	LCDText_Locate(1, col);
	LCDText_WriteChar(c1);
	LCDText_Locate(2, col);
	LCDText_WriteChar(c2);
}

void shiftFuelIndicator(int * currentFuelCol) {
	// Erase current fuel indicator:
	LCDText_Locate(1, *currentFuelCol);
	LCDText_WriteChar(' ');
	LCDText_Locate(2, *currentFuelCol);
	LCDText_WriteChar(' ');

	// Increment fuel indicator position:
	if (++*currentFuelCol > LCD_DDRAM_LENGTH) *currentFuelCol = 1;

	// Print fuel indicator to LCD:
	switch(fuel) {
		case 8:
			printToCol(*currentFuelCol, FUEL4_CHAR, FUEL4_CHAR);
			break;
		case 7:
			printToCol(*currentFuelCol, FUEL3_CHAR, FUEL4_CHAR);
			break;
		case 6:
			printToCol(*currentFuelCol, FUEL2_CHAR, FUEL4_CHAR);
			break;
		case 5:
			printToCol(*currentFuelCol, FUEL1_CHAR, FUEL4_CHAR);
			break;
		case 4:
			printToCol(*currentFuelCol, ' ', FUEL4_CHAR);
			break;
		case 3:
			printToCol(*currentFuelCol, ' ', FUEL3_CHAR);
			break;
		case 2:
			printToCol(*currentFuelCol, ' ', FUEL2_CHAR);
			break;
		case 1:
			printToCol(*currentFuelCol, ' ', FUEL1_CHAR);
			break;
		default:
			printToCol(*currentFuelCol, ' ', ' ');
			break;
	}
}

void shiftCar(CAR *car) {
	// Erase current car:
	LCDText_Locate(car->last_row, car->back_column);
	LCDText_WriteChar(' ');
	LCDText_Locate(car->last_row, car->front_column);
	LCDText_WriteChar(' ');

	// Increment car position:
	if (++car->back_column > LCD_DDRAM_LENGTH) car->back_column = 1;
	if (++car->front_column > LCD_DDRAM_LENGTH) car->front_column = 1;

	// Write car:
	LCDText_Locate(car->row, car->back_column);
	LCDText_WriteChar(CAR_BACK_CHAR);
	LCDText_Locate(car->row, car->front_column);
	LCDText_WriteChar(CAR_FRONT_CHAR);

	// Update last_row:
	car->last_row = car->row;
}

void randomiseFuel(int from, int to, int map[LCD_DISPLAY_ROWS][LCD_DDRAM_LENGTH]) {
	int gaps[LCD_DDRAM_LENGTH] = {0};
	int ct = 0;
	for (int i = from; i <= to; i++) {
		if ((map[0][i-1] == 0) && (map[1][i-1] == 0)) {
			gaps[ct++] = i-1;
		}
	}
	srand(RTC_GetSeconds()); // Set randomiser seed to RTC time_t value

	int col = gaps[rand()%ct];
	int row = rand()%2;

	// Write obstacle in LCD:
	LCDText_Locate(row+1, col+1);
	LCDText_WriteChar(FUEL_CHAR);

	map[row][col] = 2;
}

void randomiseObstacles(int from, int to, int map[LCD_DISPLAY_ROWS][LCD_DDRAM_LENGTH]) {
	for (int i = from; i <= to; i++) {
		// Erase obstacles from array of obstacles:
		map[0][i-1] = 0;
		map[1][i-1] = 0;

		// Erase obstacles from LCD:
		LCDText_Locate(1, i);
		LCDText_WriteChar(' ');
		LCDText_Locate(2, i);
		LCDText_WriteChar(' ');
	}
	srand(RTC_GetSeconds()); // Set randomiser seed to RTC time_t value

	// Randomise first column:
	int col = from;
	col += rand()%3+3; // Always leave a space of 2-4 columns between each obstacle

	int row;

	while(col <= to) {
		// Randomise row:
		row = rand()%2+1;

		// Write obstacle in LCD:
		LCDText_Locate(row, col);
		LCDText_WriteChar(BARRIER_CHAR);
		// Write obstacle in array of obstacles:
		map[row-1][col-1] = 1;

		// Randomise next column:
		col += rand()%3+3; // Always leave a space of 2-4 columns between each obstacle
	}
}

void checkForFuelGrab(CAR *car, int map[LCD_DISPLAY_ROWS][LCD_DDRAM_LENGTH]) {
	if ((map[car->row-1][car->back_column-1] == 2)) {
		xSemaphoreTake(semFUEL, portMAX_DELAY);
		fuel += 3;
		if (fuel > MAX_FUEL) fuel = MAX_FUEL;
		map[car->row-1][car->back_column-1] = 0;
		xSemaphoreGive(semFUEL);
	}
	else if ((map[car->row-1][car->front_column-1] == 2)) {
		xSemaphoreTake(semFUEL, portMAX_DELAY);
		fuel += 3;
		if (fuel > MAX_FUEL) fuel = MAX_FUEL;
		map[car->row-1][car->front_column-1] = 0;
		xSemaphoreGive(semFUEL);
	}
}

bool checkForLoss(CAR *car, int map[LCD_DISPLAY_ROWS][LCD_DDRAM_LENGTH]) {
	return (map[car->row-1][car->back_column-1] == 1) || (map[car->row-1][car->front_column-1] == 1) || (fuel <= 0);
}

void explodeCar(CAR * car) {
	// Write car:
	LCDText_Locate(car->row, car->back_column);
	LCDText_WriteChar('*');
	LCDText_Locate(car->row, car->front_column);
	LCDText_WriteChar('*');
}

void pregame(void) {
	LCDText_Locate(1, 1);
	LCDText_Printf("Press any button");
	LCDText_Locate(2, 1);
	LCDText_Printf("to start.");

	BUTTON_WaitRelease(0);

	BUTTON_Read(); // Wait for user to ready up
	LCDText_Clear();
}

void game(void) {
	AXIS axis;
	GAME_INFO info = {.init = true, .row = 1};

	xSemaphoreTake(semGAME_END, portMAX_DELAY);
	gameEnd = false;
	xSemaphoreGive(semGAME_END);

	xQueueSend(queueUPDATE_GAME, &info, portMAX_DELAY);
	info.init = false;

	for (;;) {
		WAIT_SYS_Ms(GAME_RATE);
		if (gameEnd) break;
		axis = ADXL_GetAxis();
		if (axis.y < -INCLINATION_THRESHOLD) info.row = 1;
		if (axis.y > INCLINATION_THRESHOLD) info.row = 2;
		xQueueSend(queueUPDATE_GAME, &info, portMAX_DELAY);
	}
}

void postgame(void) {
	LCDText_Home();
	LCDText_Printf("Game over. You");
	LCDText_Locate(2, 1);
	LCDText_Printf("scored %d!", points);

	BUTTON_WaitRelease(0);
	BUTTON_Read();
}



/*
===========================================================================================================================================================
===========================================================================================================================================================

	CONFIGURATION MODE

===========================================================================================================================================================
===========================================================================================================================================================
*/

void config(void) {
	LCDText_Locate(1, 1);
	LCDText_Printf("> Time Config");
	LCDText_Locate(2, 1);
	LCDText_Printf("  Score Config");

	BUTTON_WaitRelease(0);

	int volatile push_events = 0; // Buttons events
	int volatile last_option = 0; // Last selected option
	int volatile option = 0; // Current selected option

	while (1) {
		// State Machine:
		switch (option) {
			case 0: // Time Configuration option highlighted
				LCDText_Locate(1, 1);
				LCDText_Printf("> Time Config ");
				LCDText_Locate(2, 1);
				LCDText_Printf("  Score Config");
				break;
			case 1: // Best Score Configuration option highlighted
				switch(last_option) {
					case 0:
						LCDText_Locate(1, 1);
						LCDText_Printf("  Time Config ");
						LCDText_Locate(2, 1);
						LCDText_Printf("> Score Config");
						break;
					case 2:
						LCDText_Locate(1, 1);
						LCDText_Printf("> Score Config");
						LCDText_Locate(2, 1);
						LCDText_Printf("  Name Config ");
						break;
				}
				break;
			case 2:
				LCDText_Locate(1, 1);
				LCDText_Printf("  Score Config");
				LCDText_Locate(2, 1);
				LCDText_Printf("> Name Config ");
		}

		// Input Handler:
		if ((push_events = BUTTON_GetButtonsReleaseEvents(&bitmap)) != 0) {
			if (push_events & B1) { // Go to upper option
				last_option = option;
				if (++option > (CONFIG_OPTIONS-1)) option = CONFIG_OPTIONS-1;
			}
			else if (push_events & B2) { // Go to lower option
				last_option = option;
				if (--option < 0) option = 0;
			}
			else if (push_events & B3) { // Select option
				switch (option) {
					case 0: // Time Configuration:
						state = STATE_CONFIG_DATE;
						return;
					case 1: // Score Configuration:
						state = STATE_CONFIG_ERASE_SCORES;
						return;
					case 2: // Name Configuration:
						username[0] += 32; // Lower case
						state = STATE_CONFIG_NAME;
						return;
				}
			}
		}
		if ((bitmap & B1) && (bitmap & B2)) { // Check for buttons hold, and go back to menu
			LCDText_Clear();
			LCDText_Locate(1, 1);
			LCDText_Printf("Returning to");
			LCDText_Locate(2, 1);
			LCDText_Printf("menu...");

			uint32_t start = WAIT_SYS_GetElapsedMs(0);
			bool exit = false;

			while (WAIT_SYS_GetElapsedMs(start) < BUTTONS_HOLD_TIME) {
				if (!(BUTTON_Hit() & B1) || !(BUTTON_Hit() & B2)) { // If any of the buttons were released
					exit = true;
					break;
				}
			}

			if (exit) continue;

			state = STATE_IDLE; // If user held the buttons until this point, return to Idle Mode
			return;
		}
	}
}


/*
===========================================================================================================================================================
===========================================================================================================================================================

	TIME CONFIGURATION:

===========================================================================================================================================================
===========================================================================================================================================================
*/

void timeConfig(void) {

	RTC_Disable(); // Disable RTC counters, to stop the clock while adjusting it

	int volatile push_events = 0; // Buttons events

	int field = 0; // [0, 5] -> Every time field, excluding day of year and seconds
	RTC_TIME_FIELD timeFields[TIME_CONFIG_FIELDS] = {YEAR, MONTH, DOM, DOW, HOUR, MIN};

	while (1) {
		printDateTime(blinked ? timeFields[field] : NONE);

		if ((push_events = BUTTON_GetButtonsReleaseEvents(&bitmap)) != 0) {
			if (push_events & B1) { // Increment
				RTC_IncrementField(timeFields[field]);
			}
			else if (push_events & B2) { // Decrement
				RTC_DecrementField(timeFields[field]);
			}
			else if (push_events & B3) { // Confirm value
				if (++field > (TIME_CONFIG_FIELDS-1)) { // If it's the last field
					RTC_Enable(); // Enable RTC counters again
					break; // Get out
				}
			}
		}
	}
}


/*
===========================================================================================================================================================
===========================================================================================================================================================

	SCORE CONFIGURATION:

===========================================================================================================================================================
===========================================================================================================================================================
*/

void scoresErased(void) {
	LCDText_Locate(1, 1);
	LCDText_Printf("Erased. Press");
	LCDText_Locate(2, 1);
	LCDText_Printf("any button.");

	BUTTON_WaitRelease(0);
	BUTTON_Read();
}

bool yesOrNo(void(*f)(void)) {

	LCDText_Locate(1, 1);
	LCDText_Printf("> Yes");
	LCDText_Locate(2, 1);
	LCDText_Printf("  No");

	BUTTON_WaitRelease(0);

	int volatile push_events = 0; // Buttons events
	int volatile option = 0; // 0 for yes, 1 for no

	while (1) {
		// State Machine:
		switch (option) {
			case 0: // Yes option highlighted
				LCDText_Locate(1, 1);
				LCDText_Printf("> Yes");
				LCDText_Locate(2, 1);
				LCDText_Printf("  No");
				break;
			case 1: // No option highlighted
				LCDText_Locate(1, 1);
				LCDText_Printf("  Yes");
				LCDText_Locate(2, 1);
				LCDText_Printf("> No");
				break;
		}

		// Input Handler:
		if ((push_events = BUTTON_GetButtonsReleaseEvents(&bitmap)) != 0) {
			if (push_events & B1) { // Go to upper option
				if (++option > 1) option = 1;
			}
			else if (push_events & B2) { // Go to lower option
				if (--option < 0) option = 0;
			}
			else if (push_events & B3) { // Select option
				switch (option) {
					case 0: // Yes:
						(*f)();
						BUTTON_WaitRelease(0);
						return true;
					case 1: // No:
						return false;
				}
			}
		}
	}
}



/*
===========================================================================================================================================================
===========================================================================================================================================================

	FREERTOS UTILS:

===========================================================================================================================================================
===========================================================================================================================================================
*/


/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
	/* This function will get called if a task overflows its stack. */

	( void ) pxTask;
	( void ) pcTaskName;

	for( ;; );
}
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

void vConfigureTimerForRunTimeStats(void) {
	const unsigned long TCR_COUNT_RESET = 2, CTCR_CTM_TIMER = 0x00, TCR_COUNT_ENABLE = 0x01;

	/* This function configures a timer that is used as the time base when
	collecting run time statistical information - basically the percentage
	of CPU time that each task is utilising.  It is called automatically when
	the scheduler is started (assuming configGENERATE_RUN_TIME_STATS is set
	to 1). */

	/* Power up and feed the timer. */
	LPC_SC->PCONP |= (1 << 2);
	LPC_SC->PCLKSEL0 = (LPC_SC->PCLKSEL0 & (~(0x3<<4))) | (0x01 << 4);

	/* Reset Timer 1 */
	LPC_TIM1->TCR = TCR_COUNT_RESET;

	/* Just count up. */
	LPC_TIM1->CTCR = CTCR_CTM_TIMER;

	/* Prescale to a frequency that is good enough to get a decent resolution,
	but not too fast so as to overflow all the time. */
	LPC_TIM1->PR =  ( configCPU_CLOCK_HZ / 10000UL ) - 1UL;

	/* Start the counter. */
	LPC_TIM1->TCR = TCR_COUNT_ENABLE;
}
/*-----------------------------------------------------------*/
