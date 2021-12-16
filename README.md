# OLED_GPS_Clock

Simple GPS Clock.

Uses serial GPS receiver - anything that sends the basic NMEA GPGGA message will do.  9600 is assumed; change if needed.

Uses a SH110X OLED 128x64 pixels. 

Uses the Adafruit GFX and SH110X OLED libraries.

Developed on an AVR 32U4 board, for which you should set up in the Arduino IDE as "Lilypad Arduino USB".  Should work on pretty much anything that has serial and I2C.

Connections as follows; check your particular board for the Rx pin (should be labelled) and I2C SDA/SCL pins (probably won't be):

GND from AVR to GPS and OLED
3V3 from AVR to GPS and OLED
SDA from AVR to OLED
SCL from AVR to OLED
Rx from AVR to GPS Tx
