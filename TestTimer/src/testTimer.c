/*
===============================================================================
 Name        : testTimer.c
 Author      : PedroG
 Version     : 1.0
 Copyright   : PedroG
 Description : Timer Experiment
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
    WAIT_Init(TIM0);
    BUTTON_Init();
    LED_Init(false);

    printf("BUTTON TO LED (TIMER)\n");

    volatile int bitmap = 0;

    while(1) {
    	bitmap = BUTTON_Read();
    	if (bitmap & B1) {
    		LED_On();
    		WAIT_TIM0_Us(500000);
    		LED_Off();
    	}
    	else if (bitmap & B2) {
    		LED_On();
    		WAIT_TIM0_Us(1000000);
    		LED_Off();
    	}
    	else if (bitmap & B3) {
    		LED_On();
    		WAIT_TIM0_Us(1500000);
    		LED_Off();
    	}
    	else printf("Unexpected behaviour!");
    }
    return 0;
}
