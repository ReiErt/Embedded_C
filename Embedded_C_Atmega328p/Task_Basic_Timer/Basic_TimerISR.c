#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

// each clock cycle = 50ns at 50MHz. 
// 50ns * prescaler = 3200ns
// 3200ns * (256 - 178) = 1,000,000ns
// 1000000ns * 1000 = 1 second

ISR(TIMER0_OVF_vect){
	static uint16_t counter = 0;
	TCNT0 = 178;
	counter++;
	if (counter==1000){
		counter = 0;
		PORTB ^= (1<<PORTB1);
	}
}


int main(void)
{
    DDRB |= (1<<PORTB1)|(1<<PORTB0);
	TCNT0 = 178;
	TCCR0B &= ~((1<<CS02)|(1<<CS01)|(1<<CS00));
	//set Clk 256 prescaler with each clock cycle 50ns
	TCCR0B |= (1<<CS02)|(1<<CS00);
	TIMSK0 |= (1<<TOIE0);
	sei();
    while (1) 
    {
		_delay_ms(200);
		PORTB ^= (1<<PORTB0);
    }
}
