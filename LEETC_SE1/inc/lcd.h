/*
* @file		lcd.h
* @brief	Contains the LCD output API.
* @version	1.0
* @date		Nov 2020
* @author	PedroG
*
* Copyright(C) 2020-2025, PedroG
* All rights reserved.
 */

#ifndef LCD_H_
#define LCD_H_

/** @defgroup LCD LCD
 * This package provides the core capabilities for lcd output functions.
 * @{
 */

/** @defgroup LCD_Public_Functions LCD Public Functions
 * @{
 */


#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "wait.h"

#ifdef FREERTOS
	#include "FreeRTOS.h"
	#include "task.h"
	#include "queue.h"
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
	 * @brief	Stack size of LCD Writer task.
	 * @brief	Only needed in FreeRTOS environment.
	 */
	#define TASK_LCD_WRITER_STACK_SIZE configMINIMAL_STACK_SIZE*1

	/**
	 * @brief	Priority of LCD Writer task.
	 * @brief	Only needed in FreeRTOS environment.
	 */
	#define TASK_LCD_WRITER_PRIORITY tskIDLE_PRIORITY + 1
#endif

/**
 * @brief	Mask for the LCD pins.
 */
#define LCD_PINS_MASK ((1 << RS) | (1 << EN) | (1 << DB7) | (1 << DB6) | (1 << DB5) | (1 << DB4))

/**
 * @brief	Low nibble of a byte.
 */
#define LCD_LOW_NIBBLE 0x0F

/**
 * @brief	Standard command/Data writing time in us.
 * @note	This time does not apply for the following commands: 'Clear Display' and 'Return Home'.
 */
#define LCD_STD_TIME 39

/**
 * @brief	Exceptional command writing time in us.
 * @note	This time applies for the following commands: 'Clear Display' and 'Return Home'.
 */
#define LCD_EXC_TIME 3000

/**
 * @brief	First address for custom characters in CGRAM.
 */
#define LCD_CGRAM 0x40

/**
 * @brief	Columns of the LCD display.
 */
#define LCD_DISPLAY_COLUMNS 16

/**
 * @brief	Rows of the LCD display.
 */
#define LCD_DISPLAY_ROWS 2

/**
 * @brief	Length of the LCD DDRAM.
 */
#define LCD_DDRAM_LENGTH 40

/**
 * @brief	Length of custom char map.
 */
#define LCD_CHARMAP_LENGTH 8

/**
 * @brief	Structure containing information to write to LCD:
 */
typedef struct {
	uint8_t cmd; // Type of command
	char ch; // Character
	char str[LCD_DDRAM_LENGTH * LCD_DISPLAY_ROWS]; // String
	int row; // Row
	int col; // Column
	unsigned char location; // Location
	unsigned char charmap[LCD_CHARMAP_LENGTH]; // Character Map
} LCD_INFO;

/**
 * @brief 	LCD Commands.
 */
typedef enum {
	LCD_CMD_WRITE_CHAR, /*!< Write char. */
	LCD_CMD_WRITE_STRING, /*!< Write string. */
	LCD_CMD_CLEAR, /*!< Clear. */
	LCD_CMD_HOME, /*!< Home. */
	LCD_CMD_LOCATE, /*!< Locate cursor. */
	LCD_CMD_CREATE_CHAR, /*!< Create custom char. */
	LCD_CMD_SHIFT_RIGHT, /*!< Shift right. */
	LCD_CMD_SHIFT_LEFT /*!< Shift left. */
} LCD_CMD_TYPE;


/**
 * @brief	Data bus bits for LCD.
 */
typedef enum {
	DB7 = 9, /*!< Pin 5 */
	DB6 = 8, /*!< Pin 6 */
	DB5 = 7, /*!< Pin 7 */
	DB4 = 6, /*!< Pin 8 */
	EN = 0, /*!< Pin 9 */
	RS = 1 /*!< Pin 10 */
} LCD_PINS;

/**
 * @brief	LCD Used Commands.
 */
typedef enum {
	CLEAR = 0x01, /*!< Clear display. */
	HOME = 0x02, /*!< Return cursor and display to original position. */
	FUNCTION_SET = 0x28, /*!< Select interface data length and number of display lines. */
	OFF = 0x08, /*!< Turn OFF display, cursor and blink of cursor. */
	ON = 0x0C, /*!< Turn ON display and cursor. */
	ENTRY = 0x06, /*!< Set cursor move direction and display shift. */
} LCD_CMDS;

/**
 * @brief	Directions to shift display.
 */
typedef enum {
	RIGHT = 0x1C, /*!< Shift display right. */
	LEFT = 0x18 /*!< Shift display left. */
} LCD_SHIFT_DIR;


/*
 *
 *
 *
 * Functions:
 *
 *
 *
 */


/**
 * @brief	Initialises the LCDText API.
 * @return  0 if succeeded, -1 if failed.
 * @note	This function must be called prior to any other LCDText functions.
 * @note    This function uses timer3 from "wait.h". If timer3 is already in use,
 * 			initialisation fails.
 */
int32_t LCDText_Init(void);

/**
 * @brief	Writes a char in the same location as cursor.
 * @param   ch: -> Char to be written.
 * @note	This command takes around 38 us.
 * @note	In FreeRTOS environment, this function returns immediately, even tho command only takes effect after said time.
 */
void LCDText_WriteChar(char ch);

/**
 * @brief	Writes a string from the location of the cursor.
 * @param   str: -> String to be written.
 * @note	This command takes around 38 us for each character.
 * @note	In FreeRTOS environment, this function returns immediately, even tho command only takes effect after said time.
 */
void LCDText_WriteString(char *str);

/**
 * @brief	Clears Display.
 * @note	This command takes around 1.52 ms.
 * @note	In FreeRTOS environment, this function returns immediately, even tho command only takes effect after said time.
 */
void LCDText_Clear(void);

/**
 * @brief	Return cursor and display to original position.
 * @note	This command takes around 1.52 ms.
 * @note	In FreeRTOS environment, this function returns immediately, even tho command only takes effect after said time.
 */
void LCDText_Home(void);

/**
 * @brief	Place cursor in row[1:2] and column[1:16] of display. If parameters are absurd, automatically sets to a possible location.
 * @note	This command takes around 38 us.
 * @note	In FreeRTOS environment, this function returns immediately, even tho command only takes effect after said time.
 * @param	row: -> Row to set cursor in.
 * @param	column: -> Column to set cursor in.
 */
void LCDText_Locate(int row, int column);

/**
 * @brief	Create a custom character in a relative address starting from 0x40.
 * @note	This command takes around 38 us.
 * @note	In FreeRTOS environment, this function returns immediately, even tho command only takes effect after said time.
 * @param	location: -> CGRAM relative address to create char in. Should be between 0 and 7.
 * @param	charmap: -> Custom character pixels map. Should be a char array with 7 positions.
 */
void LCDText_CreateChar(unsigned char location, const unsigned char charmap[]);

/**
 * @brief	Writes to LCD Display as 'printf()' would write to stdout.
 * @note	This command takes around 38 us for each character to be written.
 * @note	In FreeRTOS environment, this function returns immediately, even tho command only takes effect after said time.
 * @param	fmt: -> String to format.
 */
void LCDText_Printf(char *fmt, ...);

/**
 * @brief	Shifts display in a certain direction.
 * @note	This command takes around 38 us.
 * @note	In FreeRTOS environment, this function returns immediately, even tho command only takes effect after said time.
 * @param	dir: -> Direction to shift display.
 */
void LCDText_ShiftDisplay(LCD_SHIFT_DIR dir);

/**
 * @}
 */


/**
 * @}
 */

#endif /* LCD_H_ */
