/*
 * rtc.c
 *
 *  Created on: Dec 2020
 *      Author: PedroG
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include "rtc.h"

static int getDaysInMonth();


void RTC_Init(time_t seconds)
{
	LPC_SC->PCONP |= RTC_PCONP_ENABLE; // Enable RTC

	LPC_RTC->CCR = RTC_DISABLE; // Disable RTC time counters
	LPC_RTC->AMR = 0x00; // Disable alarms
	LPC_RTC->CIIR = 0x00; // Disable interrupts

	RTC_SetSeconds(seconds); // Set RTC time
	RTC_Enable(); // Enable RTC time counters
}

void RTC_GetValue(struct tm *dateTime)
{
	dateTime->tm_sec = LPC_RTC->SEC;
	dateTime->tm_min = LPC_RTC->MIN;
	dateTime->tm_hour = LPC_RTC->HOUR;
	dateTime->tm_mday = LPC_RTC->DOM;
	dateTime->tm_wday = LPC_RTC->DOW;
	dateTime->tm_yday = LPC_RTC->DOY;
	dateTime->tm_mon = LPC_RTC->MONTH;
	dateTime->tm_year = LPC_RTC->YEAR;
}

void RTC_SetValue(struct tm *dateTime)
{
	LPC_RTC->SEC = dateTime->tm_sec;
	LPC_RTC->MIN = dateTime->tm_min;
	LPC_RTC->HOUR = dateTime->tm_hour;
	LPC_RTC->DOM = dateTime->tm_mday;
	LPC_RTC->DOW = dateTime->tm_wday;
	LPC_RTC->DOY = dateTime->tm_yday;
	LPC_RTC->MONTH = dateTime->tm_mon;
	LPC_RTC->YEAR = dateTime->tm_year;
}

time_t RTC_GetSeconds(void)
{
	struct tm dateTime = {0};
	RTC_GetValue(&dateTime); // Write RTC time to structure tm
	return mktime(&dateTime); // mktime() -> returns the seconds since Epoh (01/01/1970), given a structure tm
}

void RTC_SetSeconds(time_t seconds)
{
	RTC_SetValue(localtime(&seconds));
}

void RTC_Enable(void) {
	LPC_RTC->CCR |= RTC_ENABLE; // Enable time counters
}

void RTC_Disable(void) {
	LPC_RTC->CCR = RTC_DISABLE; // Disable time counters
}

void RTC_IncrementField(RTC_TIME_FIELD field)
{
	int aux;

	switch(field) {
		case YEAR:
			aux = (LPC_RTC->YEAR)+1; // Increment
			LPC_RTC->YEAR = (aux > 4095) ? 0 : aux; // Set bounds [0 : 4095]
			break;
		case MONTH:
			aux = (LPC_RTC->MONTH)+1; // Increment
			LPC_RTC->MONTH = (aux > 11) ? 0 : aux; // Set bounds [0 : 11]  -> Documentation says [1 : 12], but it seems to not be the case...
			break;
		case DOM:
			aux = (LPC_RTC->DOM)+1; // Increment
			LPC_RTC->DOM = (aux > getDaysInMonth()) ? 1 : aux; // Set bounds [1 : DAYS_IN_MONTH]
			break;
		case DOW:
			aux = (LPC_RTC->DOW)+1; // Increment
			LPC_RTC->DOW = (aux > 6) ? 0 : aux; // Set bounds [0 : 6]
			break;
		case HOUR:
			aux = (LPC_RTC->HOUR)+1; // Increment
			LPC_RTC->HOUR = (aux > 23) ? 0 : aux; // Set bounds [0 : 23]
			break;
		case MIN:
			aux = (LPC_RTC->MIN)+1; // Increment
			LPC_RTC->MIN = (aux > 59) ? 0 : aux; // Set bounds [0 : 59]
			break;
		case SEC:
			aux = (LPC_RTC->SEC)+1; // Increment
			LPC_RTC->SEC = (aux > 59) ? 0 : aux; // Set bounds [0 : 59]
			break;
		default:
			break;
	}
}

void RTC_DecrementField(RTC_TIME_FIELD field)
{
	int aux;
	switch(field) {
		case YEAR:
			aux = (LPC_RTC->YEAR)-1; // Decrement
			LPC_RTC->YEAR = (aux < 0) ? 4095 : aux; // Set bounds [0 : 4095]
			break;
		case MONTH:
			aux = (LPC_RTC->MONTH)-1; // Decrement
			LPC_RTC->MONTH = (aux < 0) ? 11 : aux; // Set bounds [0 : 11]  -> Documentation says [1 : 12], but it seems to not be the case...
			break;
		case DOM:
			aux = (LPC_RTC->DOM)-1; // Decrement
			LPC_RTC->DOM = (aux < 1) ? getDaysInMonth() : aux; // Set bounds [1 : 31]
			break;
		case DOW:
			aux = (LPC_RTC->DOW)-1; // Decrement
			LPC_RTC->DOW = (aux < 0) ? 6 : aux; // Set bounds [0 : 6]
			break;
		case HOUR:
			aux = (LPC_RTC->HOUR)-1; // Decrement
			LPC_RTC->HOUR = (aux < 0) ? 23 : aux; // Set bounds [0 : 23]
			break;
		case MIN:
			aux = (LPC_RTC->MIN)-1; // Decrement
			LPC_RTC->MIN = (aux < 0) ? 59 : aux; // Set bounds [0 : 59]
			break;
		case SEC:
			aux = (LPC_RTC->SEC)-1; // Decrement
			LPC_RTC->SEC = (aux < 0) ? 59 : aux; // Set bounds [0 : 59]
			break;
		default:
			break;
	}
}

static int getDaysInMonth() {
	int february_days = 28;
	int year = LPC_RTC->YEAR;
	int month = LPC_RTC->MONTH + 1;

	if ((year%4 == 0) && (year%100 != 0)) {
		february_days = 29;
	}

	if (month == 2) return february_days;
	else if (month == 1 || month == 3 || month == 5 || month == 8 || month == 10 || month == 12) return 31;
	else return 30;
}
