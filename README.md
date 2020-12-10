Tapudisk is an unfinished, simple, alpha-quality project for the Arduino Mega, able to load C64 tap and d64 files.
It's a very basic IEC-only implementation (no fastloader support at all), and even then, not
a lot of testing has been done.

It includes code from  <a href="https://github.com/sweetlilmre/tapuino">tapuino</a>, but not all functionality is mapped to the UI.
The idea behind this project was to test a single architecture for accessing different C64 mods, as opposed to different, unrelated boards.

Obviously, the arduino hardware is not powerful enough to actually support much more, so the project was left at this point.

Libraries used:

Adafruit_GFX
MCUFRIEND_kbv

Pinout:
LCD_CS : Pin A3 
LCD_CD : Pin A2 
LCD_WR : Pin A1 
LCD_RD : Pin A0 
LCD_RESET A4 

SD card CS : 53

IEC Pins:

ATN : Pin 25 
CLOCK : Pin 27
DATA : Pin 23