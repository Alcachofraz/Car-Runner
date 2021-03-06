/*
* @file		ntp.h
* @brief	Contains the NTP API.
* @version	1.0
* @date		May 2021
* @author	PedroG
*
* Copyright(C) 2020-2025, PedroG
* All rights reserved.
 */

#ifndef NETWORK_H_
#define NETWORK_H_

/** @defgroup NTP NTP
 * This package provides the core capabilities for NTP interface functions.
 * @{
 */

/** @defgroup NTP_Public_Functions NTP Public Functions
 * @{
 */


#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "esp.h"

#include "MQTTPacket.h"
#include "transport.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"



/*
 *
 *
 * Constants:
 *
 *
 */

/**
 * @brief	NTP packet structure for NTP transmitting and receiving.
 */
typedef struct {
	uint8_t li_vn_mode; 		/*!< Eight bits. li, vn, and mode.
										li. Two bits. Leap indicator.
										vn. Three bits. Version number of the protocol.
										mode. Three bits. Client will pick mode 3 for client. */
	uint8_t stratum; 			/*!< Eight bits. Stratum level of the local clock. */
	uint8_t poll; 				/*!< Eight bits. Maximum interval between successive messages. */
	uint8_t precision; 			/*!< Eight bits. Precision of the local clock. */

	uint32_t rootDelay; 		/*!< 32 bits. Total round trip delay time. */
	uint32_t rootDispersion; 	/*!< 32 bits. Max error aloud from primary clock source. */
	uint32_t refId; 			/*!< 32 bits. Reference clock identifier. */

	uint32_t refTm_s; 			/*!< 32 bits. Reference time-stamp seconds. */
	uint32_t refTm_f; 			/*!< 32 bits. Reference time-stamp fraction of a second. */

	uint32_t origTm_s; 			/*!< 32 bits. Originate time-stamp seconds. */
	uint32_t origTm_f; 			/*!< 32 bits. Originate time-stamp fraction of a second. */

	uint32_t rxTm_s; 			/*!< 32 bits. Received time-stamp seconds. */
	uint32_t rxTm_f; 			/*!< 32 bits. Received time-stamp fraction of a second. */

	uint32_t txTm_s; 			/*!< 32 bits. Transmit time-stamp seconds. Important. */
	uint32_t txTm_f; 			/*!< 32 bits. Transmit time-stamp fraction of a second. */
} NTP_PACKET; 			// Total: 384 bits or 48 bytes.

/**
 * @brief	Keep alive parameter.
 */
#define CONNECTION_KEEPALIVE_S 	60UL

/**
 * @brief	MQTT Address for score publishing.
 */
#define MQTT_ADDRESS			"iot-se2021.ddns.net"

/**
 * @brief	MQTT Port for score publishing.
 */
#define MQTT_PORT				1883

/**
 * @brief	MQTT Device Token for score publishing.
 */
#define MQTT_DEVICE_TOKEN		"SE2-BEST-RECORDS"

/**
 * @brief	Group Identification for score publishing.
 */
#define GROUP_ID 9

/**
 * @brief	Score Publisher Task Stack Size.
 */
#define TASK_SCORE_PUBLISHER_STACK_SIZE configMINIMAL_STACK_SIZE*2

/**
 * @brief	Score Publisher Task Priority.
 */
#define TASK_SCORE_PUBLISHER_PRIORITY tskIDLE_PRIORITY + 1

/**
 * @brief	Network State Machine states.
 */
typedef enum {
	NETWORK_STATE_IDLE = 0,
	NETWORK_STATE_INIT = 1,
	NETWORK_STATE_CONNECT = 2,
	NETWORK_STATE_WAIT_CONNECT = 3,
	NETWORK_STATE_PUBLISH = 4,
	NETWORK_STATE_DISCONNECT = 5
} NEWTORK_STATE;



/*
 *
 *
 * Functions:
 *
 *
 */

/**
 * @brief	Initialise Network API.
 * @return	True if successful, false otherwise.
 * @note	Must be called prior to other NETWORK functions.
 */
bool NETWORK_Init(void);

/**
 * @brief	Connect to Access Point.
 * @param	ssid: -> SSID of Access Point.
 * @param	password: -> Password of Access Point.
 * @return	True if successful, false otherwise.
 */
bool NETWORK_ConnectToAP(char * ssid, char * password);

/**
 * @brief	Get current time seconds.
 * @return	Seconds since 01/01/1970 00:00:00.
 */
uint32_t NETWORK_GetSeconds(void);

/**
 * @brief	Publish a score.
 * @param	scoreSend: -> Score to publish.
 */
void NETWORK_PublishScore(int scoreSend);


/**
 * @}
 */


/**
 * @}
 */

#endif /* NETWORK_H_ */
