/*
 * ADC.h
 *
 * Created: 10.05.2022 23:20:17
 *  Author: Ertman
 */ 

#ifndef ADC_H_
#define ADC_H_

#ifndef U_REF
#define U_REF 5
#endif

#ifndef ADC_PRECISION
#define ADC_PRECISION 1024 //2^10
#endif

#ifndef ADC_RESOLUTION_M
#define ADC_RESOLUTION_M ((U_REF*1000000) / ADC_PRECISION)
#endif

#include "UART.h"
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

volatile uint8_t old_ADMUX_REF = 0;

uint16_t read_adc(uint8_t port)
{
	if(port>7) return 0;
	
	ADMUX &= ~((1<<REFS1) | (1<<REFS0) | (1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (1<<MUX0)); //clear ADMUX
		
	ADMUX |= (port & 0x0f); //set port. this line only manipulates first four bits
	
	ADMUX |= (1<<REFS0) | (0<<REFS1); //set reference to VCC (5 volt)
	
	if((ADMUX&(0xC0)) != old_ADMUX_REF)
	{
		old_ADMUX_REF = ADMUX&(0xC0);
		_delay_ms(5);
	}
	
	ADCSRA &= ~((1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0) | (1<<ADSC)); //clears ADCSRA

	ADCSRA |= (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2); //set prescaler to 128
	
	ADCSRA |= (1<<ADSC); //start conversion		
	
	while((ADCSRA & (1<<ADSC)) != 0); //waits for conversion to end. ADSC is 0 when finished
	
	//ADCL has to be read first
	return ADC;	 
}

//function maps adc_value to range [0 to 1024] and uses that percentage to map to max
// ex) input convert_adc_value_to_8_bit(500,249) returns 121.7
// ex) input convert_adc_value_to_8_bit(512,249) returns 124.6 
// ex) input convert_adc_value_to_8_bit(1024,249)returns 249
uint8_t convert_adc_value_to_8_bit(uint16_t adc_value, uint8_t max)
{	
	uint32_t temp = (uint32_t)adc_value * 1000;	
	temp /= 1023;
	temp *= max;
	temp /= 1000;
	
	return (uint8_t) temp;
}


uint16_t to_mv(uint16_t adc_value)
{
		return (adc_value * ADC_RESOLUTION_M)/ 1000;
}

int16_t read_temp()
{
	ADMUX &= ~((1<<REFS1) | (1<<REFS0) | (1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (1<<MUX0)); //clear ADMUX
	 
	ADMUX |= (1<<MUX3); //set port	
	ADMUX |= (1<<REFS0) | (1<<REFS1); //set reference to internal reference voltage of 1.1V
	
	if((ADMUX&(0xC0)) != old_ADMUX_REF)
	{
		old_ADMUX_REF = ADMUX&(0xC0);
		_delay_ms(5);
	}
	
	ADCSRA &= ~((1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0) | (1<<ADSC)); //clears ADCSRA
	
	ADCSRA |= (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2); //set prescaler to 128
	
	
	ADCSRA |= (1<<ADSC); //start conversion
	
	while((ADCSRA & (1<<ADSC)) != 0); //waits for conversion to end
	
	//ADCL has to be read first
	return ((ADCL | (ADCH << 8)) - 342); //offset
}


void init_adc()
{
	PRR |= (0<<PRADC); //disable Power Reduction ADC
	ADCSRA |= (1<<ADEN); //enable ADC	
}


#endif /* ADC_H_ */