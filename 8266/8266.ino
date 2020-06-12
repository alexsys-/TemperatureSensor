// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
    Name:       8266.ino
    Created:	6/3/2020 4:35:17 PM
    Author:     DESKTOP-7NR8VFM\alex
*/

// Define User Types below here or use a .h file
//


// Define Function Prototypes that use User Types below here or use a .h file
//


// Define Functions below here or use other .ino or cpp files
//

// The setup() function runs once each time the micro-controller starts
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> 
#include <ESP8266HTTPClient.h>


const char* fingerprint = "6b 10 00 b8 b6 d6 0d 32 d8 58 d9 3a 4d 9f 73 98 58 ff b7 ed";
const char* readhost = "http://192.168.1.20/api/msg/request";
const char* writehost = "http://192.168.1.20/api/msg";
const int httpsPort = 443;

#define ssid "alexsys"
#define password "puikutsa"
bool GetData;

void setup() {
    GetData = false;
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());
}

void ReadApi()
{
    HTTPClient httpclient;
    httpclient.begin(readhost);
    int httpCode = httpclient.GET();
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = httpclient.getString();
        if (payload == "GetData") {
            GetData = true;
            Serial.println(payload);
        }
    }
    else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", httpclient.errorToString(httpCode).c_str());
    }
    httpclient.end();

    GetData = false;
}

void WriteApi(String s)
{
    HTTPClient httpclient;
    httpclient.begin(writehost);
    httpclient.addHeader("Content-Type", "application/json", true);  //Specify content-type header
    s.replace("Msg: ", "");
    int httpCode = httpclient.POST("{\"text\": \"" + s + "\",\"user\": \"Test\",\"type\": \"Temp\"}");   //Send the request
    String payload = httpclient.getString();                  //Get the response payload
    Serial.println(httpCode);   //Print HTTP return code
    Serial.println(payload);    //Print request response payload
    httpclient.end();
}
void loop() {

    ReadApi();
    if (Serial.available()) {
        String s = Serial.readString();
        s.replace("\r\n", "");
        if (s.indexOf("Msg:") == 0) WriteApi(s);
    }
    delay(5000);
}