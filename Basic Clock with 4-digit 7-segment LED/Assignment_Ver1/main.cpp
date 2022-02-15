/*
 * Assignment_Ver1.cpp
 *
 * Created: 1/17/2021 11:20:51 AM
 * Author : Kim Le
 */ 


#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define SegOne   0x01
#define SegTwo   0x02
#define SegThree 0x04
#define SegFour  0x08

// Global array which define patten for digit 0 - 9 in 7 segment LED
char seg_code_d[]={0x80, 0xF1, 0x44, 0x60, 0x31, 0x22, 0x02, 0xF0, 0x00, 0x20};
volatile int i = 0, j = 0, counting = 0, count = 0, flag = 0, hour_min = 0, countAlarm = 0;
volatile int cntMin = 0, cntMin10 = 0, cntHr = 0, cntHr10 =0;

// Define State for FSM
enum clock_state {TIME_MODE, SET_MODE};
enum time_state {HOUR, MINUTE};
enum clock_state state;
enum time_state time;

void Button_SetTime(){
	TCCR1B |= (1 << CS12 ); // Set the prescaler of 256
	TCNT0 = 0x00;			// Initial timer counter 0 
}

int main (void){
	
	// Set up Output port D, B, and C to control the 4-digit 7-segment LEDs
	DDRD = 0xF7;
	DDRC = 0x3F;
	DDRB |= (1<<PORTB4);
	
	DDRB &= ~(1<<PORTB0)|(1<<PORTB1)|(1<<PORTB5)|(1<<PORTB3); // Input signal from button to change time in Set mode

	// Set up Timer prescaler 256
	Button_SetTime();
	
	// Initial state is set mode
	state = SET_MODE;
	// Initial state in set mode is set minute
	time = MINUTE;
	while(1)
	{
		// Check button PINB5 state low to change the state of FSM 
		if(!(PINB & (1<<PINB5)) && state == SET_MODE){
			_delay_ms(200);		// delay 200 ms for debounce
			state = TIME_MODE;
		}
		else if(!(PINB & (1<<PINB5)) && state == TIME_MODE){
			_delay_ms(200);
			state = SET_MODE;
			time = MINUTE;
		}
		// If state is Set mode, check button PINB3 state low to switch between Hour and Minute
		if(state == SET_MODE){
			if(!(PINB & (1<<PINB3)) && time == MINUTE){
				_delay_ms(200);
				time = HOUR;
			}
			else if(!(PINB & (1<<PINB3)) && time == HOUR){
				_delay_ms(200);
				time = MINUTE;
			}
		}
		switch(state)
		{
			// Time mode
			case TIME_MODE:
			// Timer counter frequency 0.5Hz
			if (TCNT1 >= 62499)
			{
				counting++;
				countAlarm++;
				TCNT1 = 0;
			}
			// Every 60s, The minute is increased by 1 and second reset to 0
			if (counting > 59)
			{
				countAlarm = 0;
				counting = 0;
				count++;
				i++;
			}
			// Every 60 min, the hour is increased by 1 and minute reset to 0
			if (i > 59)
			{
				i = 0;
				j++;
			}
			// Every 24 hour, the hour is reset to 0
			if (j > 23)
			{
				j = 0;
			}
			// Buzzer(alarm)/ LED turn on every 2 min
			if(count == 2){
				PORTB |= (1<<PORTB4);
				count = 0;
			} 
			// Buzzer(alarm)/ LED turns off after 5s
			if (countAlarm == 5)
			{
				PORTB &= ~(1<<PORTB4);
			}
			break;
			
			// Set mode
			case SET_MODE:
			
			// Buzzer/LED always off in this mode
			PORTB &= ~(1<<PORTB4);
			
			// FSM to change the hour and minute increment and decrement
			switch(time)
			{
				// State Set hour
				case HOUR:
				// Check PINB1 state low for increment
				if (!(PINB & (1<<PINB1)))
				{
					_delay_ms(200);
					j++;
					if(j > 23){
						j = 0;
					}
				}
				// Check PINB0 state low for decrement
				else if (!(PINB & (1<<PINB0)))
				{
					_delay_ms(200);
					j--;
					if(j < 0){
						j = 23;
					}
				}
				break;
				// State Set minute
				case MINUTE:
				if (!(PINB & (1<<PINB1)))
				{
					_delay_ms(200);
					i++;
					if(i > 59){
						i = 0;
					}
				}
				else if (!(PINB & (1<<PINB0)))
				{
					_delay_ms(200);
					i--;
					if(i < 0){
						i = 59;
					}
				}
				break;
			}
			break;	
		}
		// 4-digit are connect from Digit 1 - 4 to port C Pin 3 - 0 respectively
		// Hour is defined by j which increase every 60 counts in minute
		cntHr = j;
		cntHr10 = cntHr / 10;		// The tens are calculated by division of integer
		PORTC = SegOne;
		PORTD = seg_code_d[cntHr10];
		_delay_ms(1);
		
		cntHr10 = cntHr % 10;		// The units are calculated by modulo
		PORTC = SegTwo;
		PORTD = seg_code_d[cntHr10];
		_delay_ms(1);
		
		// Minute is defined by i which increase every 60 counts in second
		cntMin = i;
		cntMin10 = cntMin / 10;
		PORTC = SegThree;
		PORTD = seg_code_d[cntMin10];
		_delay_ms(1);
		
		cntMin10 = cntMin % 10;
		PORTC = SegFour;
		PORTD = seg_code_d[cntMin10];
		_delay_ms(1);
	}
}