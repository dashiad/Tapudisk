
Tapudisk is an unfinished, simple, alpha-quality project for the Arduino Mega, able to load C64 tap and d64 files.
It's a very basic IEC-only implementation (no fastloader support at all), and even then, not
a lot of testing has been done.

It includes code from  <a href="https://github.com/sweetlilmre/tapuino">tapuino</a>, but not all functionality is mapped to the UI.
The idea behind this project was to test a single architecture for accessing different C64 mods, as opposed to different, unrelated boards.

Obviously, the arduino hardware is not powerful enough to actually support much more, so the project was left at this point.

The screen used is an ILI9341. My pin setup is a bit wierd, because my tests were done using an Arduino Uno TFT shield. Using an arduino mega shield
like  <a href="https://es.aliexpress.com/item/4000056270428.html?spm=a2g0o.detail.1000014.11.4a267932J214IO&gps-id=pcDetailBottomMoreOtherSeller&scm=1007.14976.200453.0&scm_id=1007.14976.200453.0&scm-url=1007.14976.200453.0&pvid=08316ec4-b06d-4155-ba14-64698dda2bc9&_t=gps-id:pcDetailBottomMoreOtherSeller,scm-url:1007.14976.200453.0,pvid:08316ec4-b06d-4155-ba14-64698dda2bc9,tpp_buckets:668%230%23131923%2323_668%23808%233772%23903_668%23888%233325%2314_4976%230%23200453%2313_4976%232711%237538%23490_4976%233104%239653%236_4976%234052%2318550%236_4976%233141%239888%2310_668%232846%238115%23832_668%232717%237564%23683_668%231000022185%231000066058%230_668%233422%2315392%23273_4452%230%23194218%230_4452%233474%2317029%23810_4452%233098%239599%23392_4452%233564%2316062%2389">this</a>, 
would be better. Then, pin assigments should be updated in Tapudisk.ino

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