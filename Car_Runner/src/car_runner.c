/*
===============================================================================
 Name        : car_runner.c
 Author      : PedroG
 Version     : 1.0
 Copyright   : PedroG
 Description : LCD Car game
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include "car_runner.h"


/*
 * 	Dummy function.
 */
void dummy(void) {}


/*
===========================================================================================================================================================
===========================================================================================================================================================

	STATE MACHINE:

===========================================================================================================================================================
===========================================================================================================================================================
*/

void stateMachine(void) {
	LCDText_Clear();
	switch (state) {
		case IDLE:
			idle();
			break;
		case CONFIG:
			config();
			break;
		case CONFIG_DATE:
			timeConfig();
			state = CONFIG;
			break;
		case CONFIG_ERASE_SCORES:
			state = (yesOrNo(SCORE_Erase) ? SCORES_ERASED : CONFIG);
			break;
		case CONFIG_NAME:
			getUsername();
			state = CONFIG;
			break;
		case SCORES_ERASED:
			scoresErased();
			state = CONFIG;
			break;
		case PREGAME:
			pregame();
			state = GAME;
			break;
		case GAME:
			game();
			state = POSTGAME;
			break;
		case POSTGAME:
			postgame();
			state = IDLE;
			break;
	}
}


/*
===========================================================================================================================================================
===========================================================================================================================================================

	USERNAME:

===========================================================================================================================================================
===========================================================================================================================================================
*/

/*
 *  Refers to the highlighted field (for blinking
 * purposes).
 */
bool off;

/*
 * 	Handler for the blinking delay.
 * 	Will invert 'off' (for blinking purposes).
 */
void blinkHandler(void) {
	off = !off;
}

void getUsername(void) {

	LCDText_Locate(1, 1);
	LCDText_Printf("User name:");

	off = false;
	int volatile push_events = 0; // Buttons events
	int ch = 0; // [0, 10] -> Character selected, 11 is the special character '\0'
	char aux[NAME_LENGTH+1]; // Auxiliary variable for blinking

	while (1) {
		if (!WAIT_IsBusy(TIM0)) WAIT_IRQ0_Us(HIGHLIGHT_BLINK_TIME, blinkHandler); // If it's not busy, start counting

		LCDText_Locate(2, 1);
		if (off) {
			strcpy(aux, username);
			aux[ch] = ' ';
			LCDText_Printf(aux);
		}
		else LCDText_Printf(username);

		if ((push_events = BUTTON_GetButtonsReleaseEvents()) != 0) {
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
	while (WAIT_IsBusy(TIM0)); // Wait for timer0
}


/*
===========================================================================================================================================================
===========================================================================================================================================================

	TIME:

===========================================================================================================================================================
===========================================================================================================================================================
*/

void updateAndPrintTime(RTC_TIME_FIELD without) {

	struct tm dateTime; // Structure in which RTC time will be written
	char buf[LCD_DISPLAY_COLUMNS]; // Buffer that will hold the time information

	RTC_GetValue(&dateTime); // Write RTC time to structure

	strftime(buf, LCD_DISPLAY_COLUMNS, "%a %d/%m/%Y", &dateTime); // Send structure information to the buffer

	// Add desired blank field, if 'without' != 'NONE':
	switch(without) {
		case DOW:
			buf[0] = ' ';
			buf[1] = ' ';
			buf[2] = ' ';
			break;
		case DOM:
			buf[4] = ' ';
			buf[5] = ' ';
			break;
		case MONTH:
			buf[7] = ' ';
			buf[8] = ' ';
			break;
		case YEAR:
			buf[10] = ' ';
			buf[11] = ' ';
			buf[12] = ' ';
			buf[13] = ' ';
			break;
		default:
			break;
	}
	LCDText_Locate(1, 1);
	LCDText_Printf(buf); // Write first half to LCD

	strftime(buf, LCD_DISPLAY_COLUMNS, "%H:%M:%S", &dateTime);
	switch(without) {
		case HOUR:
			buf[0] = ' ';
			buf[1] = ' ';
			break;
		case MIN:
			buf[3] = ' ';
			buf[4] = ' ';
			break;
		case SEC:
			buf[6] = ' ';
			buf[7] = ' ';
			break;
		default:
			break;
	}
	LCDText_Locate(2, 1);
	LCDText_Printf(buf); // Write second half to LCD
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
 * 	Current score showing in LCD
 */
int currentScore;

/*
 * 	Handler for current score delay.
 * 	Will increment current score.
 */
void currentScoreHandler(void) {
	if (++currentScore > SCORE_NUM-1) currentScore = 0;
}

/*
 * 	Function to run in 'BUTTON_WaitRelease()'.
 */
void idleWait(void) {
	updateAndPrintTime(NONE);
}

void idle(void) {
	currentScore = 0;

   	int volatile side = 0; // 0 to show time, 1 to show bets score
   	int volatile push_events = 0; // Buttons events

   	BUTTON_WaitRelease(idleWait);

   	while (1) {
   		Score bestScore;

   		// State Machine:
   		switch (side) {
   			case 0: // Showing time
				updateAndPrintTime(NONE);
				break;
   			case 1: // Showing best score
   				if (!WAIT_IsBusy(TIM1)) WAIT_IRQ1_Us(SCORE_SHOW_TIME, currentScoreHandler); // Call 'currentScoreHandler()' in 'SCORE_SHOW_TIME' microseconds
   				SCORE_Get(&bestScore, currentScore);
				printScore(&bestScore, currentScore);
				break;
   		}

   		// Input Handler:
   		if ((push_events = BUTTON_GetButtonsReleaseEvents()) != 0) {
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
				while (WAIT_IsBusy(TIM0) || WAIT_IsBusy(TIM1)) { // Wait for Timer0 and Timer1 before starting game
					LCDText_Locate(1, 1);
					LCDText_Printf("Starting game...");
				}
				state = PREGAME;
				return;
			}
   		}
   		if ((BUTTON_Hit() & B1) && (BUTTON_Hit() & B2)) { // Check for buttons hold, and enter Configuration Mode

			if (WAIT_IsBusy(TIM0)) continue; // If previous Timer0 interrupt hasn't arrived, get out

			LCDText_Clear();
			LCDText_Locate(1, 1);
			LCDText_Printf("Entering config");
			LCDText_Locate(2, 1);
			LCDText_Printf("mode...");

			WAIT_IRQ0_Us(BUTTONS_HOLD_TIME, dummy); // Interrupt in BUTTONS_HOLD_TIME microseconds
			while (WAIT_IsBusy(TIM0)) {
				if (!(BUTTON_Hit() & B1) || !(BUTTON_Hit() & B2)) { // If any of the buttons were released
					break;
				}
			}
			if (!WAIT_IsBusy(TIM0)) {
				state = CONFIG; // If user held the buttons until this point, enter Configuration Mode
				return;
			}
			LCDText_Clear();
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

/*
 * 	Current points.
 */
int points;

/*
 * 	Current speed.
 */
int fuel;

/*
 * 	Interrupt handler for points increase delay:
 */
void busyPointsHandler(void) {
	points++;
	fuel--;
}

void changeRow(CAR * car, int row) {
	if ((row != 1) && (row != 2)) return;
	car->row = row;
}

int getInclination(void) {
	signed short coords[] = {0, 0, 0};
	ADXL_GetAxis(coords);

	if (coords[1] < -INCLINATION_THRESHOLD) return 1;
	if (coords[1] > INCLINATION_THRESHOLD) return 2;
	return 0;
}

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
		fuel += 3;
		if (fuel > MAX_FUEL) fuel = MAX_FUEL;
		map[car->row-1][car->back_column-1] = 0;
	}
	else if ((map[car->row-1][car->front_column-1] == 2)) {
		fuel += 3;
		if (fuel > MAX_FUEL) fuel = MAX_FUEL;
		map[car->row-1][car->front_column-1] = 0;
	}
}

bool checkForLoss(CAR *car, int map[LCD_DISPLAY_ROWS][LCD_DDRAM_LENGTH]) {
	return (map[car->row-1][car->back_column-1] == 1) || (map[car->row-1][car->front_column-1] == 1) || (fuel <= 0);
}

void pregame(void) {
	LCDText_Locate(1, 1);
	LCDText_Printf("Press any button");
	LCDText_Locate(2, 1);
	LCDText_Printf("to start.");

	BUTTON_Read(); // Wait for user to ready up
}

void game(void) {

	// Car:
	CAR car = {.row = 1, .last_row = 1, .front_column = CAR_POSITION + 1, .back_column = CAR_POSITION};

	// Array that represents all current obstacles in LCD DDRAM
	int map[LCD_DISPLAY_ROWS][LCD_DDRAM_LENGTH] = {0};

	int currentFuelCol = 1;

	points = 1;
	fuel = MAX_FUEL;

	randomiseObstacles(INITIAL_OBSTACLE_GAP, LCD_DDRAM_LENGTH, map); // Randomise obstacles
	randomiseFuel(INITIAL_OBSTACLE_GAP, LCD_DDRAM_LENGTH, map); // Randomise fuel galleons

	while(1) {
		changeRow(&car, getInclination());

		if (!WAIT_IsBusy(TIM0)) {
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

			// Verify if car hit an obstacle:
			if (checkForLoss(&car, map)) break; // Lose

			checkForFuelGrab(&car, map);

			// Game delay:
			WAIT_IRQ0_Us(GAME_RATE, dummy);
		}
		if (!WAIT_IsBusy(TIM1)) WAIT_IRQ1_Us(FUEL_RATE, busyPointsHandler); // Points delay
	}
}

void postgame(void) {
	LCDText_Home();
	LCDText_Printf("Game over. You");
	LCDText_Locate(2, 1);
	LCDText_Printf("scored %d!", points);

	SCORE_Save(points, username, NAME_LENGTH+1);

	BUTTON_WaitRelease(0);
	BUTTON_Read();

	while(WAIT_IsBusy(TIM0) || WAIT_IsBusy(TIM1)) { // Wait for Timer0 and Timer1
		LCDText_Locate(1, 1);
		LCDText_Printf("Returning");
		LCDText_Locate(2, 1);
		LCDText_Printf("to menu...");
	}
}


/*
===========================================================================================================================================================
===========================================================================================================================================================

	CONFIGURATION NAME

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
		if ((push_events = BUTTON_GetButtonsReleaseEvents()) != 0) {
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
						while(WAIT_IsBusy(TIM0)); // Wait for Timer0
						state = CONFIG_DATE;
						return;
					case 1: // Score Configuration:
						while(WAIT_IsBusy(TIM0)); // Wait for Timer0
						state = CONFIG_ERASE_SCORES;
						return;
					case 2: // Name Configuration:
						while(WAIT_IsBusy(TIM0)); // Wait for Timer0
						username[0] += 32; // Lower case
						state = CONFIG_NAME;
						return;
				}
			}
		}
		if ((BUTTON_Hit() & B1) && (BUTTON_Hit() & B2)) { // Check for buttons hold, and go back to menu
			if (WAIT_IsBusy(TIM0)) continue; // If previous interrupt hasn't arrived, get out

			LCDText_Clear();
			LCDText_Locate(1, 1);
			LCDText_Printf("Returning to");
			LCDText_Locate(2, 1);
			LCDText_Printf("menu...");

			WAIT_IRQ0_Us(BUTTONS_HOLD_TIME, dummy); // Interrupt in 2 seconds
			while (WAIT_IsBusy(TIM0)) {
				if (!(BUTTON_Hit() & B1) || !(BUTTON_Hit() & B2)) { // If any of the buttons were released
					break;
				}
			}
			if (!WAIT_IsBusy(TIM0)) { // If user held the buttons until this point, return to Idle Mode
				state = IDLE;
				return;
			}
			LCDText_Clear();
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
		if (!WAIT_IsBusy(TIM0)) WAIT_IRQ0_Us(HIGHLIGHT_BLINK_TIME, blinkHandler); // If it's not busy, start counting

		if (off) updateAndPrintTime(timeFields[field]); // Set field to blank
		else updateAndPrintTime(NONE); // Set field to its current value

		if ((push_events = BUTTON_GetButtonsReleaseEvents()) != 0) {
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
	while(WAIT_IsBusy(TIM0));
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
		if ((push_events = BUTTON_GetButtonsReleaseEvents()) != 0) {
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

	MAIN:

===========================================================================================================================================================
===========================================================================================================================================================
*/

/*
 * 	Main:
 */
int main(void) {

 	SystemInit();
	BUTTON_Init();
	RTC_Init(0, INONE);
	if (LCDText_Init() < 0) {
		printf("LCD could not initialise.\n");
		return 0;
	}
	if (WAIT_Init(IRQ0) < 0) {
		printf("WAIT Resource 0 could not initialise.\n");
		return 0;
	}
	if (WAIT_Init(IRQ1) < 0) {
		printf("WAIT Resource 1 could not initialise.\n");
		return 0;
	}
	ADXL_Init(0, 0);
	SCORE_Init(EEPROM);

	LCDText_CreateChar(CAR_BACK_CHAR, car_back_charmap);
	LCDText_CreateChar(CAR_FRONT_CHAR, car_front_charmap);
	LCDText_CreateChar(BARRIER_CHAR, barrier_charmap);
	LCDText_CreateChar(FUEL_CHAR, fuel_charmap);
	LCDText_CreateChar(FUEL1_CHAR, fuel1_charmap);
	LCDText_CreateChar(FUEL2_CHAR, fuel2_charmap);
	LCDText_CreateChar(FUEL3_CHAR, fuel3_charmap);
	LCDText_CreateChar(FUEL4_CHAR, fuel4_charmap);

	for (int i = 0; i < NAME_LENGTH; i++) {
		username[i] = 'a';
	}
	username[NAME_LENGTH] = '\0';

	getUsername();

	while(1) {
		stateMachine();
	}

    return 0 ;
}
