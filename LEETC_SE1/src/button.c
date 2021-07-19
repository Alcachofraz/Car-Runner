/*
 * button.c
 *
 *  Created on: Nov 2020
 *      Author: PedroG
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif


#include "button.h"

static uint32_t dummy;


volatile static uint32_t previous_state = 0;

void BUTTON_Init() {
	WAIT_Init(SYS);
	LPC_GPIO2->FIODIR &= ~BUTTONS_MASK;
	previous_state = BUTTON_Hit();
}

int32_t BUTTON_Hit() {
	return ((~LPC_GPIO2->FIOPIN) & BUTTONS_MASK); // ~ in order to set bit to 1, when button is pressed
}

int32_t BUTTON_Read() {
	volatile int bitmap = 0;
	while (bitmap == 0) {
		bitmap = BUTTON_Hit();
	}
	return bitmap;
}

uint32_t BUTTON_GetButtonsEvents(uint32_t * current) {
	if (current == 0) current = &dummy;

	*current = BUTTON_Hit();
	if (*current == previous_state) return 0;
	uint32_t aux1 = *current ^ previous_state; // Identify the changing bits and return 1 on those positions

	WAIT_SYS_Ms(DEBOUNCE_TIME);
	uint32_t aux2 = (*current = BUTTON_Hit()) ^ previous_state; // Again, for debounce purposes

	previous_state = *current;
	return aux1 & aux2;
}

uint32_t BUTTON_GetButtonsPushEvents(uint32_t * current) {
	if (current == 0) current = &dummy;

	*current = BUTTON_Hit();
	if (*current == previous_state) return 0;
	uint32_t aux1 = *current & (~previous_state); // Identify the changing bits that are now 0 and return 1 on those positions

	WAIT_SYS_Ms(DEBOUNCE_TIME);
	uint32_t aux2 = (*current = BUTTON_Hit()) & (~previous_state); // Again, for debounce purposes

	previous_state = *current;
	return aux1 & aux2;
}

uint32_t BUTTON_GetButtonsReleaseEvents(uint32_t * current) {
	if (current == 0) current = &dummy;

	*current = BUTTON_Hit();
	if (*current == previous_state) return 0;
	uint32_t aux1 = (~(*current)) & previous_state; // Identify the changing bits that are now 1 and return 1 on those positions

	WAIT_SYS_Ms(DEBOUNCE_TIME);
	uint32_t aux2 = (~(*current = BUTTON_Hit())) & previous_state; // Again, for debounce purposes

	previous_state = *current;
	return aux1 & aux2;
}

void BUTTON_WaitRelease(void (*f)(void)) {
	while(BUTTON_Hit() != 0) { // Wait for user to release all buttons
		if (f != 0) f();
	}
	BUTTON_GetButtonsEvents(0); // Update previous_state
}

