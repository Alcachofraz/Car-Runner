/*
 * car_runner_rtos.h
 *
 *  Created on: 24 Jun 2021
 *      Author: PedroG
 */

#ifndef CAR_RUNNER_RTOS_H_
#define CAR_RUNNER_RTOS_H_

/** @defgroup CAR_RUNNER_RTOS CAR_RUNNER_RTOS
 * This package provides the Car Runner Mini-Game in RTOS capabilities.
 * @{
 */

/** @defgroup CAR_RUNNER_RTOS_Public_Functions CAR_RUNNER_RTOS Public Functions
 * @{
 */


/**
 *
 *
 * Includes
 *
 *
 */

#include <network.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "score.h"

#include "wait.h"
#include "lcd.h"
#include "adxl.h"
#include "button.h"
#include "rtc.h"
#include "led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"



/*
===========================================================================================================================================================
===========================================================================================================================================================

	TASKS PRIORITY:

===========================================================================================================================================================
===========================================================================================================================================================
*/

/**
 * @note	Accelerometer Task Priority.
 */
#define TASK_ACC_PRIORITY tskIDLE_PRIORITY + 1

/**
 * @note	Date Task Priority.
 */
#define TASK_DATE_PRIORITY tskIDLE_PRIORITY + 1

/**
 * @note	Points Task Priority.
 */
#define TASK_POINTS_PRIORITY tskIDLE_PRIORITY + 1

/**
 * @note	State Machine Task Priority.
 */
#define TASK_STATE_MACHINE_PRIORITY tskIDLE_PRIORITY + 1

/**
 * @note	Score Count Task Priority.
 */
#define TASK_SCORE_COUNT_PRIORITY tskIDLE_PRIORITY + 1

/**
 * @note	Update Game Task Priority.
 */
#define TASK_UPDATE_GAME_PRIORITY tskIDLE_PRIORITY + 1

/**
 * @note	Fuel Task Priority.
 */
#define TASK_FUEL_PRIORITY tskIDLE_PRIORITY + 1

/**
 * @note	Blink Task Priority.
 */
#define TASK_BLINK_PRIORITY tskIDLE_PRIORITY + 1

/**
 * @note	Blink Task Priority.
 */
#define TASK_NETWORK_MANAGER_PRIORITY tskIDLE_PRIORITY + 1



/*
===========================================================================================================================================================
===========================================================================================================================================================

	TASKS STACK SIZE:

===========================================================================================================================================================
===========================================================================================================================================================
*/

/**
 * @note	Accelerometer Task Stack Size.
 */
#define TASK_ACC_STACK_SIZE configMINIMAL_STACK_SIZE*1

/**
 * @note	Date Task Stack Size.
 */
#define TASK_DATE_STACK_SIZE configMINIMAL_STACK_SIZE*2

/**
 * @note	Points Task Stack Size.
 */
#define TASK_POINTS_STACK_SIZE configMINIMAL_STACK_SIZE*0.5

/**
 * @note	State Machine Task Stack Size.
 */
#define TASK_STATE_MACHINE_STACK_SIZE configMINIMAL_STACK_SIZE*3

/**
 * @note	Score Count Task Stack Size.
 */
#define TASK_SCORE_COUNT_STACK_SIZE configMINIMAL_STACK_SIZE*0.5

/**
 * @note	Update Game Task Stack Size.
 */
#define TASK_UPDATE_GAME_STACK_SIZE configMINIMAL_STACK_SIZE*5

/**
 * @note	Fuel Task Stack Size.
 */
#define TASK_FUEL_STACK_SIZE configMINIMAL_STACK_SIZE*0.5

/**
 * @note	Blink Task Stack Size.
 */
#define TASK_BLINK_STACK_SIZE configMINIMAL_STACK_SIZE*0.5

/**
 * @note	Blink Task Stack Size.
 */
#define TASK_NETWORK_MANAGER_STACK_SIZE configMINIMAL_STACK_SIZE*2

/*
===========================================================================================================================================================
===========================================================================================================================================================

	APPLICATION RELATED CONSTANTS:

===========================================================================================================================================================
===========================================================================================================================================================
*/

/**
 * @brief	Number of entries in Configuration Mode menu.
 * @note    Shouldn't be changed.
 */
#define CONFIG_OPTIONS 3

/**
 * @brief	Number of time fields to configure in Time Configuration Mode.
 * @note    Shouldn't be changed.
 */
#define TIME_CONFIG_FIELDS 6

/**
 * @brief	Necessary time to hold buttons, in order to enter/exit Configuration Mode.
 */
#define BUTTONS_HOLD_TIME 2000

/**
 * @brief	Time between different scores being displayed, in Idle Mode.
 */
#define SCORE_SHOW_TIME 3000

/**
 * @brief	Blinking time, used to highlight fields.
 */
#define HIGHLIGHT_BLINK_TIME 300

/**
 * @brief	Points increment rate, in Game Mode.
 */
#define POINTS_RATE 1000

/**
 * @brief	Game update rate (i.e. time in us between each "frame"), in Game Mode.
 */
#define GAME_RATE 300

/**
 * @brief	Fuel tank decrement rate, in Game Mode.
 */
#define FUEL_RATE 2000

/**
 * @brief   Column in which user will visualise the car (should be < 10).
 */
#define CAR_POSITION 3

/**
 * @brief   Columns with no obstacles as of starting the game.
 */
#define INITIAL_OBSTACLE_GAP CAR_POSITION + 5

/**
 * @brief   Maximum fuel.
 * @note    Shouldn't be changed.
 */
#define MAX_FUEL 8

/**
 * @brief	Value ADXL345's Y-Axis needs to surpass, to count as row change.
 */
#define INCLINATION_THRESHOLD 60

/**
 * @brief	LCD special char for Car Back.
 * @note    Shouldn't be changed.
 */
#define CAR_BACK_CHAR 0

/**
 * @brief	LCD special char for Car Front.
 * @note    Shouldn't be changed.
 */
#define CAR_FRONT_CHAR 1

/**
 * @brief	LCD special char for Barrier.
 * @note    Shouldn't be changed.
 */
#define BARRIER_CHAR 2

/**
 * @brief	LCD special char for Fuel Galleon.
 * @note    Shouldn't be changed.
 */
#define FUEL_CHAR 3

/**
 * @brief	LCD special char for Fuel indicator at 1/4.
 * @note    Shouldn't be changed.
 */
#define FUEL1_CHAR 4

/**
 * @brief	LCD special char for Fuel indicator at 2/4.
 * @note    Shouldn't be changed.
 */
#define FUEL2_CHAR 5

/**
 * @brief	LCD special char for Fuel indicator at 3/4.
 * @note    Shouldn't be changed.
 */
#define FUEL3_CHAR 6

/**
 * @brief	LCD special char for Fuel indicator at 4/4.
 * @note    Shouldn't be changed.
 */
#define FUEL4_CHAR 7

/**
 * @brief 	Access Point SSID to connect to.
 */
#define SSID "NOS-1360" //"NOS-1360" //"AndroidAP" //"HUAWEI" //"ZON-5A70"

/**
 * @brief 	Password to SSID.
 */
#define PASS "HPW572MH" //"HPW572MH" //"charmander" //"12345678" //"e3dbde08bf4f"



/*
===========================================================================================================================================================
===========================================================================================================================================================

	APPLICATION RELATED ENUMS:

===========================================================================================================================================================
===========================================================================================================================================================
*/

/**
 * @brief	States of the state machine:
 */
typedef enum
{
	STATE_IDLE = 0, /*!< Main menu. */
	STATE_CONFIG = 1, /*!< Configuration menu. */
	STATE_CONFIG_DATE = 2, /*!< Date configuration. */
	STATE_CONFIG_NAME = 3, /*!< Name configuration. */
	STATE_CONFIG_ERASE_SCORES = 4, /*!< Clear scores. */
	STATE_SCORES_ERASED = 5, /*!< Screen indicating scores were erased. */
	STATE_PREGAME = 6, /*!< Screen that waits for user to ready up. */
	STATE_INGAME = 7, /*!< Actual game. */
	STATE_POSTGAME = 8 /*!< Screen that shows user's score. */
} STATE;

/**
 * @brief	Idle mode for Idle Task:
 */
typedef enum
{
	IDLE_SHOW_DATE, /*!< Idle task showing date. */
	IDLE_SHOW_SCORES /*!< Idle task showing score. */
} IDLE_MODE;



/*
===========================================================================================================================================================
===========================================================================================================================================================

	APPLICATION RELATED STRUCTURES:

===========================================================================================================================================================
===========================================================================================================================================================
*/

/**
 * @brief	Structure that will hold the Car's current location on the LCD.
 */
typedef struct {
	int row; /*!< Current LCD row car is in */
	int last_row; /*!< LCD row car was last frame */
	int front_column; /*!< Current column car front is in */
	int back_column; /*!< Current column car back is in */
} CAR;

/**
 * @brief	Idle mode for Idle Task:
 */
typedef struct
{
	bool init; /*!< Initialising flag. */
	uint32_t row; /*!< Row car is on. */
} GAME_INFO;

/**
 * @brief	Date Time String Struct.
 */
typedef struct
{
	char date[LCD_DISPLAY_COLUMNS]; /*!< String containing date. */
	char time[LCD_DISPLAY_COLUMNS]; /*!< String containing time. */
} DATE_TIME;




/*
===========================================================================================================================================================
===========================================================================================================================================================

	CUSTOM CHARS:

===========================================================================================================================================================
===========================================================================================================================================================
*/

/**
 * @brief	Char map for Car Back.
 */
const unsigned char car_back_charmap[] = {
	0x00,
	0x06,
	0x07,
	0x06,
	0x07,
	0x06,
	0x00,
	0x00
};

/**
 * @brief	Char map for Fuel Galleon.
 */
const unsigned char fuel_charmap[] = {
	0x00,
	0x1C,
	0x07,
	0x0F,
	0x0F,
	0x0F,
	0x00,
	0x00
};

/**
 * @brief	Char map for Car Front.
 */
const unsigned char car_front_charmap[] = {
	0x00,
	0x0C,
	0x1E,
	0x02,
	0x1E,
	0x0C,
	0x00,
	0x00
};

/**
 * @brief	Char map for Barrier.
 */
const unsigned char barrier_charmap[] = {
	0x02,
	0x0E,
	0x02,
	0x02,
	0x02,
	0x0E,
	0x02,
	0x00
};

/**
 * @brief	Char map for 4/4 fuel indicator.
 */
const unsigned char fuel4_charmap[] = {
	0x1f,
	0x1f,
	0x1f,
	0x1f,
	0x1f,
	0x1f,
	0x1f,
	0x1f
};

/**
 * @brief	Char map for 3/4 fuel indicator.
 */
const unsigned char fuel3_charmap[] = {
	0x00,
	0x00,
	0x1f,
	0x1f,
	0x1f,
	0x1f,
	0x1f,
	0x1f
};

/**
 * @brief	Char map for 2/4 fuel indicator.
 */
const unsigned char fuel2_charmap[] = {
	0x00,
	0x00,
	0x00,
	0x00,
	0x1f,
	0x1f,
	0x1f,
	0x1f
};

/**
 * @brief	Char map for 1/4 fuel indicator.
 */
const unsigned char fuel1_charmap[] = {
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x1f,
	0x1f
};



/*
===========================================================================================================================================================
===========================================================================================================================================================

	TASKS:

===========================================================================================================================================================
===========================================================================================================================================================
*/

/**
 * @brief	Task to periodically get date from RTC, and overwrite queueDATE with the most recent object.
 */
void taskDATE(void *pvParameters);

/**
 * @brief	Task to increment points variable, every POINTS_RATE ms.
 */
void taskPOINTS(void *pvParameters);

/**
 * @brief	Task to run application state machine.
 */
void taskSTATE_MACHINE(void *pvParameters);

/**
 * @brief	Task to increment scoreCount, every SCORE_SHOW_TIME ms. This variable indicates which score should be showing in idle mode.
 */
void taskSCORE_COUNT(void *pvParameters);

/**
 * @brief	Task to run game. Waits for user input (from ADXL) and updates LCD.
 */
void taskUPDATE_GAME(void * pvParameters);

/**
 * @brief	Task to decrement fuel, every FUEL_RATE ms.
 */
void taskFUEL(void * pvParameters);

/**
 * @brief	Task to invert blinked value, every HIGHLIGHT_BLINK_TIME ms. This variable indicates which score should be showing in idle mode.
 */
void taskBLINK(void * pvParameters);

/**
 * @brief	Task to manage Network functionalities.
 */
void taskNETWORK_MANAGER(void * pvParameters);



/*
===========================================================================================================================================================
===========================================================================================================================================================

	USERNAME:

===========================================================================================================================================================
===========================================================================================================================================================
*/

/**
 * @brief	Prompts an input interface to the LCD, so the user can enter his name.
 * @note	Use B1 and B2 to increment and decrement, respectively, each character field, using B3 to advance.
 * @note	This function never clears the LCD display.
 */
void getUsername(void);



/*
===========================================================================================================================================================
===========================================================================================================================================================

	DATE TIME:

===========================================================================================================================================================
===========================================================================================================================================================
*/

/**
 * @brief	Receive current Date Time from queueDATE, and print to LCD.
 * @param   without: -> Field to blink. If blink isn't wanted, NONE should be passed.
 * @note	This function never clears the LCD display.
 */
void printDateTime(RTC_TIME_FIELD without);



/*
===========================================================================================================================================================
===========================================================================================================================================================

	SCORE:

===========================================================================================================================================================
===========================================================================================================================================================
*/

/**
 * @brief	Will print specified Score to the LCD.
 * @param   score: -> Pointer to specified Score.
 * @param   n: -> Placement of score, starting at 0 (p.e. if 'score' is the second best, write 1).
 * @note	This function never clears the LCD display.
 */
void printScore(Score *score, int n);



/*
===========================================================================================================================================================
===========================================================================================================================================================

	IDLE MODE:

===========================================================================================================================================================
===========================================================================================================================================================
*/

/**
 * @brief	Idle Mode. The fundamental state of the program. Shows RTC time and best scores, alternately.
 * @note	Use B1 and B2 to scroll between 'Time' and 'Best Scores'.
 * @note	Use B3 to enter Game Mode.
 * @note    Hold B1 and B2 for a certain amount of time to enter Configuration Mode.
 * @note	This function never clears the LCD display.
 */
void idle(void);



/*
===========================================================================================================================================================
===========================================================================================================================================================

	GAME MODE:

===========================================================================================================================================================
===========================================================================================================================================================
*/

/**
 * @brief	Print to an LCD column.
 * @param   col: -> Column where chars should be printed.
 * @param   c1: -> Char to print in row 1.
 * @param   c2: -> Char to print in row 2.
 */
void printToCol(int col, unsigned char c1, unsigned char c2);

/**
 * @brief	Shift fuel indicator in LCD.
 * @param   currentFuelCol: -> Column where fuel indicator is, currently.
 * @note    Fuel indicator will be shifted to the right of 'currentFuelCol'.
 * @note	This function calls 'printToCol()' to print the fuel indicator to the next column.
 * @note    The LCD display is never shifted in this function. Only the fuel indicator is shifted.
 */
void shiftFuelIndicator(int * currentFuelCol);

/**
 * @brief	Shifts the whole car (i.e. front and back) 1 LCD column to the right.
 * @param   car: -> Pointer to Car.
 * @note    The LCD display is never shifted in this function. Only the car is shifted.
 */
void shiftCar(CAR *car);

/**
 * @brief	Randomises a unique fuel galleon in a specified range of the LCD DDRAM. Also updates array 'o' with such information.
 * @param   from: -> Randomisation will start in this column.
 * @param   to: -> Randomisation will end in this column.
 * @param   map: -> Array representing all objects currently in LCD DDRAM (2 for fuel galleon, 1 for obstacle, 0 for nothing).
 * @note    Only places the fuel in a column where there's no obstacles in either row.
 */
void randomiseFuel(int from, int to, int map[LCD_DISPLAY_ROWS][LCD_DDRAM_LENGTH]);

/**
 * @brief	Randomises new obstacles in a specified range of the LCD DDRAM. Also updates array 'o' with such information.
 * @param   from: -> Randomisation will start in this column.
 * @param   to: -> Randomisation will end in this column.
 * @param   map: -> Array representing all objects currently in LCD DDRAM (2 for fuel galleon, 1 for obstacle, 0 for nothing).
 * @note    A space of at least 2 columns is always left between each obstacle.
 */
void randomiseObstacles(int from, int to, int map[LCD_DISPLAY_ROWS][LCD_DDRAM_LENGTH]);

/**
 * @brief	Checks if any part of the Car (i.e. front or body) has hit a fuel galleon.
 * @param   car: -> Pointer to Car.
 * @param   map: -> Array representing all objects currently in LCD DDRAM (2 for fuel galleon, 1 for obstacle, 0 for nothing).
 * @note    If a galleon has been hit, fuel is added to the fuel indicator.
 */
void checkForFuelGrab(CAR *car, int map[LCD_DISPLAY_ROWS][LCD_DDRAM_LENGTH]);

/**
 * @brief	Checks if any part of the Car (i.e. front or body) has hit a obstacle.
 * @param   car: -> Pointer to Car.
 * @param   map: -> Array representing all objects currently in LCD DDRAM (2 for fuel galleon, 1 for obstacle, 0 for nothing).
 * @return  If an obstacle has been hit, returns true. Else, returns false.
 */
bool checkForLoss(CAR *car, int map[LCD_DISPLAY_ROWS][LCD_DDRAM_LENGTH]);

/**
 * @brief	Animate explosion in car position.
 * @param   car: -> Pointer to Car.
 */
void explodeCar(CAR * car);

/**
 * @brief	State to wait for user to ready up.
 * @note	Hit any button to start the game.
 */
void pregame(void);

/**
 * @brief	State that runs the actual game.
 * @note	Tilt the ADXL345 component to change the row the Car is on.
 * @note	This function never clears the LCD display.
 */
void game(void);

/**
 * @brief	State to show user's score.
 * @note	Hit any button to go to menu.
 */
void postgame(void);



/*
===========================================================================================================================================================
===========================================================================================================================================================

	CONFIGURATION MODE

===========================================================================================================================================================
===========================================================================================================================================================
*/

/**
 * @brief	State in which the user can configure some features.
 * @note	Use B1 and B2 to scroll through the different options.
 * @note    Use B3 to select highlighted option.
 * @note	This function never clears the LCD display.
 */
void config(void);



/*
===========================================================================================================================================================
===========================================================================================================================================================

	TIME CONFIGURATION:

===========================================================================================================================================================
===========================================================================================================================================================
*/

/**
 * @brief	State in which the user may configure RTC time.
 * @note	Use B1 and B2 to increment and decrement, respectively, the highlighted time field.
 * @note    Use B3 advance to next time field.
 * @note	This function never clears the LCD display.
 */
void timeConfig(void);



/*
===========================================================================================================================================================
===========================================================================================================================================================

	SCORE CONFIGURATION:

===========================================================================================================================================================
===========================================================================================================================================================
*/

/**
 * @brief	State that tells the user the scores were erased.
 * @note	Hit any button to go back.
 * @note	This function never clears the LCD display.
 */
void scoresErased(void);

/**
 * @brief	Prompts the user with a 'Yes or No' interface, and runs the specified function if 'Yes' option is selected.
 * @param   f: -> Pointer to function that shall be ran upon selection of 'Yes' option.
 * @return  returns true if 'yes' was chosen, or false if 'no' was chosen.
 * @note	Use B1 and B2 to scroll through 'Yes' and 'No' options.
 * @note    Use B3 to select highlighted option.
 * @note	This function never clears the LCD display.
 */
bool yesOrNo(void(*f)(void));



/**
 * @}
 */


/**
 * @}
 */


#endif /* CAR_RUNNER_RTOS_H_ */
