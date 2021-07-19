/*
 * Atthor: Joshua DeNio
 * Date: 11/15/2019
 * Description:
 *  This program will collect water levels and temperature data to be sent to a Raspberry Pi and later displayed online.
 */

 #include <Wire.h>
 #include <rgb_lcd.h>
 #include <DallasTemperature.h>
 #include <OneWire.h>

 rgb_lcd lcd ;

// Pin constants **********************************************
// Note: pin 13 goes high when reset dont usse this pin

// high and low bobbers
const int inHigh =  3;
const int inLow  =  2;

// Ultrasonic sensor pins tide-pool side
const int tideTriggerPin = 6 ;
const int tideEchoPin = 7 ;

// Ultrasonic sensor pins ray tank side
const int rayTriggerPin = 12 ;
const int rayEchoPin = 13 ;

//Referance voltage pin
const int inRef = 8 ;

// LEDs to indicate on or off
const int highLED = 9 ;
const int lowLED = 10 ;

// Tempurature sensor pin
const int tempIn = 4 ;

// End of pin constants *******************************************

// Declarations

float raySensorHeight = 81.5 ;
float tideSensorHeight = 93 ;

float tankWidth = 122.5 ;
float tankLength = 367.00 ;

int printDelay = 3000;

const int debounceDelay = 100 ;
const int pulseDelay = 1000 ;
const int lcdDelay = 500 ;

// LED variables

boolean highTide = false ;
boolean lowTide = false ;

// Bools for filling and draining
boolean filling = false ;
boolean draining = false ;


// Wire buss for tempurature sensor
OneWire oneWire(tempIn) ;
DallasTemperature sensors(&oneWire) ;

// Sensor varialbles - Initially set all the switches LOW (not tripped)
int sensorHigh = LOW;
int sensorLow = LOW;
int sensorRef = LOW;

// Sensor reading variabls
int readingHighSensor;
int readingLowSensor;

//Functions -----------------------------------------------------------------

// Prints one string to the lcd
void printToLCD1(String str1)
{
  // Clear the LCD
  lcd.clear() ;
  
  //print to the LCD
  lcd.print(str1) ;
  delay(printDelay) ;
}

// Prints two strings to the LCD
void printToLCD2(String str1, String str2)
{
  //clear LCD
  lcd.clear();
  
  // print to the lcd
  lcd.print(str1);
  lcd.setCursor(0,1);
  lcd.print(str2);
  delay(printDelay);
}

//updates lcd with current state
void dumpState( String tideDepth, String rayDepth, String temp)
{
  printToLCD1("Sending Log");

  //print tide state to the lcd
  lcd.clear();
  showTideState();
  delay(printDelay);

  // Print tide pool depth to lcd
  lcd.clear();
  lcd.print("Tide Pool");
  lcd.setCursor(0,1);
  lcd.print("Depth: ");
  lcd.print(tideDepth);
  lcd.print(" cm");
  delay(printDelay);


  //print ray tank depth to lcd
  lcd.clear();
  lcd.print("Ray Tank");
  lcd.setCursor(0,1);
  lcd.print("Depth: ");
  lcd.print(rayDepth);
  lcd.print(" cm");
  delay(printDelay);

  // print temp to lcd
  lcd.clear();
  lcd.print("Temperature:");
  lcd.setCursor(0,1);
  lcd.print(temp+" C");
  delay(printDelay);
}

float getWaterDepth(int triggerPin, int echoPin, float sensorHeight)
{
  float duration;
  float distance;
  float waterDepth;
    
    // send ultrasonic pulse
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);
    
    // see how long pulse takes to return to echo pin
    duration = pulseIn(echoPin, HIGH, 50000);
  
  // Change time to get pulse back to centimeters
    //distance = ((duration / 2) / 29.1);
    distance = duration * 0.0343/0.8638/2;
    waterDepth = sensorHeight - distance;
    
  return waterDepth;
  }

float averageWaterDepth(int triggerPin, int echoPin, float sensorHeight)
{
  float distance1 = getWaterDepth(triggerPin, echoPin, sensorHeight);
  float distance2 = getWaterDepth(triggerPin, echoPin, sensorHeight);
  float distance3 = getWaterDepth(triggerPin, echoPin, sensorHeight);
  float lowestMeasure;

  if(distance1 >= distance2){
    lowestMeasure = distance1;
  }
  else{
    lowestMeasure = distance2;
  }
  if(distance3 > lowestMeasure){
    lowestMeasure = distance3;
  }
  return lowestMeasure;
}

//calculates the total water volume in liters.
String getTotalVolume(float tDepth, float rDepth)
{

  //calculate the total water volume
  float volume = tankLength*tankWidth*(tDepth+rDepth);

  //convert to Liters
  volume = volume/1000;
  
  return String(volume);
}

//gets the tide state.
String getTide()
{
  String tide = "NA";

  if (highTide == true)
  {
    tide = "High Tide";
  }
  else if (lowTide == true)
  {
    tide = "Low Tide";
  }
  else if (filling == true)
  {
    tide = "Incoming";
  }
  else if (draining == true)
  {
    tide = "Outgoing";
  }
  
  return tide;
}

//gets the temperature reading.
String getTemp()
{
  String tmp = "";

  sensors.requestTemperatures();
  tmp =String(sensors.getTempCByIndex(0));

  return tmp;
}

//create and send log files to PI
// and display temp, ray tank depth and tide pool depth
void sendLog()
{
  //String logStr1 = "LOG,22,9,500,outgoing,26";
  String logStr = "";
  String tideStr = getTide();
  float tideDepth = averageWaterDepth(tideTriggerPin,tideEchoPin, tideSensorHeight);
  float rayDepth = averageWaterDepth(rayTriggerPin,rayEchoPin, raySensorHeight);
  String temp = getTemp();

  logStr = "LOG," + String(tideDepth);
  logStr += "," + String(rayDepth);
  logStr += "," + getTotalVolume(tideDepth, rayDepth);
  logStr += "," + tideStr;
  logStr += "," + temp;

  Serial.println(logStr);
  dumpState( String(tideDepth), String(rayDepth), temp);
  }

// Prints the tide state to the LCD
void showTideState()
{
  lcd.clear();

  if (highTide == true)
  {
    lcd.setRGB(0,255,100);
    lcd.print("High Tide");
  }
  else if (lowTide == true)
  {
    lcd.setRGB(0,255,0);
    lcd.print("Low Tide");
  }
  else if(filling == true)
  {
    lcd.setRGB(0,255,50);
    lcd.print("Tide");
    lcd.setCursor(0,1);
    lcd.print("Incoming");
  }
  else if (draining == true)
  {
    lcd.setRGB(0,255,0);
    lcd.print("Tide");
    lcd.setCursor(0,1);
    lcd.print("Outgoing");
  }
  else
  {
    lcd.setRGB(0,255,0);
    lcd.print("Calculating");
    lcd.setCursor(0,1);
    lcd.print("Tide");
  }
  
  delay(1000);
}

// Sets highTide, lowTide, filling and draining
void checkTide()
{ 
  // Set inHigh if top float is triggered
  if (digitalRead( inHigh ) == LOW )
  {
    highTide = true;
    lowTide = false;
  }
  else
  {
    highTide = false;
  }

  // Set inLow if low float is triggered
  if (digitalRead( inLow ) == LOW )
  {
    lowTide = true;
    highTide = false;
  }
  else
  {
    lowTide = false;
  }

  // Using highTide, lowTide set filling and draining to the proper values  
  if (highTide == true && filling == true)
  {
    filling = false;
    draining = true; 
  }
  else if (lowTide == true && draining == true)
  {
    filling = true;
    draining = false;
  }
  else if (filling == false && draining == false)
  {
    if (highTide == true)
    {
      draining = true;
    }
    else if (lowTide == true)
    {
      filling = true;
    }
  }
  
}

// Setup **********************************
void setup()
{
  // Start serial connection so that we can print to the LCD screen for testing
  Serial.begin(115200);

  // set up the LCD's number of columns and rows
  lcd.begin(16, 2);
  // make lcd green
  lcd.setRGB(0, 255, 0);

  // Set the various digital input pins
  //pinMode( inHigh, INPUT );
  //pinMode( inLow, INPUT );
  pinMode( inHigh, INPUT_PULLUP );
  pinMode( inLow, INPUT_PULLUP );
  pinMode( inRef, INPUT );
  pinMode( tideEchoPin, INPUT );
  pinMode( rayEchoPin, INPUT );
  
  // Set the various digital output pins
  pinMode( highLED, OUTPUT );
  pinMode( lowLED, OUTPUT );
  pinMode( tideTriggerPin, OUTPUT );
  pinMode( rayTriggerPin, OUTPUT );

  sensors.begin();

}

// Main loop ***********************************
void loop()
{
  checkTide();
  showTideState();
  sendLog();
  delay(3000);
}
