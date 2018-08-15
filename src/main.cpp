
/*
  YABC-Yet-Another-Blynk-Controller-Extended
  Copyright (c) 2018 Leonardo Bispo.  All right reserved.
  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <FS.h>                   //this needs to be first, or it all crashes and burns...

//#define BLYNK_DEBUG               /* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial        /* Comment this out to disable prints and save space */

#include <Arduino.h>              
#include <ArduinoJson.h>          //Save Wifi Credentials, blynk auth, etc....
#include <ESP8266WiFi.h>          //Blynk and Wifi stuff, must have
#include <BlynkSimpleEsp8266.h>   //Blynk and Wifi stuff, must have
//For OTA
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
//Wifi manager portal library
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <untrol_WiFiManager.h>        //adapted Library https://github.com/ldab/untrol_WiFiManager
//For I2C and temperature sensors
#include <Adafruit_BME280.h>
#include <SparkFunHTU21D.h>
#include <WEMOS_SHT3X.h>
#include <Adafruit_AM2315.h>
#include <Adafruit_BMP085.h>
#include <Wire.h>                 //I2C
#include <OneWire.h>              //DS18B20
#include <DallasTemperature.h>    //DS18B20

//*************************FUNCTIONS TO CONTROL LED AND COLLOR*********************////
#include <Adafruit_NeoPixel.h>    // Library: https://github.com/adafruit/Adafruit_NeoPixel

#define BOARD_LED_PIN     13            // Set if your LED is WS2812 RGB
#define LED_BRIGHTNESS    32            // 0..255 brightness control

#define DIMM(x)    (((x)*(LED_BRIGHTNESS))/255)
#define RGB(r,g,b) (DIMM(r) << 16 | DIMM(g) << 8 | DIMM(b) << 0)

enum Colors {
  COLOR_BLACK   = RGB(0x00, 0x00, 0x00),
  COLOR_WHITE   = RGB(0xFF, 0xFF, 0xE7),
  COLOR_BLUE    = RGB(0x0D, 0x36, 0xFF),
  COLOR_BLYNK   = RGB(0x2E, 0xFF, 0xB9),
  COLOR_RED     = RGB(0xFF, 0x10, 0x08),
  COLOR_MAGENTA = RGB(0xA7, 0x00, 0xFF),
};

Adafruit_NeoPixel rgb = Adafruit_NeoPixel(1, BOARD_LED_PIN, NEO_GRB + NEO_KHZ800);

uint32_t colorToBlink = COLOR_BLACK;
//*************************************************************************************////

#define BOARD_BUTTON_PIN  0             // Pin where user button is attached
#define SDA_PIN           12
#define SCL_PIN           14
#define RLY1_PIN          4
#define RLY2_PIN          5
#define RLY3_PIN          15

#define DEC_PL            1             //decimal places for String conversion'

//timerID names
int timerTOread;
int timerTOcontrol;
int timerTOsend;
int timerToLED;

String  EmailSubject  = "🚨 {DEVICE_NAME} ALARM";

//*************************************USER CONFIGURATION THAT CAN BE CHANGED***********************************************//
float SetPoint            = 20;          //temperature set-point
float Differential        = 2;           //temperature control differential
bool  HEATING_RLY1        = true;             //Heating assigned to relay 1
bool  COOLING_RLY2        = false;             //Cooling assigned to relay 2
#define SENDFREQ           5130          //frequency to send data to Blynk. Keep it closer to 1000msec.
#define TIMETOWIFIPORTAL   3000          //time in milliseconds to activate portal
//**************************************************************************************************************************//

float temperature  = NAN;
float humidity     = NAN;
float lowAlarm     = NAN;
float highAlarm    = NAN;
int   int_probe_type = 0;            //convert char to int to use Switch case
int   button_pressed = 0;            //virtual button variable

uint16_t  readfrequency;             //frequency to read probes, different for each type
uint16_t  buttonpressed=0;

char blynk_token[34]  = "";
char probe_type[1]    = "";

bool      shouldSaveConfig = false;      //flag for saving data
bool      LEDflick         = false;      //flag for blink physical led
bool      tempAlarmFlag    = false;      //flag to alarm, and avoid multiple Notification
bool      faultyProbeFlag  = false;      //flag to alarm, and avoid multiple Notification
bool      rly1On           = false;
bool      rly2On           = false;
bool      rly3On           = false;

BlynkTimer timer;                  //Up To 16 Timers
WidgetLED led1(V21);
WidgetLED led2(V22);
WidgetLED led3(V23);

//Create an instance of the object
WiFiManager wifiManager;
Adafruit_BME280 bme;
HTU21D myHumidity;
SHT3X sht30(0x44);                      //alternativelly 0x45;
Adafruit_AM2315 am2315;
Adafruit_BMP085 bmp;
OneWire oneWire(SCL_PIN);               // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire);    // Pass our oneWire reference to Dallas Temperature.

BLYNK_CONNECTED() {
  Blynk.syncAll();  //sync all values from App when hardware connects;

  if (digitalRead(RLY1_PIN) == LOW) led1.off(); //may not need it
  else led1.on();
  if (digitalRead(RLY2_PIN) == LOW) led2.off(); //may not need it
  else led2.on();
  if (digitalRead(RLY3_PIN) == LOW) led3.off(); //may not need it
  else led3.on();
}
BLYNK_WRITE(V11){
  //reads the setppoint
  SetPoint = param.asFloat();
}
BLYNK_WRITE(V12){
  //reads the differential
  Differential = param.asFloat();
}
BLYNK_WRITE(V32){
  //reads low alarm
  lowAlarm = param.asFloat();
}
BLYNK_WRITE(V33){
  //reads high alarm
  highAlarm = param.asFloat();
}
BLYNK_WRITE(V40){
  //receive button state from Blynk App
  button_pressed = param.asInt();
}

void setRGB(uint32_t color) {
  rgb.setPixelColor(0, color);
  rgb.show();
}

void initLED() {
  rgb.begin();
  setRGB(COLOR_BLACK);
}

void blinkLED(){
    if (LEDflick) {
      setRGB(COLOR_BLACK);
      LEDflick = !LEDflick;
    }
    else {
      setRGB(colorToBlink);
      LEDflick = !LEDflick;
    }
}

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void DS18B20gettemp(){
  float temp_temp = sensors.getTempCByIndex(0);
  if      (temp_temp == 85) return;             //make sure it doesn't pick up the Error
  else if (temp_temp < -50) temperature = NAN;  //NAN if probe is disconnected.
  else     temperature = temp_temp;             //else all good.

  timer.enable(timerTOcontrol);
}

void readProbe()
{
  switch (int_probe_type)
  {
    case 1:{
      //Serial.println("Read BME280");
      //bme.takeForcedMeasurement(); // has no effect in normal mode
      temperature = bme.readTemperature();
      humidity    = bme.readHumidity();
      if ((!temperature && !humidity) || temperature<-100){
        temperature = NAN;
        humidity    = NAN;
      }
      break;}

    case 2:{
      //Serial.println("Read HTU21D");
      temperature = myHumidity.readTemperature();
      humidity    = myHumidity.readHumidity();
      if (temperature > 300) temperature = NAN;
      if (humidity    > 300) humidity    = NAN;
      break;}

    case 3:{
      //Serial.println("Read SHT30");
      if(sht30.get()==0){
          temperature = sht30.cTemp;
          humidity    = sht30.humidity;
        }
        else {
          Serial.println("Could not Read the probe");
          temperature = NAN;
          humidity    = NAN;
        }
      break;}

    case 4:{
      //Serial.println("Read AM2315");
      //temperature = am2315.readTemperature();
      //humidity    = am2315.readHumidity();
      // * readingings (the AM2315 manual advisess that continuous samples must be at least 2 seconds apart)
      // * Calling this method avoids the double I2C request.
      am2315.readTemperatureAndHumidity(temperature, humidity);
    break;}

    case 5:{
      //Serial.println("Read DS18B20");
      sensors.setWaitForConversion(false);
      sensors.requestTemperatures();
      //DS18B20 requires some time to measure temp, Blynk doesn't like waiting for it.
      timer.setTimeout(800, DS18B20gettemp);       //do it once after 800msec;
    break;}

    case 6:{
      //this one crashes a lot when disconnected.
      //Serial.println("Read BMP180");
      temperature = bmp.readTemperature();
      timer.enable(timerTOcontrol);   //temp degC
    break;}

    default:{
      //Serial.println ("no probe selected read loop");
      temperature = NAN;
      humidity    = NAN;
    break;}
  }
  //Serial.print("Temperature: ");
  //Serial.println(temperature);
  //Serial.print("Humidity: ");
  //Serial.println(humidity);
}

void button_change(void)
{
  colorToBlink = COLOR_BLUE;
  WiFiManagerParameter custom_probe_type("probetype", 1, HTML_DROPDOWN);
  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 34);

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
  wifiManager.addParameter(&custom_probe_type);
  wifiManager.addParameter(&custom_blynk_token);

  //reset settings - for testing
  //wifiManager.resetSettings();

  //sets timeout until configuration portal gets turned off in seconds
  wifiManager.setTimeout(120);

  if (!wifiManager.startConfigPortal(Host_Name.c_str())) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  strcpy(probe_type, custom_probe_type.getValue());
  strcpy(blynk_token, custom_blynk_token.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["probe_type"] = probe_type;
    json["blynk_token"] = blynk_token;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();

    delay(500);
    ESP.restart();
  }//end save
}

void SendStuff()   // This function sends every X second to Virtual Pin
{
  Blynk.virtualWrite(V1, String(temperature,DEC_PL));
  Blynk.virtualWrite(V2, String(humidity,DEC_PL));

  //check alarms;
  if (isnan(temperature) && isnan(humidity)){
    if (Blynk.connected() && !faultyProbeFlag)
    {
      Blynk.notify("Probe disconnected");
      Blynk.email(EmailSubject, wifiManager.htmlemail("Probe disconnected"));
      faultyProbeFlag = true;
    }
  }
  else if (temperature < lowAlarm && !tempAlarmFlag){
    Blynk.notify(String(temperature,DEC_PL) + "ºC Low temp alarm");
    Blynk.email(EmailSubject, wifiManager.htmlemail(String(temperature,DEC_PL) + "ºC Low temp alarm"));
    tempAlarmFlag = true;
  }
  else if (temperature > highAlarm && !tempAlarmFlag){
    Blynk.notify(String(temperature,DEC_PL) + "ºC High temp alarm");
    Blynk.email(EmailSubject, wifiManager.htmlemail(String(temperature,DEC_PL) + "ºC High temp alarm"));
    tempAlarmFlag = true;
  }
  else if (temperature > lowAlarm && temperature < highAlarm)
  {
    tempAlarmFlag = false;
  }
  else faultyProbeFlag = false;
}

void initBME280()
{
    if (! bme.begin(0x76)) Serial.println("Could not find BME280 sensor, check wiring, try address 0x77");//******check initialization;

    Serial.println("-- Custom Sensing Scenario --");
    Serial.println("normal mode, 1x temperature / 1x humidity / 0x pressure oversampling");
    Serial.println("= pressure off, filter off");
    bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                    Adafruit_BME280::SAMPLING_X1,   // temperature
                    Adafruit_BME280::SAMPLING_NONE, // pressure
                    Adafruit_BME280::SAMPLING_X1,   // humidity
                    Adafruit_BME280::FILTER_OFF );
}

void spiffsinit(){
  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(probe_type, json["probe_type"]);
          strcpy(blynk_token, json["blynk_token"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
}

void HandleRLYControl(){
  //if sensor not sending temperature turn relay OFF for safety
  if (isnan(temperature) && rly1On) {
    rly1On  = false;
    led1.off();
    led2.off();
    digitalWrite(RLY1_PIN, LOW);
    digitalWrite(RLY2_PIN, LOW);
  }

  else if (!isnan(temperature))
  {
      if(HEATING_RLY1){
        if (temperature < (SetPoint - Differential) && !rly1On)   {
          led1.on();
          digitalWrite(RLY1_PIN, HIGH);
          rly1On    = true;
        }
        else if (temperature >= SetPoint && rly1On) {
          led1.off();
          digitalWrite(RLY1_PIN, LOW);
          rly1On  = false;
        }
      }
      if (COOLING_RLY2){
          if (temperature > (SetPoint + Differential) && !rly2On)   {
            led2.on();
            digitalWrite(RLY2_PIN, HIGH);
            rly2On  = true;
          }
          else if (temperature <= SetPoint && rly2On) {
            led2.off();
            digitalWrite(RLY2_PIN, LOW);
            rly2On  = false;
          }
      }
  }
  //MANUAL RELAY CONTROL
  if (button_pressed && !rly3On){
    rly3On  = true;
    led3.on();
    digitalWrite(RLY3_PIN, HIGH);
  }
  else if((!button_pressed && rly3On) || !Blynk.connected()){
    rly3On  = false;
    led3.off();
    digitalWrite(RLY3_PIN, LOW);
  }
}

void setup() {
    // put your setup code here, to run once:
    WiFi.hostname(Host_Name);

    Serial.begin(74880);

    spiffsinit();                     //SPIFFS intialization
    initLED();                        //LED intialization

    pinMode(BOARD_BUTTON_PIN, INPUT_PULLUP);
    pinMode(RLY1_PIN, OUTPUT);
    pinMode(RLY2_PIN, OUTPUT);
    pinMode(RLY3_PIN, OUTPUT);

    digitalWrite(RLY1_PIN, LOW);
    digitalWrite(RLY2_PIN, LOW);
    digitalWrite(RLY3_PIN, LOW);

    //enabled when Wifi connect;
    timerTOsend       = timer.setInterval(SENDFREQ, SendStuff);          //send stuff to the cloud every 15 second
    timerToLED        = timer.setInterval(850L, blinkLED);  //blink virtual LED every 0.85sec - LED
    timerTOcontrol    = timer.setInterval(2000L, HandleRLYControl);      //CHANGE FREQUENCY
    timer.enable(timerTOcontrol);

    int_probe_type = atoi(probe_type);              //conver char to int.
    switch (int_probe_type) {
      case 1:{
        Wire.begin(SDA_PIN, SCL_PIN);               //CHANGE I2C PINS TO SDA 12 AND SCL 14
        Serial.println("Probe BME280 Selected");
        readfrequency = 1050;                        // suggested rate is 1 measurement per second
        initBME280();
        break;}
      case 2:{
        Wire.begin(SDA_PIN, SCL_PIN);               //CHANGE I2C PINS TO SDA 12 AND SCL 14
        Serial.println("Probe HTU21D Selected");
        readfrequency = 900;
        myHumidity.begin();
        break;}
      case 3:{
        Wire.begin(SDA_PIN, SCL_PIN);               //CHANGE I2C PINS TO SDA 12 AND SCL 14
        Serial.println("Probe SHT30 Selected");
        readfrequency = 900;
        break;}
      case 4:{
        Wire.begin(SDA_PIN, SCL_PIN);               //CHANGE I2C PINS TO SDA 12 AND SCL 14
        Serial.println("Probe AM2315 Selected");
        readfrequency = 2137;
        if (!am2315.begin()) Serial.println("Sensor not found, check wiring & pullups!");
        break;}
      case 5:{
        sensors.begin();
        Serial.println("Probe DS18B20 Selected");
        humidity = NAN;
        readfrequency = 2000;
        timer.disable(timerTOcontrol);
        break;}
      case 6:{
        //probably remove it*****delete*****
        Wire.begin(SDA_PIN, SCL_PIN);               //CHANGE I2C PINS TO SDA 12 AND SCL 14
        if (!bmp.begin()) {
        Serial.println("Could not find a valid BMP085 sensor, check wiring!");
        }
        Serial.println("Probe BP180 Selected");
        humidity = NAN;
        readfrequency = 2000;
        timer.disable(timerTOcontrol);
        break;}
      default:{
        readfrequency = 10000;
        Serial.println ("no probe selected Setup loop");
        timer.disable(timerTOcontrol);    //enable it after reading the probe once
      break;}
    }

    timerTOread       = timer.setInterval(readfrequency, readProbe);

    Blynk.config(blynk_token);
    ArduinoOTA.begin();
}

void loop() {
    // put your main code here, to run repeatedly:
    timer.run();
    ArduinoOTA.handle();

    if (!digitalRead(BOARD_BUTTON_PIN)) {
      if (buttonpressed == 0)   buttonpressed = millis();
      else if (millis() - buttonpressed >= TIMETOWIFIPORTAL)
      {
      button_change();      //If button is pressed, start configuration portal
      }
    }
    else buttonpressed = 0;

    if(WiFi.status()!=WL_CONNECTED) {
      //DISABLE READINGS and control for safety
      timer.disable(timerTOsend);
      timer.disable(timerTOread);
      colorToBlink = COLOR_RED;
    }
    else {
      colorToBlink = COLOR_BLYNK;
      Blynk.run();
      timer.enable(timerTOread);
      timer.enable(timerTOsend);
    }
}
