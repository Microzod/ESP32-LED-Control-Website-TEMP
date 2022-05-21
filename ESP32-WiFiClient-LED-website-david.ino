/*
Copyright (C) 2021 wk & david
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, version 3.
The above copyright notice, this permission notice and the word "NIGGER" shall be included in all copies or substantial portions of the Software.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "SPIFFS.h"
#include "FS.h"
#include <ArduinoJson.h>
#include <stdlib.h>         // function used: int atoi (const char * str);

#include <LTC2633Library.h>
#include "colorTemperature.h"
#include <TimeLib.h>
#include <TimeAlarms.h>
#include "NTP_stuff.h"
#include "OTA.h"
#include "credentials.h"
#include "esp32_pins.h"
#include "functions.h"
#include <IRremote.hpp>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/timer.h"
#include "esp_timer.h"


colorTemperature LED(26, 27, LTC2633_CA0_VCC, &Wire);

typedef struct
{        
    String status;
    String startOfDay;
    String endOfDay;
    time_t sunriseLength;
    time_t sunsetLength;
    
    
    time_t startOfDay_t;
    time_t endOfDay_t;
    time_t sunriseLength_t;
    time_t sunsetLength_t;
    uint16_t cct;
    uint16_t intensityControl;
    
    int8_t status_number;
    uint8_t isOnOrOff;
    uint8_t isAutoActive;
    bool newValue;
} serverDataStruct;

typedef struct
{
    String status;
    time_t startOfDay_t;
    time_t endOfDay_t;
    time_t sunriseLength_t;
    time_t sunsetLength_t;
    uint16_t cct;
    uint16_t intensityControl;

    uint8_t startOfDay_isActive;
    uint8_t endOfDay_isActive;

    uint8_t triggerSunrise;
    uint8_t triggerSunset;
    
    int8_t status_number;
    uint8_t isOnOrOff;
    uint8_t isAutoActive;
    bool newValue;
} dataResultStruct;

serverDataStruct espReceiveData_struct = {"0", "0", "0", 0};
dataResultStruct espResult_struct = {"0", 0};

String hostname = "esp.lan";
WebServer server(80);

// Do you need to create & format a filesystem
#define FILESYSTEM SPIFFS
// You only need to format the filesystem once
#define FORMAT_FILESYSTEM true
//#define Serial Serial

File fsUploadFile;
//String status;
//String ramp;

// David functions:
void compareData(serverDataStruct *s, dataResultStruct *result);
void serialPrintDataStruct(serverDataStruct *s);
void getData(serverDataStruct *s);
//void parseData(serverDataStruct *s);

// Wille functions:
void entryPoint( );
void serverSetup();
bool exists(String path);
void writeArray();
void readArray();
void requestInfo();
bool handleFileRead(String path);
void handleNotFound();
void handleRoot();
void writeFile(const char * path, String message);
void appendFile(const char * path, const char * message);

DynamicJsonDocument doc(1024);
JsonObject obj;

void setup()
{
    pinMode(BUTTON_pin, INPUT);
    pinMode(IR_OUT_pin, INPUT);
    pinMode(RF_REMOTE_pin, INPUT);
    pinMode(LED1_pin, OUTPUT);
    pinMode(SSR_A_pin, OUTPUT);
    pinMode(SSR_B_pin, OUTPUT);
        digitalWrite(LED1_pin, HIGH);
        digitalWrite(SSR_A_pin, LOW);
        digitalWrite(SSR_B_pin, LOW);
        
    Serial.begin(115200);
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE)); // Print which program is running.
    Serial.setDebugOutput(true);
    //pinMode(GPIO15, INPUT);

    serverSetup();
}

void loop()
{
    #ifdef defined(ESP32_RTOS) && defined(ESP32)
    #else // If you do not use FreeRTOS, you have to regulary call the handle method.
        //ArduinoOTA.handle();
    #endif
    
    server.handleClient();
    delay(2);

    
}


void compareData(serverDataStruct *s, dataResultStruct *result)
{
    
    
    if (!(s->status == result->status))
    {
        result->newValue = true;
    }
    else if (!(s->startOfDay_t == result->startOfDay_t))
    {
        result->newValue = true;
    }
    else if (!(s->endOfDay_t == result->endOfDay_t))
    {
        result->newValue = true;
    }
    else if (!(s->sunriseLength_t == result->sunriseLength_t))
    {
        result->newValue = true;
    }
    else if (!(s->sunsetLength_t == result->sunsetLength_t))
    {
        result->newValue = true;
    }
    else if (!(s->cct == result->cct))
    {
        result->newValue = true;
    }
    else if (!(s->intensityControl == result->intensityControl))
    {
        result->newValue = true;
    }

    if (result->newValue)
    {
        result->status           = s->status;
        result->startOfDay_t     = s->startOfDay_t;
        result->endOfDay_t       = s->endOfDay_t;
        result->sunriseLength_t  = s->sunriseLength_t;
        result->sunsetLength_t   = s->sunsetLength_t;
        result->cct              = s->cct;
        result->intensityControl = s->intensityControl;
        
        result->status_number    = s->status_number;
        result->isOnOrOff        = s->isOnOrOff;
        result->isAutoActive     = s->isAutoActive;
        
        serialPrintDataStruct(&espResult_struct);
        
        /*
        // Use new values for LED settings:
        CCT.setSunriseTime(result->startOfDay_t);
        if (startOfDay_isActive)
            CCT.enableSunriseTrigger();
            
        CCT.setSunsetTime(result->endOfDay_t);
        if (endOfDay_isActive)
            CCT.enableSunsetTrigger();
            
        CCT.setSunriseDuration(result->sunriseLength_t);
        CCT.setSunsetDuration(result->sunsetLength_t);
        CCT.setColorTemperature(result->cct);
        CCT.setIntensityInteger(result->intensityControl);
        CCT.writeToLedInteger();
        */
    }
    
    //int round_step_s = (s->intensityStep + 0.5);  int round_step_result = (result->intensityStep + 0.5);
    //int round_s = int(s->intensityControl + 0.5); int round_result = int(result->intensityControl + 0.5);
    //if (!(round_step_s == round_step_result))
    //    result->newValue = true;
    //
    //if (!(round_s == round_result))
    //    result->newValue = true;
}

void serialPrintDataStruct(dataResultStruct *s)
{
    Serial.println("Data parsed from server json file:");
    Serial.print("Status: ");
    Serial.println(s->status);

    time_t startOfDay_total = s->startOfDay_t;
    
    time_t startOfDay_minutes = startOfDay_total / 60;
    time_t startOfDay_seconds = startOfDay_total % 60;
    
    time_t startOfDay_hours = startOfDay_minutes / 60;
    startOfDay_minutes = startOfDay_minutes % 60;

    time_t endOfDay_total = s->endOfDay_t;
    
    time_t endOfDay_minutes = endOfDay_total / 60;
    time_t endOfDay_seconds = endOfDay_total % 60;
    
    time_t endOfDay_hours = endOfDay_minutes / 60;
    endOfDay_minutes = endOfDay_minutes % 60;
    
    Serial.print("Start of day(seconds): ");
    Serial.print(s->startOfDay_t);
    Serial.println(" s");
    Serial.print("Start of day(hh:mm:ss): ");
    if (startOfDay_hours < 10)
        Serial.print("0");
    Serial.print(startOfDay_hours);
    Serial.print(":");
    if (startOfDay_minutes < 10)
        Serial.print("0");
    Serial.print(startOfDay_minutes);
    Serial.print(":");
    if (startOfDay_seconds < 10)
        Serial.print("0");
    Serial.println(startOfDay_seconds);
    

    Serial.print("End of day(seconds): ");
    Serial.print(s->endOfDay_t);
    Serial.println(" s");
    Serial.print("End of day(hh:mm:ss) - ");
    if (endOfDay_hours < 10)
        Serial.print("0");
    Serial.print(endOfDay_hours);
    Serial.print(":");
    if (endOfDay_minutes < 10)
        Serial.print("0");
    Serial.print(endOfDay_minutes);
    Serial.print(":");
    if (endOfDay_seconds < 10)
        Serial.print("0");
    Serial.println(endOfDay_seconds);
    
    Serial.print("Sunrise Length: ");
    Serial.println(s->sunriseLength_t);
    
    Serial.print("Sunset Length: ");
    Serial.println(s->sunsetLength_t);
    
    Serial.print("Correlated Color Temperature(degrees Kelvin): ");
    Serial.print(s->cct);
    Serial.println("°K");
    
    Serial.print("Intensity Control Level(0-4095): ");
    Serial.println(s->intensityControl);

    Serial.print("status_number: ");
    Serial.println(s->status_number);
    Serial.print("isOnOrOff: ");
    Serial.println(s->isOnOrOff);
    Serial.print("isAutoActive: ");
    Serial.println(s->isAutoActive);
}

void getData(serverDataStruct *s)
{
    File file = FILESYSTEM.open("/config.json");
    char data[1024];

    while(file.available())
    {
        file.readBytes(data,sizeof(data));
    }
    
    file.close();

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, data);
    JsonObject obj = doc.as<JsonObject>();


    // Vet ej om följande(resten utav denna functionen) kommer att fungera???
    String status =         obj["status"];          s->status = status;
    String startOfDay =     obj["startOfDay"];      s->startOfDay = startOfDay;
    String endOfDay =       obj["endOfDay"];        s->endOfDay = endOfDay;
    s->sunriseLength =      obj["sunriseLength"];
    s->sunsetLength =       obj["sunsetLength"];
    s->cct =                obj["cct"];
    s->intensityControl =   obj["intensityControl"];
    
//}

//void parseData(serverDataStruct *s)
//{

    if (s->status == "on")
    {
        s->status_number = 1;
        s->isOnOrOff = 1;
        s->isAutoActive = 0;
    }
    else if (s->status == "auto")
    {
        s->status_number = -1;
        s->isOnOrOff = 1;
        s->isAutoActive = 1;
    }
    else if (s->status == "off")
    {
        s->status_number = 0;
        s->isOnOrOff = 0;
        s->isAutoActive = 0;
    }

    char tempArray[2];
    int i;
    time_t Hours;
    time_t Minutes;
    tempArray[0] = s->startOfDay[0];
    tempArray[1] = s->startOfDay[1];
    i = atoi(tempArray);
    Hours = (time_t)i;
    
    tempArray[0] = s->startOfDay[3];
    tempArray[1] = s->startOfDay[4];
    i = atoi(tempArray);
    Minutes = (time_t)i;

    s->startOfDay_t = (60 * Minutes) + (3600 * Hours);
    
    tempArray[0] = s->endOfDay[0];
    tempArray[1] = s->endOfDay[1];
    i = atoi(tempArray);
    Hours = (time_t)i;
    
    tempArray[0] = s->endOfDay[3];
    tempArray[1] = s->endOfDay[4];
    i = atoi(tempArray);
    Minutes = (time_t)i;

    s->endOfDay_t = (60 * Minutes) + (3600 * Hours);
    
    s->sunriseLength_t = (60 * s->sunriseLength);
    s->sunsetLength_t = (60 * s->sunsetLength);

    //serialPrintDataStruct(s);
}

// This function should be used to interface with the rest of the system
void entryPoint( )
{
    
    
    // load the config file in order to access stored values
    readArray();
    
    getData(&espReceiveData_struct);
    //parseData(&espReceiveData_struct);
    //serialPrintDataStruct(&espResult_struct);

}

void serverSetup()
{
    Serial.println("Starting filesystem");
    
    FILESYSTEM.begin();

        
    if(FORMAT_FILESYSTEM)
    {
        FILESYSTEM.format();
        writeFile("/index.htm", indexData);
    }
       
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(mySSID);

    WiFi.begin(mySSID, myPASSWORD);
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(hostname.c_str());

    while(WiFi.status() != WL_CONNECTED)
    {
        delay(100);
        Serial.print(".");
    }

    
    Serial.println("\nConnected!");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Hostname: ");
    Serial.println(hostname);
        
    if(MDNS.begin("esp32"))
    {
        Serial.println("MDNS responder started");
    }

    server.on("/", HTTP_GET, handleRoot);

    server.onNotFound([]()
    {
        Serial.println(server.uri());
        if (!handleFileRead(server.uri()))
        {
            server.send(404, "text/plain", "404");
        }
    });

    server.begin();
    Serial.println("HTTP server started");
    //setupOTA("ESP32-black-stand-offs", mySSID, myPASSWORD);
    setupNTP();
}

bool exists(String path)
{
    bool yes = false;
    File file = FILESYSTEM.open(path, "r");

    if(!file.isDirectory())
    {
        yes = true;
    }

    file.close();
    return yes;
}

void writeArray()
{
    DynamicJsonDocument obj(1024);
    String data;

    for(uint8_t i = 0; i < server.args(); i++)
    {
        obj[server.argName(i)] = server.arg(i);
    }

    serializeJson(obj, data);
    writeFile("/config.json", data);
    
    data = "";
    serializeJsonPretty(obj, data);
    writeFile("/config_pretty.json", data);
    //readArray();
}

void readArray()
{
    File file = FILESYSTEM.open("/config.json");
    char data[1024];

    while(file.available())
    {
        file.readBytes(data,sizeof(data));
    }
    
    file.close();
    //Serial.println(data);

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, data);
    //JsonObject obj = doc.as<JsonObject>();
    obj = doc.as<JsonObject>();
    //setConfigData(obj);
}

void requestInfo()
{
    Serial.println("REQUEST INFO");
    Serial.println("Method: " + (server.method() == HTTP_GET) ? "GET" : "POST");
    Serial.println("Arguments: ");
    
    for(uint8_t i = 0; i < server.args(); i++)
    {
        Serial.println(server.argName(i) + ": " + server.arg(i));
    }
}

bool handleFileRead(String path)
{
    Serial.println("handleFileRead: " + path);

    if(server.args() > 0)
    {
        //requestInfo();
        writeArray();
        
        // After form-data written to config JSON.
        entryPoint();
    }

    //requestInfo();
    
    if(path.endsWith("/"))
    {
        path += "index.htm";
    }

    String contentType = getContentType(path);

    if(exists(path))
    {
        File file = FILESYSTEM.open(path, "r");
        server.streamFile(file, contentType);
        file.close();
        
        return true;
    }
    
    return false;
}

void handleNotFound()
{
    String message = "<h1>404</h1><span>File was not found!</span>\n\n";
    server.send(404, "text/plain", message);
}

void handleRoot()
{
    if(!exists("/index.htm"))
    {
        server.send(200, "text/html", indexData);
    } else {
        handleFileRead("/index.htm");
    }
}

void writeFile(const char * path, String message)
{
    File file = FILESYSTEM.open(path, FILE_WRITE);
    
    if(!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }

    if(file.print(message))
    {
        Serial.print("Wrote to: ");
        Serial.println(path);
    } else
    {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(const char * path, const char * message)
{
    Serial.printf("Appending to file: %s\n", path);

    File file = FILESYSTEM.open(path, FILE_APPEND);
    if(!file)
    {
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message))
    {
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }

    file.close();
}





/*
//#define ESP32_RTOS  // Uncomment this line if you want to use the code with freertos only on the ESP32
                      //     Has to be done before including "OTA.h"
#include "OTA.h"
#include "credentials.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "SPIFFS.h"
#include "FS.h"
#include <ArduinoJson.h>
#include "functions.h"
#include "NTP_stuff.h"
#include <stdlib.h>     // int atoi (const char * str);

typedef struct
{
    String status;
    String startOfDay;
    time_t startOfDay_hours;
    time_t startOfDay_minutes;
    String endOfDay;
    time_t endOfDay_hours;
    time_t endOfDay_minutes;
    time_t rampIncreament;
    time_t rampTime;
    uint16_t cct;
    uint16_t intensityStep;
    uint16_t intensityControl;
} wifiDataStruct;


String hostname = "esp.lan";
WebServer server(80);

// Do you need to create & format a filesystem
#define FILESYSTEM SPIFFS
// You only need to format the filesystem once
#define FORMAT_FILESYSTEM true
//#define Serial Serial

File fsUploadFile;
//String status;
//String ramp;


bool exists(String path);
void writeArray();
void readArray();
void requestInfo();
bool handleFileRead(String path);
void handleFileList();
void handleNotFound();
void handleRoot();
void writeFile(const char * path, String message);
void appendFile(const char * path, const char * message);

DynamicJsonDocument doc(1024);
JsonObject obj;

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    //pinMode(GPIO15, INPUT);
    
    Serial.println("Starting filesystem");
    
    FILESYSTEM.begin();
        
    if(FORMAT_FILESYSTEM) {
        FILESYSTEM.format();
        writeFile("/index.htm", indexData);
    }
       
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(mySSID);

    WiFi.begin(mySSID, myPASSWORD);
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(hostname.c_str());

    while(WiFi.status() != WL_CONNECTED)
    {
        delay(100);
        Serial.print(".");
    }

    
    Serial.println("\nConnected!");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Hostname: ");
    Serial.println(hostname);
        
    if(MDNS.begin("esp32"))
    {
        Serial.println("MDNS responder started");
    }

    server.on("/", HTTP_GET, handleRoot);

    server.onNotFound([]()
    {
        Serial.println(server.uri());
        if (!handleFileRead(server.uri()))
        {
            server.send(404, "text/plain", "404");
        }
    });

    server.begin();
    Serial.println("HTTP server started");
    //setupOTA("ESP32-black-stand-offs", mySSID, myPASSWORD);
    //setupNTP();
}

void loop()
{
    
    //Serial.println("Waiting for HTTP-requests");

    server.handleClient();
    delay(2);

    
}

//    String status;
//    time_t startOfDay;
//    time_t endOfDay;
//    uint16_t rampIncreament;
//    time_t rampTime;
//    uint16_t cct;
//    uint16_t intensityStep;
//    uint16_t intensityControl;

void getData(wifiDataStruct *s)
{
    File file = FILESYSTEM.open("/config.json");
    char data[1024];

    while(file.available()){
        file.readBytes(data,sizeof(data));
    }
    
    file.close();

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, data);
    JsonObject obj = doc.as<JsonObject>();


    // Vet ej om följande(resten utav denna functionen) kommer att fungera???
    String status =       obj["status"];
    String startOfDay =   obj["startOfDay"];
    String endOfDay =     obj["endOfDay"];
    s->rampIncreament =   obj["rampIncreament"];
    s->rampTime =         obj["rampTime"];
    s->cct =              obj["cct"];
    s->intensityStep =    obj["intensityStep"];
    s->intensityControl = obj["intensityControl"];
    

    char tempArray[2];
    int i;
    
    tempArray[0] = startOfDay[0];
    tempArray[1] = startOfDay[1];
    i = atoi (tempArray);
    s->startOfDay_hours = (time_t)i;
    
    tempArray[0] = startOfDay[5];
    tempArray[1] = startOfDay[6];
    i = atoi (tempArray);
    s->startOfDay_minutes = (time_t)i;
    
    tempArray[0] = endOfDay[0];
    tempArray[1] = endOfDay[1];
    i = atoi (tempArray);
    s->endOfDay_hours = (time_t)i;
    
    tempArray[0] = endOfDay[5];
    tempArray[1] = endOfDay[6];
    i = atoi (tempArray);
    s->endOfDay_minutes = (time_t)i;
}

bool exists(String path)
{
    bool yes = false;
    File file = FILESYSTEM.open(path, "r");

    if(!file.isDirectory())
    {
        yes = true;
    }

    file.close();
    return yes;
}

void writeArray()
{
    DynamicJsonDocument obj(1024);
    String data;

    for(uint8_t i = 0; i < server.args(); i++)
    {
        obj[server.argName(i)] = server.arg(i);
    }

    serializeJson(obj, data);
    writeFile("/config.json", data);
    
    data = "";
    serializeJsonPretty(obj, data);
    writeFile("/config_pretty.json", data);
    //readArray();
}

void readArray()
{
    File file = FILESYSTEM.open("/config.json");
    char data[1024];

    while(file.available())
    {
        file.readBytes(data,sizeof(data));
    }
    
    file.close();
    //Serial.println(data);

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, data);
    //JsonObject obj = doc.as<JsonObject>();
    obj = doc.as<JsonObject>();
    //setConfigData(obj);
}

void requestInfo()
{
    Serial.println("REQUEST INFO");
    Serial.println("Method: " + (server.method() == HTTP_GET) ? "GET" : "POST");
    Serial.println("Arguments: ");
    
    for(uint8_t i = 0; i < server.args(); i++)
    {
        Serial.println(server.argName(i) + ": " + server.arg(i));
    }
}

// This function should be used to interface with the rest of the system
void entryPoint( ) {
    Serial.println("* This code should run whenever a update / change has occured to the configuration. *");
    
    // load the config file in order to access stored values
    readArray();
    
    // Get status value
    const char* status = obj["status"];
    Serial.println(status);

    // Get start of day value
    const char* start = obj["startOfDay"];
    Serial.print("Start of day: ");
    Serial.println(start);

     // Get start of day value
    const char* endOfDay = obj["endOfDay"];
    Serial.print("End of day: ");
    Serial.println(endOfDay);

     // Ramp increment
    int ramp = obj["rampIncreament"];
    Serial.print("Ramp increment: ");
    Serial.println(ramp);
    
    // Ramp time
    float rampTime = obj["rampTime"];
    Serial.print("Ramp time: ");
    Serial.println(rampTime);

}


bool handleFileRead(String path)
{
    Serial.println("handleFileRead: " + path);

    if(server.args() > 0)
    {
        //requestInfo();
        writeArray();
        
        // After form-data written to config JSON.
        entryPoint();
    }

    //requestInfo();
    
    if(path.endsWith("/"))
    {
        path += "index.htm";
    }

    String contentType = getContentType(path);

    if(exists(path))
    {
        File file = FILESYSTEM.open(path, "r");
        server.streamFile(file, contentType);
        file.close();
        
        return true;
    }
    
    return false;
}

void handleNotFound()
{
    String message = "<h1>404</h1><span>File was not found!</span>\n\n";
    server.send(404, "text/plain", message);
}

void handleRoot()
{
    if(!exists("/index.htm"))
    {
        server.send(200, "text/html", indexData);
    } else {
        handleFileRead("/index.htm");
    }
}

void writeFile(const char * path, String message)
{
    File file = FILESYSTEM.open(path, FILE_WRITE);
    
    if(!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }

    if(file.print(message))
    {
        Serial.print("Wrote to: ");
        Serial.println(path);
    } else
    {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(const char * path, const char * message)
{
    Serial.printf("Appending to file: %s\n", path);

    File file = FILESYSTEM.open(path, FILE_APPEND);
    if(!file)
    {
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message))
    {
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }

    file.close();
}

*/


/*
Copyright (C) 2021 wk & david
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, version 3.
The above copyright notice, this permission notice and the word "NIGGER" shall be included in all copies or substantial portions of the Software.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>
*/
/*
 * ERRORS & THINGS TO CHANGE/FIX/ADD:
 * 
 * ¤1: När man trycker på "SAVE" så byts Status alltid ut emot 'On' alternativet.
 * 
 */

//#define ESP32_RTOS  // Uncomment this line if you want to use the code with freertos only on the ESP32
                      //     Has to be done before including "OTA.h"
/*
#include "Arduino.h"
#include "OTA.h"
#include "credentials.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "SPIFFS.h"
#include "FS.h"
#include <ArduinoJson.h>
#include "functions.h"
#include "NTP_stuff.h"
#include <stdlib.h>     // int atoi (const char * str);
//#include "serverManagement.h"


#define FILESYSTEM SPIFFS      // // Do you need to create & format a filesystem
#define FORMAT_FILESYSTEM true // You only need to format the filesystem once
#define Serial Serial

struct serverDataStruct
{        
    String status;
    String startOfDay;
    String endOfDay;
    time_t rampUpTime;
    time_t rampDownTime;
    uint16_t cct;
    double intensityStep;
    double intensityControl;
    
    time_t startOfDay_minutes;
    time_t startOfDay_hours;
    time_t startOfDay_t;
    time_t endOfDay_minutes;
    time_t endOfDay_hours;
    time_t endOfDay_t;
    time_t rampUpTime_t;
    time_t rampDownTime_t;
};

struct dataResultStruct
{
    String status;
    int8_t status_num;
    time_t startOfDay_t;
    time_t endOfDay_t;
    time_t rampUpTime_t;
    time_t rampDownTime_t;
    uint16_t cct;
    double intensityStep;
    double intensityControl;
};

serverDataStruct espReceiveData_struct = {"0", "0", "0", 0};
dataResultStruct espResultData_struct = {"0", 0};






String hostname = "esp.lan";
WebServer server(80);

// Do you need to create & format a filesystem
#define FILESYSTEM SPIFFS
// You only need to format the filesystem once
#define FORMAT_FILESYSTEM true
//#define Serial Serial

File fsUploadFile;

void serialPrintDataStruct(serverDataStruct *s);
void getData(serverDataStruct *s);
void parseData(serverDataStruct *s);
void serverSetup();
void entryPoint( );
bool exists(String path);
void writeArray();
void readArray();
void requestInfo();
bool handleFileRead(String path);
void handleNotFound();
void handleRoot();
void writeFile(const char * path, String message);
void appendFile(const char * path, const char * message);

DynamicJsonDocument doc(1024);
JsonObject obj;


void setup()
{
    //serverSetup();
    
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println("Starting filesystem");
    
    FILESYSTEM.begin();
    
    if(FORMAT_FILESYSTEM)
    {
        FILESYSTEM.format();
        writeFile("/index.htm", indexData);
    }
       
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(mySSID);

    WiFi.begin(mySSID, myPASSWORD);
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(hostname.c_str());

    while(WiFi.status() != WL_CONNECTED)
    {
        delay(100);
        Serial.print(".");
    }
    
    Serial.println("\nConnected!");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Hostname: ");
    Serial.println(hostname);
    
    if(MDNS.begin("esp32"))
    {
        Serial.println("MDNS responder started");
    }
    
    server.on("/", HTTP_GET, handleRoot);
    
    server.onNotFound([]()
    {
        Serial.println(server.uri());
        if (!handleFileRead(server.uri()))
        {
            server.send(404, "text/plain", "404");
        }
    });
    
    server.begin();
    setupNTP();
    //setupOTA("ESP32-black-stand-offs", mySSID, myPASSWORD);
    
    Serial.println("HTTP server & NTP server started");
    
}

void loop()
{
    #ifdef defined(ESP32_RTOS) && defined(ESP32)
    #else // If you do not use FreeRTOS, you have to regulary call the handle method.
        //ArduinoOTA.handle();
    #endif
    
    server.handleClient();
    delay(2);

    
}

void serverSetup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    //pinMode(GPIO15, INPUT);
    
    

    //pinMode(GPIO15, INPUT);
    
    Serial.println("Starting filesystem");
    
    FILESYSTEM.begin();

        
    if(FORMAT_FILESYSTEM) {
        FILESYSTEM.format();
        writeFile("/index.htm", indexData);
    }
       
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(mySSID);

    WiFi.begin(mySSID, myPASSWORD);
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(hostname.c_str());

    while(WiFi.status() != WL_CONNECTED)
    {
        delay(100);
        Serial.print(".");
    }

    
    Serial.println("\nConnected!");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Hostname: ");
    Serial.println(hostname);
        
    if(MDNS.begin("esp32"))
    {
        Serial.println("MDNS responder started");
    }

    server.on("/", HTTP_GET, handleRoot);

    server.onNotFound([]()
    {
        Serial.println(server.uri());
        if (!handleFileRead(server.uri()))
        {
            server.send(404, "text/plain", "404");
        }
    });

    server.begin();
    Serial.println("HTTP server started");
    //setupOTA("ESP32-black-stand-offs", mySSID, myPASSWORD);
    setupNTP();
}

void serialPrintDataStruct(serverDataStruct *s)
{
    Serial.println("Data parsed from server json file:");
    //Serial.println("");
    Serial.print("Status: ");
    Serial.println(s->status);

    Serial.print("Start of day: ");
    Serial.println(s->startOfDay);
    Serial.print("Start of day(hours): ");
    Serial.println(s->startOfDay_hours);
    Serial.print("Start of day(minutes): ");
    Serial.println(s->startOfDay_minutes);
    Serial.print("Start of day(seconds): ");
    Serial.println(s->startOfDay_t);

    Serial.print("End of day: ");
    Serial.println(s->endOfDay);
    Serial.print("End of day(hours): ");
    Serial.println(s->endOfDay_hours);
    Serial.print("End of day(minutes): ");
    Serial.println(s->endOfDay_minutes);
    Serial.print("End of day(seconds): ");
    Serial.println(s->endOfDay_t);
    
    Serial.print("Ramp-Up Time: ");
    Serial.println(s->rampUpTime_t);
    
    Serial.print("Ramp-Down Time: ");
    Serial.println(s->rampDownTime_t);
    
    Serial.print("Color Temperature(CCT): ");
    Serial.println(s->cct);
    
    Serial.print("Intensity Step Size: ");
    Serial.println(s->intensityStep);
    
    Serial.print("Intensity Control Level: ");
    Serial.println(s->intensityControl);
}

void getData(serverDataStruct *s)
{
    File file = FILESYSTEM.open("/config.json");
    char data[1024];

    while(file.available()){
        file.readBytes(data,sizeof(data));
    }
    
    file.close();

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, data);
    JsonObject obj = doc.as<JsonObject>();


    // Vet ej om följande(resten utav denna functionen) kommer att fungera???
    String status =         obj["status"];      s->status = status;
    String startOfDay =     obj["startOfDay"];  s->startOfDay = startOfDay;
    String endOfDay =       obj["endOfDay"];    s->endOfDay = endOfDay;
    s->rampUpTime =         obj["rampUpTime"];
    s->rampDownTime =       obj["rampDownTime"];
    s->cct =                obj["cct"];
    s->intensityStep =      obj["intensityStep"];
    s->intensityControl =   obj["intensityControl"];
}

void parseData(serverDataStruct *s)
{
    char tempArray[2];
    int i;
    //time_t hours;
    //time_t minutes;
    tempArray[0] = s->startOfDay[0];
    tempArray[1] = s->startOfDay[1];
    i = atoi(tempArray);
    s->startOfDay_hours = (time_t)i;
    
    tempArray[0] = s->startOfDay[3];
    tempArray[1] = s->startOfDay[4];
    i = atoi(tempArray);
    s->startOfDay_minutes = (time_t)i;

    s->startOfDay_t = (60 * s->startOfDay_minutes) + (3600 * s->startOfDay_hours);
    
    tempArray[0] = s->endOfDay[0];
    tempArray[1] = s->endOfDay[1];
    i = atoi(tempArray);
    s->endOfDay_hours = (time_t)i;
    
    tempArray[0] = s->endOfDay[3];
    tempArray[1] = s->endOfDay[4];
    i = atoi(tempArray);
    s->endOfDay_minutes = (time_t)i;

    s->endOfDay_t = (60 * s->endOfDay_minutes) + (3600 * s->endOfDay_hours);
    
    s->rampUpTime_t = (s->rampUpTime * 60);
    s->rampDownTime_t = (s->rampDownTime * 60);

    //serialPrintDataStruct(s);
}

// This function should be used to interface with the rest of the system
void entryPoint( )
{
    
    
    // load the config file in order to access stored values
    readArray();
    
    getData(&espReceiveData_struct);
    parseData(&espReceiveData_struct);
    serialPrintDataStruct(&espReceiveData_struct);

}

bool exists(String path)
{
    bool yes = false;
    File file = FILESYSTEM.open(path, "r");

    if(!file.isDirectory())
    {
        yes = true;
    }

    file.close();
    return yes;
}

void writeArray()
{
    DynamicJsonDocument obj(1024);
    String data;

    for(uint8_t i = 0; i < server.args(); i++)
    {
        obj[server.argName(i)] = server.arg(i);
    }

    serializeJson(obj, data);
    writeFile("/config.json", data);
    
    data = "";
    serializeJsonPretty(obj, data);
    writeFile("/config_pretty.json", data);
    //readArray();
}

void readArray()
{
    File file = FILESYSTEM.open("/config.json");
    char data[1024];

    while(file.available())
    {
        file.readBytes(data,sizeof(data));
    }
    
    file.close();
    //Serial.println(data);

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, data);
    //JsonObject obj = doc.as<JsonObject>();
    obj = doc.as<JsonObject>();
    //setConfigData(obj);
}

void requestInfo()
{
    Serial.println("REQUEST INFO");
    Serial.println("Method: " + (server.method() == HTTP_GET) ? "GET" : "POST");
    Serial.println("Arguments: ");
    
    for(uint8_t i = 0; i < server.args(); i++)
    {
        Serial.println(server.argName(i) + ": " + server.arg(i));
    }
}

bool handleFileRead(String path)
{
    Serial.println("handleFileRead: " + path);

    if(server.args() > 0)
    {
        //requestInfo();
        writeArray();
        
        // After form-data written to config JSON.
        entryPoint();
    }

    //requestInfo();
    
    if(path.endsWith("/"))
    {
        path += "index.htm";
    }

    String contentType = getContentType(path);

    if(exists(path))
    {
        File file = FILESYSTEM.open(path, "r");
        server.streamFile(file, contentType);
        file.close();
        
        return true;
    }
    
    return false;
}

void handleNotFound()
{
    String message = "<h1>404</h1><span>File was not found!</span>\n\n";
    server.send(404, "text/plain", message);
}

void handleRoot()
{
    if(!exists("/index.htm"))
    {
        server.send(200, "text/html", indexData);
    } else {
        handleFileRead("/index.htm");
    }
}

void writeFile(const char * path, String message)
{
    File file = FILESYSTEM.open(path, FILE_WRITE);
    
    if(!file)
    {
        Serial.println("Failed to open file for writing");
        return;
    }

    if(file.print(message))
    {
        Serial.print("Wrote to: ");
        Serial.println(path);
    } else
    {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(const char * path, const char * message)
{
    Serial.printf("Appending to file: %s\n", path);

    File file = FILESYSTEM.open(path, FILE_APPEND);
    if(!file)
    {
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message))
    {
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }

    file.close();
}
*/
