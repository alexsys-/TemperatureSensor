#include <SoftwareSerial.h>
#include <VM_mem_check.h>
#include <VM_DBG.h>
#include <splash.h>
#include <Adafruit_SSD1306.h>
#include <RobotIRremoteTools.h>
#include <RobotIRremoteInt.h>
#include <RobotIRremote.h>
#include <VM_Boards.h>
/*
 Name:		TemperatureSensor.ino
 Created:	6/3/2020 4:08:49 PM
 Author:	alex
*/
#include <ssd1306_uart.h>
#include <ssd1306_generic.h>
#include <ssd1306_fonts.h>
#include <ssd1306_console.h>
#include <ssd1306_8bit.h>
#include <ssd1306_1bit.h>
#include <ssd1306_16bit.h>
#include <ssd1306.h>
#include <sprite_pool.h>
#include <nano_gfx.h>
#include <nano_gfx_types.h>
#include <nano_engine.h>
#include <font6x8.h>
#include <Adafruit_BME280.h>
#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "DHT.h"

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 12, 2);
float h, t;
String GetData()
{
    h = 0; t = 0;
    delay(3000);
    h = dht.readHumidity();
    t = dht.readTemperature();
    if (isnan(h) || isnan(t)) {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
    }
    String s = "Msg: Temperature=" + String(t, 2) + ",Humidity " + String(h, 2);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temperature=" + String(t, 2));
    lcd.setCursor(0, 1);
    lcd.print("Humidity=" + String(h, 2));

    return s;

}
void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    Serial1.begin(115200);
    dht.begin();
    lcd.init();
    lcd.backlight();
}

void loop() {
    if (Serial1.available() && Serial1.readString().startsWith("GetData")) Serial1.println(GetData());
    //Serial.println(GetData());
    //delay(2000);
}