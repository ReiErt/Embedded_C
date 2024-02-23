/*
 * Blatt6a.c
 *
 * Created: 10.01.2022 14:26:49
 * Author : Ertman
 */ 
#define F_CPU 16000000ul

#include <avr/io.h>
#include "UART.h"
#include "ADC.h"
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdio.h>

const char msg [] PROGMEM = "Starte Messung:\n";
const uint8_t msg_length = 15;

int main(void)
{
	
	USART_Init();
	init_adc();
	
	for(uint8_t i = 0; i<msg_length; i++) USART_Transmit(pgm_read_byte(msg+i));
	
    while (1) 
    {		
		USART_Transmit_number(to_mv(read_adc(1<<MUX2)));
		USART_Transmit('m');
		USART_Transmit('v');
		
		USART_Transmit(' ');		
		
		char str[10];
		sprintf(str, "%dC\n", read_temp());
		
		for(uint8_t i = 0; i<10; i++) USART_Transmit(str[i]);
		
		
		_delay_ms(1000);				
    }
}

