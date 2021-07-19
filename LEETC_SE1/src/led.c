/*
 * led.c
 *
 *  Created on: Nov 2020
 *      Author: PedroG
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif


#include "led.h"


void LED_Init(bool state)
{
	LPC_GPIO0->FIODIR |= LED2; // Define LED2 as output
	if (state) {
		LPC_GPIO0->FIOSET = LED2; // Turn on LED
	}
	else LPC_GPIO0->FIOCLR = LED2; // Turn off LED
}

bool LED_GetState(void)
{
	return LED2 == (LPC_GPIO0->FIOPIN & LED2);
}

void LED_On(void)
{
	LPC_GPIO0->FIOSET = LED2; // Turn on LED
}

void LED_Off(void)
{
	LPC_GPIO0->FIOCLR = LED2; // Turn off LED
}

void LED_Invert(void) // Invert LED state
{
	LED_GetState() ? LED_Off() : LED_On();
}
