/*
 * esp.c
 *
 *  Created on: May 2021
 *      Author: PedroG
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif


#include "esp.h"


#ifdef FREERTOS
	/**
	 * Binary semaphore to control access to global state.
	 */
	static SemaphoreHandle_t semESP = NULL;
#endif


static int len = 0;

static char AT_RECEIVING_BUFFER[AT_RESPONSE_MAX_LENGTH];

static uint32_t ESP_ConfigListAP(bool sort) {
	ESP_WriteString("AT+CWLAPOPT=");
	ESP_WriteString(sort ? "1" : "0");
	ESP_WriteString(",2");
	ESP_WriteString(AT_CMD_SUFFIX);
	return ESP_WaitForString(ESP_TIMEOUT_MS, 2, "OK", "ERROR");
}


bool ESP_Init(unsigned int baud) {
	WAIT_Init(SYS);

	if (!UART_Init(baud)) return false;

	#ifdef FREERTOS
		if ((semESP = xSemaphoreCreateMutex()) == NULL) {
			printf("Semaphore ESP could not be created.\n");
			return 0;
		}
	#endif

	LPC_GPIO0->FIODIR |= ESP_RST; // Define RST as output

	// Reset Module:
	LPC_GPIO0->FIOCLR = ESP_RST; // Reset Module
	WAIT_SYS_Ms(500);

	// Enable Module:
	LPC_GPIO0->FIOSET = ESP_RST;
	WAIT_SYS_Ms(500);

	char temp;
	while(ESP_GetChar(&temp));

	return true;
}

uint32_t ESP_Test(void) {
	ESP_WriteString("AT");
	ESP_WriteString(AT_CMD_SUFFIX);
    return (ESP_WaitForString(ESP_TIMEOUT_MS, 1, "OK"));
}

uint32_t ESP_Restart(void) {
	ESP_WriteString("AT+RST");
	ESP_WriteString(AT_CMD_SUFFIX);
    if (ESP_WaitForString(ESP_TIMEOUT_MS, 1, "OK") != 0) return false;
    return ESP_WaitForString(ESP_TIMEOUT_MS, 1, "ready");
}

uint32_t ESP_Version(void) {
	ESP_WriteString("AT+GMR");
	ESP_WriteString(AT_CMD_SUFFIX);
	return ESP_WaitForString(ESP_TIMEOUT_MS, 1, "OK");
}

uint32_t ESP_EnableEcho(bool echo) {
    if (echo) ESP_WriteString("ATE1");
    else ESP_WriteString("ATE0");
    ESP_WriteString(AT_CMD_SUFFIX);
    return ESP_WaitForString(ESP_TIMEOUT_MS, 1, "OK");
}

uint32_t ESP_Mode(ESP_MODE mode, bool save_to_flash) {
	ESP_WriteString("AT+CWMODE_");
	ESP_WriteString(save_to_flash ? "DEF=" : "CUR=");
	switch (mode) {
		case ESP_STATION:
			ESP_WriteString("1");
			break;
		case ESP_SOFTAP:
			ESP_WriteString("2");
			break;
		case ESP_BOTH:
			ESP_WriteString("3");
			break;
	}
	ESP_WriteString(AT_CMD_SUFFIX);
	return ESP_WaitForString(ESP_TIMEOUT_MS, 1, "OK");
}

uint32_t ESP_ConfigureConnection(ESP_CONNECTION con) {
	ESP_WriteString("AT+CIPMUX=");
	switch (con) {
		case ESP_SINGLE:
			ESP_WriteString("0");
			break;
		case ESP_MULTIPLE:
			ESP_WriteString("1");
			break;
	}
	ESP_WriteString(AT_CMD_SUFFIX);
	return ESP_WaitForString(ESP_TIMEOUT_MS, 3, "OK", "ALREADY CONNECTED", "ERROR");
}

uint32_t ESP_ListAP(char list[MAX_AP_NUM][MAX_SSID_LENGTH], bool sort) {
	if (ESP_ConfigListAP(sort) != 0) return -2;

	ESP_WriteString("AT+CWLAP");
	ESP_WriteString(AT_CMD_SUFFIX);

	uint32_t i;
	for (i = 0; i < MAX_AP_NUM; i++) {
		ESP_ReadString(AT_RECEIVING_BUFFER, ESP_TIMEOUT_MS);
		if (strcmp(AT_RECEIVING_BUFFER, "ERROR") == 0 || strcmp(AT_RECEIVING_BUFFER, "OK") == 0) {
			break;
		}

		int response_index = 9; // Response char index (starts at 9 to ignore <"+CWLAP:(">)
		char ch; // Current char

		for (int j = 0; j < MAX_SSID_LENGTH; j++) { // Iterate through each char:
			ch = AT_RECEIVING_BUFFER[response_index++];
			if (ch == '"') { // End
				list[i][j] = '\0';
				break;
			}
			else list[i][j] = ch;
		}
	}

	return i;
}

uint32_t ESP_Connect(char * ssid, char * pass, bool save_to_flash) {
	ESP_WriteString("AT+CWJAP");
	ESP_WriteString(save_to_flash ? "_DEF" : "_CUR");
	ESP_WriteString("=\"");
	ESP_WriteString(ssid);
	ESP_WriteString("\",\"");
	ESP_WriteString(pass);
	ESP_WriteString("\"");
	ESP_WriteString(AT_CMD_SUFFIX);

	return ESP_WaitForString(ESP_LONG_TIMEOUT_MS, 2, "OK", "FAIL");
}

uint32_t ESP_Disconnect(void) {
	ESP_WriteString("AT+CWQAP");
	ESP_WriteString(AT_CMD_SUFFIX);
	return ESP_WaitForString(ESP_LONG_TIMEOUT_MS, 1, "OK");
}

uint32_t ESP_Ip(unsigned char* ip) {
	ESP_WriteString("AT+CIFSR");
	ESP_WriteString(AT_CMD_SUFFIX);
    char received;
    do {
        if (!ESP_ReadChar(&received, ESP_TIMEOUT_MS)) return -1;
    } while (received < '0' || received > '9');
    for (unsigned char i = 0; i < 4; i++) {
    	ip[i] = 0;
        do {
        	ip[i] = 10 * ip[i] + received - '0';
        	if (!ESP_ReadChar(&received, ESP_TIMEOUT_MS)) return -1;
        } while (received >= '0' && received <= '9');
        if (!ESP_ReadChar(&received, ESP_TIMEOUT_MS)) return -1;
    }
    return ESP_WaitForString(ESP_TIMEOUT_MS, 1, "OK");
}

#ifdef FREERTOS
	void ESP_Take(void) {
		xSemaphoreTake(semESP, portMAX_DELAY);
	}
#endif

#ifdef FREERTOS
	void ESP_Give(void) {
		xSemaphoreGive(semESP);
	}
#endif

uint32_t ESP_Start(ESP_PROTOCOL protocol, char * ip, char * port, char * keepalive) {
	ESP_WriteString("AT+CIPSTART=\"");
    if (protocol == ESP_TCP) {
        ESP_WriteString("TCP");
    } else {
        ESP_WriteString("UDP");
    }
    ESP_WriteString("\",\"");
    ESP_WriteString(ip);

    ESP_WriteString("\",");
    ESP_WriteString(port);

    if (protocol == ESP_TCP && keepalive != 0) {
		ESP_WriteString(",");
		ESP_WriteString(keepalive);
    }

    ESP_WriteString(AT_CMD_SUFFIX);

    return ESP_WaitForString(ESP_LONG_TIMEOUT_MS, 3, "OK", "ALREADY CONNECTED", "ERROR");
}

uint32_t ESP_Close(void) {
	ESP_WriteString("AT+CIPCLOSE");
    ESP_WriteString(AT_CMD_SUFFIX);

	return ESP_WaitForString(ESP_LONG_TIMEOUT_MS, 1, "OK");
}

uint32_t ESP_Status(void) {
	ESP_WriteString("AT+CIPSTATUS");
	ESP_WriteString(AT_CMD_SUFFIX);
	ESP_WaitFor("STATUS:", ESP_TIMEOUT_MS);
	uint32_t ret = ESP_WaitForString(ESP_TIMEOUT_MS, 4, "2", "3", "4", "5") + 2;
	ESP_ReadString(AT_RECEIVING_BUFFER, ESP_TIMEOUT_MS);
	//printf("%s\n", AT_RECEIVING_BUFFER);
	return ret;
}

uint32_t ESP_SendData(char * data, int len) {
    ESP_WriteString("AT+CIPSEND=");
    char length[sizeof(int)];
    sprintf(length, "%u", len);
    ESP_WriteString(length);
    ESP_WriteString(AT_CMD_SUFFIX);
    ESP_WaitFor(">", ESP_TIMEOUT_MS);
    ESP_WriteBuffer(data, len);
    return ESP_WaitForString(ESP_TIMEOUT_MS, 3, "SEND OK", "SEND FAIL", "ERROR");
}

uint32_t ESP_ReceiveData(char * data, uint32_t max_len) {
	char received;
	int bytes = 0; // Current 'data' receiving index
	char len_str[5]; // Max of 4 characters (max length is 2048 for an IPD packet)
	do {
		if (len == 0) {
			// Wait for +IPD
			ESP_WaitFor("+IPD,", ESP_LONG_TIMEOUT_MS);

			// Read length:
			int i;
			for (i = 0;; i++) {
				if (!ESP_ReadChar(&received, ESP_TIMEOUT_MS)) return -1;
				if (received == ':') break;
				len_str[i] = received;
			}
			len_str[i] = '\0';
			char * dummy;
			len = strtol(len_str, &dummy, 10);
		}

		// Read to data 'len' bytes, or until 'max_len' is met:
		while (len > 0) {
			if (!ESP_ReadChar(&data[bytes], ESP_TIMEOUT_MS)) return -1;
			len--;
			if (bytes++ >= max_len - 1) break;
		}
	} while (max_len > bytes); // While max_len hasn't been met

    return bytes;
}

uint32_t ESP_WaitForString(uint32_t timeout, int resNum, ...) {
	va_list valist;
	va_start(valist, resNum);

	char * res[resNum];

	for (int i = 0; i < resNum; i++) {
		res[i] = va_arg(valist, char*);
	}

	while (ESP_ReadString(AT_RECEIVING_BUFFER, timeout) > 0) {
		for (int i = 0; i < resNum; i++) {
			if (strcmp((char *) AT_RECEIVING_BUFFER, res[i]) == 0) {
				va_end(valist);
				return i;
			}
		}
	}
	va_end(valist);
	return -1;
}

uint32_t ESP_WaitFor(char * response, uint32_t timeout) {
	uint32_t bytes = 0;
	int i = 0;
	char ch;
	do {
		if (!ESP_ReadChar(&ch, timeout)) break;
		AT_RECEIVING_BUFFER[bytes] = ch;
		bytes++;
		if (ch == response[i]) {
			i++;
		}
		else i = 0;
	} while (response[i] != 0);
	return bytes;
}

uint32_t ESP_WriteString(char * cmd) {
	return UART_WriteString((unsigned char *) cmd);
}

uint32_t ESP_WriteBuffer(char * cmd, uint32_t len) {
	return UART_WriteBuffer((unsigned char *) cmd, len);
}

void ESP_WriteChar(char ch) {
	UART_WriteChar(ch);
}

bool ESP_ReadChar(char * ch, uint32_t timeout) {
	return UART_ReadChar((unsigned char *) ch, timeout);
}

bool ESP_GetChar(char * ch) {
	return UART_GetChar((unsigned char *) ch);
}

uint32_t ESP_ReadString(char * response, uint32_t timeout) {
	return UART_ReadString((unsigned char *) response, timeout);
}

uint32_t ESP_ReadBuffer(char * response, uint32_t len, uint32_t timeout) {
	return UART_ReadBuffer((unsigned char *) response, len, timeout);
}


