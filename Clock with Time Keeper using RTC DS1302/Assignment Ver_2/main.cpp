/*
 * Assignment_Ver2.cpp
 *
 * Created: 1/17/2021 11:20:51 AM
 * Author : Kim Le
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include "virtuabotixRTC.h"
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include "liquid_crystal_i2c.c"
#include "i2c_master.c"

// Define state for FSM
enum clock_state {SET_TIME_KEEPER_MODE, IDLE_MODE};
enum time_state {HOUR, MINUTE};
enum clock_state state;
enum time_state time;

int main(void)
{
	DDRB |= (1<<PORTB0);
	
	i2c_master_init(I2C_SCL_FREQUENCY_100);
	LiquidCrystalDevice_t device = lq_init(0x3f, 16, 2, LCD_5x8DOTS); // intialize 4-lines display
	lq_turnOnBacklight(&device); // simply turning on the backlight
	virtuabotixRTC myRTC(5, 6, 7); //PinD 5, 6, 7 are used for CLK, Data, RST respectively
	
	char snum[5];
	char mnum[5];
	char hnum[5];
	state = IDLE_MODE;
	while(1) {
		// Check Button PINB 5 state low to switch from IDLE mode to Set time keeper mode
		// The system is designed to change state only one
		// When the button is pressed (state low), write time to DS1302 RTC module
		if(!(PINB & (1<<PINB5)) && state == IDLE_MODE){
			//myRTC.setDS1302Time(55, 29, 13, 5, 23, 1, 2021); // Write Time/Date to the DS1302 RTC module
			_delay_ms(200);
			state = SET_TIME_KEEPER_MODE;
		// Button cannot switch back to idle mode after change to set time keeper mode
		} else if(!(PINB & (1<<PINB5)) && state == SET_TIME_KEEPER_MODE){
			_delay_ms(200);
			state = SET_TIME_KEEPER_MODE;
		}
		switch(state)
		{
			case SET_TIME_KEEPER_MODE:
				lq_clear(&device);
				myRTC.updateTime();			// Update time from DS1302 to LCD display constantly
				lq_setCursor(&device,0,0); // moving cursor to the next line
				lq_print(&device, "Time Clock");
				lq_setCursor(&device,1,0); // moving cursor to the next line
				lq_print(&device, itoa(myRTC.hours,hnum,10));	// Display Hour
				lq_print(&device, ":");
				lq_print(&device, itoa(myRTC.minutes,mnum,10));	// Display minute
				lq_print(&device, ":");
				lq_print(&device, itoa(myRTC.seconds,snum,10));	// Display sec
				_delay_ms(1000);
				break;
			// Display welcome message in idel mode
			case IDLE_MODE:
				lq_setCursor(&device,0,0); // moving cursor to the next line
				lq_print(&device, "Hello User!");
				lq_setCursor(&device,1,0); // moving cursor to the next line
				lq_print(&device, "Welcome To Clock");
				break;
		}
		
		
	}
}
