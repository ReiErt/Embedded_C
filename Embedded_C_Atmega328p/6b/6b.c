 /*
 * Blatt6b.c
 *
 * Created: 18.10.2022 12:35:17
 * Author : Ertman
 */ 

#define F_CPU 16000000UL

#include <util/delay.h>
#include <avr/io.h>
#include "PWM.h"
#include "ADC.h"
#include "UART.h"

int main(void)
{	
	USART_Init();
	//set_duty_cycle_pwm0(read_adc(0));
	//set_duty_cycle_pwm1(read_adc(0));
	init_pwm0(249);	
	init_pwm1(249);	
	init_adc();	
	
	while(1)
	{		
		// the argument convert_adc_value_to_8_bit is moving, depending on pentiometer
		// convert_adc_value_to_8_bit itself is out duty cycle out of max
		// max = top = OCR0A = 249
		// 249 means we have 1KHz pulse wave. Since pre-scaler is 64
		set_duty_cycle_pwm0(convert_adc_value_to_8_bit(read_adc(1<<MUX2), 249));	
		//set_duty_cycle_pwm1(convert_adc_value_to_8_bit(read_adc(1<<MUX2), 249));
		// set_duty_cycle applies value to OCR0B
		

		
		//read Spannung from Pentometer at MUX2 => ADC4 = PORTC4
		USART_Transmit_number(to_mv(read_adc(1<<MUX2)));
		USART_Transmit('m');
		USART_Transmit('v');
		
		USART_Transmit(' ');
		
		char str[10];
		sprintf(str, "%dC\n", read_temp());
		
		for(uint8_t i = 0; i<10; i++) USART_Transmit(str[i]);

		char duty[10];
		uint8_t duty_temp = OCR0B;
		sprintf(duty, " %d duty cycle \n", duty_temp);
		for(uint8_t i = 0; i<10; i++) USART_Transmit(duty[i]);
		
		_delay_ms(1000);
		
			
}
}




//  -------------- 6b -- Aufgabe 4 -------------------------

uint8_t global_duty_cycle_calc_flag = 0; // is set when we should calculate
uint16_t global_old_ICP1_counter = 0;
uint16_t global_new_ICP1_counter = 0;

input_capture_init(){
	sei();
	// note to me: physically connect ausgang PWM to ICP1 input pin on arudino
	
	//Input Capture nur an timer1
	TCCR1B |= (1<<CS11); // pre-scaler auf 64 = 250KHz
	TCCR1B |= (1<<ICES1);   // rising edge is trigger
}

//ICP capture Interrupt
ISR(TIMER1_CAPT_vect){
	static int8_t count = 0;
	 global_old_ICP1_counter = global_new_ICP1_counter;
	// global_new_ICP1_counter = (ICR1L | (ICR1H << 8); //read value over 2 registers
	 global_new_ICP1_counter = (ICR1H << 8);
	 global_new_ICP1_counter = ICR1L;
	 
	
	TCCR1B ^= (1<<ICES1); // toggle 
	if(count == 1)
	{
		global_duty_cycle_calc_flag = 1;
	}
	count++;
	count %= 2;
	
	
	//this goes in PWM.h
	//calculates the percent of PWM
	if(global_duty_cycle_calc_flag = 1){
		
		global_old_ICP1_counter = global_new_ICP1_counter - global_old_ICP1_counter;
		global_duty_cycle_calc_flag = 0;
		
		//multiply number * 64 to find the clock ticks.
		uint32_t temp = global_old_ICP1_counter*64;
		// 16MHz = 63ns
		
		// multiply 63ns * old to get the length of duty cycle in time
		temp = temp * 63;
		
		// Duty Cycle is t(on) / Period
		// t(on) / 1KHZ = percent
		uint32_t global_final_result = temp / 1000;
	}
				
	
}
		// print to terminal from main
		char duty_char[30];
		sprintf(duty_char, " %d duty cycle found on pin \n", final_result);
		for(uint8_t i = 0; i<10; i++) USART_Transmit(duty_char[i]);