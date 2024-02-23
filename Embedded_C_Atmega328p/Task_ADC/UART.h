/*
 * UART.h
 *
 * Created: 26.04.2022 16:42:27
 *  Author: Ertman
 */ 


#ifndef UART_H_
#define UART_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#ifndef BAUDRATE
#define BAUDRATE 9600
#endif

#ifndef BAUD_CONST
#define BAUD_CONST (((F_CPU/(BAUDRATE*16UL)))-1)
#endif

#define BUFFER_SIZE 32 //in byte
#define UPPER_LIMIT 75.0f
#define LOWER_LIMIT 25.0f

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

volatile unsigned char buffer [BUFFER_SIZE];
volatile uint8_t ptr_curr = 0;
volatile uint8_t ptr_curr_read = 0;
volatile uint8_t unread_elements = 0;
volatile uint8_t upper_limit = BUFFER_SIZE / UPPER_LIMIT;
volatile uint8_t lower_limit = BUFFER_SIZE / LOWER_LIMIT;
volatile bool buffer_upper = false;
volatile bool dont_send = false;

void USART_Transmit(unsigned char data);

ISR(USART_RX_vect)
{
	unsigned char udr0_temp = UDR0;
	if(udr0_temp == 0x13) dont_send = true;
	if(udr0_temp == 0x11) dont_send = false;
	else
	{
		buffer[ptr_curr] = udr0_temp;
		ptr_curr = (ptr_curr+1) % BUFFER_SIZE;
		unread_elements = (unread_elements+1) % BUFFER_SIZE;
		if(unread_elements > upper_limit)
		{
			USART_Transmit(0x13);
			buffer_upper = true;
		}
	}
}

void USART_Init(){
	UBRR0H = (BAUD_CONST >> 8);
	UBRR0L = BAUD_CONST;
	UCSR0B |= (1<<RXEN0)|(1<<TXEN0) | (1<<RXCIE0);
	sei();
}



void USART_Transmit(unsigned char data){
	if(!dont_send)
	{
		while(!(UCSR0A & (1<<UDRE0))) ;
		UDR0 = data;
	}
}

void USART_Transmit_number(uint16_t n)
{
	char number [5];
	for(int i = 4; i >= 0; i--)
	{
		number[i] = n % 10;
		n -= number[i];
		n /= 10;		
	}
		
	bool prev_zero = true;
	for(int i = 0; i<4; i++)
	{
		if(!number[i]==0 || !prev_zero)
		{
			USART_Transmit(number[i]+48);
			prev_zero = false;
		}
	}
	USART_Transmit(number[4]+48);
}

bool USART_Receive(char* elem){
	if(unread_elements > 0){
		(*elem) = buffer[ptr_curr_read];
		ptr_curr_read = (ptr_curr_read+1) % BUFFER_SIZE;
		unread_elements--;
		if(buffer_upper && unread_elements < lower_limit)
		{
			USART_Transmit(0x11);
			buffer_upper = false;
		}
		
		return true;
	}
	return false;
}

#endif /* UART_H_ */