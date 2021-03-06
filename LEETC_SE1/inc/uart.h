/*
 * uart.h
 *
 *  Created on: 28 Apr 2021
 *      Author: pedro
 */

#ifndef UART_H_
#define UART_H_

/** @defgroup UART UART
 * This package provides the core capabilities for uart functions.
 * @{
 */

/** @defgroup UART_Public_Functions UART Public Functions
 * @{
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include "wait.h"


/*
 *
 *
 * Macros:
 *
 *
 */


// Head -> Write ; Tail -> Read

/**
 * @brief	Ring Buffer size.
 */
#define UART_RBUFSIZE 				(1 << 11)

/**
 * @brief	Ring Buffer mask.
 */
#define RBUF_MASK 					(UART_RBUFSIZE-1)

/**
 * @brief	Check if Ring Buffer is full.
 * @param	head: -> Which head.
 * @param	tail: -> Which tail.
 */
#define RBUF_IS_FULL(head, tail) 	((tail & RBUF_MASK) == ((head+1) & RBUF_MASK))

/**
 * @brief	Check if Ring Buffer will be full.
 * @param	head: -> Which head.
 * @param	tail: -> Which tail.
 */
#define RBUF_WILL_FULL(head, tail) 	((tail & RBUF_MASK) == ((head + 2) & RBUF_MASK))

/**
 * @brief	Check if Ring Buffer is empty.
 * @param	head: -> Which head.
 * @param	tail: -> Which tail.
 */
#define RBUF_IS_EMPTY(head, tail) 	((head & RBUF_MASK) == (tail & RBUF_MASK))

/**
 * @brief	Reset Ring Buffer pointer.
 * @param	bufidx: -> pointer.
 */
#define RBUF_RESET(bufidx) 			(bufidx = 0)

/**
 * @brief	Increment Ring Buffer pointer.
 * @param	bufidx: -> pointer.
 */
#define RBUF_INCR(bufidx) 			(bufidx = (bufidx+1) & RBUF_MASK)


/*
 *
 *
 * Constants:
 *
 *
 */



/**
 * @brief	Max length of UART String.
 */
#define UART_MAX_SRING_LENGTH 1024

/**
 * @brief	UART default timeout in ms.
 */
#define UART_TIMEOUT_MS 1000

/**
 * @brief	UART PCLK.
 */
#define UART_PCLK_VAL 0x00

/**
 * @brief	UART DLL Register mask bit.
 */
#define UART_DLL_MASKBIT		((uint8_t)0xFF)

/**
 * @brief	Load UART DLL with value.
 * @param	div: -> Value.
 */
#define UART_LOAD_DLL(div)		((div) & UART_DLL_MASKBIT)

/**
 * @brief	UART DLM Register mask bit.
 */
#define UART_DLM_MASKBIT		((uint8_t)0xFF)

/**
 * @brief	Load UART DLM with value.
 * @param	div: -> Value.
 */
#define UART_LOAD_DLM(div)  	(((div) >> 8) & UART_DLM_MASKBIT)

/**
 * @brief	UART FDR Register mask bit.
 */
#define UART_FDR_MASKBIT		((uint8_t)0xFF)

/**
 * @brief	Load UART DLM with value.
 * @param	add: -> Add value.
 * @param	mul: -> Mul value.
 */
#define UART_LOAD_FDR(add, mul) ((uint32_t) ((add & 0x0F) | ((mul << 4) & 0xF0)) & UART_FDR_MASKBIT)

/**
 * @brief	UART Ring Buffer structure.
 */
typedef struct {
	uint32_t txWrite; /*!< Write to TX. */
	uint32_t txRead; /*!< Read from TX. */
	uint32_t rxWrite; /*!< Write to RX. */
	uint32_t rxRead; /*!< Read from RX. */
	uint8_t tx[UART_RBUFSIZE]; /*!< TX buffer. */
	uint8_t rx[UART_RBUFSIZE]; /*!< RX buffer. */
} UART_RBUF_Type;

/**
 * @brief	UART Peripheral PCONP enable.
 */
typedef enum {
	UART0_PCONP_ENABLE = 	(1 << 3), /*!< Power/clock control bit for UART0. */
	UART2_PCONP_ENABLE = 	(1 << 24), /*!< Power/clock control bit for UART2. */
	UART3_PCONP_ENABLE = 	(1 << 25) /*!< Power/clock control bit for UART3. */
} UART_PCONP_ENABLE;

/**
 * @brief	UART FCR bits.
 */
typedef enum {
	UART_FCR_FIFO_EN = 		(1 << 0), /*!< Enable FIFO. */
	UART_FCR_RX_RS = 		(1 << 1), /*!< Clear all bytes in RX FIFO. */
	UART_FCR_TX_RS = 		(1 << 2), /*!< Clear all bytes in TX FIFO. */
	UART_FCR_DMA_SEL = 		(1 << 3), /*!< Select DMA Mode (if UART_FCR_FIFO_EN is set). */
	UART_FCR_TRG_LEV0 = 	(0 << 6) /*!< Trigger RX level 0. */
} UART_FCR_BITS;

/**
 * @brief	UART LCR bits.
 */
typedef enum {
	UART_LCR_DLAB_EN = 		(1 << 7), /*!< Enable Divisor Latch Access Bit. */
	UART_LCR_BREAK_EN = 	(1 << 6), /*!< Break Control. */
	UART_LCR_BITMASK = 		(0xFF)
} UART_LCR_BITS;

/**
 * @brief	UART TER bits.
 */
typedef enum {
	UART_TER_TXEN = 		(1 << 7) /*!< Output THR to TXD. */
} UART_TER_BITS;

/**
 * @brief	UART IER bits.
 */
typedef enum {
	UART_IER_RBRINT_EN = 	(1 << 0), /*!< RBR Interrupt Enable. */
	UART_IER_THREINT_EN = 	(1 << 1), /*!< THRE Interrupt Enable. */
	UART_IER_RLSINT_EN = 	(1 << 2), /*!< RX Line Status Interrupt Enable. */
	UART_IER_BITMASK = 		(775)
} UART_IER_BITS;

/**
 * @brief	UART IIR bits.
 */
typedef enum {
	UART_IIR_INTID_THRE = 	(1 << 1), /*!< THRE Interrupt. */
	UART_IIR_INTID_RDA = 	(2 << 1), /*!< Receive Data Available. */
	UART_IIR_INTID_RLS = 	(3 << 1), /*!< Receive Line Status */
	UART_IIR_INTID_CTI = 	(6 << 1), /*!< Character Time-out Indicator. */
	UART_IIR_INTID_MASK = 	(7 << 1), /*!< Mask for Interrupt Identification. */
	UART_IIR_INTSTAT_PEND = (1 << 0) /*< Interrupt Status. */
} UART_IIR_BITS;

/**
 * @brief	UART LSR bits.
 */
typedef enum {
	UART_LSR_RDR = 			(1 << 0), /*!< Receiver Data Ready. */
	UART_LSR_OE = 			(1 << 1), /*!< Overrun Error. */
	UART_LSR_PE = 			(1 << 2), /*!< Parity Error. */
	UART_LSR_FE = 			(1 << 3), /*!< Framing Error. */
	UART_LSR_BI = 			(1 << 4), /*!< Break Interrupt. */
	UART_LSR_THRE = 		(1 << 5), /*!< Transmitter Holding Register Empty. */
	UART_LSR_TEMT =			(1 << 6), /*!< Transmitter Empty. */
	UART_LSR_RXFE = 		(1 << 7), /*!< Error in RX FIFO. */
} UART_LSR_BITS;

/**
 * @brief	UART Peripheral pin function.
 */
typedef enum {
	UART0_PINFUNCTION = 	(0x01), /*!< Pin function for UART0. */
	UART2_PINFUNCTION = 	(0x01), /*!< Pin function for UART2. */
	UART3_PINFUNCTION = 	(0x03) /*!< Pin function for UART3. */
} UART_PINFUNCTION;

/**
 * @brief	UART Peripheral pin mode.
 */
typedef enum {
	UART0_PINMODE = 		(0x01), /*!< Pin mode for UART0. */
	UART2_PINMODE = 		(0x01), /*!< Pin mode for UART2. */
	UART3_PINMODE = 		(0x03) /*!< Pin mode for UART3. */
} UART_PINMODE;


/*
 *
 *
 * Functions:
 *
 *
 */



/**
 * @brief	Initialise UART.
 * @note	This function must be called prior to other UART functions.
 * @param	baud: -> Baud rate (max: 115200)
 * @note	If baud is passed as 0, default 9600 will be set.
 * @return	True if successful, false otherwise.
 */
bool UART_Init(uint32_t baud);

/**
 * @brief	Check if there is an unread character in RX FIFO.
 * @return	True if there's an unread character in RX, false otherwise.
 */
bool UART_IsChar(void);

/**
 * @brief	Get character from RX FIFO.
 * @param	ch: -> Where character shall be written to.
 * @return	True if there was an unread character in RX, false otherwise.
 * @note	This function is non-blocking.
 */
bool UART_GetChar(unsigned char *ch);

/**
 * @brief	Read character from RX FIFO.
 * @param	ch: -> Where character shall be written to.
 * @param	timeout: -> Maximum time in ms the function should block, waiting for data to arrive.
 * @return	True if a character was read, false if timeout was met.
 * @note	This function is blocking (until timeout is met).
 */
bool UART_ReadChar(unsigned char *ch, uint32_t timeout);

/**
 * @brief	Write character to TX FIFO.
 * @param	ch: -> The character to write.
 */
void UART_WriteChar(unsigned char ch);

/**
 * @brief	Read array of characters from RX FIFO.
 * @param	buffer: -> Where buffer shall be written to.
 * @param	len: -> Buffer length expected.
 * @param	timeout: -> Maximum time in ms the function should block, waiting for data to arrive.
 * @return	Number of characters read.
 * @note	This function is blocking (until timeout is met).
 */
uint32_t UART_ReadBuffer(unsigned char *buffer, uint32_t len, uint32_t timeout);

/**
 * @brief	Write array of characters to TX FIFO.
 * @param	buffer: -> Array to write.
 * @param	len: -> Buffer length.
 * @return	Number of characters written.
 */
uint32_t UART_WriteBuffer(unsigned char * buffer, uint32_t len);

/**
 * @brief	Read String from RX FIFO.
 * @param	str: -> Where String shall be written to.
 * @param	timeout: -> Maximum time in ms the function should block, waiting for data to arrive.
 * @return	Number of characters read.
 * @note	This function is blocking (until timeout is met).
 */
uint32_t UART_ReadString(unsigned char *str, uint32_t timeout);

/**
 * @brief	Write String to TX FIFO.
 * @param	str: -> String to write.
 * @return	Number of characters written.
 */
uint32_t UART_WriteString(unsigned char *str);

/**
 * @brief	Printf to TX FIFO.
 * @param	format: -> Format String.
 * @param	...: -> Printf paramters.
 */
void UART_Printf(char *format, ...);


/**
 * @}
 */


/**
 * @}
 */

#endif /* UART_H_ */
