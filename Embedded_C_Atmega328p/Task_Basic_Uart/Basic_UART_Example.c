/*
 * Created: 03.05.2022 15:51:25
 * Author : Ertman
 * UART

TASK 1: 
When a character is received on the UART RX signal line, an ISR interrupt is triggered. The received character is saved in a 32-byte ring buffer. 
Data is read from the 32 byte ring buffer.

TASK 2:
Implement SW flow control using XOn/XOff in both the send function and the receive Interrupt Service Routine. 
If the ring buffer is full, we block incoming data to read and free space on the buffer.
Build a hysteresis when sending XOff and XOn so that the frequency of XOff and XOn remains as low as possible.

 */

#define F_CPU 16000000UL

#define BAUDRATE 9600
#define BAUD_CONST (((F_CPU/(BAUDRATE*16UL)))-1)

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define BUFFER_SIZE 32
uint8_t volatile Head = 0;
uint8_t volatile Tail = 0;
uint8_t volatile isFull_Flag = 0;
unsigned char Buffer[BUFFER_SIZE];

uint8_t flow_controll = 0;
uint8_t flow_controll_recive = 0;
#define Xoff 0x13
#define Xon 0x11


void USART_Init();
int is_Buffer_Full();
uint8_t is_Buffer_Empty();
void Write_to_Buffer(unsigned char data);
unsigned char Read_from_Buffer();
uint8_t USART_Transmit(unsigned char data);
unsigned char USART_Receive();
uint8_t distance();

ISR(USART_RX_vect)
{
    unsigned char temp = UDR0;
    if(temp == Xoff && flow_controll_recive == 0)
    {
         flow_controll_recive = 1;                    // Transmitter Enable aus
    }
    else if(temp == Xon && flow_controll_recive == 1)
    {
         flow_controll_recive = 0;                    // Tramsmitter Enable an
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


int main(void)
{
    char name[1000];
    UCSR0B = ~(1<<RXCIE0);
    UCSR0B = (1<<RXCIE0);
    USART_Transmit(Xon);
    sei();
    

    int g = 0;
    const char meldung[]="Hier ATmega. Wer da?";
    USART_Init();
    for(int f=0;meldung[f]!='\0';f++)
        USART_Transmit(meldung[f]);
    unsigned char temp;
    while (1)
    {
        temp = USART_Receive();
        if(temp == 0)
        {
            continue;
        }
        else
        {
            name[g] = temp;
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
            USART_Transmit(name[f]);
            g = 0;
            USART_Transmit(0x0d);
        }
    }
}


void USART_Init(){
    UBRR0H = (BAUD_CONST >> 8);
    UBRR0L = BAUD_CONST;
    UCSR0B |= (1<<RXEN0)|(1<<TXEN0);
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

    while(!(UCSR0A & (1<<UDRE0))) ;        // solange 1 in UDREn ist
    UDR0 = data;
    return(1);
}


unsigned char USART_Receive()
{
    if(flow_controll == 1 && (distance()  <= 3) )                    //Flowcontroll
    {
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
        temp = Head + (BUFFER_SIZE-1-Tail);
    }
    return (temp);
}
