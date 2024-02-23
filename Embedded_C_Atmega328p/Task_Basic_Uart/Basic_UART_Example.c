/*
 * Basic_USART
 *
 * 
 * Author : Reilly
 */ 

#define F_CPU 16000000UL

#define BAUDRATE 9600
#define BAUD_CONST (((F_CPU/(BAUDRATE*16UL)))-1)  //Baudrate in UBRRH und UBRRL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>


void USART_Init(){
	UBRR0H = (BAUD_CONST >> 8);	//implied Typecast to unsigned Char. 8 MSBs from 16 bit used
	UBRR0L = BAUD_CONST;		// 8 LSBs from 16 bit used
	UCSR0B |= (1<<RXEN0)|(1<<TXEN0);
}


void USART_Transmit(unsigned char data){
	while(!(UCSR0A & (1<<UDRE0))) ;			//Blocking function. Data is sent when URD0 is free
	UDR0 = data;							
}


unsigned char USART_Receive(){
	while(!(UCSR0A & (1<<RXC0))) ;
	return UDR0;
}

int main(void)
{
	char name[10];
	int g = 0;
	const char meldung[]="Hello ATmega. Anybody home?";
	USART_Init();
	for(int f=0;meldung[f]!='\0';f++)	//Send string to PC, character for character
		USART_Transmit(meldung[f]);
    while (1) 
    {
		name[g] = USART_Receive();
		if (name[g]!=0x0d){
			USART_Transmit(name[g]);
			g++;
			continue;	
		}
		USART_Transmit(0x0d);
		USART_Transmit('H');
		USART_Transmit('i');
		USART_Transmit(' ');
		for(int f=0;f<g;f++)
			USART_Transmit(name[f]-32);
		g = 0;
		USART_Transmit(0x0d);
    }
}

