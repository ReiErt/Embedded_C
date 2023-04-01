/*
 * PWM.h
 *
 * Created: 11.05.2022 09:58:15
 *  Author: Ertman
 */ 
#ifndef PWM_H_
#define PWM_H_

#include <avr/interrupt.h>
#include <avr/io.h>
 
void set_duty_cycle_pwm1(uint16_t duty_cycle)
{
	OCR1A = duty_cycle;
}
  
void init_pwm1(uint16_t top)
{
	//grey lien compare is OCR1A.
	//Timer/Counter1 Control Register A
	TCCR1A |= (1<<COM1A1) | (1<<WGM11); //set fast pwm mode ICR1 as TOP; set non-inverting mode
	//Timer/Counter1 Control Register B
	TCCR1B |=  (1<<WGM13) | (1<<WGM12) | (1<<CS11) | (1<<CS10); //set fast pwm mode; and set prescaler to 64
	//TCCR1B |=  (1<<WGM13) | (1<<WGM12) | (1<<CS12); //set fast pwm mode; set pre-scaler to 256!!!!
	ICR1 = top;
	
	DDRB |= (1<<PORTB1);	//set OC1A as output
}


void set_duty_cycle_pwm0(uint8_t duty_cycle)
{
	OCR0B = duty_cycle;
}  

void init_pwm0(uint8_t top)
{	
	//here is specify settings for the Grey compare line OCR0B.
	TCCR0B |= (1<<CS01) | (1<<CS00); //set prescaler to 64
	
	// set fast PWM Mode. OCRA is top. 
	TCCR0B |= (1<<WGM02);
	TCCR0A |= (1<<WGM00)| (1<<WGM01);
	
	
	// Fast PWM Mode + top is OCRA + at compare, go to bottom
	TCCR0A |= (1<<COM0B1); //non-inverting mode
	
	OCR0A = top; //set OCR0A top value. does not generate PWM
	
	OCR0B = 200; //set duty cycle to 100%. OCR0B makes the duty cycles
	
	DDRD |= (1<<PORTD5);	//set OCR0B as output	
}

#endif /* PWM_H_ */