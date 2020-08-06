/*
 Name:		_8226Device.ino
 Created:	8/5/2020 5:58:34 PM
 Author:	alex
*/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> 
#include <ESP8266HTTPClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

SSD1306  display(0x3c, 5, 4);//d1 d2

String readhost = "http://207.181.220.212/api/msg/request";
String writehost = "http://207.181.220.212/api/msg";

String ssid = "alexsys";
String password = "puikutsa";

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;
int greenLed = 16;
String button = "Waiting…";
void ICACHE_RAM_ATTR interrupt0() // Right
{
    button = "Right";
}

void ICACHE_RAM_ATTR interrupt12() // Down
{
    button = "Down";
}

void ICACHE_RAM_ATTR interrupt13() // Up
{
    button = "Up";
}

void ICACHE_RAM_ATTR interrupt14() // Push
{
    button = "Push";
}

bool connect()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("");
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());
    delay(5000);
    return WiFi.status();
}

void setup() {
    pinMode(0, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(0), interrupt0, FALLING); // Right
    pinMode(12, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(12), interrupt12, FALLING); // Down
    pinMode(13, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(13), interrupt13, FALLING); // Up
    pinMode(14, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(14), interrupt14, FALLING); // Push
    //interrupts();
    Serial.begin(115200);
    Wire.begin(5, 4);
    display.init();
    connect();
    /*WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());*/
    pinMode(greenLed, OUTPUT);
    bme.begin(0x76);
}

String GetData()
{
    String s="Temperature=" + String(bme.readTemperature(), 2) + " C,Humidity=" + String(bme.readHumidity(), 2) + " %,Pressure=" + String(bme.readPressure() / 100.0F, 2) + " hPa,Altitude=" + String(bme.readAltitude(SEALEVELPRESSURE_HPA), 2) + " m";
    return s;
}

bool ReadApi()
{
    HTTPClient httpclient;
    httpclient.begin(readhost);
    int httpCode = httpclient.GET();
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = httpclient.getString();
        if (payload == "GetData") {
            httpclient.end();
            return true;
        }
    }
    else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", httpclient.errorToString(httpCode).c_str());
    }
    httpclient.end();

    return false;
}

void WriteApi(String s)
{
    HTTPClient httpclient;
    httpclient.begin(writehost);
    httpclient.addHeader("Content-Type", "application/json", true);  //Specify content-type header
    int httpCode = httpclient.POST("{\"text\": \"" + s + "\",\"user\": \"8226\",\"type\": \"Temp\"}");   //Send the request
    String payload = httpclient.getString();                  //Get the response payload
    Serial.println(httpCode);   //Print HTTP return code
    Serial.println(payload);    //Print request response payload
    httpclient.end();
}

void displayData(String s) {
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawStringMaxWidth(0, 0, 128,s);
    display.display();
}

void loop() {
    displayData(GetData());
    if (WiFi.isConnected()) {
        if (ReadApi()) WriteApi(GetData());
        digitalWrite(greenLed, LOW);
    }
    else digitalWrite(greenLed, HIGH);
    delay(5000);
}