/*
===============================================================================
 Name        : testTimerInterrupt.c
 Author      : PedroG
 Version     : 2.0
 Copyright   : PedroG
 Description : Interruptive Timer Experiment
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

void Int_Handler(void);

/*
 * This program's purpose is to test interrupt requests.
 * Test the ability to play with LEDs and output console, while the timer is counting.
 * When the timer's done counting, the program will execute 'Int_Handler(void)'.
 */

int main(void) {

    SystemInit();
    WAIT_Init(IRQ0);
    BUTTON_Init();
    LED_Init(false);

    printf("BUTTON TO LED/CONSOLE (IRQ)\n");

    volatile int bitmap = 0;
    bool pressed = true;

    while(1) {
    	bitmap = BUTTON_GetButtonsEvents();
    	if (bitmap & B1) {
    	    WAIT_IRQ0_Us(4000000, Int_Handler);
    	}
    	else if (bitmap & B2) {
    		printf("Button 2 %s.\n", (pressed ? "pressed" : "released"));
    		pressed = !pressed;
    	}
    	else if (bitmap & B3) {
    		LED_Invert();
    	}
    }
    return 0;
}

void Int_Handler(void) {
	printf("Interruption!\n");
}
