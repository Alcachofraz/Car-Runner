/*
===============================================================================
 Name        : testButtonsLED.c
 Author      : PedroG
 Version     : 1.0
 Copyright   : PedroG
 Description : Button to LED Experiment
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

int main(void) {

    SystemInit();
    WAIT_Init(SYS);
    BUTTON_Init();
    LED_Init(false);

    printf("BUTTON TO LED (SYSTICK)\n");

    volatile int bitmap = 0;

    while(1) {
    	bitmap = BUTTON_Read();
    	if (bitmap & B1) {
    		LED_On();
    		WAIT_SYS_Ms(500);
    		LED_Off();
    	}
    	else if (bitmap & B2) {
    		LED_On();
    		WAIT_SYS_Ms(1000);
    		LED_Off();
    	}
    	else if (bitmap & B3) {
    		LED_On();
    		WAIT_SYS_Ms(1500);
    		LED_Off();
    	}
    	else printf("Unexpected behaviour!");
    }
    return 0;
}


