/*
===============================================================================
 Name        : testFlash.c
 Author      : PedroG
 Version     : 1.0
 Copyright   : PedroG
 Description : Program to test Flash Memory
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wait.h"
#include "button.h"
#include "led.h"
#include "lcd.h"
#include "rtc.h"
#include "flash.h"

int main(void) {

	SystemInit();
	BUTTON_Init();
	RTC_Init(0, INONE);
	if (LCDText_Init() < 0) {
		printf("LCD could not initialise.\n");
		return 0;
	}

	int * address = (int *) FLASH_START_ADDRESS_29;
	int * array = (int *) malloc(256);
	array[0] = *address;

	FLASH_EraseSectors(29, 29);

	printf("%d ", FLASH_WriteData(29, address, array, 256)); // Returns 0x00
	printf("%d\n", FLASH_VerifyData(address, array, 4)); // Returns 0x00

    while(1) {
    	if (BUTTON_GetButtonsEvents() & BUTTON_Hit()) {
			array[0]++;
			FLASH_EraseSectors(29, 29);
			printf("%d ", FLASH_WriteData(29, address, array, 256)); // Returns 0x00
			printf("%d\n", FLASH_VerifyData(address, array, 4)); // Returns 0x0A
    	}
    	LCDText_Locate(1, 1);
    	LCDText_Printf("%d", array[0]);
    }
    return 0 ;
}
