/*
===============================================================================
 Name        : testEEPROM.c
 Author      : PedroG
 Version     : 1.0
 Copyright   : PedroG
 Description : Program to test EEPROM
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
#include "eeprom.h"

int main(void) {

	SystemInit();
	BUTTON_Init();
	EEPROM_Init();

	if (LCDText_Init() < 0) {
		printf("LCD could not initialise.\n");
		return 0;
	}


	int send[16] = {0};
	int receive[16] = {0};

	for (int i = 0; i < 16; i++)
		printf("%d", send[i]);

	printf("\n");

	printf("%d\n", EEPROM_Write((char *)send, 16*sizeof(int)));
	printf("%d\n", EEPROM_Read((char *)receive, 16*sizeof(int)));

	for (int i = 0; i < 16; i++)
		printf("%d", receive[i]);



/*
	char send[6] = "hello";
	char receive[6];

	printf("%s\n", send);

	printf("%d\n", EEPROM_Write((char *)send, 6));
	printf("%d\n", EEPROM_Read((char *)receive, 6));

	printf("%s\n", receive);
*/

    while(1) {}
    return 0 ;
}
