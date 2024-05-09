/*
 * events.h
 *
 * Created: 26.04.2022 18:54:05
 *  Author: Reilly
 */ 

#ifndef EVENTS_H_
#define EVENTS_H_

#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/common.h>
#include <avr/eeprom.h>


void setEvent (uint8_t counter);
uint8_t eventIsSet(uint8_t counter);
void clearEvent(uint8_t counter);
volatile uint16_t EventCounter = 0;

volatile uint8_t timer_triggerd = 0;
volatile uint8_t TimerCounter = 0;
volatile uint8_t StartTimerCounter = 0;
void setTimer(uint8_t timer_, uint32_t time_, void (*timer_callback_)());
void set_start_Timer(uint8_t timer_, uint32_t time_);
void startTimer(uint8_t timer);
void cancelTimer(uint8_t timer);
void Timer(uint32_t counter_now);
struct Timer_box{
	uint32_t time;
	uint32_t start_time;
	uint32_t ntime;
	void (*timer_callback)();
}big_timer_box[5];
uint16_t Start_time_eeprom[5] EEMEM;

// ----------------------------------------------------------------- Events ------------------------------------------------------------------------

void setEvent(uint8_t counter)
{
	unsigned char sreg_old = SREG;
	cli();
	EventCounter = EventCounter | ( 1 << (counter) );// Counter Bit auf 1 setzen
	SREG = sreg_old;
}

void clearEvent(uint8_t counter)
{
	unsigned char sreg_old = SREG;
	cli();
	EventCounter = EventCounter & ( ~ ( 1 << (counter) ) ); // Counter Bit auf Null setzten
	SREG = sreg_old;
}

uint8_t eventIsSet(uint8_t counter)
{
	return( ( EventCounter >> (counter ) & 1 ) == 1 );
}

// ----------------------------------------------------------------- Timer -------------------------------------------------------------------------------------

void Timer(uint32_t counter_now)
{
	uint32_t ntime; // Current Zeit
	unsigned char sreg_old = SREG;
	cli(); 		// disable interrupts
	for(int i = 0; i < 5; i++)
	{
		if(((TimerCounter >> i) & 1) == 1)
		{
			big_timer_box[i].ntime = counter_now;
		}

		if( ((TimerCounter >> i) & 1) == 1 && ((StartTimerCounter >> i) & 1) == 0)
		{
			big_timer_box[i].start_time = counter_now;
			//printf("Startzeit wird geholt\n");
			StartTimerCounter = StartTimerCounter | (1<<i); // bestimmtes bit auf 1 setzen um keine neuen Startzeiten zu bekommen
		}
	}
	ntime = counter_now;
	for (int i = 0; i < 5; i++)
	{
		if(eeprom_read_word(Start_time_eeprom+i) && ((TimerCounter >> i & 1 ) == 1) &&  ((ntime - big_timer_box[i].start_time) >= eeprom_read_word(Start_time_eeprom+i)))
		{
			timer_triggerd |= (1<<i);
			StartTimerCounter = StartTimerCounter & (~(1<<i)); // set bit i to 0 to get new start value
		}
	}
	SREG = sreg_old;
}

void setTimer(uint8_t timer_, uint32_t time_, void (*timer_callback_)())
{
	unsigned char sreg_old = SREG;
	cli();
	big_timer_box[timer_].time = time_;
	big_timer_box[timer_].timer_callback = timer_callback_;
	
	SREG = sreg_old;
}
void set_start_Timer(uint8_t timer_, uint32_t time_)
{
	unsigned char sreg_old = SREG;
	cli();
	big_timer_box[timer_].time = time_;
	eeprom_write_word(Start_time_eeprom+timer_,time_);
	SREG = sreg_old;
}

void startTimer(uint8_t timer)
{
	unsigned char sreg_old = SREG;
	cli();
	TimerCounter = TimerCounter | (1 << timer);
	StartTimerCounter = StartTimerCounter & (~(1<<timer)); 
	SREG = sreg_old;
}

void cancelTimer(uint8_t timer)
{
	unsigned char sreg_old = SREG;
	cli();
	TimerCounter = TimerCounter & ( ~ (1 << timer) );
	SREG = sreg_old;
}

#endif /* EVENTS_H_ */
