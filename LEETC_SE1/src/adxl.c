/*
 * adxl.c
 *
 *  Created on: Jan 2021
 *      Author: PedroG
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif


#include "adxl.h"


#ifdef FREERTOS
	static QueueHandle_t queueADXL; // ADXL Queue
	void ADXL_AxisTask(void *pvParameters); // ADXL Axis Updater Task
#endif


// Buffers for individual writing operations:
static signed short txBuffer[MAX_LENGTH] = {0};
static signed short rxBuffer[MAX_LENGTH] = {0};

static char getDataRate(int frequency) {
	if (frequency >= 2000000) return 0x0e;
	else if (frequency >= 400000) return 0x0d;
	else if (frequency >= 200000) return 0x0c;
	else if (frequency >= 100000) return 0x0b;
	else if (frequency >= 50000) return 0x0a;
	else if (frequency >= 25000) return 0x09;
	else if (frequency >= 12500) return 0x08;
	else if (frequency >= 6250) return 0x07;
	else if (frequency >= 3130) return 0x06;
	else if (frequency >= 1560) return 0x05;
	else if (frequency >= 780) return 0x04;
	else if (frequency >= 390) return 0x03;
	else if (frequency >= 200) return 0x02;
	else if (frequency >= 100) return 0x01;
	else return 0x00;
}

static void ADXL_ChipSelect(bool enable) {
	if (enable) LPC_GPIO0->FIOCLR = (1 << CS);
	else LPC_GPIO0->FIOSET = (1 << CS);
}

static int32_t ADXL_Transfer(signed short *txBuffer, signed short *rxBuffer, int length) {
	ADXL_ChipSelect(true);
	if (SPI_Transfer((unsigned short *) txBuffer, (unsigned short *) rxBuffer, length) < 0) return -1;
	ADXL_ChipSelect(false);
	return 0;
}

int32_t ADXL_Init(int frequency, int dataResolution) {
	// Initialise SPI:
	SPI_Init();

	if (frequency == 0) frequency = 1562500;

	if (SPI_VerifyFrequency(frequency) < 0) return -1;

	LPC_PINCON->PINSEL1 &= ~(0x03 << 0); // CS
	LPC_GPIO0->FIODIR |= (1 << CS); // Output

	// Set !CS to high (i.e. disable CS):
	ADXL_ChipSelect(false);

	// Configure SPI with 16 bits of data, CPHA = 1 and CPOL = 1:
	if (SPI_ConfigTransfer(frequency, 8, 0x03) < 0) return -1;

	// Send data format:
	txBuffer[0] = DATA_FORMAT;
	txBuffer[1] = ADXL_RESOLUTION_BIT | dataResolution;
	if (ADXL_Transfer(txBuffer, rxBuffer, 2) < 0) return -1;

	// Send data rate:
	txBuffer[0] = BW_RATE;
	txBuffer[1] = getDataRate(frequency);
	if (ADXL_Transfer(txBuffer, rxBuffer, 2) < 0) return -1;

	// Send FIFO settings:
	txBuffer[0] = FIFO_CTL;
	txBuffer[1] = ADXL_FIFO_MODE | ADXL_SAMPLE_BITS;
	if (ADXL_Transfer(txBuffer, rxBuffer, 2) < 0) return -1;

	// Send power saving features (get out of standby mode and enter measurement mode):
	txBuffer[0] = POWER_CTL;
	txBuffer[1] = ADXL_MEASURE_BIT;
	if (ADXL_Transfer(txBuffer, rxBuffer, 2) < 0) return -1;

	#ifdef FREERTOS
		if (xTaskCreate(ADXL_AxisTask, (const char * const) "ADXL Axis Task", TASK_ADXL_AXIS_STACK_SIZE, NULL, TASK_ADXL_AXIS_PRIORITY, NULL) != pdPASS) {
			printf("ADXL Axis Task could not be created.\n");
			return -1;
		}

		if ((queueADXL = xQueueCreate(1, sizeof(AXIS))) == NULL) {
			printf("Could not initialise queueADXL");
			return -1;
		}
	#endif

	return 0;
}

static AXIS ADXL_BareGetAxis() {
	AXIS axis = {.x = 0, .y = 0, .z = 0};
	txBuffer[0] = (ADXL_READ_BIT | ADXL_MB_BIT | DATAX0);
	if (ADXL_Transfer(txBuffer, rxBuffer, 7) < 0) return axis;
	axis.x = (rxBuffer[2] << 8) | rxBuffer[1];
	axis.y = (rxBuffer[4] << 8) | rxBuffer[3];
	axis.z = (rxBuffer[6] << 8) | rxBuffer[5];
	return axis;
}

AXIS ADXL_GetAxis() {
	#ifdef FREERTOS
		AXIS axis;
		xQueueReceive(queueADXL, &axis, portMAX_DELAY);
		return axis;
	#else
		return ADXL_BareGetAxis();
	#endif
}

int ADXL_GetDevId(void) {
	txBuffer[0] = (ADXL_READ_BIT | DEVID);
	if (ADXL_Transfer(txBuffer, rxBuffer, 2) < 0) return -1;
	return rxBuffer[1] & 0xff;
}

#ifdef FREERTOS
	void ADXL_AxisTask(void * pvParameters) {
		AXIS axis;
		for (;;) {
			axis = ADXL_BareGetAxis();
			xQueueOverwrite(queueADXL, &axis);
		}
	}
#endif

