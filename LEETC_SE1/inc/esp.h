/*
* @file		esp.h
* @brief	Contains the ESP-8266 API.
* @version	1.0
* @date		May 2021
* @author	PedroG
*
* Copyright(C) 2020-2025, PedroG
* All rights reserved.
 */

#ifndef ESP_H_
#define ESP_H_

/** @defgroup ESP ESP
 * This package provides the core capabilities for ESP-8266 interface functions.
 * @{
 */

/** @defgroup ESP_Public_Functions ESP Public Functions
 * @{
 */


#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include "uart.h"
#include "wait.h"

#ifdef FREERTOS
	#include "FreeRTOS.h"
	#include "semphr.h"
#endif


/*
 *
 *
 * Constants:
 *
 *
 */

/**
 * @brief	Default timeout time.
 */
#define ESP_TIMEOUT_MS 2000

/**
 * @brief	Long timeout time.
 */
#define ESP_LONG_TIMEOUT_MS 20000

/**
 * @brief	Max length of packet received.
 */
#define AT_MAX_IPD_PACKET 2048

/**
 * @brief	Suffix to all AT Commands.
 */
#define AT_CMD_SUFFIX "\r\n"

/**
 * @brief	Max length considered for an AT Command Response.
 */
#define AT_RESPONSE_MAX_LENGTH 1024

/**
 * @brief	ESP_01 Reset pins.
 */
#define ESP_RST (1 << 27)

/**
 * @brief	Max size of a CWLAP AP response.
 */
#define MAX_CWLAP_SIZE 512

/**
 * @brief	Maximum number of AP's to request.
 */
#define MAX_AP_NUM 10

/**
 * @brief	Maximum length of AP's SSID.
 */
#define MAX_SSID_LENGTH 128


/**
 * @brief	Network protocol useb by ESP.
 */
typedef enum {
	ESP_TCP, /*!< TCP Protocol. */
	ESP_UDP /*!< UDP Protocol. */
} ESP_PROTOCOL;

/**
 * @brief	ESP transmission mode.
 */
typedef enum {
	ESP_STATION, /*!< Station Mode. */
	ESP_SOFTAP, /*!< Access Point Mode. */
	ESP_BOTH /*!< Both Station and Access Point Mode. */
} ESP_MODE;

/**
 * @brief	ESP connection type.
 */
typedef enum {
	ESP_SINGLE, /*!< Single Connection. */
	ESP_MULTIPLE, /*!< Multiple Connection. */
} ESP_CONNECTION;

/**
 * @brief	Structure representing Access Point.
 * @note	This object was left unused, but it could be used in ListAP function, with the right parser.
 */
typedef struct {
	char ecn[2]; /*!< Encryption method: <br>
						0: OPEN <br>
						1: WEP <br>
						2: WPA_PSK <br>
						3: WPA2_PSK <br>
						4: WPA_WPA2_PSK <br>
						5: WPA2_Enterprise (AT can NOT connect to WPA2_Enterprise AP for now.) */
	char ssid[56]; /*!< SSID of AP */
	char rssi[16]; /*!< Strength of the signal */
	char mac[18]; /*!< MAC Addresses are always 12-digits long + 5 ';'s */
	char channel[8]; /*!< Channel number. */
	char freq_offset[8]; /*!< Frequency offset in KHz. The value of ppm is freq_offset/2.4. */
	char freq_cali[8]; /*!< Calibration for frequency offset. */
	char pairwise_cipher[2]; /*!< Pairwise cipher: <br>
									0: CIPHER_NONE <br>
									1: CIPHER_WEP40 <br>
									2: CIPHER_WEP104 <br>
									3: CIPHER_TKIP <br>
									4: CIPHER_CCMP <br>
									5: CIPHER_TKIP_CCMP <br>
									6: CIPHER_UNKNOWN */
	char group_cipher[2]; /*!< Definitions of cipher types are same as "pairwise_cipher". */
	char bgn[2]; /*!< Consists of 3 bits: <br>
						- Bit0 is for 802.11b mode; <br>
						- Bit1 is for 802.11g mode; <br>
						- Bit2 is for 802.11n mode. <br>
						If the value of the bit is 1, the corresponding 802.11 mode is enabled. */
	char wps[2]; /*!< 0, WPS is disabled; 1, WPS is enabled. */
} ESP_AP;


/*
 *
 *
 * Functions:
 *
 *
 */


/**
 * @brief	Test if ESP-8266 module is properly started.
 * @param	baud: -> Baud Rate.
 * @return  true if it is started, false otherwise.
 */
bool ESP_Init(unsigned int baud);

/**
 * @brief	Test if ESP-8266 module is properly started.
 * @return  0 if it is started, -1 otherwise.
 */
uint32_t ESP_Test(void);

/**
 * @brief	Restart ESP-8266 module.
 * @return  0 if it was properly restarted, -1 otherwise.
 */
uint32_t ESP_Restart(void);

/**
 * @brief	Read version information.
 * @return  0 if successful, -1 otherwise.
 */
uint32_t ESP_Version(void);

/**
 * @brief	Enable/disable command echoing.
 * @param   echo: -> If true, echo is enabled.
 * @return  0 if successful, -1 otherwise.
 */
uint32_t ESP_EnableEcho(bool echo);

/**
 * @brief	Set WiFi mode
 * @param   mode: -> Can be ESP_STATION or ESO_SOFTAP.
 * @param	save_to_flash: -> Whether to save configuration to flash or not.
 * @return  0 if mode was correctly set, -1 otherwise.
 */
uint32_t ESP_Mode(ESP_MODE mode, bool save_to_flash);

/**
 * @brief	Set Connection type.
 * @param   con: -> Can be ESP_SINGLE or ESO_MULTIPLE.
 * @return  0 if connection type was correctly set;
 * 				1 if a connection is already established;
 * 				2 if an error was received from ESP-8266;
 * 				-1 if an UART error occurred.
 */
uint32_t ESP_ConfigureConnection(ESP_CONNECTION con);

/**
 * @brief	Get list of nearby Access Points.
 * @param   list: -> Array of AP's where list should be written.
 * @param   sort: -> If true, results will be sorted according to the RSSI parameter (signal strength).
 * @return  Number of AP's written to list.
 */
uint32_t ESP_ListAP(char list[MAX_AP_NUM][MAX_SSID_LENGTH], bool sort);

/**
 * @brief 	Connect to an access point.
 * @param 	ssid: -> The SSID to connect to.
 * @param 	pass: -> The password of the network.
 * @param	save_to_flash: -> Whether to save configuration to flash or not.
 * @return 	0 if connection started successfully;
 * 				1 if an error was received from ESP-8266;
 * 				-1 if an UART error occurred.
 */
uint32_t ESP_Connect(char * ssid, char * pass, bool save_to_flash);

/**
 * @brief	Disconnect from access point.
 * @return	0 if successfully disconnected, -1 otherwise.
 */
uint32_t ESP_Disconnect(void);

/**
 * @brief	Get the current local IPv4 address.
 * @param   ip: -> String in which IP will be written.
 * @return	0 if IP was successfully read, -1 otherwise.
 * @note    The result will not be stored as a string but byte by byte.
 * 			For example, for the IP 192.168.0.1, the value of store_in
 * 			will be: {0xc0, 0xa8, 0x00, 0x01}.
 */
uint32_t ESP_Ip(unsigned char * ip);

#ifdef FREERTOS
	/**
	 * @brief	Take ESP driver. From now on, driver is busy, and won't be used on other tasks.
	 * @note	This function is blocking.
	 */
	void ESP_Take(void);
#endif

#ifdef FREERTOS
	/**
	 * @brief	Give ESP driver. From now on, driver isn't busy, and can be used on other tasks.
	 * @note	This function is blocking.
	 */
	void ESP_Give(void);
#endif

/**
 * @brief	Open a TCP or UDP connection.
 * @param   protocol: -> Either ESP_TCP or ESP_UDP.
 * @param   ip: -> String containing the IP or host name to connect to.
 * @param   port: -> The port to connect to.
 * @param	keepalive: -> Detection time interval in which TCP connection is kept alive. Only relevant when protocol = ESP_TCP. Max of 7200.
 * @return  0 if connection successfully started;
 * 				1 if a connection was already started, but driver didn't keep track of it;
 * 				2 if an error was received from ESP-8266;
 * 				-1 if an UART error occurred;
 * 				-2 if a connection was already started.
 */
uint32_t ESP_Start(ESP_PROTOCOL protocol, char * ip, char * port, char * keepalive);

/**
 * @brief	Close current connection.
 * @return  0: Successfully closed connection.
 * 			-1: Error.
 */
uint32_t ESP_Close(void);

/**
 * @brief	Get status of connection.
 * @return  2: The ESP8266 Station is connected to an AP and its IP is obtained.
			3: The ESP8266 Station has created a TCP or UDP transmission.
			4: The TCP or UDP transmission of ESP8266 Station is disconnected.
			5: The ESP8266 Station does NOT connect to an AP.
 */
uint32_t ESP_Status(void);

/**
 * @brief	Send data over connection.
 * @param   data: -> Data to send.
 * @param   len: -> Length of data.
 * @return  0 if data was sent correctly;
 * 				1 if ESP-8266 failed to send the data;
 * 				2 if an error was received from ESP-8266;
 * 				-1 if an UART error occurred.
 */
uint32_t ESP_SendData(char * data, int len);

/**
 * @brief	Receive data over connection. Waits for +IPD and reads data until max_len is met or data ends.
 * 			If last call to this function left data in the buffer, this function will first look at that data, and only then wait for a new +IPD, if max_len hasn't been met.
 * @param   data: -> Where data will be written to.
 * @param   max_len: -> Maximum length expected. If this length is met before length of +IPD, only max_len is returned. Although, UART still reads the rest, so it's not left abandoned.
 * @return	Bytes read.
 */
uint32_t ESP_ReceiveData(char * data, uint32_t max_len);

/**
 * @brief	Send AT string.
 * @param   cmd: -> String.
 * @return 	Number of bytes put.
 */
uint32_t ESP_WriteString(char * cmd);

/**
 * @brief	Receive AT string response.
 * @param   response: -> Response received.
 * @param	timeout: -> UART timeout in ms.
 * @return	Number of bytes got.
 */
uint32_t ESP_ReadString(char * response, uint32_t timeout);

/**
 * @brief	Send AT buffer.
 * @param   cmd: -> Buffer.
 * @param   len: -> Buffer length in bytes.
 * @return	Number of bytes put.
 */
uint32_t ESP_WriteBuffer(char * cmd, uint32_t len);

/**
 * @brief	Receive AT buffer response.
 * @param   response: -> Buffer.
 * @param   len: -> Buffer length in bytes.
 * @param	timeout: -> UART timeout in ms.
 * @return	Number of bytes got.
 */
uint32_t ESP_ReadBuffer(char * response, uint32_t len, uint32_t timeout);

/**
 * @brief	Send a character to ESP.
 * @return 	Number of bytes got.
 * @param   ch: -> Character to write.
 */
void ESP_WriteChar(char ch);

/**
 * @brief	Receive a character from ESP.
 * @param	ch: -> Where character will be written to.
 * @param	timeout: -> UART timeout in ms.
 * @return  True if a character was read, false if timeout was met.
 * @note	If no character is found, function will block, waiting for one.
 */
bool ESP_ReadChar(char * ch, uint32_t timeout);

/**
 * @brief	Receive a character from ESP, if there is one.
 * @param	ch: -> Pointer to character read.
 * @return 	True if a character was read. False if no character was found.
 */
bool ESP_GetChar(char * ch);

/**
 * @brief	Wait for ESP to meet one of a certain amount of string responses.
 * @param	timeout: -> UART timeout in ms.
 * @param	resNum: -> Number of responses to consider.
 * @param	...: -> String responses.
 * @return  The index of response met.
 * @note	This function reads from UART string by string, until a response is met.
 */
uint32_t ESP_WaitForString(uint32_t timeout, int resNum, ...);

/**
 * @brief	Wait for ESP to meet a certain response (not necessarily a string with '\0').
 * @param	response: -> String response.
 * @param	timeout: -> UART timeout in ms.
 * @return  Number of bytes read.
 * @note	This function reads from UART char by char, until response is met.
 */
uint32_t ESP_WaitFor(char * response, uint32_t timeout);


/**
 * @}
 */


/**
 * @}
 */

#endif /* ESP_H_ */
