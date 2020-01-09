## RTC_EEPROM - DS3231 and AT24C32 library<br>
C code to talk to the real-time clock and 4K EEPROM modules (they're usually
sold together)<br>
<br>
Written by Larry Bank<br>
Copyright (c) 2018 BitBank Software, Inc.<br>
Project started 1/5/2018<br>
<br>
bitbank@pobox.com<br>

This code re-invents the wheel that so many others have invented. There are a
ton of Arduino and C libraries which talk to these inexpensive boards. I wrote
this because I learn about IoT parts by reading the datasheet and writing my
own code. I didn't write code to support every possible option, but the majority
of the functionality is exposed by my functions. This will work fine with boards
that come with the AT24C32 EEPROM and those without.<br>

The Linux sample program now allows you to set or display the time from the
command line. It will use the local (not GMT) time+date when asked to set the
DS3231 time from the system time. This is especially handy to quickly set the
correct time and date on those little DS3231 breakouts designed for the Raspberry Pi.<br>

![DS3231](/rpi_ds3231.jpg?raw=true "DS3231 RPI breakout")

See the README file in the Arduino folder for instructions on using the library
with the Arduino IDE.

