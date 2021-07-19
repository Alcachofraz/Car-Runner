/**
* @file		car_runner.h
* @brief	Car Runner mini-game.
* @version	1.0
* @date		Jan 2021
* @author	PedroG
*
* Copyright(C) 2020-2025, PedroG
* All rights reserved.
*/

#ifndef CAR_RUNNER_H_
#define CAR_RUNNER_H_

/** @defgroup CAR_RUNNER CAR_RUNNER
 * This package provides the Car Runner Mini-Game.
 * @{
 */

/** @defgroup CAR_RUNNER_Public_Functions CAR_RUNNER Public Functions
 * @{
 */


#include <cr_section_macros.h>
#include <score.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wait.h"
#include "button.h"
#include "led.h"
#include "lcd.h"
#include "rtc.h"
#include "spi.h"
#include "adxl.h"


/*
 *
 *
 * Constants:
 *
 *
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
#define BUTTONS_HOLD_TIME 2000000

/**
 * @brief	Time between different scores being displayed, in Idle Mode.
 */
#define SCORE_SHOW_TIME 3000000

/**
 * @brief	Blinking time, used to highlight fields.
 */
#define HIGHLIGHT_BLINK_TIME 300000

/**
 * @brief	Game update rate (i.e. time in us between each "frame"), in Game Mode.
 */
#define GAME_RATE 300000

/**
 * @brief	Fuel tank decrement rate, in Game Mode.
 */
#define FUEL_RATE 2000000

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
 * @brief	LCD special char for Barrier.
 * @note    Shouldn't be changed.
 */
#define BARRIER_CHAR 2

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
 * @brief	States of the state machine:
 */
typedef enum
{
	IDLE = 0, /*!< Main menu. */
	CONFIG = 1, /*!< Configuration menu. */
	CONFIG_DATE = 2, /*!< Date configuration. */
	CONFIG_NAME = 3, /*!< Name configuration. */
	CONFIG_ERASE_SCORES = 4, /*!< Clear scores. */
	SCORES_ERASED = 5, /*!< Screen indicating scores were erased. */
	PREGAME = 6, /*!< Screen that waits for user to ready up. */
	GAME = 7, /*!< Actual game. */
	POSTGAME = 8 /*!< Screen that shows user's score. */
} STATE;

/**
 * @brief	Char map for Car Back.
 */
unsigned char car_back_charmap[] = {
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
unsigned char fuel_charmap[] = {
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
unsigned char car_front_charmap[] = {
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
unsigned char barrier_charmap[] = {
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
unsigned char fuel4_charmap[] = {
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
unsigned char fuel3_charmap[] = {
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
unsigned char fuel2_charmap[] = {
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
unsigned char fuel1_charmap[] = {
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x1f,
	0x1f
};


/**
 *
 *
 * Variables:
 *
 *
 */

/**
 * @brief	Current state (starts as IDLE).
 */
volatile STATE state = IDLE;

/**
 * @brief	Name of current user.
 */
char username[NAME_LENGTH+1];


/**
 *
 *
 * Functions:
 *
 *
 */


/**
 * @brief	State manager.
 * @note	Will decide which state to call, based in 'STATE' ENUM value stored in 'state'.
 * @note	Will clear the LCD display in every state transition.
 */
void stateMachine(void);

/**
 * @brief	Prompts an input interface to the LCD, so the user can enter his name.
 * @note	Use B1 and B2 to increment and decrement, respectively, each character field, using B3 to advance.
 * @note	This function doesn't clear the LCD display, upon exit.
 * @note    This function uses Timer0. However, it's guaranteed that Timer0 is no longer busy upon this function's return.
 */
void getUsername(void);

/**
 * @brief	Will print RTC time to the LCD.
 * @param   without: -> Specified field not to be printed (this can be used for highlight blinking purposes).
 * @note	This function doesn't clear the LCD display, upon exit.
 */
void updateAndPrintTime(RTC_TIME_FIELD without);

/**
 * @brief	Will print specified Score to the LCD.
 * @param   score: -> Pointer to specified Score.
 * @param   n: -> Placement of score (p.e. if 'score' is the second best, write 1).
 * @note	This function doesn't clear the LCD display, upon exit.
 */
void printScore(Score *score, int n);

/**
 * @brief	Idle Mode. The fundamental state of the program. Shows RTC time and best scores, alternately.
 * @note	Use B1 and B2 to scroll between 'Time' and 'Best Scores'.
 * @note	Use B3 to enter Game Mode.
 * @note    Hold B1 and B2 for a certain amount of time to enter Configuration Mode.
 * @note	This function never clears the LCD display, upon exit.
 * @note    This function uses Timer0 and Timer1.
 * @note    When entering Game Mode, it's guaranteed both timers are no longer busy.
 * @note    When entering Configuration Mode, it's guaranteed that Timer0 is no longer busy, but not Timer1 (since it won't be used).
 */
void idle(void);

/**
 * @brief	Changes LCD row specified 'Car' is on.
 * @param   car: -> Pointer to Structure Car.
 * @param   row: -> New row.
 * @note	If row is different from 1 or 2, this function does nothing.
 */
void changeRow(CAR * car, int row);

/**
 * @brief	Get ADXL345 Y-Axis and check for threshold (defined in macro) exceeds.
 * @return  0 if no threshold has been exceeded. -1 for left threshold, 1 for right threshold.
 */
int getInclination(void);

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
 * @brief	State to wait for user to ready up.
 * @note	Hit any button to start the game.
 */
void pregame(void);

/**
 * @brief	State that runs the actual game.
 * @note	Tilt the ADXL345 component to change the row the Car is on.
 * @note    This function uses Timer0 and Timer1.
 */
void game(void);

/**
 * @brief	State to show user's score.
 * @note	Hit any button to go to menu.
 * @note    This function waits for timer0 and timer1, both used in 'game()'.
 */
void postgame(void);

/**
 * @brief	State in which the user can configure some features.
 * @note	Use B1 and B2 to scroll through the different options.
 * @note    Use B3 to select highlighted option.
 * @note    This function uses Timer0.
 * @note    When selecting an option or going back to Idle Mode, it's guaranteed that Timer0 is no longer busy.
 */
void config(void);

/**
 * @brief	State in which the user may configure RTC time.
 * @note	Use B1 and B2 to increment and decrement, respectively, the highlighted time field.
 * @note    Use B3 advance to next time field.
 * @note    This function uses Timer0.
 * @note    When leaving this function, it's guaranteed that Timer0 is no longer busy.
 */
void timeConfig(void);

/**
 * @brief	State that tells the user the scores were erased.
 * @note	Hit any button to go back.
 */
void scoresErased(void);

/**
 * @brief	Prompts the user with a 'Yes or No' interface, and runs the specified function if 'Yes' option is selected.
 * @param   f: -> Pointer to function that shall be ran upon selection of 'Yes' option.
 * @return  returns true if 'yes' was chosen, or false if 'no' was chosen.
 * @note	Use B1 and B2 to scroll through 'Yes' and 'No' options.
 * @note    Use B3 to select highlighted option.
 */
bool yesOrNo(void(*f)(void));


/**
 * @}
 */


/**
 * @}
 */

#endif /* CAR_RUNNER_H_ */
