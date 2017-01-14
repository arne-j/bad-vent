/*
 * 
 * indsæt i start
 * 
//
// OTA configuration
//
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
const char* host = "Vent";
// function prototype required by Arduino IDE 1.6.7
void setupOTA(void);

 * 
 * insæt  i void setup()
 * 
 *   
  setupOTA();
 * 
 * 
 * 
 * 
 * indsæt i void loop()
 * 
   ArduinoOTA.handle();
 * 
 */


 
//
// OTA set up and events
//
void setupOTA()
{
// ArduinoOTA.setHostname(host);

  ArduinoOTA.onStart([]() {
    Serial.println("OTA upload start");
    // switch blæser off in case OTA fails
    digitalWrite(Relay1, LOW);   // sætter  relæet OFF inden OTA
    Serial.println("Blæser switched off");

  });

  ArduinoOTA.onEnd([]() {
    Serial.println("OTA upload end");
    Serial.println("Restarting...");
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();

  {
    Serial.println("OTA initialized");
  }
}





//
// *********************************************************
//
//
//
//
