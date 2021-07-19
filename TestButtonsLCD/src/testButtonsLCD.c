/*
===============================================================================
 Name        : testButtonsLCD.c
 Author      : PedroG
 Version     : 1.0
 Copyright   : PedroG
 Description : LCD Experiment that writes bitmap of pressed buttons.
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <stdio.h>

#include "wait.h"
#include "button.h"
#include "led.h"
#include "lcd.h"

/*
 * Takes an integer, reads binary value, and returns 1s and 0s in decimal.
 */
int toBinary(int n) {
	int bin = 0;
	int rem, i = 1;
	while (n != 0) {
		rem = n % 2;
		n /= 2;
		bin += rem * i;
		i *= 10;
	}
	return bin;
}

int main(void) {

    SystemInit();
    BUTTON_Init();
    LED_Init(false);
    if (LCDText_Init() < 0) {
    	printf("LCD could not initialise.\n");
    	return 0;
    }

    int volatile bitmap = 0;
    int volatile events = 0;

    LCDText_Locate(1, 1);
	LCDText_Printf("%016d", toBinary(bitmap >> 16));

	LCDText_Locate(2, 1);
	LCDText_Printf("%016d", toBinary(bitmap));

    printf("BUTTON TO LCD\n");

    while(1) {
    	if ((events = BUTTON_GetButtonsEvents()) != 0) {
    		bitmap ^= events;

    		LCDText_Locate(1, 1);
    		LCDText_Printf("%016d", toBinary(bitmap >> 16));

    		LCDText_Locate(2, 1);
    		LCDText_Printf("%016d", toBinary(bitmap));
    	}
    }
    return 0;
}
