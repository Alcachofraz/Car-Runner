/*
 * spi.c
 *
 *  Created on: Dec 2020
 *      Author: PedroG
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif


#include "spi.h"


static uint32_t SPI_GetPCLK(void);


static uint32_t SPI_GetPCLK(void) {
	switch ((LPC_SC->PCLKSEL0 >> 16) & 0x03) {
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

void SPI_Init(void) {
	LPC_PINCON->PINSEL0 |= ((SPI_PIN_FUNTION & 0x03) << 30); // SCK
	LPC_PINCON->PINSEL1 |= ((SPI_PIN_FUNTION & 0x03) << 2); // MISO
	LPC_PINCON->PINSEL1 |= ((SPI_PIN_FUNTION & 0x03) << 4); // MOSI

	LPC_SC->PCLKSEL0 &= ~(0x03 << 16); // Set PCLK to CCLK/4

	LPC_SC->PCONP |= SPI_PCONP_ENABLE;

	//SPI_ClearFlags();
}

int32_t SPI_ConfigTransfer(int frequency, int bitData, int mode) {
	// Validate bitData:
	if ((bitData < 8) || (bitData > 16)) return -1;

	// PCLK cycles that make up an SPI clock:
	LPC_SPI->SPCCR = ((SPI_GetPCLK()/frequency) & SPI_CCR_MASK);

	// SPI Control register:
	LPC_SPI->SPCR = 0; // Clear everything
	LPC_SPI->SPCR |= (((bitData << 8) & SPI_CR_BITS_MASK) | MSTR | ((mode & 0x03) << 3) | BIT_ENABLE);

	return 0;
}

int32_t SPI_Transfer(unsigned short *txBuffer, unsigned short *rxBuffer, int length) {
	for (int i=0; i<length; i++) {
		// Write:
		LPC_SPI->SPDR = txBuffer[i];
		while ((LPC_SPI->SPSR & SPIF_MASK) == 0) { // Wait for transfer completed flag
			if ((LPC_SPI->SPSR & SPI_STATUS_ERROR_MASK) != 0) return -1;
		}
		// Read:
		rxBuffer[i] = LPC_SPI->SPDR;
	}
	return 0;
}

int SPI_VerifyFrequency(int frequency) {
	int pclk = SPI_GetPCLK();
	if (pclk/frequency % 2) return -1;
	if (pclk/frequency < 8) return -1;
	return 0;
}

