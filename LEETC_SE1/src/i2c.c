/*
 * i2c.c
 *
 *  Created on: Dec 2020
 *      Author: PedroG
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif


#include "i2c.h"


static char I2C1WriteBuffer[I2C_BUFFER_LENGTH]; // Data being transmitted in I2C1 interface
static char I2C2WriteBuffer[I2C_BUFFER_LENGTH]; // Data being transmitted in I2C2 interface

static char I2C1ReadBuffer[I2C_BUFFER_LENGTH]; // Data being received in I2C1 interface
static char I2C2ReadBuffer[I2C_BUFFER_LENGTH]; // Data being received in I2C2 interface

static int I2C1WriteLength; // Length of data to be transmitted in I2C1 interface in bytes
static int I2C2WriteLength; // Length of data to be transmitted in I2C2 interface in bytes

static int I2C1ReadLength; // Length of data to be received in I2C1 interface in bytes
static int I2C2ReadLength; // Length of data to be received in I2C2 interface in bytes

static int I2C1WriteIndex; // Current byte to be transmitted in I2C1 interface
static int I2C2WriteIndex; // Current byte to be transmitted in I2C2 interface

static int I2C1ReadIndex; // Current byte to be received in I2C1 interface
static int I2C2ReadIndex; // Current byte to be received in I2C2 interface

static I2C_STATE I2C1State; // Current state in I2C1 device driver
static I2C_STATE I2C2State; // Current state in I2C2 device driver

static uint32_t I2C_GetPCLK(void) {
	switch ((LPC_SC->PCLKSEL0 >> 6) & 0x03) {
		case 0x00:
			return SystemCoreClock/4;
		case 0x01:
			return SystemCoreClock;
		case 0x02:
			return SystemCoreClock/2;
		case 0x03:
			return SystemCoreClock/8;
		default:
			return -1;
	}
}

static int I2C_IsEnabled(IRQn_Type IRQ) { // See if i2c interrupts are enabled
	return NVIC->ISER[((uint32_t)(IRQ) >> 5)] & (1 << ((uint32_t)(IRQ) & 0x1F));
}


void I2C1_IRQHandler(void) { // I2C1 interrupt handler

	switch (LPC_I2C1->I2STAT) {
		case I2C_START: // Start
			I2C1WriteIndex = 0;
			LPC_I2C1->I2DAT = I2C1WriteBuffer[I2C1WriteIndex++];
			LPC_I2C1->I2CONCLR = STA;
			I2C1State = I2C_BUSY;
			break;

		case I2C_REPEATED_START: // Repeated Start
			I2C1ReadIndex = 0;
			LPC_I2C1->I2DAT = I2C1WriteBuffer[I2C1WriteIndex++];
			LPC_I2C1->I2CONCLR = STA;
			I2C1State = I2C_BUSY;
			break;

		case I2C_SLAW_ACK: // [SLA + W] transmitted and ACK received
			if (I2C1WriteLength == 1) { // There's no data to send
				LPC_I2C1->I2CONSET = STO; // Set STOP flag
				I2C1State = I2C_IDLE;
			}
			else {
				LPC_I2C1->I2DAT = I2C1WriteBuffer[I2C1WriteIndex++];
			}
			break;

		case I2C_SLAW_NACK: // [SLA + W] transmitted and NACK received
			LPC_I2C1->I2CONSET = STO;
			I2C1State = I2C_IDLE;
			break;

		case I2C_DATAW_ACK: // Data byte transmitted, ACK received
			if (I2C1WriteIndex < I2C1WriteLength) {
				LPC_I2C1->I2DAT = I2C1WriteBuffer[I2C1WriteIndex++]; // Keep transmitting
			}
			else { // There's nothing more to transmit
				if (I2C1ReadLength != 0) { // There's something to receive
					LPC_I2C1->I2CONSET = STA; // Repeated start for read operation
				}
				else {
					LPC_I2C1->I2CONSET = STO;
					I2C1State = I2C_IDLE;
				}
			}
			break;

		case I2C_DATAW_NACK: // Data byte transmitted, NACK received
			LPC_I2C1->I2CONSET = STO;
			I2C1State = I2C_IDLE;
			break;

		case I2C_ARBITRATION_LOST: // Arbitration lost. This API doesn't handle with multiple Master operations.
			LPC_I2C1->I2CONSET = STO;
			I2C1State = I2C_IDLE;
			break;

		case I2C_SLAR_ACK: // [SLA + R] transmitted and ACK received
			if (I2C1ReadLength > 0) {
				// Will go to I2C_DATAR_ACK
				LPC_I2C1->I2CONSET = AA; // Assert ACK
			}
			else {
				// Will go to I2C_DATAR_NACK
				LPC_I2C1->I2CONCLR = AA; // Assert NACK
			}
			break;

		case I2C_SLAR_NACK: // [SLA + R] transmitted and NACK received.
			LPC_I2C1->I2CONSET = STO;
			I2C1State = I2C_IDLE;
			break;

		case I2C_DATAR_ACK: // Data byte received, ACK returned
			I2C1ReadBuffer[I2C1ReadIndex++] = LPC_I2C1->I2DAT;
			if (I2C1ReadIndex < I2C1ReadLength) {
				LPC_I2C1->I2CONSET = AA; // Assert ACK
			}
			else {
				LPC_I2C1->I2CONCLR = AA; // Assert NACK
			}
			break;

		case I2C_DATAR_NACK: // Data byte received, NACK returned
			I2C1ReadBuffer[I2C1ReadIndex++] = LPC_I2C1->I2DAT;
			LPC_I2C1->I2CONSET = STO;
			I2C1State = I2C_IDLE;
			break;

		default:
			LPC_I2C1->I2CONSET = STO;
			I2C1State = I2C_IDLE;
			break;
	}
	LPC_I2C1->I2CONCLR = SI;
}

void I2C2_IRQHandler(void) { // I2C2 interrupt handler
	switch (LPC_I2C2->I2STAT) {
			case I2C_START: // Start
				I2C2WriteIndex = 0;
				I2C2ReadIndex = 0;
				LPC_I2C2->I2DAT = I2C2WriteBuffer[I2C2WriteIndex++];
				LPC_I2C2->I2CONCLR = STA;
				I2C2State = I2C_BUSY;
				break;

			case I2C_REPEATED_START: // Repeated Start
				I2C2WriteIndex = 0;
				I2C2ReadIndex = 0;
				LPC_I2C2->I2DAT = I2C2WriteBuffer[I2C2WriteIndex++];
				LPC_I2C2->I2CONCLR = STA;
				I2C2State = I2C_BUSY;
				break;

			case I2C_SLAW_ACK: // [SLA + W] transmitted and ACK received
				if (I2C2WriteLength == 1) { // There's no data to be read
					LPC_I2C2->I2CONSET = STO; // Set STOP flag
					I2C2State = I2C_IDLE;
				}
				else {
					LPC_I2C2->I2DAT = I2C2WriteBuffer[I2C2WriteIndex++];
				}
				break;

			case I2C_SLAW_NACK: // [SLA + W] transmitted and NACK received
				LPC_I2C2->I2CONSET = STO;
				I2C2State = I2C_IDLE;
				break;

			case I2C_DATAW_ACK: // Data byte transmitted, ACK received
				if (I2C2WriteIndex < I2C2WriteLength) LPC_I2C2->I2DAT = I2C2WriteBuffer[I2C2WriteIndex++]; // Keep transmitting
				else { // There's nothing more to transmit
					LPC_I2C2->I2CONSET = STO;
					I2C2State = I2C_IDLE;
				}
				break;

			case I2C_DATAW_NACK: // Data byte transmitted, NACK received
				LPC_I2C2->I2CONSET = STO;
				I2C2State = I2C_IDLE;
				break;

			case I2C_ARBITRATION_LOST: // Arbitration lost. This API doesn't handle with multiple Master operations.
				LPC_I2C2->I2CONSET = STO;
				I2C2State = I2C_IDLE;
				break;

			case I2C_SLAR_ACK: // [SLA + R] transmitted and ACK received
				if (I2C2ReadLength > 0) {
					LPC_I2C2->I2CONSET = AA; // Assert ACK
				}
				else { // Will jump to I2C_DATAR_NACK
					LPC_I2C2->I2CONCLR = AA; // Assert NACK
				}
				break;

			case I2C_SLAR_NACK: // [SLA + R] transmitted and NACK received.
				LPC_I2C2->I2CONSET = STO;
				I2C2State = I2C_IDLE;
				break;

			case I2C_DATAR_ACK: // Data byte received, ACK returned
				I2C2ReadBuffer[I2C2ReadIndex++] = LPC_I2C2->I2DAT;
				if (I2C2ReadIndex < I2C2ReadLength) {
					LPC_I2C2->I2CONSET = AA; // Assert ACK
				}
				else {
					LPC_I2C2->I2CONCLR = AA; // Assert NACK
				}
				break;

			case I2C_DATAR_NACK: // Data byte received, NACK returned
				I2C2ReadBuffer[I2C2ReadIndex++] = LPC_I2C2->I2DAT;
				LPC_I2C2->I2CONSET = STO;
				I2C2State = I2C_IDLE;
				break;

			default:
				LPC_I2C2->I2CONSET = STO;
				I2C2State = I2C_IDLE;
				break;
		}
		LPC_I2C2->I2CONCLR = SI;
}

void I2C1_Init() {
	WAIT_Init(SYS);

	LPC_SC->PCONP |= I2C1_PCONP_ENABLE;

	LPC_SC->PCLKSEL1 &= ~(0x03 << 6); // Set PCLK to CCLK/4

	LPC_PINCON->PINSEL1 &= ~(0x0F << 6); // Clear SDA1 and SCL1 PINSEL BITS
	LPC_PINCON->PINSEL1 |= ((I2C1_PINSEL & 0x03) << 6); // SDA1
	LPC_PINCON->PINSEL1 |= ((I2C1_PINSEL & 0x03) << 8); // SCL1

	LPC_PINCON->PINMODE1 &= ~((0x3<<6)|(0x3<<8));
	LPC_PINCON->PINMODE1 |= ((0x2<<6)|(0x2<<8));

	LPC_PINCON->PINMODE_OD0 |= ((0x1<<19)|(0x1<<20));

	if (!I2C_IsEnabled(I2C1_IRQn)) { // If it's not already enabled:
		NVIC_EnableIRQ(I2C1_IRQn); // Enable interrupts for I2C1 interface
	}

	LPC_I2C1->I2CONCLR = AA | SI | STA | I2EN; // Clear control flags (STOP flag never needs clearing)

	LPC_I2C1->I2CONSET = I2EN; // Enable interrupts
}

void I2C2_Init() {
	WAIT_Init(SYS);

	LPC_SC->PCONP |= I2C2_PCONP_ENABLE;

	LPC_SC->PCLKSEL1 &= ~(0x03 << 20); // Set PCLK to CCLK/4

	LPC_PINCON->PINSEL0 &= ~(0x0F << 20); // Clear SDA1 and SCL1 PINSEL BITS
	LPC_PINCON->PINSEL0 |= ((I2C2_PINSEL & 0x03) << 20); // SDA1
	LPC_PINCON->PINSEL0 |= ((I2C2_PINSEL & 0x03) << 22); // SCL1

	LPC_PINCON->PINMODE0 &= ~((0x03<<20)|(0x03<<22));
	LPC_PINCON->PINMODE0 |= ((0x02<<20)|(0x2<<22));

	LPC_PINCON->PINMODE_OD0 |= ((0x01<<10)|(0x1<<11));

	if (!I2C_IsEnabled(I2C2_IRQn)) { // If it's not already enabled:
		NVIC_EnableIRQ(I2C2_IRQn); // Enable interrupts for I2C2 interface
	}

	LPC_I2C2->I2CONCLR = AA | SI | STA | I2EN; // Clear control flags (STOP flag never needs clearing)

	LPC_I2C2->I2CONSET = I2EN; // Enable interrupts
}

int I2C1_Configure(int frequency, int rSize, char * wBuffer, int wSize) {
	if (rSize < 0 || wSize < 1) return -1;

	int freq_div = I2C_GetPCLK()/frequency;
	LPC_I2C1->I2SCLH = freq_div >> 1; // Set duty cycle (high)
	LPC_I2C1->I2SCLL = freq_div >> 1; // Set duty cycle (low)

	memcpy(I2C1WriteBuffer, wBuffer, wSize + (rSize != 0)); // Copy data. (rSize != 0) represents the SLA + R byte, if a read operation is to be performed
	I2C1WriteLength = wSize;

	I2C1ReadLength = rSize;

	return 0;
}

int I2C2_Configure(int frequency, int rSize, char * wBuffer, int wSize) {
	if (rSize < 0 || wSize < 1) return -1;

	int freq_div = I2C_GetPCLK()/frequency;
	LPC_I2C2->I2SCLH = freq_div >> 1; // Set duty cycle (high)
	LPC_I2C2->I2SCLL = freq_div >> 1; // Set duty cycle (low)

	memcpy(I2C2WriteBuffer, wBuffer, wSize + (rSize != 0)); // Copy data. (rSize != 0) represents the SLA + R byte, if a read operation is to be performed
	I2C2WriteLength = wSize;

	I2C2ReadLength = rSize;

	return 0;
}

int I2C1_Start() {
	LPC_I2C1->I2CONSET = STA;

	uint32_t start = WAIT_SYS_GetElapsedMs(0);
	while (1) {
		if (I2C1State == I2C_BUSY) return 0; // Started
		if (WAIT_SYS_GetElapsedMs(start) > I2C_TIMEOUT_MS) return -1; // Timed out
	}
}

int I2C2_Start() {
	LPC_I2C2->I2CONSET = STA;

	uint32_t start = WAIT_SYS_GetElapsedMs(0);
	while (1) {
		if (I2C2State == I2C_BUSY) return 0; // Started
		if (WAIT_SYS_GetElapsedMs(start) > I2C_TIMEOUT_MS) return -1; // Timed out
	}
}


void I2C1_Stop() {
	LPC_I2C1->I2CONSET = STO; // Set STOP flag
	LPC_I2C1->I2CONCLR = SI; // Clear SI flag

	while (LPC_I2C1->I2CONSET & STO); // Wait for STOP to be cleared

	I2C1State = I2C_IDLE;
}

void I2C2_Stop() {
	LPC_I2C2->I2CONSET = STO; // Set STOP flag
	LPC_I2C2->I2CONCLR = SI; // Clear SI flag

	while (LPC_I2C2->I2CONSET & STO); // Wait for STOP to be cleared

	I2C2State = I2C_IDLE;
}

int I2C1_Engine() {
	LPC_I2C1->I2CONSET = STA; // Set START flag

	I2C1State = I2C_BUSY;

	uint32_t start = WAIT_SYS_GetElapsedMs(0);
	while (I2C1State == I2C_BUSY) {
		if (WAIT_SYS_GetElapsedMs(start) > I2C_TIMEOUT_MS) { // Timed out
			I2C1State = I2C_IDLE;
			return -1;
		}
	}
	LPC_I2C1->I2CONCLR = STA; // Clear START;

	return 0;
}

int I2C2_Engine() {
	LPC_I2C2->I2CONSET = STA; // Set START flag

	I2C2State = I2C_BUSY;

	uint32_t start = WAIT_SYS_GetElapsedMs(0);
	while (I2C2State == I2C_BUSY) {
		if (WAIT_SYS_GetElapsedMs(start) > I2C_TIMEOUT_MS) { // Timed out
			I2C2State = I2C_IDLE;
			return -1;
		}
	}
	LPC_I2C2->I2CONCLR = STA; // Clear START;

	return 0;
}

void I2C1_GetBuffer(char * buffer) {
	memcpy(buffer, I2C1ReadBuffer, I2C1ReadLength);
}

void I2C2_GetBuffer(char * buffer) {
	memcpy(buffer, I2C2ReadBuffer, I2C2ReadLength);
}
