# Digital-Clock-with-AVR-Microcontroller
There are 2 different project in this repository which are both about configure a digital clock using Arduino (ATmega328p) Microcontroller.
- 1st is the basic clock that display time with 4-digit 7-segment LED that will show us the hour and minutes.
Initially, I configure the 4 digit (1 – 4) of the LEDs display through Arduino Port C (Pin 0 - 3) respectively. Then, setup:
•	Button connect to Port B pin 0 to decrease time in set mode.
•	Button connect to Port B pin 1 to increase time in set mode.
•	Button connect to Port B pin 3 to switch between state of Minute and Hour of Set mode.
•	Button connect to Port B pin 5 to switch between the two state set mode and time mode.
•	LED connect to port B pin 4. 
The initial state is Set mode, we can switch back and forth between Time mode and Set mode by press button connect to port B Pin 5. 
The LED/Buzzer connect to Port B Pin 4 goes on every 2 mins and last for 5s before goes off in Time mode only.

- 2nd is the digital clock that display time using 16x2 LCD and I2C. The clock is able to keep updating the time with realtime clock using RTC DS1302 Module.
I configure the SCL and SDA pins of the I2C through port C Pin 5 and 4 respectively. 
When the button connect to port B pin 5 is pressed, the state will change from idle mode to set time keeper mode and the pre – defined time will be written to the RTC DS1302 module through MCU.
In order to communicate between the DS1302 and the MCU, I configure the port D pin 5, 6, 7 and connect them to pin CLK (serial clock), Data (I/O), and RST (RAM - CE) respectively. 
To write data to RTC module and read data from it for the first time, we used burst mode which write all information data such as hour, minute and second in 31 bytes (Maximum data store in RAM) and keep time by reading data (transfer) from the RTC Ram 1 byte at a time to update the clock time. 
When data is read or write in either single or burst data, the CE pin will be put as high. 
