/*
 * Assignment_Ver2.cpp
 *
 * Created: 1/17/2021 11:20:51 AM
 * Author : Kim Le
 */ 
                                                                                 
#include "virtuabotixRTC.h"                                                                           
#include <avr/io.h>    
#include <util/delay.h>                                                                                                    
//Set Pin Assignments
#define DS1302_SCLK_PIN   SCLK                // Serial Clock CLK Pin     
#define DS1302_IO_PIN     IO                  // the Data I/O DAT Pin             
#define DS1302_CE_PIN     C_E                 // the Chip Enable RST Pin             
                                                                                                        
//Conversion Macros                                                                              
#define bcd2bin(h,l)    (((h)*10) + (l))                                                           
#define bin2bcd_h(x)   ((x)/10)                                                                      
#define bin2bcd_l(x)    ((x)%10)                                                                        
                                                                                                         
//Set Register Names
#define DS1302_SECONDS           0x80                                                            
#define DS1302_MINUTES           0x82                                                          
#define DS1302_HOURS             0x84                                                              
#define DS1302_DATE              0x86                                                            
#define DS1302_MONTH             0x88                                                           
#define DS1302_DAY               0x8A                                                           
#define DS1302_YEAR              0x8C                                                             
#define DS1302_CLOCK_BURST       0xBE                                                            
#define DS1302_CLOCK_BURST_WRITE 0xBE                                                             
#define DS1302_CLOCK_BURST_READ  0xBF                                                               
#define DS1302_RAMSTART          0xC0                                                        
#define DS1302_RAMEND            0xFC                                                     
#define DS1302_RAM_BURST         0xFE                                                       
#define DS1302_RAM_BURST_WRITE   0xFE                                                          
#define DS1302_RAM_BURST_READ    0xFF                                                           
                                                                                                    
//Bit Defines
//  Defines for the bits, to be able to change between bit number and binary definition.               
#define DS1302_D0 0                                                                                        
#define DS1302_D1 1                                                                               
#define DS1302_D2 2                                                                              
#define DS1302_D3 3                                                                               
#define DS1302_D4 4                                                                                  
#define DS1302_D5 5                                                                                
#define DS1302_D6 6                                                                                 
#define DS1302_D7 7                                                                                    
                                                                                                      
//Random Bits
//  Bit for reading (bit in address)                                                                   
#define DS1302_READBIT DS1302_D0                         // READBIT=1: read instruction                 
                                                                                                  
//  Bit for clock (0) or ram (1) area, called R/C-bit (bit in address)                             
#define DS1302_RC DS1302_D6                                                                          
                                                                                                  
//  Seconds Register                                                                              
#define DS1302_CH DS1302_D7                              // 1 = Clock Halt, 0 = start              
                                                                                                 
//  Hour Register                                                                                
#define DS1302_AM_PM DS1302_D5                           // 0 = AM, 1 = PM                      
#define DS1302_12_24 DS1302_D7                           // 0 = 24 hour, 1 = 12 hour             
                                                                                                  
//  Enable Register                                                                                   
#define DS1302_WP DS1302_D7                              // 1 = Write Protect, 0 = enabled         
                                                                                                  
//  Trickle Register                                                                               
#define DS1302_ROUT0 DS1302_D0                                                                    
#define DS1302_ROUT1 DS1302_D1                                                                     
#define DS1302_DS0   DS1302_D2                                                                     
#define DS1302_DS1   DS1302_D2                                                                    
#define DS1302_TCS0  DS1302_D4                                                                    
#define DS1302_TCS1  DS1302_D5                                                                      
#define DS1302_TCS2  DS1302_D6                                                                     
#define DS1302_TCS3  DS1302_D7                                                                         
                                                                                                         
//  virtuabotixRTC Function                      
//  This is the CONSTRUCTOR of the class.  It sets the pins assignments for the component, as well as    
//  the Clock Enable and the trickle charge disablement.                                                
virtuabotixRTC::virtuabotixRTC(uint8_t inSCLK, uint8_t inIO, uint8_t inC_E)   {                      
  SCLK = inSCLK;                                                                                     
  IO = inIO;                                                                                          
  C_E = inC_E;                                                                                         
  DS1302_write (DS1302_ENABLE, 0);                         // Sets the Clock Enable to ON.              
  DS1302_write (DS1302_TRICKLE, 0x00);                     // Disable Trickle Charger.                 
}                  

//  ds1302_struct Structure
//  This is the structure of the rtc object. This assigns how many bits, and their location for each of 
//  the elements within the time data.                                                                 
// This struct sets the format for the Time fields on the DS1302
struct ds1302_struct  {                                                                              
  uint8_t Seconds:4;                                      // 4-bits to hold low decimal digits 0-9    
  uint8_t Seconds10:3;                                    // 3-bits to hold high decimal digit 0-5      
  uint8_t CH:1;                                           // 1-bit to hold CH = Clock Halt             
  uint8_t Minutes:4;                                      // 4-bits to hold low decimal digits 0-9      
  uint8_t Minutes10:3;                                    // 3-bits to hold high decimal digit 0-5       
  uint8_t reserved1:1;                                                                              
  union  {                                                                                             
    struct  {                                             // 24-hour section                          
      uint8_t Hour:4;                                     // 4-bits to hold low decimal digits 0-9  
      uint8_t Hour10:2;                                   // 2-bits to hold high decimal digits 0-2   
      uint8_t reserved2:1;                                                                             
      uint8_t hour_12_24:1;                               // 1-bit to set 0 for 24 hour format         
    } h24;                                                                                             
    struct  {                                             // 12 hour section                             
      uint8_t Hour:4;                                     // 4-bits to hold low decimal digits 0-9      
      uint8_t Hour10:1;                                   // 2-bits to hold high decimal digits 0-2      
      uint8_t AM_PM:1;                                    // 1-bit to set AM or PM, 0 = AM, 1 = PM      
      uint8_t reserved2:1;                                                                               
      uint8_t hour_12_24:1;                               // 1-bit to set 1 for 12 hour format           
    } h12;                                                                                             
  };                                                                                                     
  uint8_t Date:4;                                         // 4-bits to hold low decimal digits 0-9      
  uint8_t Date10:2;                                       // 2-bits to hold high decimal digits 0-3     
  uint8_t reserved3:2;                                                                                 
  uint8_t Month:4;                                        // 4-bits to hold low decimal digits 0-9       
  uint8_t Month10:1;                                      // 3-bits to hold high decimal digits 0-5     
  uint8_t reserved4:3;                                                                                  
  uint8_t Day:3;                                          // 3-bits to hold decimal digit 1-7           
  uint8_t reserved5:5;                                                                                  
  uint8_t Year:4;                                         // 4-bits to hold high decimal digit 20    
  uint8_t Year10:4;                                       // 4-bits to hold high decimal digit 14        
  uint8_t reserved6:7;                                                                                
  uint8_t WP:1;                                           // WP = Write Protect                   
};                   
// instantiation of the struct object
ds1302_struct rtc;     // Creates the Real Time Clock object

//  DS1302_clock_burst_read Function Begin
//  This function reads 8 bytes clock data in burst mode from the DS1302..                                                  
void virtuabotixRTC::DS1302_clock_burst_read( uint8_t *p)  {                                        
  int i;                                                                                          
  _DS1302_start();                                                                                 
                                                                                                       
// Instead of the address, the CLOCK_BURST_READ command is issued the I/O-line is released for the data  
  _DS1302_togglewrite( DS1302_CLOCK_BURST_READ, true);                                                  
                                                                                                        
  for( i=0; i<8; i++)  {                                                                                
    *p++ = _DS1302_toggleread();                                                                   
  }                                                                                                    
  _DS1302_stop();                                                                           
}                                     
                                                                    
//  DS1302_clock_burst_write Function Begin           
//  This function writes 8 bytes clock data in burst mode to the DS1302.                       
void virtuabotixRTC::DS1302_clock_burst_write( uint8_t *p)  { 
  int i;                                                                                            
  _DS1302_start();                                                                             
                                                                                              
// Instead of the address, the CLOCK_BURST_WRITE command is issued.  The I/O-line is not released   
  _DS1302_togglewrite( DS1302_CLOCK_BURST_WRITE, false);                                         
                                                                                                  
  for( i=0; i<8; i++)  {                                                                           
    // the I/O-line is not released                                                             
    _DS1302_togglewrite( *p++, false);                                                        
  }                                                                                              
  _DS1302_stop();                                                                                    
}

//  DS1302_read Function Begin
//  This function reads a byte from the DS1302 (clock or ram).                                          
uint8_t virtuabotixRTC::DS1302_read(int address)  {                                                  
  uint8_t data;                                                                                  
                                                                                                  
  // set lowest bit (read bit) in address                                                        
  bitSet( address, DS1302_READBIT);                                                               
                                                                                                  
  _DS1302_start();                                                                  
  // the I/O-line is released for the data                                                   
  _DS1302_togglewrite( address, true);                                                        
  data = _DS1302_toggleread();                                                                 
  _DS1302_stop();                                                                           
                                                                                               
  return (data);                                                                                        
}                                               
                                                                      
// DS1302_write Function                                                                      
void virtuabotixRTC::DS1302_write( int address, uint8_t data)  {                        
  // clear lowest bit (read bit) in address
  bitClear(address, DS1302_READBIT);                
  _DS1302_start();                                                                                
                                                                                         
  // don't release the I/O-line                                                                      
  _DS1302_togglewrite(address, false);                                                                
                                                                                                     
  // don't release the I/O-line                                                                       
  _DS1302_togglewrite( data, false);                                                                   
  _DS1302_stop();                                                                                     
}
 
// DS1302_start Function
// Since the DS1302 has pull-down resistors, the signals are low (inactive) until the DS1302 is used.
void virtuabotixRTC::_DS1302_start( void )  {
  PORTD &= ~(1 << DS1302_CE_PIN)|(1 << DS1302_SCLK_PIN);	// By default, PIN RST & CLK are disable     
  DDRD |= (1 << DS1302_CE_PIN)|(1 << DS1302_SCLK_PIN)|(1 << DS1302_IO_PIN) ; // Output PIND 5, 6, 7 for CLK, Data, RST Pin respectively 
  PORTD |= (1 << DS1302_CE_PIN);							// start the session
  _delay_ms(4)    ;											// tCC = 4ms
}

// DS1302_stop Function
// A helper function to finish the communication.
void virtuabotixRTC::_DS1302_stop( void )  {
  PORTD &= ~(1 << DS1302_CE_PIN); // Turn off PIN RST
  _delay_ms(4);                   // tCWH = 4ms
}

// DS1302_toggleread Function Begin
// A helper function for reading a byte with bit toggle.  This function assumes that the SCLK is still high.
uint8_t virtuabotixRTC::_DS1302_toggleread( void )  {
  uint8_t i, data;
  data = 0;

// Issue a clock pulse for the next databit.  If the 'togglewrite' function was used before this
// function, the SCLK is already high.
  for( i = 0; i <= 7; i++ )  {   
	PORTD |= (1 << DS1302_SCLK_PIN);
	_delay_ms(1);
	
    // Clock down, data is ready after some time.
    PORTD &=~ (1 << DS1302_SCLK_PIN);  
	_delay_ms(1);

    // read bit, and set it in place in 'data' variable
	bitWrite( data, i,  (PIND & (1<<DS1302_IO_PIN)));                                                                                              //|    |
  }
  return( data );
}

//DS1302_togglewrite Function Begin
// A helper function for writing a byte with bit toggle.  The 'release' parameter is for a read after   
// this write.  It will release the I/O-line and will keep the SCLK high.                               
void virtuabotixRTC::_DS1302_togglewrite( uint8_t data, uint8_t release)  {               
	int i;                                                                                       
                                                                                           
	for( i = 0; i <= 7; i++ )  {                                                                     
    // set a bit of the data on the I/O-line                                                                                                        
	if(bitRead(data, i))
	{
		PORTD |= (1 << DS1302_IO_PIN);
	}else PORTD &= ~(1 << DS1302_IO_PIN);
    _delay_ms(1);                                                                                   
    // clock up, data is read by DS1302                             
	PORTD |= (1 << DS1302_SCLK_PIN);
    _delay_ms(1);      
                                                                                                         
//  If this write is followed by a read, the I/O-line should be released after the last bit, before the  
//  clock line is made low.  This is according the datasheet. If I/O line would not be released, a shortcut spike on the I/O-line could happen.         
    if( release && i == 7 )  {                                                                           
	  DDRD &= ~(1 << DS1302_IO_PIN) ;                                                                
    }  else  {                                                                                           
	  PORTD &= ~(1 << DS1302_SCLK_PIN) ;                                                            
	   _delay_ms(1);
    }                                                                                                    
  }                                                                                                      
}                        
                                                                                                        
// setDS1302Time Function Begin 
// A function to set the initial time within the DS1302 Real Time Clock.                               
void virtuabotixRTC::setDS1302Time(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t dayofweek,  
                                   uint8_t dayofmonth, uint8_t month, int year)
{
  seconds    = seconds;                                                                       
  minutes    = minutes;                                                                      
  hours      = hours;                                                                       
  dayofweek  = dayofweek;                                                                        
  dayofmonth = dayofmonth;                                                                       
  month      = month;                                                                               
  year       = year;                                                                              
                                                                                                   
//This will fill the output array of the rtc object with ZEROs
  memset ((char *) &rtc, 0, sizeof(rtc));                                                                
//This contains the conversion to BCD, and assigns to the elements 
  rtc.Seconds         = bin2bcd_l( seconds);                                                            
  rtc.Seconds10       = bin2bcd_h( seconds);                                                             
  rtc.CH              = 0;                                       // 1 for Clock Halt, 0 to run           
  rtc.Minutes         = bin2bcd_l( minutes);                                                             
  rtc.Minutes10       = bin2bcd_h( minutes);                                                           
  rtc.h24.Hour        = bin2bcd_l( hours);                                                             
  rtc.h24.Hour10      = bin2bcd_h( hours);                                                               
  rtc.h24.hour_12_24  = 0;                                       // 0 for 24 hour format               
  rtc.Date            = bin2bcd_l( dayofmonth);                                                          
  rtc.Date10          = bin2bcd_h( dayofmonth);                                                         
  rtc.Month           = bin2bcd_l( month);                                                             
  rtc.Month10         = bin2bcd_h( month);                                                               
  rtc.Day             = dayofweek;                                                                       
  rtc.Year            = bin2bcd_l( year - 2000);                                                         
  rtc.Year10          = bin2bcd_h( year - 2000);                                                         
  rtc.WP              = 0;                                                                               
                                                                                                         
// Write all clock data at once (burst mode)
  DS1302_clock_burst_write((uint8_t *) &rtc);                                                          
}
                                                                    
// updateTime Function Begin                                     
//  A function that updates the time.  All other functions that access the time after the initial set    
//  will call this function for the actual updating time.  This is the only function that updates the    
//  time to the current data.                                                                            
void virtuabotixRTC::updateTime() {                                                                      
                                                                                                        
  DS1302_clock_burst_read( (uint8_t *) &rtc);               // Read all clock data at once (burst mode). 
                                                                                                        
  char buffer[80];                                          // the code uses 70 characters.             
  seconds     = ( rtc.Seconds10  * 10 ) + rtc. Seconds;                                                
  minutes     = ( rtc. Minutes10 * 10 ) + rtc.Minutes;                                               
  hours       = ( rtc.h24.Hour10 * 10 ) + rtc.h24.Hour;                                             
  dayofweek   = ( rtc.Day );                                                                          
  dayofmonth  = ( rtc.Date10  * 10 ) + rtc.Date;                                                         
  month       = ( rtc.Month10 * 10 ) + rtc.Month;                                                
  year        = ( rtc.Year10  * 10 ) + rtc.Year + 2000;                                           
}