# SoilMoistureSensor
This script measures the moisture content in the soil and tracks the days since the soil was "wet".

A watering event is registered every time the soil is "Wet". 
 
The date of the watering event is then stored in the DS1302 clock module and you can even turn the Arduino off and power it on when you want to check on the plant. The time difference is calculted using https://en.wikipedia.org/wiki/Julian_day

I recommend conducting your own tests to get the correct values [0,...,255] for your wet soil and dry soil. These values appear to vary between soils and sensors and depend on Vin (5V or 3.3V).

What's needed:

- DS1302 Real time clock module
- 0.91 inch OLED display
- any Arduino with sufficient storage space (168p has too little space available!)

Wiring Diagram:

![alt text](https://github.com/LeoBay/SoilMoistureSensor/blob/master/Wiring%20Diagram.PNG)

You can find a 3D-printable housing here:

https://www.thingiverse.com/thing:3857846
