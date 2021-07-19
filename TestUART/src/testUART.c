/*
===============================================================================
 Name        : testUART.c
 Author      : PedroG
 Version     : 1.0
 Copyright   : PedroG
 Description : UART test
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
#include "uartM.h"

#define BUFFER_WIDTH 64

int main(void) {
	printf("Uart test pooling\n");
	WAIT_Init(SYS);
	LED_Init(false);
	UARTM_Initialize(115200);

	LPC_GPIO2->FIODIR |= (1 << 27);
	LPC_GPIO2->FIOCLR = (1 << 27);
	WAIT_SYS_Ms(500);
	LPC_GPIO2->FIOSET = (1 << 27);

	UARTM_WriteString("AT+RST\r\n");

	while(1){
		printf("rx = %c\n", UARTM_ReadChar());
		WAIT_SYS_Ms(500);
	}


	/*
	int tstCh = '0';
	while(1) {
		LED_On();
		UART_WriteChar(tstCh);
		if (++tstCh > '9') tstCh = '0';
		LED_Off();
		printf("rx = %c\n", UART_ReadChar());
		WAIT_Milliseconds(500);
	}
	*/

	return 0;
}
