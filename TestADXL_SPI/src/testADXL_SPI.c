/*
===============================================================================
 Name        : testADXL_SPI.c
 Author      : PedroG
 Version     : 1.0
 Copyright   : PedroG
 Description : Program to test ADXL345 with SPI
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
#include "adxl.h"

int main(void) {

	SystemInit();
	BUTTON_Init();
	RTC_Init(0, INONE);
	if (WAIT_Init(TIM0) < 0) {
		printf("WAIT Resource 0 could not initialise.\n");
		return 0;
	}
	if (LCDText_Init() < 0) {
		printf("LCD could not initialise.\n");
		return 0;
	}
	ADXL_Init(0, 0);

	printf("%d", ADXL_GetDevId());

	signed short coords[] = {0, 0, 0};

    while(1) {
    	ADXL_GetAxis(coords);
    	LCDText_Locate(1, 1);
    	LCDText_Printf("x=%04d", coords[0]);
    	LCDText_Locate(1, 9);
    	LCDText_Printf("y=%04d", coords[1]);
    	LCDText_Locate(2, 1);
    	LCDText_Printf("z=%04d", coords[2]);
    	WAIT_TIM0_Us(200000);
    }
    return 0;
}
