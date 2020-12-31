/*********************************************************************
 * Adapted from GadgetReboot's script by Leo Rauschenberger
 * adapted from: https://github.com/GadgetReboot/Arduino/blob/master/Uno/SSD1306_Soil_Meter/SSD1306_Soil_Meter.ino
 * requires OLED display and Ds1302 clock module
 * This example requires the SSD1306 display driver by Adafruit: https://github.com/adafruit/Adafruit_SSD1306
 * Video detailing the capacitive soil moisture measurement probe: https://www.youtube.com/watch?v=a-pqcmCr79I
 * Connections:
 * DS1302:  
   CE        -> Arduino D2
   I/O       -> Arduino D3
   SCLK      -> Arduino D4
 * OLED (I2C protocol)
   SDA       -> Arduino A4
   SCL       -> Arduino A5
   VCC       -> 3.3 V to 5 V
   GND       -> GND
 * Soil Moisture Sensor
   AOUT       -> Arduino A0
   VCC        -> 3.3 V to 5 V (note: Calibration may depend on VCC)
   GND        -> GND
   
*********************************************************************/

#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <DS1302.h>
DS1302_RAM ramBuffer;

// Init the DS1302  (2,3,4 are the pin definitions)
DS1302 rtc(2, 3, 4);

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

const int wetProbe      = 330;   // wet readings are around 1.5v or analog input value ~300
const int dryProbe      = 610;   // dry readings are around   3v or analog input value ~620

// on 3.3 V
// Values for blue sensor:
// Sensor 1: 330 - 610
// Sensor 2: 300 - 575


// Analog Sensor Input Pin
int sensorInput         = 0;    // soil probe is on analog input 0

// Variables
int validSensorReading  = 0;    // valid sensor analog reading to record
int sensorReading;              // new sensor reading to evaluate and record or discard
int sensorResult;               // scaled sensor data [0..4] = [wet, damp, moist, dry]
int sensorPercent;              // in Percent

int previousSensorResult = 5;
int RAMDOW;
String waterDOW;
int DaysPassed = 0;

long JulianDate(int Y, int M, int D){
  long centuries = Y/100;
  long leaps = centuries/4;                             
  long leapDays = 2 - centuries + leaps;     // note is negative!!
  long yearDays = 365.25 * (Y + 4716);       // days until 1 jan this year
  long monthDays = 30.6001* (M + 1);         // days until 1st month
  long jd = leapDays + D + monthDays + yearDays -1524.5;
  return jd;
}


//----------------------------------------------------------------------------------------------
void setup()   {

  //Serial.begin(9600);           // some debug messages will display sensor raw data

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C address of the display
  display.clearDisplay();                     // clear the display buffer

  // Set the clock to run-mode, and disable the write protection
  rtc.halt(false);
  rtc.writeProtect(false);
  
  // Use lines to set the DS1302, then reupload sketch with the lines commented out!!
  //rtc.setDOW(SATURDAY);        // Set Day-of-Week to e.g. SATURDAY
  //rtc.setTime(12, 00, 0);      // Set the time to e.g. 12:00:00 (24hr format)
  //rtc.setDate(8, 6, 2020);     // Set the date to e.g. August 6th, 2020 

  // If necessary, clear the RAM. This will fill the RAM with zeros. Must be commented out in final upload!!
  //for (int i=0; i<31; i++) ramBuffer.cell[i]=0;
  //rtc.writeBuffer(ramBuffer); 
}

//--------------------------------------------------------------------------------------------------------------
void loop() {
  sensorReading = analogRead(sensorInput);
  if (abs(validSensorReading - sensorReading) > 10) {
    validSensorReading = sensorReading;
  }

  //Serial.print ("Old Sensor Reading: ");
  //Serial.println (validSensorReading);
  //Serial.print ("New Sensor Reading: ");
  //Serial.println (sensorReading);

  sensorResult = map(validSensorReading, wetProbe, dryProbe, 0, 4);  // scale analog input to a smaller range for wet to dry

  //Serial.print ("Scaled Sensor Reading 0-4: ");
  //Serial.println (sensorResult);
  
  sensorPercent = map(validSensorReading, dryProbe, wetProbe, 0, 100);  // scale analog input to a smaller range for wet to dry
  //Serial.print ("Percentage: ");
  //Serial.print (sensorPercent);
  //Serial.print (" %");
  //Serial.println ();

  // Clock data
  //Serial.println(rtc.getTimeStr());
  //Serial.println(rtc.getDOWStr(FORMAT_SHORT));
  //Serial.println(rtc.getDateStr());
  

  // display the correct soil moisture level on the display
  // lower voltages represent more wet levels
  switch (sensorResult) {
    case 0:
      displayTextProbe("Wet");
      break;
    case 1:
      displayTextProbe("Damp");
      break;
    case 2:
      displayTextProbe("Moist");
      break;
    case 3:
      displayTextProbe("Dry");
      break;
    case 4:    // same as case 3, due to how map works.
      displayTextProbe("Dry");
      break;
  }

   // Check what's written in the RTC Ram at position 0 (0x0 if not written to yet)
  RAMDOW = rtc.peek(0);

  // Check for watering event (change from non-wet to wet)
  if (sensorResult == 0 & previousSensorResult != 0){
    //waterDOW = rtc.getDOWStr(FORMAT_SHORT);
    waterDOW  = "WATER!";
    // next time, the new sensorResult will be compared to the one in this loop
    previousSensorResult = 0;
    // set position 0 to value of day of the week 1 Monday to 7 for Sunday
    rtc.poke(0,rtc.getTime().dow);
    // set position 1 to value of year (2019 can't be saved so instead 19 will be
    rtc.poke(1,rtc.getTime().year-2000);
     // set position 2 to value of month
    rtc.poke(2,rtc.getTime().mon);
     // set position 3 to value of day of month
    rtc.poke(3,rtc.getTime().date);
  }

  // If the sensor hasn't recorded a wet result yet, the day of watering is not avaiable
  else if (sensorResult != 0 & RAMDOW == 0 & previousSensorResult == 5){
    waterDOW = "NEVER";
    DaysPassed = 1000;
  }

  //in any other event 
  else if (RAMDOW != 0){
    previousSensorResult = sensorResult;
    // get position 1 - value of year
    int Yram = rtc.peek(1)+2000;
     // get position 2 - value of month
    int Mram = rtc.peek(2);
     // get position 3 - value of day of month
    int Dram = rtc.peek(3);
    // 
    int Ytoday = rtc.getTime().year;
    int Mtoday = rtc.getTime().mon;
    int Dtoday = rtc.getTime().date;
    DaysPassed = JulianDate(Ytoday, Mtoday, Dtoday) - JulianDate(Yram, Mram, Dram);
    switch(RAMDOW){
      case 1:
      waterDOW = "MON";
      break;
      case 2:
      waterDOW = "TUE";
      break;
      case 3:
      waterDOW = "WEN";
      break;
      case 4:
      waterDOW = "THU";
      break;
      case 5:
      waterDOW = "FR";
      break;
      case 6:
      waterDOW = "SAT";
      break;
      case 7:
      waterDOW = "SUN";
      break;
    }
  }
  delay(100);
}

// sensorDisplay is a new variable which contains the text handed over from the sensorResult

void displayTextProbe(const char * sensorDisplay) {
    display.setTextSize(2.2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.clearDisplay();
    display.println(sensorDisplay);
    display.setTextSize(4);
    display.setCursor(80, 5);
    display.print(sensorPercent);
    display.setTextSize(2.2);
    display.setCursor(0, 17);
    //display.println(waterDOW);
    display.println(DaysPassed);
    display.display();
    delay(100);
}
