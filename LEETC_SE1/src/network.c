/*
 * ntp.c
 *
 *  Created on: May 2021
 *      Author: PedroG
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif


#include "network.h"


static QueueHandle_t queuePUBLISH_SCORE = NULL;

static int score = 0;

void NETWORK_ScorePublisherTask(void *pvParameters);


static bool NETWORK_Connect(const char *host, const unsigned short int port, const unsigned short int keepalive);
static int NETWORK_Send(unsigned char *address, unsigned int bytes);
static int NETWORK_Recv(unsigned char *address, unsigned int maxbytes);


static uint32_t ntohl(uint32_t netlong) {
	return __builtin_bswap32(netlong);
}

static void MQTT_GetInitialiser(MQTTString * copyTo) {
	MQTTString topicString = MQTTString_initializer;
	memcpy(copyTo, &topicString, sizeof(MQTTString));
}

bool NETWORK_Init(void) {
	if (xTaskCreate(NETWORK_ScorePublisherTask, (const char * const) "SCORE Publisher Task", TASK_SCORE_PUBLISHER_STACK_SIZE, NULL, TASK_SCORE_PUBLISHER_PRIORITY, NULL) != pdPASS) {
		printf("SCORE Publisher Task could not be created.\n");
		return false;
	}

	if ((queuePUBLISH_SCORE = xQueueCreate(8, sizeof(int))) == NULL) {
		printf("Could not initialise queueSCORE");
		return false;
	}

	return true;
}

bool NETWORK_ConnectToAP(char * ssid, char * password) {
	if (ESP_Init(115200)) {
		if (ESP_EnableEcho(false) == 0) {
			//printf("Echo Successful!\n");
			if (ESP_Mode(ESP_BOTH, false) == 0) {
				//printf("Mode Successful!\n");
				if (ESP_ConfigureConnection(ESP_SINGLE) == 0) {
					//printf("Configure Connection Successful!\n");
					if (ESP_Connect(ssid, password, false) == 0) {
						//printf("Connect Successful!\n");
						return true;
					}
					else printf("Connect Failed.\n");
				}
				else printf("Configure Connection Failed.\n");
			}
			else printf("Mode Failed.\n");
		}
		else printf("Echo Failed.\n");
	}
	else printf("Initialisation Failed.\n");
	return false;
}

uint32_t NETWORK_GetSeconds(void) {
	ESP_Take();

	// Create and zero out the packet. All 48 bytes worth:
	NTP_PACKET packet = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	memset(&packet, 0, sizeof(NTP_PACKET));

	/**
	 * Set the first byte's bits to 0001 1011.
	 * li = 0, vn = 3, and mode = 3.
	 * The rest will be left set to zero.
	 */
	packet.li_vn_mode = 0xE3;
	packet.stratum = 0;
	packet.poll = 6;
	packet.precision = 0xEC;

	NTP_PACKET ret = {0};

	if (ESP_Start(ESP_UDP, "pool.ntp.org", "123", 0) == 0) {
		//printf("Start Successful!\n");
		if (ESP_SendData((char *) &packet, sizeof(NTP_PACKET)) == 0) {
			//printf("Send Successful!\n");
			if (ESP_ReceiveData((char *) &ret, sizeof(NTP_PACKET)) < 0) {
				printf("Timed out\n");
			}
			if (ESP_Close() == 0) {
				ESP_Give();
				return ntohl(ret.txTm_s) - 2208988800 + 3600;
			}
			else printf("Close Failed.\n");
		}
		else printf("Send Failed.\n");
	}
	else printf("Start Failed.\n");

	ESP_Give();

	return -1;
}

static bool NETWORK_Connect(const char *host, const unsigned short int port, const unsigned short int keepalive) {
	char portStr[5];
	char keepaliveStr[5];
	sprintf(portStr, "%u", port);
	sprintf(keepaliveStr, "%u", keepalive); // Max of 7200
	uint32_t ret = ESP_Start(ESP_TCP, (char *) host, portStr, keepaliveStr);
	return (ret == 0 || ret == 1);
}

static int NETWORK_Send(unsigned char *address, unsigned int bytes) {
	if (ESP_SendData((char *) address, bytes) != 0) return 0;
	return bytes;
}

static int NETWORK_Recv(unsigned char *address, unsigned int maxbytes) {
	return ESP_ReceiveData((char *) address, maxbytes);
}

void NETWORK_PublishScore(int scoreSend) {
	xQueueSend(queuePUBLISH_SCORE, &scoreSend, portMAX_DELAY);
}

void NETWORK_ScorePublisherTask(void *pvParameters) {
	unsigned char buffer[128];
	MQTTTransport transporter;
	MQTTString topicString;
	int result;
	int length;

	// ESP8266 Transport Layer
	static transport_iofunctions_t iof = { NETWORK_Send, NETWORK_Recv };
	int transport_socket = transport_open(&iof);

	int state = NETWORK_STATE_IDLE;
	for (;;) {
		switch (state) {
			case NETWORK_STATE_IDLE:
				xQueueReceive(queuePUBLISH_SCORE, &score, portMAX_DELAY);
				ESP_Take();
				state++;
			case NETWORK_STATE_INIT:
				if (NETWORK_Connect(MQTT_ADDRESS, MQTT_PORT, CONNECTION_KEEPALIVE_S)) {
					state++;
				}
				break;
			case NETWORK_STATE_CONNECT:
				transporter.sck = &transport_socket;
				transporter.getfn = transport_getdatanb;
				transporter.state = 0;

				MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
				connectData.MQTTVersion = 3;
				connectData.username.cstring = MQTT_DEVICE_TOKEN;
				connectData.clientID.cstring = MQTT_DEVICE_TOKEN;
				connectData.keepAliveInterval = CONNECTION_KEEPALIVE_S * 2;
				length = MQTTSerialize_connect(buffer, sizeof(buffer), &connectData);
				// Send CONNECT to the mqtt broker.
				if ((result = transport_sendPacketBuffer(transport_socket, buffer, length)) == length) {
					state++;
				} else {
					state = NETWORK_STATE_INIT;
				}
				break;
			case NETWORK_STATE_WAIT_CONNECT:
				// Wait for CONNACK response from the MQTT broker.
				while (true) {
					if ((result = MQTTPacket_readnb(buffer, sizeof(buffer), &transporter)) == CONNACK) {
						// Check if the connection was accepted.
						unsigned char sessionPresent, connack_rc;
						if ((MQTTDeserialize_connack(&sessionPresent, &connack_rc, buffer,
								sizeof(buffer)) != 1) || (connack_rc != 0)) {
							state = NETWORK_STATE_INIT;
							break;
						} else {
							state++;
							break;
						}
					} else if (result == -1) {
						state = NETWORK_STATE_INIT;
						break;
					}
				}
				break;
			case NETWORK_STATE_PUBLISH:
				MQTT_GetInitialiser(&topicString);
				topicString.cstring = "v1/devices/me/telemetry";
				unsigned char payload[64];
				length = sprintf((char *) payload, "{\"group\":%d, \"score\":%d}", GROUP_ID, score);
				length = MQTTSerialize_publish(buffer, sizeof(buffer), 0, 0, 0, 0, topicString, payload, length);
				// Send PUBLISH to the MQTT broker.
				if ((result = transport_sendPacketBuffer(transport_socket, buffer, length)) == length) {
					state++;
				} else {
					state = NETWORK_STATE_INIT;
				}
				break;
			case NETWORK_STATE_DISCONNECT:
				if (ESP_Close() == 0) state = NETWORK_STATE_IDLE;
				ESP_Give();
			default:
				state = NETWORK_STATE_IDLE;
			}
	}

	vTaskDelete(NULL);
}

