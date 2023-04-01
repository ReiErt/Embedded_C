/*
 * Ab_4.c
 *
 * Created: 03.05.2022 15:51:25
 * Author : Reilly
 */

#define F_CPU 16000000UL

#define BAUDRATE 9600
#define BAUD_CONST (((F_CPU/(BAUDRATE*16UL)))-1)

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "events.h"

#define BUFFER_SIZE 32
uint8_t volatile Head = 0;
uint8_t volatile Tail = 0;
uint8_t volatile isFull_Flag = 0;
unsigned char Buffer[BUFFER_SIZE];

uint8_t flow_controll = 0;
uint8_t flow_controll_recive = 0;
#define Xoff 0x13
#define Xon 0x11

volatile uint8_t option = 0;

void USART_Init();
int is_Buffer_Full();
uint8_t is_Buffer_Empty();
void Write_to_Buffer(unsigned char data);
unsigned char Read_from_Buffer();
uint8_t USART_Transmit(unsigned char data);
unsigned char USART_Receive();
uint8_t distance();
void HUD(uint8_t hud_register);
void message_output(const volatile char *message);
void message_output_flash(const volatile char *message);
void Timer_software(unsigned char o, uint8_t timer);
void test();

// save Zeichenkette in Flash
volatile const char Message_default[] __attribute__((progmem)) = "Wrong input Wrong input Wrong input Wrong input Wrong input Wrong input\n";
volatile const char Message_0[] PROGMEM = "\nSelect Timer\n1. Timer0\n2. Timer1\n3. Timer2\n4. Timer3\n5. Timer4\n";
volatile const char Message_1_1[] PROGMEM= "\nFor Timer";
volatile const char Message_1_2[] PROGMEM= "Select Action\n1. Start Timer\n2. Stop Timer\n3. Set Timer\n4. Show Start Time\n5. back\n";
volatile const char Message_2[] PROGMEM= "Stop time of Timer ";
volatile const char Message_3[] PROGMEM= "\nStart time is ";


void setTimer_serial(uint8_t timer);

void ichwarhier()
{
	char buffer[] = "\n ich war hier\n";
	message_output(buffer);
}


// when data is completely received, this interrupt is called (RXCn Flag in UCSRnA!)
// interrupt says receive is complete, we can start reading
// DATA RECEIVED IN HARDWARE REGISTER. WE READY TO WRITE TO SOFTWARE BUFFER
ISR(USART_RX_vect)
{
	//reading from UDR0 deletes the flag that causes interrupt
    unsigned char temp = UDR0;
	//if we received Xoff and we aren't receiving Head
    if(temp == Xoff && flow_controll_recive == 0)
    {
		// flag showing we no longer receive. buffer full.
		// Our head is too long. Tail must catch up ! we must READ! no sending in this time
		//Tramsmitter Enable aus
         flow_controll_recive = 1;                   
    }
	// if we received Xon and we are receive Head
    else if(temp == Xon && flow_controll_recive == 1)
    {
		// flag showing we are receiving again
		// Our tail has caught up with head. OPEN THE FLOOD GATES
		//Tramsmitter Enable an
         flow_controll_recive = 0;                   
    }
    else
    {
        Write_to_Buffer(temp);
        if( (flow_controll == 0) && ( distance()  >= (BUFFER_SIZE-8) ) )        //Flow controll
        {
            USART_Transmit('$');
            USART_Transmit(Xoff);
            flow_controll = 1;
        }
    }

    
}

// main program that navigates the GUI
void test()
{
    USART_Init();
	char number='0';
	uint8_t timer=0;
	sei();
		
	// we get the starting menu before going into the while(1) schleife
	HUD(0);
	unsigned char temp;
    while (1)
    {
		// if no character received, we wait until we get one
		temp = USART_Receive();
		if(temp == 0)
		{
			continue;
		}
		else
		{
			// if input is no carrage return
			if (temp!=0x0d)
			{
				number = temp;
				USART_Transmit(number);
				continue;
			}
			//valid input?
			if (number != 0)
			{
				// if we want to activate a timer
				if(option == 1)
				{
					Timer_software(number,timer);
					number = 0;
				}
				// if input is valid
				if (number >= 49 && number <= 53 && option == 0)
				{
					option = 1;
					timer = number - 49;
					HUD(((1<<3)|timer));
					
				}
				else if(option == 0 && number != 0)
				{
					USART_Transmit('\n');
					USART_Transmit(number);
					HUD((31<<3));
					HUD(0);
					//ichwarhier();
				}
			}

			
		}
    }
}



void USART_Init(){
	//high register. UBRR is 16 bit. all registers are 8 bit.
	//shift MSB 8 spaces towards LSB
    UBRR0H = (BAUD_CONST >> 8);
	//low register => receives the 8 LSBs
    UBRR0L = BAUD_CONST;
	// enable receiver and transmitter
	UCSR0B &= ~((1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0));
    UCSR0B |= (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
	USART_Transmit(Xon);
}


int is_Buffer_Full()
{
    return(isFull_Flag);
}

uint8_t is_Buffer_Empty()
{
    uint8_t temp = 0;
    if((Head==Tail) && isFull_Flag != 1)
    {
        temp = 1;
    }
    return(temp);
}

void Write_to_Buffer(unsigned char data)
{
    unsigned char sreg_old = SREG;
    cli();
    if(!(is_Buffer_Full()))                        // Wenn full kann man nichts mehr eingeben?
    {
        Buffer[Head] = data;
        Head = (Head+1) % (BUFFER_SIZE-1);
        if(Head == Tail)
        {
            isFull_Flag = 1;
        }
    }
    SREG = sreg_old;
}

unsigned char Read_from_Buffer()
{
    unsigned char data = 0x0d;
    if(!(is_Buffer_Empty()))
    {
        data = Buffer[Tail];
        isFull_Flag = 0;
		//atmega328p read buffer tail moves forward
        Tail = (Tail+1) % (BUFFER_SIZE-1);
    }
    return(data);
}

uint8_t USART_Transmit(unsigned char data)
{
    if(flow_controll_recive)
    {
        flow_controll_recive = 0;
        return(0);
    }
	
	//warte-schleife. before writing, we need to check that we can
	//verUNDe register UCSR0A and bit UDRE0 to see if the bit is set(bzw. free)
	// if bit is 1, then transmit register is empty and  we can send(write)
    while(!(UCSR0A & (1<<UDRE0))) ;        // solange 1 in UDREn ist
	// wait until 0 in UDRE0. bzw. the register has nothing more to send, so that we can send.
    UDR0 = data;
	//successfully sent
    return(1);
}


unsigned char USART_Receive()
{
    if(flow_controll == 1 && (distance()  <= 3) )                    //Flowcontroll
    {
		// tell computer go! send data to us
        USART_Transmit(Xon);
        flow_controll = 0;
    }
    if(!(is_Buffer_Empty()))
    {
        unsigned char temp = Read_from_Buffer();
        return(temp);
	}
	return(0);

    
}

uint8_t distance()
{
    uint8_t temp = 0;
    if(Head >= Tail)
    {
        temp = Head - Tail;
    }
    else
    {
		//atmega328p save buffer head moves forward
		// we first save info, then read it. reading moves tail forward. writing moves head forward.
        temp = Head + (BUFFER_SIZE-1-Tail);
    }
    return (temp);
    
}


// GUI -> look at register and decide how to handle
void HUD(uint8_t hud_register)
{
	// if none of first 5 MSBs set
	// if {0000 0XXX}
	if( (hud_register >>3) == 0 )
	{
	
		//select timer menu
		message_output_flash(Message_0);
	}
	
	// if only bit 3 is set
	// {0000 1XXX}
	if( (hud_register >>3) == 1)
	{
		// for timer :
		message_output_flash(Message_1_1);
		
		// add 48 to hud_register => sets bit4 and bit5
		// displays the number of the timer we have selected
		USART_Transmit((hud_register & 7)+48);
		USART_Transmit(' ');
		
		message_output_flash(Message_1_2);
	}
	// if only bit 4 is set
	// {0001 0XXX}
	if( (hud_register >>3) == 2)
	{
		// Stop time of timer
		message_output_flash(Message_2);

		//and 48 is added to hud_register => sets bit4 and bit5
		// displays the number of the timer we have selected
		USART_Transmit((hud_register & 7)+48);
	}
	
	// if bit 3 and 4 are set
	// {0010 0XXX}
	if( (hud_register >>3) == 3)
	{
		// start time is: 
		message_output_flash(Message_3);
	}
	
	// if first 5 MSBs are set
	if( (hud_register >>3) == 31)
	{

		// wrong input
		message_output_flash(Message_default);
	}
		
}

void message_output(const volatile char *message)
{
	for(int i = 0; message[i] != '\0'; i++)
	{
		USART_Transmit(message[i]);
	}
}

void message_output_flash(const volatile char *message)
{
	char temp;
	//travel through memory character by character / byte by byte
	for(int i = 0; pgm_read_byte(message + i) != '\0'; i++)
	{
		//read from flash
		temp = pgm_read_byte(message + i);		
		USART_Transmit(temp);
	}
}

void number_output(uint32_t n)
{
	uint8_t counter = 0;

	unsigned char number[32];
	if (n == 0)
		USART_Transmit('0');
	while (n > 0)
	{
		number[counter] = (n % 10)+48;
		n = n / 10;
		counter++;
	}
	for (int i = counter-1 ; i >= 0; i--)
	{
		USART_Transmit(number[i]);
	}
	
}

void Timer_software(unsigned char o, uint8_t timer)
{
	uint32_t temp = 0;
	uint8_t reg_temp = 0;
	switch(o)
	{
		// start timer
		case '1':
		startTimer(timer);
		USART_Transmit('\n');
		break;
		
		// stop timer
		case '2':
		cancelTimer(timer);
		USART_Transmit('\n');
		temp = big_timer_box[timer].ntime;
		// sets fourth bit
		reg_temp |= ((2 << 3) | timer);
		//interface with gui
		HUD(reg_temp);
		USART_Transmit(' ');
		number_output(temp);
		USART_Transmit(' ');
		USART_Transmit('m');
		USART_Transmit('s');
		break;
		
		// set timer
		case '3':
		setTimer_serial(timer);
		break;
		
		// show start timer
		case '4':
		reg_temp |= ((3<<3)| timer);
		HUD(reg_temp);
		//number_output(big_timer_box[timer].time);
		// write to eeprom
		number_output(eeprom_read_word(Start_time_eeprom+timer));
		//number_output(55);
		USART_Transmit('\n');
		break;
		
		// back. send us to start
		case '5':
		HUD(0);
		option = 0;
		break;
		
		// {XXX1 1111} becomes {1111 1000}
		default:
		HUD((31<<3));		
	}
}

unsigned char input()
{
	unsigned char temp = USART_Receive();
	if(temp != 0)
	{
		USART_Transmit(temp);
		return(temp);
	}
	return(0);
}

void setTimer_serial(uint8_t timer)
{
	USART_Transmit('\n');
	unsigned char temp = 0;
	uint32_t number = 0;
	int i = 0;
	while (temp!=0x0d && i < 32)
	{		
		temp = input();
		if (temp >= 48 && temp <= 57)
		{
			if(i!= 0)
			{
				number = number *10;				
			}
			number = number + (temp - 48);
			i++;
		}
	}
	set_start_Timer(timer,number);
	// write to eeprom
	eeprom_write_word(Start_time_eeprom+timer,(uint16_t)number);
	
}

