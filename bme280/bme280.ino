/*
 Name:		bme280.ino
 Created:	6/25/2020 7:19:40 PM
 Author:	alex
*/
#include <splash.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_MonoOLED.h>
#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;
LiquidCrystal_I2C lcd(0x27, 12, 2);
String GetData()
{
    delay(1000);
    
    String s = "Msg: Temperature=" + String(bme.readTemperature(),2) + " C,Humidity=" + String( bme.readHumidity(),2) + " %,Pressure=" + String(bme.readPressure() / 100.0F,2) + " hPa,Altitude=" + String(bme.readAltitude(SEALEVELPRESSURE_HPA),2) + " m";

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temperature=" + String(bme.readTemperature(),2));
    lcd.setCursor(0, 1);
    lcd.print("Humidity=" + String(bme.readHumidity(),2));
    return s;
}
void setup() {
	Serial.begin(115200);
    Serial1.begin(115200);
    unsigned status;
    status = bme.begin(0x76);
    lcd.init();
    lcd.backlight();
}

// the loop function runs over and over again until power down or reset
void loop() {
    if (Serial1.available() && Serial1.readString().startsWith("GetData")) Serial1.println(GetData());
}
