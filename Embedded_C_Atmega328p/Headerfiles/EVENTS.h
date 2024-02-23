/*
 * events.h
 *
 * Created: 26.04.2022 18:54:05
 *  Author: Blaki
 */ 


#ifndef EVENTS_H_
#define EVENTS_H_

#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/common.h>
#include <avr/eeprom.h>

// event functions
void setEvent (uint8_t counter);
uint8_t eventIsSet(uint8_t counter);
void clearEvent(uint8_t counter);

// global variables
volatile uint16_t EventCounter = 0;
volatile uint8_t timer_triggerd = 0;
volatile uint8_t TimerCounter = 0;		//tells us how many software timers exist
volatile uint8_t StartTimerCounter = 0; //the timer has a new start time bekommen

// Timer functions
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
	// bit auf 1 setzen
	EventCounter = EventCounter | ( 1 << (counter) );
	SREG = sreg_old;
}

void clearEvent(uint8_t counter)
{
	unsigned char sreg_old = SREG;
	cli();
	// bit loeschen
	EventCounter = EventCounter & ( ~ ( 1 << (counter) ) );
	SREG = sreg_old;
}

uint8_t eventIsSet(uint8_t counter)
{
	// moves bit to LSB and checks if true 
	return( ( EventCounter >> (counter ) & 1 ) == 1 );
}

// ----------------------------------------------------------------- Timer -------------------------------------------------------------------------------------

void Timer(uint32_t counter_now)
{
	uint32_t ntime; // Derzeitige Zeit
	unsigned char sreg_old = SREG;
	cli();
	for(int i = 0; i < 5; i++)
	{
		// checking first four bits from LSB to MSB to see if
		// the timer exists, it gets angelegt with a certain time duration
		if(((TimerCounter >> i) & 1) == 1)
		{
			big_timer_box[i].ntime = counter_now;
		}
		
		// checking first four bits from LSB to MSB to see if
		// TimerCounter is set AND StartTimerCounter is not set
		// if timer exists but has not started, then we note the start time.
		// then we note that that timer has started
		//printf("Timercounter %i\n",TimerCounter);
		if( ((TimerCounter >> i) & 1) == 1 && ((StartTimerCounter >> i) & 1) == 0)
		{
			big_timer_box[i].start_time = counter_now;
			//printf("Startzeit wird geholt\n");
			// timer starts
			StartTimerCounter = StartTimerCounter | (1<<i);
		}
	}
	ntime = counter_now;
	for (int i = 0; i < 5; i++)
	{
		if(/*(big_timer_box[i].time != 0) &&*/ eeprom_read_word(Start_time_eeprom+i) && ((TimerCounter >> i & 1 ) == 1) &&  ((ntime - big_timer_box[i].start_time) >= /*big_timer_box[i].time*/ eeprom_read_word(Start_time_eeprom+i)))
		{
			//printf("ntime = %i starttime = %i Zeitabgelaufen nach %i \n",ntime, big_timer_box[0].start_time, (ntime-big_timer_box[0].start_time));
			timer_triggerd |= (1<<i);
			StartTimerCounter = StartTimerCounter & (~(1<<i)); // bestimmtes bit auf 0 setzen um neuen startwert zu holen
			
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
	eeprom_write_word(Start_time_eeprom+timer_,time_);			//funktiniert nicht?
	SREG = sreg_old;
}

void startTimer(uint8_t timer)
{
	unsigned char sreg_old = SREG;
	cli();
	// when timer starts counting, set bit at TimerCounter
	TimerCounter = TimerCounter | (1 << timer);		
	// when timer starts counting, clear bit at StartTimerCounter		
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