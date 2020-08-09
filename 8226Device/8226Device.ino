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
#include <ESP8266WebServer.h>
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

ESP8266WebServer server;
char* apssid = "8266";
IPAddress local_ip(192, 168, 11, 2);
IPAddress gateway(192, 168, 11, 1);
IPAddress netmask(255, 255, 255, 0);
bool saving = false;
char webpage[] PROGMEM = R"=====(
<html>
<head>
</head>
<body>
  <form>
    <fieldset>
      <div>
        <label for="ssid">SSID</label>      
        <input value="" id="ssid" placeholder="SSID">
      </div>
      <div>
        <label for="password">PASSWORD</label>
        <input type="password" value="" id="password" placeholder="PASSWORD">
      </div>
      <div>
        <button class="primary" id="savebtn" type="button" onclick="myFunction()">SAVE</button>
      </div>
    </fieldset>
  </form>
</body>
<script>
function myFunction()
{
  console.log("button was clicked!");
  var ssid = document.getElementById("ssid").value;
  var password = document.getElementById("password").value;
  var data = {ssid:ssid, password:password};
  var xhr = new XMLHttpRequest();
  var url = "/settings";
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      // Typical action to be performed when the document is ready:
      if(xhr.responseText != null){
        console.log(xhr.responseText);
      }
    }
  };
  xhr.open("POST", url, true);
  xhr.send(JSON.stringify(data));
};
</script>
</html>
)=====";
bool wifiConnect();
bool handleSettingsUpdate();
void start_config();
void read();

SSD1306  display(0x3c, 5, 4);//d1 d2

String readhost = "http://207.181.220.212/api/msg/request";
String writehost = "http://207.181.220.212/api/msg";

//String ssid = "alexsys";
//String password = "puikutsa";

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;
int greenLed = 16;
String button = "";
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

/*bool connect()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("");
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());
    delay(5000);
    return WiFi.status();
}*/

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
    if (!LittleFS.begin()) {
        Serial.println("Failed to mount file system");
        return;
    }
    display.init();

    wifiConnect();

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
    if (button == "Up") {
        start_config();
        button = "";
    }
    if (button == "Down") {
        read();
        button = "";
    }
    displayData(GetData());
    if (WiFi.isConnected()) {
        if (ReadApi()) WriteApi(GetData());
        digitalWrite(greenLed, LOW);
    }
    else digitalWrite(greenLed, HIGH);
    delay(1000);
}
void start_config()
{
    WiFi.softAPdisconnect(true);
    WiFi.disconnect();

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(local_ip, gateway, netmask);
    WiFi.softAP(apssid);
    server.on("/", []() {server.send_P(200, "text/html", webpage); });
    server.on("/settings", HTTP_POST, handleSettingsUpdate);

    server.begin();
    saving = true;
    while (saving)
    {
        server.handleClient();
    }
    
    WiFi.softAPdisconnect(true);
    WiFi.disconnect();
    wifiConnect();

}
bool handleSettingsUpdate()
{
    String data = server.arg("plain");
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, data);
    Serial.println(data);
    
    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile) {
        Serial.println("Failed to open config file for writing");
        return false;
    };
    if (serializeJson(doc, configFile) == 0) {
        Serial.println(F("Failed to write to file"));
    };
    
    Serial.println("Data Saved to File");
    configFile.close();
    
    server.send(200, "application/json", "{\"status\" : \"ok\"}");
    saving = false;
    server.close();
    return true;
}

bool wifiConnect()
{
    WiFi.softAPdisconnect(true);
    WiFi.disconnect();
    delay(1000);
    
    File configFile = LittleFS.open("/config.json", "r");

    if (configFile) {
        Serial.println("File Open");
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        //configFile.close();
        DynamicJsonDocument doc(1024);
        auto error = deserializeJson(doc, buf.get());
        if (error) {
            Serial.println("Failed to parse config file");
            return false;
        }
        const char* ssid = doc["ssid"];
        const char* password = doc["password"];
        Serial.println(ssid);
        Serial.println(password);
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");
            if ((unsigned long)(millis() - startTime) >= 5000) break;
        }
        Serial.println(WiFi.localIP());
    } else Serial.println("File not open");
    return true;
}

void read()
{
    File file2 = LittleFS.open("/config.json", "r");

    if (!file2) {
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.println("File Content:");

    while (file2.available()) {

        Serial.write(file2.read());
    }

    file2.close();
}