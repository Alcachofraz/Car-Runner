/*
 * lcd.c
 *
 *  Created on: Nov 2020
 *      Author: PedroG
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif


#include "lcd.h"


/**
 *
 *
 * USES TIMER3
 *
 *
 */

#ifdef FREERTOS
	static QueueHandle_t queueLCD; // LCD queue
	void LCD_WriterTask(void *pvParameters); // LCD writer task
#endif


static void LCD_SetNibble(int rs, int ch);

static void LCD_PulseEnable(int us);

static void LCD_WriteCommand(char cmd);

static void LCD_SetNibble(int rs, int nib)
{
	LPC_GPIO0->FIOCLR = LCD_PINS_MASK;
	LPC_GPIO0->FIOSET = (nib << DB4);
	LPC_GPIO0->FIOSET = (rs << RS);
}

static void LCD_PulseEnable(int us)
{
	LPC_GPIO0->FIOSET = (1 << EN);
	WAIT_TIM3_Us(us);
	LPC_GPIO0->FIOCLR = (1 << EN);
	WAIT_TIM3_Us(us);
}

static void LCD_WriteCommand(char cmd)
{
	// High nibble (1st 4 bits):
	LCD_SetNibble(0, ((cmd >> 4) & LCD_LOW_NIBBLE));
	LCD_PulseEnable(LCD_STD_TIME);

	// High nibble (1st 4 bits):
	LCD_SetNibble(0, (cmd & LCD_LOW_NIBBLE));
	LCD_PulseEnable(LCD_STD_TIME);
}

static void LCD_WriteChar(char ch)
{
	// High nibble (1st 4 bits):
	LCD_SetNibble(1, ((ch >> 4) & LCD_LOW_NIBBLE));
	LCD_PulseEnable(LCD_STD_TIME);

	// High nibble (1st 4 bits):
	LCD_SetNibble(1, (ch & LCD_LOW_NIBBLE));
	LCD_PulseEnable(LCD_STD_TIME);
}

static void LCD_WriteString(char *str)
{
	int i = 0;
	char ch = '0';
	while((ch = *(str + (i++))) != '\0') {
		LCD_WriteChar(ch);
	}
}

static void LCD_Clear(void)
{
	LCD_SetNibble(0, (CLEAR >> 4) & LCD_LOW_NIBBLE);
	LCD_PulseEnable(LCD_EXC_TIME);

	LCD_SetNibble(0, CLEAR & LCD_LOW_NIBBLE);
	LCD_PulseEnable(LCD_EXC_TIME);
}

static void LCD_Home(void)
{
	LCD_SetNibble(0, (HOME >> 4) & LCD_LOW_NIBBLE);
	LCD_PulseEnable(LCD_EXC_TIME);

	LCD_SetNibble(0, HOME & LCD_LOW_NIBBLE);
	LCD_PulseEnable(LCD_EXC_TIME);
}

static void LCD_Locate(int row, int column)
{
	// Verify parameters
	row = (row > 2 || row < 1) ? ((row > 2) ? 2 : 1) : row;
	column = (column > (LCD_DDRAM_LENGTH * LCD_DISPLAY_ROWS) || column < 1)
			? ((column > (LCD_DDRAM_LENGTH * LCD_DISPLAY_ROWS)) ? (LCD_DDRAM_LENGTH * LCD_DISPLAY_ROWS)
					: 1) : column;

	int cmd = 0x80;

	// Set row:
	if (row == 2) cmd |= 0x40;

	// Set column:
	cmd += (column - 1);

	LCD_WriteCommand(cmd);
}

static void LCD_CreateChar(unsigned char location, const unsigned char charmap[])
{
	// Validate location:
	location = (location >= LCD_CHARMAP_LENGTH || location < 0) ? ((location >= LCD_CHARMAP_LENGTH) ? (LCD_CHARMAP_LENGTH - 1) : 0) : location;

	LCD_WriteCommand(LCD_CGRAM + (location*8)); // Send the CGRAM address of custom character (to be created)

	for (int i = 0; i < LCD_CHARMAP_LENGTH; i++) {
		LCD_WriteChar(charmap[i]);
	}
}

static void LCD_ShiftDisplay(LCD_SHIFT_DIR dir)
{
	LCD_WriteCommand(dir);
}

int32_t LCDText_Init(void)
{
	LPC_GPIO0->FIODIR |= LCD_PINS_MASK;

	if (WAIT_Init(TIM3) < 0) return -1;
	WAIT_TIM3_Us(41000);

	LCD_SetNibble(0, 0x03);
	LCD_PulseEnable(LCD_STD_TIME);
	WAIT_TIM3_Us(5000);

	LCD_SetNibble(0, 0x03);
	LCD_PulseEnable(LCD_STD_TIME);
	WAIT_TIM3_Us(200);

	LCD_SetNibble(0, 0x03);
	LCD_PulseEnable(LCD_STD_TIME);
	WAIT_TIM3_Us(200);

	LCD_SetNibble(0, HOME);
	LCD_PulseEnable(LCD_STD_TIME);
	WAIT_TIM3_Us(200);

	LCD_WriteCommand(FUNCTION_SET); // Select 4-bit data bus interface length and two-line display
	LCD_WriteCommand(OFF); // Turn off display and cursor
	LCD_Clear();
	LCD_WriteCommand(ENTRY); // Set entry mode
	LCD_WriteCommand(ON); // Turn on display, cursor and blinking of cursor
	LCD_Locate(1, 1);

	#ifdef FREERTOS
		if (xTaskCreate(LCD_WriterTask, (const char * const) "LCD Writer Task", TASK_LCD_WRITER_STACK_SIZE, NULL, TASK_LCD_WRITER_PRIORITY, NULL) != pdPASS) {
			printf("LCD Writer Task could not be created.\n");
			return -1;
		}

		if ((queueLCD = xQueueCreate(16, sizeof(LCD_INFO))) == NULL) {
			printf("Could not initialise queueLCD");
			return -1;
		}
	#endif

	return 0;
}

void LCDText_WriteChar(char ch)
{
	#ifdef FREERTOS
		LCD_INFO info;
		info.cmd = LCD_CMD_WRITE_CHAR;
		info.ch = ch;
		xQueueSend(queueLCD, &info, portMAX_DELAY);
	#else
		LCD_WriteChar(ch);
	#endif
}

void LCDText_WriteString(char *str)
{
	#ifdef FREERTOS
		if (strlen(str) > LCD_DDRAM_LENGTH * LCD_DISPLAY_ROWS) return;
		LCD_INFO info;
		info.cmd = LCD_CMD_WRITE_STRING;
		strcpy(info.str, str);
		xQueueSend(queueLCD, &info, portMAX_DELAY);
	#else
		LCD_WriteString(str);
	#endif
}

void LCDText_Clear(void)
{
	#ifdef FREERTOS
		LCD_INFO info;
		info.cmd = LCD_CMD_CLEAR;
		xQueueSend(queueLCD, &info, portMAX_DELAY);
	#else
		LCD_Clear();
	#endif
}

void LCDText_Home(void)
{
	#ifdef FREERTOS
		LCD_INFO info;
		info.cmd = LCD_CMD_HOME;
		xQueueSend(queueLCD, &info, portMAX_DELAY);
	#else
		LCD_Home();
	#endif
}

void LCDText_Locate(int row, int column)
{
	#ifdef FREERTOS
		LCD_INFO info;
		info.cmd = LCD_CMD_LOCATE;
		info.row = row;
		info.col = column;
		xQueueSend(queueLCD, &info, portMAX_DELAY);
	#else
		LCD_Locate(row, column);
	#endif
}

void LCDText_CreateChar(unsigned char location, const unsigned char charmap[])
{
	#ifdef FREERTOS
		LCD_INFO info;
		info.cmd = LCD_CMD_CREATE_CHAR;
		info.location = location;
		memcpy(info.charmap, charmap, LCD_CHARMAP_LENGTH);
		xQueueSend(queueLCD, &info, portMAX_DELAY);
	#else
		LCD_CreateChar(location, charmap);
	#endif
}

void LCDText_ShiftDisplay(LCD_SHIFT_DIR dir)
{
	#ifdef FREERTOS
		LCD_INFO info;
		switch (dir) {
			case RIGHT:
				info.cmd = LCD_CMD_SHIFT_RIGHT;
			case LEFT:
				info.cmd = LCD_CMD_SHIFT_LEFT;
		}
		xQueueSend(queueLCD, &info, portMAX_DELAY);
	#else
		LCD_ShiftDisplay(dir);
	#endif
}

void LCDText_Printf(char *fmt, ...)
{
	va_list arg;
	char str[LCD_DDRAM_LENGTH * LCD_DISPLAY_ROWS]; // LCD DDRAM has 80 addresses

	va_start (arg, fmt);

	vsprintf(str, fmt, arg);

	va_end (arg);

	#ifdef FREERTOS
		LCD_INFO info;
		info.cmd = LCD_CMD_WRITE_STRING;
		strcpy(info.str, str);
		xQueueSend(queueLCD, &info, portMAX_DELAY);
	#else
		LCD_WriteString(str);
	#endif
}

#ifdef FREERTOS
	void LCD_WriterTask(void *pvParameters) {
		LCD_INFO info;

		for (;;) {
			// Block waiting for things to write
			xQueueReceive(queueLCD, &info, portMAX_DELAY);

			switch(info.cmd) {
				case LCD_CMD_WRITE_CHAR:
					LCD_WriteChar(info.ch);
					break;
				case LCD_CMD_WRITE_STRING:
					LCD_WriteString(info.str);
					break;
				case LCD_CMD_CLEAR:
					LCD_Clear();
					break;
				case LCD_CMD_HOME:
					LCD_Home();
					break;
				case LCD_CMD_LOCATE:
					LCD_Locate(info.row, info.col);
					break;
				case LCD_CMD_CREATE_CHAR:
					LCD_CreateChar(info.location, info.charmap);
					break;
				case LCD_CMD_SHIFT_RIGHT:
					LCD_ShiftDisplay(RIGHT);
					break;
				case LCD_CMD_SHIFT_LEFT:
					LCD_ShiftDisplay(LEFT);
					break;
			}
		}
	}
#endif

