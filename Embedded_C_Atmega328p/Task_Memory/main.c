/*
 * Ab_3.c
 *
 * Created: 26.04.2022 14:05:13
 * Author : Blaki
 */


#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stddef.h> // für NULL
#include "events.h"

#define Event_50ms 0
#define Event_1s 1
#define Timer_50ms 0
#define Timer_1s 1
#define test_timer 2

volatile uint8_t i = 0; //laufzähler für LED
volatile uint8_t run = 0;

#include "serial.h"


ISR(TIMER0_OVF_vect)
{
    static uint32_t counter=0;
    TCNT0 = 6;                    // bis 250 zählen dann kommt ein interrupt (1 takt = 4us bei 64 pre scale| 250 takte entspricht dann 1ms)
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

/*
void(*callback0)()=NULL;
void(*callback1)()=NULL;
void(*callback2)()=NULL;
void(*callback3)()=NULL;
void(*callback4)()=NULL;
*/
ISR(TIMER1_OVF_vect)
{
    //TCNT1 = 0xFFF0;        // 1us
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
	/*
    if( (timer_triggerd & 0x1) == 1 && callback0!=NULL)
    {
        timer_triggerd = timer_triggerd & ~0x1;
		sei();
        callback0();
    }
    if( ((timer_triggerd & 0x2)>> 1) == 1 && callback1!=NULL)
    {
		//PORTD ^= (1<<PORTD3);
		timer_triggerd = timer_triggerd & ~0x2;
		sei();
        callback1();
    }
    if( ((timer_triggerd & 0x4)>>2) ==1 && callback2!=NULL)
    {
        timer_triggerd = timer_triggerd & ~0x4;
		sei();
        callback2();
    }
    if(((timer_triggerd & 0x8)>>3) ==1 && callback3!=NULL)
    {
        timer_triggerd = timer_triggerd & ~0x8;
		sei();
        callback3();
    }
    if(((timer_triggerd & 0x10)>>4) == 1 && callback4!=NULL)
    {
        timer_triggerd = timer_triggerd & ~0x10;
		sei();
        callback4();
    }
	*/
	/*
	if (counter1 == 2000)
	{
		PORTD ^= (1<<PORTD4);
		counter1 = 0;
	}   
	*/      
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
		touch_0 = (PIND >> PIND3) & 1;            // Pin 3 gesetzt ? / Taster 4 gedrückt?
		if(touch_0 == 0 && state_0 == 1)
		{
			run = 0;
			i = 0;
			PORTB |= 0x7;
		}
		state_0 = touch_0;
		            
		touch_1 = (PIND >> PIND4) & 1;            // Pin 4 gesetzt ? / Taster 5 gedrückt?
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
			//max = i;
		}
	}
}

int main(void)
{

    
    DDRB |= (1<<PORTB0)| (1<<PORTB1)| (1<<PORTB2) | (1<<PORTB3) | (1<<PORTB4);                  // DDRD auf output umstellen
	//PORTB |= (1<<PORTB0) | (1<<PORTB1)| (1<<PORTB2)| (1<<PORTB3)| (1<<PORTB4);					// default wert
    PORTD |= (1<<PORTD0)| (1<<PORTD1)| (1<<PORTD2)| (1<<PORTD3) | (1<<PORTD4);                    
    
    
    /* Replace with your application code */
    TCNT0 = 6;
    TCCR0B &= ~((1<<CS02)|(1<<CS01)|(1<<CS00));
    TCCR0B |= (1<<CS01) | (1<<CS00); // 64 prescale
    TIMSK0 |= (1<<TOIE0); // Timer interrupt erlaubt
    
    //TCNT1 = 0xFFF0;
	TCNT1 = 49536;
    setTimer(Timer_50ms,50,button);
	setTimer(Timer_1s,1000,toogle);
	setTimer(test_timer,1,toogle);
    TCCR1B &= ~((1<<CS02)|(1<<CS01)|(1<<CS00));
    TCCR1B |= (1<<CS00);        // kein prescale
    //callback0 = button;
	//callback1 = toogle;
	//callback2 = toogle;
    TIMSK1 |= (1<<TOIE1); // Timer interrupt erlaubt
    sei(); // alle interrupts erlauben
    
	//startTimer(test_timer);
	//startTimer(Timer_1s);
	//startTimer(Timer_50ms);
    
    while (1)
    {
		volatile uint32_t test1 = big_timer_box[0].time;
		test1++;
		test();
		
	}
}
