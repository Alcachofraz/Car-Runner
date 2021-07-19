/*
 * eeprom.c
 *
 *  Created on: 30 Mar 2021
 *      Author: pedro
 */

#include "eeprom.h"

void EEPROM_Init() {
	I2C1_Init();
}

int EEPROM_Read(char * buffer, int size) {
	char wBuffer[] = {EEPROM_ADDRESS << 1, 0x00, 0x00, (EEPROM_ADDRESS << 1) | 1};
	I2C1_Configure(EEPROM_FREQUENCY, size, wBuffer, 3);
	if (I2C1_Start() < 0) return -1;
	if (I2C1_Engine() < 0) return -1;
	I2C1_Stop();
	I2C1_GetBuffer(buffer);
	return 0;
}

int EEPROM_Write(char * buffer, int size) {
	for (int16_t n = 0; n < size; n += 32) {
		int pageLength = (size - n < 32) ? size - n : 32;

		char wBuffer[EEPROM_PAGE_LENGTH + 3];
		wBuffer[0] = EEPROM_ADDRESS << 1;
		wBuffer[1] = (n >> 8);
		wBuffer[2] = n;

		for (int i = 0; i < pageLength; i++) {
			wBuffer[3 + i] = buffer[n + i];
		}

		I2C1_Configure(EEPROM_FREQUENCY, 0, wBuffer, pageLength + 3);
		if (I2C1_Start() < 0) return -1;
		if (I2C1_Engine() < 0) return -1;
		I2C1_Stop();

		WAIT_SYS_Ms(5); // Wait between pages
	}

	return 0;
}
