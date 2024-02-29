/*
 * Ab_3.c
 *
 * Created: 26.04.2022 14:05:13
 * Author : Reilly
 */

#define F_CPU 16000000UL // CPU speed
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stddef.h>
#include "events.h"

#define Event_50ms 0
#define Event_1s 1
#define Timer_50ms 0
#define Timer_1s 1
#define test_timer 2

volatile uint8_t i = 0; //running counter for LED
volatile uint8_t run = 0;

#include "serial.h"


ISR(TIMER0_OVF_vect)
{
    static uint32_t counter=0;
    TCNT0 = 6;                    // count to 250. then interrupt. (1 clock cycle = 4us, with a prescaler of 64. 250 ticks is 1ms.
    counter++;
    if((counter%50)==0)            // 50 ms
    {        
        setEvent(Event_50ms);
    }
    if((counter%1000)==0)
    {
        counter=0;
        setEvent(Event_1s);
    }
}

ISR(TIMER1_OVF_vect)
{
    TCNT1 = 49536;
    static uint16_t counter1 = 0;
    Timer(counter1);
    counter1++;
    for (int i = 0; i < 5; i++)
	{
		if( ((timer_triggerd>>i)& 1) == 1 && big_timer_box[i].timer_callback != NULL)
		{
			timer_triggerd = timer_triggerd & ~(1<<i);
			sei();
			big_timer_box[i].timer_callback();
		}
	}
}

void toogle()
{
	PORTB |= 0x7;
	PORTB &= i;
	i++;
	if(i > 7) i = 0;
}

void button()
{
	static uint8_t touch_0 = 0;
	static uint8_t touch_1 = 0;
	static uint8_t state_0 = 0;
	static uint8_t state_1 = 0;
	uint8_t setcounter = 0;
	for(int counter = 0; counter < 10; counter ++)
	{
		touch_0 = (PIND >> PIND3) & 1;            // Check button PIND3 if pressed.
		if(touch_0 == 0 && state_0 == 1)
		{
			run = 0;
			i = 0;
			PORTB |= 0x7;
		}
		state_0 = touch_0;
		            
		touch_1 = (PIND >> PIND4) & 1;            // Check button PIND4 if pressed.
		if(touch_1 == 0 && state_1 == 1)
		{
			run = 1;
			setcounter = 1;

		}
		state_1 = touch_1;
		if(run == 1 && setcounter == 1)
		{
			setcounter = 0;
			i = (PIND & PIND0) | (PIND & PIND1) | (PIND & PIND2);
		}
	}
}

int main(void)
{
    DDRB |= (1<<PORTB0)| (1<<PORTB1)| (1<<PORTB2) | (1<<PORTB3) | (1<<PORTB4);                  // DDRD auf output umstellen
    PORTD |= (1<<PORTD0)| (1<<PORTD1)| (1<<PORTD2)| (1<<PORTD3) | (1<<PORTD4);                    
    
    TCNT0 = 6;
    TCCR0B &= ~((1<<CS02)|(1<<CS01)|(1<<CS00));
    TCCR0B |= (1<<CS01) | (1<<CS00); 	// 64 prescaler for timer interrupt 1
    TIMSK0 |= (1<<TOIE0);	 	// Enable Timer interrupt 1
    
    TCNT1 = 49536;
    setTimer(Timer_50ms,50,button);
    setTimer(Timer_1s,1000,toogle);
    setTimer(test_timer,1,toogle);
    TCCR1B &= ~((1<<CS02)|(1<<CS01)|(1<<CS00));
    TCCR1B |= (1<<CS00);        // no prescale for timer interrupt 2
    TIMSK1 |= (1<<TOIE1); 	// Enable Timer interrupt 2
    sei(); 			// Enable all interrupts
    
    while (1)
    {
	volatile uint32_t test1 = big_timer_box[0].time;
	test1++;
	test();
		
	}
}
