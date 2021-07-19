/*
 * wait.c
 *
 *  Created on: Nov 2020
 *      Author: PedroG
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif


#include "wait.h"




static volatile uint32_t __ms;
static volatile int32_t busy[5];

static int32_t setBusy(WAIT mode);

static bool busyTim0 = false;
static bool busyTim1 = false;
static bool busyTim2 = false;
static bool busyTim3 = false;

static void (*h0)(void);
static void (*h1)(void);
static void (*h2)(void);
static void (*h3)(void);

static int32_t WAIT_SYS_Init(void);
static int32_t WAIT_TIM0_Init(void);
static int32_t WAIT_IRQ0_Init(void);
static int32_t WAIT_TIM1_Init(void);
static int32_t WAIT_IRQ1_Init(void);
static int32_t WAIT_TIM2_Init(void);
static int32_t WAIT_IRQ2_Init(void);
static int32_t WAIT_TIM3_Init(void);
static int32_t WAIT_IRQ3_Init(void);

#ifndef FREERTOS // If FREERTOS is not in properties pre-processor...
	void SysTick_Handler(void)
	{
		__ms++;
	}
#endif

void TIMER0_IRQHandler(void) // Original interrupt handler
{
	LPC_TIM0->IR = TIMER_IR_MR0; // Reset interrupt bit
	LPC_TIM0->TCR = DISABLE_RESET;
	busyTim0 = false;
	if (h0 != 0) (*h0)(); // Call custom interrupt handler
}

void TIMER1_IRQHandler(void) // Original interrupt handler
{
	LPC_TIM1->IR = TIMER_IR_MR0; // Reset interrupt bit
	LPC_TIM1->TCR = DISABLE_RESET;
	busyTim1 = false;
	if (h1 != 0) (*h1)(); // Call custom interrupt handler
}

void TIMER2_IRQHandler(void) // Original interrupt handler
{
	LPC_TIM2->IR = TIMER_IR_MR0; // Reset interrupt bit
	LPC_TIM2->TCR = DISABLE_RESET;
	busyTim2 = false;
	if (h2 != 0) (*h2)(); // Call custom interrupt handler
}

void TIMER3_IRQHandler(void) // Original interrupt handler
{
	LPC_TIM3->IR = TIMER_IR_MR0; // Reset interrupt bit
	LPC_TIM3->TCR = DISABLE_RESET;
	busyTim3 = false;
	if (h3 != 0) (*h3)(); // Call custom interrupt handler
}

int32_t WAIT_Init(WAIT mode)
{
	SystemCoreClockUpdate();
	if (setBusy(mode) < 0) return -1;
	switch (mode) {
		case SYS:
			return WAIT_SYS_Init();
		case TIM0:
			return WAIT_TIM0_Init();
		case IRQ0:
			return WAIT_IRQ0_Init();
		case TIM1:
			return WAIT_TIM1_Init();
		case IRQ1:
			return WAIT_IRQ1_Init();
		case TIM2:
			return WAIT_TIM2_Init();
		case IRQ2:
			return WAIT_IRQ2_Init();
		case TIM3:
			return WAIT_TIM3_Init();
		case IRQ3:
			return WAIT_IRQ3_Init();
		default:
			return -1;
	}
}

static int32_t setBusy(WAIT mode)
{
	switch (mode) {
		case SYS:
			return (busy[0] ? -1 : (busy[0] = 1));
		case TIM0:
			return (busy[1] ? -1 : (busy[1] = 1));
		case IRQ0:
			return (busy[1] ? -1 : (busy[1] = 1));
		case TIM1:
			return (busy[2] ? -1 : (busy[2] = 1));
		case IRQ1:
			return (busy[2] ? -1 : (busy[2] = 1));
		case TIM2:
			return (busy[3] ? -1 : (busy[3] = 1));
		case IRQ2:
			return (busy[3] ? -1 : (busy[3] = 1));
		case TIM3:
			return (busy[4] ? -1 : (busy[4] = 1));
		case IRQ3:
			return (busy[4] ? -1 : (busy[4] = 1));
		default:
			return -1;
	}
}

static int WAIT_IsEnabled(IRQn_Type IRQ) { // See if timer interrupts are enabled
	return NVIC->ISER[((uint32_t)(IRQ) >> 5)] & (1 << ((uint32_t)(IRQ) & 0x1F));
}

static int32_t WAIT_SYS_Init(void) // Initialise Systick:
{
	#ifndef FREERTOS // Not FREERTOS
		SystemCoreClockUpdate();
		if (SysTick_Config(SYSTICK_FREQ) == 1) return -1;
	#endif
	return 0;
}

static int32_t WAIT_TIM0_Init(void) // Initialise Timer0
{
	LPC_SC->PCONP |= PCONP_ENABLE_TIM0;
    LPC_TIM0->CTCR = 0; // Set to timer mode

    LPC_SC->PCLKSEL0 &= ~(BITS_PCLK_TIM0); // Set PCLK to 1/4, "00" (bits[3:2] correspond to timer0)
    LPC_TIM0->PR = TIMER_PCLK*TIMER_RES - 1; // Prescale for time resolution of 1 microsecond and pclk=cclk/4

    LPC_TIM0->MCR = 0x0; // Disable all match registers
    LPC_TIM0->MCR |= 0x3; // Set MR0 to reset and interrupt

    LPC_TIM0->TCR = DISABLE_RESET;

    LPC_TIM0->IR = TIMER_IR_MR0; // Reset interrupt bit

    return 0;
}

static int32_t WAIT_IRQ0_Init(void) // Initialise Timer0 with Interrupt Request
{
	int ret = WAIT_TIM0_Init();
	if (!WAIT_IsEnabled(TIMER0_IRQn)) { // If it's not already enabled:
		NVIC_EnableIRQ(TIMER0_IRQn);
	}
	return ret;
}

static int32_t WAIT_TIM1_Init(void) // Initialise Timer0
{
	LPC_SC->PCONP |= PCONP_ENABLE_TIM1;
    LPC_TIM1->CTCR = 0; // Set to timer mode

    LPC_SC->PCLKSEL0 &= ~(BITS_PCLK_TIM1); // Set PCLK to 1/4, "00" (bits[5:4] correspond to timer1)
    LPC_TIM1->PR = TIMER_PCLK*TIMER_RES - 1; // Prescale for time resolution of 1 microsecond and pclk=cclk/4

    LPC_TIM1->MCR = 0x0; // Disable all match registers
    LPC_TIM1->MCR |= 0x3; // Set MR0 to reset and interrupt

    LPC_TIM1->TCR = DISABLE_RESET;

    LPC_TIM1->IR = TIMER_IR_MR0; // Reset interrupt bit

    return 0;
}

static int32_t WAIT_IRQ1_Init(void) // Initialise Timer0 with Interrupt Request
{
	int ret = WAIT_TIM1_Init();
	if (!WAIT_IsEnabled(TIMER1_IRQn)) { // If it's not already enabled:
		NVIC_EnableIRQ(TIMER1_IRQn);
	}
	return ret;
}

static int32_t WAIT_TIM2_Init(void) // Initialise Timer0
{
	LPC_SC->PCONP |= PCONP_ENABLE_TIM2;
    LPC_TIM2->CTCR = 0; // Set to timer mode

    LPC_SC->PCLKSEL1 &= ~(BITS_PCLK_TIM2); // Set PCLK to 1/4, "00" (bits[13:12] correspond to timer2)
    LPC_TIM2->PR = TIMER_PCLK*TIMER_RES - 1; // Prescale for time resolution of 1 microsecond and pclk=cclk/4

    LPC_TIM2->MCR = 0x0; // Disable all match registers
    LPC_TIM2->MCR |= 0x3; // Set MR0 to reset and interrupt

    LPC_TIM2->TCR = DISABLE_RESET;

    LPC_TIM2->IR = TIMER_IR_MR0; // Reset interrupt bit

    return 0;
}

static int32_t WAIT_IRQ2_Init(void) // Initialise Timer0 with Interrupt Request
{
	int ret = WAIT_TIM2_Init();
	if (!WAIT_IsEnabled(TIMER2_IRQn)) { // If it's not already enabled:
		NVIC_EnableIRQ(TIMER2_IRQn);
	}
	return ret;
}

static int32_t WAIT_TIM3_Init(void) // Initialise Timer0
{
	LPC_SC->PCONP |= PCONP_ENABLE_TIM3;
    LPC_TIM3->CTCR = 0; // Set to timer mode

    LPC_SC->PCLKSEL1 &= ~(BITS_PCLK_TIM3); // Set PCLK to 1/4, "00" (bits[15:14] correspond to timer3)
    LPC_TIM3->PR = TIMER_PCLK*TIMER_RES - 1; // Prescale for time resolution of 1 microsecond and pclk=cclk/4

    LPC_TIM3->MCR = 0x0; // Disable all match registers
    LPC_TIM3->MCR |= 0x3; // Set MR0 to reset and interrupt

    LPC_TIM3->TCR = DISABLE_RESET;

    LPC_TIM3->IR = TIMER_IR_MR0; // Reset interrupt bit

    return 0;
}

static int32_t WAIT_IRQ3_Init(void) // Initialise Timer0 with Interrupt Request
{
	int ret = WAIT_TIM3_Init();
	if (!WAIT_IsEnabled(TIMER3_IRQn)) { // If it's not already enabled:
		NVIC_EnableIRQ(TIMER3_IRQn);
	}
	return ret;
}

void WAIT_SYS_Ms(uint32_t millis)
{
	#ifdef FREERTOS // Not FREERTOS
		vTaskDelay(millis / portTICK_PERIOD_MS);
	#else
		uint32_t start = __ms;
		while ((__ms - start) < millis) {
			__WFI();
		}
	#endif
}

void WAIT_TIM0_Us(uint32_t micros)
{
	LPC_TIM0->MR0 = micros;

	LPC_TIM0->TCR = ENABLE;

	while(!(LPC_TIM0->IR & 0x1));
	LPC_TIM0->IR = TIMER_IR_MR0;

    LPC_TIM0->TCR = DISABLE_RESET;
}

void WAIT_IRQ0_Us(uint32_t micros, void(*f)(void))
{
	LPC_TIM0->MR0 = micros;
	h0 = f;

	busyTim0 = true;
	LPC_TIM0->TCR = ENABLE;
}

void WAIT_TIM1_Us(uint32_t micros)
{
	LPC_TIM1->MR0 = micros;

	LPC_TIM1->TCR = ENABLE;

	while(!(LPC_TIM1->IR & 0x1));
	LPC_TIM1->IR = TIMER_IR_MR0;

    LPC_TIM1->TCR = DISABLE_RESET;
}

void WAIT_IRQ1_Us(uint32_t micros, void(*f)(void))
{
	LPC_TIM1->MR0 = micros;
	h1 = f;

	busyTim1 = true;
	LPC_TIM1->TCR = ENABLE;
}

void WAIT_TIM2_Us(uint32_t micros)
{
	LPC_TIM2->MR0 = micros;

	LPC_TIM2->TCR = ENABLE;

	while(!(LPC_TIM2->IR & 0x1));
	LPC_TIM2->IR = TIMER_IR_MR0;

    LPC_TIM2->TCR = DISABLE_RESET;
}

void WAIT_IRQ2_Us(uint32_t micros, void(*f)(void))
{
	LPC_TIM2->MR0 = micros;
	h2 = f;

	busyTim2 = true;
	LPC_TIM2->TCR = ENABLE;
}

void WAIT_TIM3_Us(uint32_t micros)
{
	LPC_TIM3->MR0 = micros;

	LPC_TIM3->TCR = ENABLE;

	while(!(LPC_TIM3->IR & 0x1));
	LPC_TIM3->IR = TIMER_IR_MR0;

    LPC_TIM3->TCR = DISABLE_RESET;
}

void WAIT_IRQ3_Us(uint32_t micros, void(*f)(void))
{
	LPC_TIM3->MR0 = micros;
	h3 = f;

	busyTim3 = true;
	LPC_TIM3->TCR = ENABLE;
}

bool WAIT_IsBusy(WAIT timer) {
	switch (timer) {
		case TIM0:
			return busyTim0;
		case IRQ0:
			return busyTim0;
		case TIM1:
			return busyTim1;
		case IRQ1:
			return busyTim1;
		case TIM2:
			return busyTim2;
		case IRQ2:
			return busyTim2;
		case TIM3:
			return busyTim3;
		case IRQ3:
			return busyTim3;
		default:
			return false;
	}
}

uint32_t WAIT_SYS_GetElapsedMs(uint32_t start)
{
	#ifdef FREERTOS
		return pdTICKS_TO_MS(xTaskGetTickCount()) - start;
	#else
		return __ms - start;
	#endif
}
