/*
 * uart.c
 *
 *  Created on: 28 Apr 2021
 *      Author: pedro
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif


#include "uart.h"

#define MARGIN(p1, p2) (p1 > p2 ? p1 - p2 : p2 - p1)


static LPC_UART_TypeDef* UARTx;

static bool intrTxStatus;

static UART_RBUF_Type rbuffer;


/********************************************************************************
 *
 * STATIC FUNCTIONS:
 *
 ********************************************************************************/

/**
 *
 *
 * Static Declarations:
 *
 *
 */

/*
 * Get UART PCLK:
 */
static uint32_t UART_GetPCLK(void);

/*
 * Check if UART interrupt interface is enabled:
 */
static int UART_IsEnabled(IRQn_Type IRQ);

/*
 * Set divisors for baud rate:
 */
static uint8_t UART_SetDivisors(uint32_t baud);

/*
 * Error handler (simple infinite loop):
 */
static void UART_ErrorHandler(uint8_t error_type);

/*
 * Check if UART interface is busy:
 */
static bool UART_CheckBusy();

/*
 * Function to receive content to ring buffer:
 */
static void UART_IntReceive();

/*
 * Function to transmit ring buffer content:
 */
static void UART_IntTransmit();

/**
 *
 *
 * Static Functions:
 *
 *
 */

static uint32_t UART_GetPCLK(void) {
	switch ((LPC_SC->PCLKSEL1 >> 16) & 0x03) {
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

static int UART_IsEnabled(IRQn_Type IRQ) { // See if i2c interrupts are enabled
	return NVIC->ISER[((uint32_t)(IRQ) >> 5)] & (1 << ((uint32_t)(IRQ) & 0x1F));
}

static uint8_t UART_SetDivisors(uint32_t baud) {
	if (baud > 115200) return -1;
	if (baud == 0) baud = 9600;

	int pclk = UART_GetPCLK();

	double d = (double) pclk / (16 * baud); // 27,1267 with PCLK = 50 MHz and baud = 115200
	int i = (int) d;


	// Best results:
	int best[2] = {0, 1}; // Pre-scaler values, divisor and multiplier (default is 0, 1)
	double bestMargin = 0xFFFFFFFF; // Best margin between real fraction and pre-scaler generated margin
	int bestIntVal = 0; // Best integer value

	double currentFraction;
	double currentMargin;

	// i * (1 + fraction) = d <=> fraction = (d - i) / i
	for (int intVal = i/2; intVal <= i; intVal++) {
		currentFraction = (double) (d - intVal) / intVal;
		for (int divAddVal = 0; divAddVal <= 14; divAddVal++) {
			for (int mulVal = divAddVal + 1; mulVal <= 15; mulVal++) {
				if ((currentMargin = (double) MARGIN(currentFraction, (double) divAddVal/mulVal)) < bestMargin) {
					best[0] = divAddVal;
					best[1] = mulVal;
					bestMargin = currentMargin;
					bestIntVal = intVal;
				}
			}
		}
	}

	//printf("Asked Baud Rate: %f\nReal Baud Rate:  %f\n", (double) baud, (double) pclk / (16 * bestIntVal * (1 + ((double) best[0] / best[1]))));

	UARTx->LCR |= UART_LCR_DLAB_EN; // Set DLAB

	// Clean registers:
	UARTx->DLL &= ~(0xFF);
	UARTx->DLM &= ~(0xFF);
	UARTx->FDR &= ~(0xFF);

	// Write values:
	UARTx->DLL = UART_LOAD_DLL(bestIntVal);
	UARTx->DLM = UART_LOAD_DLM(bestIntVal);
	UARTx->FDR = UART_LOAD_FDR(best[0], best[1]);

	UARTx->LCR &= ~UART_LCR_DLAB_EN; // Clear DLAB
	return 0;
}

static void UART_ErrorHandler(uint8_t error_type) {
	printf("Error: %x\n", error_type);
	/*while (1) {
		error_type = error_type;
	}*/
}

static bool UART_CheckBusy() {
	if (UARTx->LSR & UART_LSR_TEMT) { // THR and TSR are empty
		return false;
	}
	return true;
}

/*static bool UART_RBR_IsChar(void) {
	return (UARTx->LSR & UART_LSR_RDR) != 0;
}*/

static bool UART_RB_IsChar(void) {
	return !RBUF_IS_EMPTY(rbuffer.rxWrite, rbuffer.rxRead);
}

static unsigned char UART_RBR_ReadChar(void) {
	while ((UARTx->LSR & UART_LSR_RDR) == 0); // Wait until data is received
	char ch = UARTx->RBR;
	return ch;
}

static bool UART_RB_ReadChar(unsigned char *ch, uint32_t timeout) {
	uint32_t start = WAIT_SYS_GetElapsedMs(0);
	while (RBUF_IS_EMPTY(rbuffer.rxWrite, rbuffer.rxRead)) {
		if (WAIT_SYS_GetElapsedMs(start) > timeout) {
			return false;
		}
	}
	*ch = rbuffer.rx[rbuffer.rxRead];
	RBUF_INCR(rbuffer.rxRead);
	return true;
}

/*static bool UART_RBR_GetChar(unsigned char *ch) {
	if (!UART_RBR_IsChar())
		return false;
	*ch = UART_RBR_ReadChar();
	return true;
}*/

static bool UART_RB_GetChar(unsigned char *ch) {
	if (!UART_RB_IsChar())
		return false;
	*ch = rbuffer.rx[rbuffer.rxRead];
	RBUF_INCR(rbuffer.rxRead);
	return true;
}

static void UART_THR_WriteChar(unsigned char ch) {
	while ((UARTx->LSR & UART_LSR_THRE) == 0); // Wait for previous transmission
	UARTx->THR = ch;
}

static void UART_RB_WriteChar(unsigned char ch) {
	while (RBUF_IS_FULL(rbuffer.txWrite, rbuffer.txRead));
	rbuffer.tx[rbuffer.txWrite] = ch;
	RBUF_INCR(rbuffer.txWrite);
}

static void UART_IntReceive() {
	UARTx->IER &= (~UART_IER_RBRINT_EN) & UART_IER_BITMASK; // Disable RBRINT Interrupts

	// Read RBR char to Ring Buffer:
	rbuffer.rx[rbuffer.rxWrite] = UART_RBR_ReadChar();
	RBUF_INCR(rbuffer.rxWrite);

	UARTx->IER |= UART_IER_RBRINT_EN; // Re-enable RBRINT Interrupts
}

static void UART_IntTransmit() {
	while (UART_CheckBusy()); // Wait until UART isn't busy

	UARTx->IER &= (~UART_IER_THREINT_EN) & UART_IER_BITMASK;

	while (!RBUF_IS_EMPTY(rbuffer.txWrite, rbuffer.txRead)
			&& ((UARTx->LSR & UART_LSR_THRE) != 0)) {
		// Write next Ring Buffer char to THR:
		UART_THR_WriteChar((unsigned char) rbuffer.tx[rbuffer.txRead]);
		RBUF_INCR(rbuffer.txRead);
	}

	if (RBUF_IS_EMPTY(rbuffer.txWrite, rbuffer.txRead)) { // Ring Buffer is empty:
		intrTxStatus = false;
		UARTx->IER &= (~UART_IER_THREINT_EN) & UART_IER_BITMASK; // Disable THRE Interrupts
	}
	else { // Ring Buffer still has data:
		intrTxStatus = true;
		UARTx->IER |= UART_IER_THREINT_EN & UART_IER_BITMASK; // Re-enable THRE Interrupts
	}
}


/********************************************************************************
 *
 * PUBLIC FUNCTIONS:
 *
 ********************************************************************************/

void UART2_IRQHandler(void) {
	uint32_t iir, lsr;
	while (!((iir = UARTx->IIR) & UART_IIR_INTSTAT_PEND)) {
		switch (iir & UART_IIR_INTID_MASK) {
			case UART_IIR_INTID_RLS: // Error of some kind:
				// Check line status
				lsr = UARTx->LSR;
				// Mask out the Receive Ready and Transmit Holding empty status
				lsr &= (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE | UART_LSR_BI | UART_LSR_RXFE);
				// If any error exist
				if (lsr) UART_ErrorHandler(lsr);
				break;
			case UART_IIR_INTID_RDA:
			case UART_IIR_INTID_CTI: // Can Read:
				UART_IntReceive();
				break;
			case UART_IIR_INTID_THRE: // Can Write:
				UART_IntTransmit();
				break;
		}
	}
}

bool UART_Init(uint32_t baud) {
	WAIT_Init(SYS);

	uint8_t tmp;

	UARTx = LPC_UART2; // We will use UART2

	LPC_PINCON->PINSEL0 &= ~(0x0F << 20); // Clear PINSEL BITS 20-23 (TX2 and RX2)
	LPC_PINCON->PINSEL0 |= ((UART2_PINFUNCTION & 0x03) << 20); // Set TXD2 pin function
	LPC_PINCON->PINSEL0 |= ((UART2_PINFUNCTION & 0x03) << 22); // Set RXD2 pin function

	LPC_PINCON->PINMODE0 &= ~(0x0F << 20); // Pin mode 00 (pull-up resistor) (for TX2 and RX2)

	LPC_PINCON->PINMODE_OD0 &= ~((1 << 10) | (1 << 11)); // Select normal mode (not open drain) for P0[10] and P0[11] (TX2 and RX2)

	LPC_SC->PCONP |= UART2_PCONP_ENABLE; // Enable UART2
	LPC_SC->PCLKSEL1 &= ~(0x03 << 16); // Clean PCLK of UART2
	LPC_SC->PCLKSEL1 |= (UART_PCLK_VAL << 16); // Set PCLK of UART2 to CCLK/2

	// FIFOs are empty
	UARTx->FCR = (UART_FCR_FIFO_EN | UART_FCR_RX_RS | UART_FCR_TX_RS);
	//UARTx->FCR = 0; // Disable FIFO

	while (UARTx->LSR & UART_LSR_RDR) { // Dummy reading - empty rx!
		tmp = UARTx->RBR;
	}
	UARTx->TER = UART_TER_TXEN;
	while (!(UARTx->LSR & UART_LSR_THRE)); // Wait for Tx complete
	UARTx->TER = 0; // Disable Tx
	UARTx->IER = 0; // Disable interrupt
	UARTx->LCR = 0; // Set LCR to default state
	UARTx->ACR = 0; // Set ACR to default state

	tmp = UARTx->LSR; // Clean status

	if (UART_SetDivisors(baud) < 0) return false;

	tmp = (UARTx->LCR & (UART_LCR_DLAB_EN | UART_LCR_BREAK_EN)) & UART_LCR_BITMASK;
	tmp |= 0x03;

	UARTx->LCR = (uint8_t) (tmp & UART_LCR_BITMASK);
	UARTx->TER |= UART_TER_TXEN;

	UARTx->FCR = UART_FCR_FIFO_EN | UART_FCR_TRG_LEV0;
	UARTx->IER = UART_IER_RBRINT_EN | UART_IER_RLSINT_EN;
	intrTxStatus = false;
	RBUF_RESET(rbuffer.rxWrite);
	RBUF_RESET(rbuffer.rxRead);
	RBUF_RESET(rbuffer.txWrite);
	RBUF_RESET(rbuffer.txRead);
	while (1) {
		uint32_t iir;
		iir = (UARTx->IIR & 0x03CF) & UART_IIR_INTID_MASK;
		if ((iir == UART_IIR_INTID_RDA) || (iir == UART_IIR_INTID_CTI)) {
			tmp = UARTx->RBR;
		}
		else break;
	}
	if (!UART_IsEnabled(UART2_IRQn)) {
		NVIC_EnableIRQ(UART2_IRQn);
	}

	return true;
}

bool UART_IsChar(void) {
	return UART_RB_IsChar();
}

bool UART_ReadChar(unsigned char *ch, uint32_t timeout) {
	return UART_RB_ReadChar(ch, timeout);
}

bool UART_GetChar(unsigned char *ch) {
	return UART_RB_GetChar(ch);
}

void UART_WriteChar(unsigned char ch) {
	UART_RB_WriteChar(ch);
	// If there's still data in Ring Buffer:
	if (!RBUF_IS_EMPTY(rbuffer.txWrite, rbuffer.txRead)) {
		// If THRE Interrupts were disabled:
		if (!intrTxStatus) {
			UART_IntTransmit();
		}
	}
}

uint32_t UART_WriteBuffer(unsigned char *buffer, uint32_t len) {
	unsigned char *data = (unsigned char *) buffer;
	uint32_t bytes = 0;

	// Write buffer to Ring Buffer:
	while ((len > 0) && (!RBUF_IS_FULL(rbuffer.txWrite, rbuffer.txRead))) { // While there's data, and RBUF is not full:
		UART_RB_WriteChar(*data);
		data++;
		bytes++;
		len--;
	}

	// If there's still data in Ring Buffer:
	if (!RBUF_IS_EMPTY(rbuffer.txWrite, rbuffer.txRead)) {
		// If THRE Interrupts were disabled:
		if (!intrTxStatus) {
			UART_IntTransmit();
		}
	}

	return bytes;
}

uint32_t UART_ReadBuffer(unsigned char *buffer, uint32_t len, uint32_t timeout) {
	unsigned char *data = (unsigned char *) buffer;
	uint32_t bytes = 0;

	//UARTx->IER &= (~UART_IER_RBRINT_EN) & UART_IER_BITMASK; // Disable RBRINT Interrupts

	while ((len > 0)) { // && (!(RBUF_IS_EMPTY(rbuffer.rxWrite, rbuffer.rxRead)))) { // While there's data, and RBUF is not empty:
		// Read data from RBUF:
		if (!UART_RB_ReadChar(data, timeout)) return -1;
		data++;
		bytes++;
		len--;
	}

	//UARTx->IER |= UART_IER_RBRINT_EN; // Re-enable RBRINT Interrupts

	return bytes;
}

uint32_t UART_WriteString(unsigned char *str) {
	return UART_WriteBuffer(str, strlen((char *)str));
}

uint32_t UART_ReadString(unsigned char *str, uint32_t timeout) {
	unsigned char ch;
	uint32_t len = 0;

	//UARTx->IER &= (~UART_IER_RBRINT_EN) & UART_IER_BITMASK; // Disable RBRINT Interrupts

	while(1) {
		if (!UART_RB_ReadChar(&ch, timeout)) return -1; // Read character from Ring Buffer
		if ((ch == '\r') || (ch == '\n')) { // Read until \r or \n
			if (len != 0) { // Wait until at least 1 char is received
			   str[len] = 0; // Once enter key is pressed, null terminate the string and break the loop
			   break;
			}
		}
		else if ((ch == '\b') && (len != 0)) {
			len--; // If backspace is pressed then decrement the index to remove the old char
		}
		else {
			str[len] = ch; // Copy the char into string and increment the index
			len++;
		}
	}

	//UARTx->IER |= UART_IER_RBRINT_EN; // Re-enable RBRINT Interrupts

	return len;
}


void UART_Printf(char *format, ...) {
	va_list arg;
	char str[UART_MAX_SRING_LENGTH];

	va_start(arg, format);
	vsprintf(str, format, arg);
	va_end(arg);

	UART_WriteString((unsigned char *) str);
}

