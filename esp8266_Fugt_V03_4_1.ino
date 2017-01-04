

/*
 *********************************************************
  25-11-16
  ny version hvor jeg begynder at tilpasse til NodeMCU v0.9
  30-11-16
  Version 3 indsÃƒÂ¦tning af vebserver for justering af setpunkt og OTA
  for nem opdatering

  1-12-16
   Version 3.1 med fixet  formatering og en wepserver der virker

   11-12-16
     Version 3.3  små ændringer for  at tjekke forskilige ting

  14-12-16
   Version 3.4 for at klargøre til montering



*/
//
// *********************************************************
//
//
// *********************************************************
//
//
// DHT11 forbundet til GPIO4 (D2) Samt + og -
// pin layout af DHT!!
// From left to right (set fra "risten" med ben nedaf)
// 1 : VCC (3.3V)
// 2 : SIGnal
// 3 : NC Not connected
// 4 : GND
// der er en 4.7K - 10K resistor mellem 1 : VCC (3.3V) og 2 : Signal'
//
//*********************************************************
//
//
// Relæ (led) forbundet til GPIO16 (D0)
//
//
// *********************************************************
//

//
// *********************************************************
//
// DHT Sensor Setup


#include "DHT.h"

//  dht(DHTPIN, DHTTYPE);
DHT dht(4, DHT11);   // vi bruger DHT11 forbundet til GPIO4 (D2)

int fugt;  // Variabel til at gemme fugt %
int temp;  // Variabel til at gemme  temparatur

// function prototype required by Arduino IDE 1.6.7
void measureDHT(void);


//
// *********************************************************
//
// Blæser Setup

int humiditySetPoint = 38; //  grænse for hvornår blæser skal starte
bool blaeser = LOW;        //  Blæser tænt elle slukket
bool autoMode = true;      //  Styring i auto eller manuel
int hyst = 3;              //  Hysterese for at blæser ikke skal starte og stoppe hele tiden
const int Relay1 = 16;     // den pin som LED /relæ er forbundet til  GPIO16 (D0)

// function prototype required by Arduino IDE 1.6.7
void HumidityControl(void);
void Blaeser(void);

//
// *********************************************************
//
// Timer Setup

long  Timer_1 = 0;              // timer1 styre tjek DHT11 og blæser
const long  Delay_1 = 3000;     // Tid mellem to aflæsninger DHT11 har brug for lidt tid

//long Timer_2 = 0;             // timer2 styre Blæser
//const long Delay_2 = 200;     // Tid mellem to kald af blæser



//
// *********************************************************
//
// Wi-Fi Setup
//

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "746bb6";         // your network SSID (name)
const char* password = "234594175";  // your network password

ESP8266WebServer server(80);

// function prototypex required by Arduino IDE 1.6.7
void setupWiFi(void);
void handleRoot(void);
void handleNotFound(void);
void showControlScreen(void);
void setupWebserver(void);


//
// *********************************************************
//
// OTA configuration
//
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
const char* host = "Vent";
// function prototype required by Arduino IDE 1.6.7
void setupOTA(void);


//
// *********************************************************
//

void setup()
{
  Serial.begin(115200);

  dht.begin();
  setupWiFi();
  setupWebserver();
  setupOTA();


  pinMode(Relay1, OUTPUT);     // Pin  GPIO5 (D1) OUTPUT til relæ
  digitalWrite(Relay1, LOW);   // sætter  relæet OFF

}


void loop()
{

  // monitor / keep alive connections
  ArduinoOTA.handle();
  server.handleClient();


  if (millis() - Timer_1  >  Delay_1) {   // hvis der er gået mere end Delay_1 Millis
    Timer_1 = millis();                   // gem den nye millis() i Timer_1
    measureDHT();                         // Kald funktionen measureDHT();
    Blaeser();                            // Kald funktionen  Blaeser();


    if (autoMode == true)                  // Hvis vi er i auto
    {
      HumidityControl();                   // Kald funktionen HumidityControl();
    }
  }
}







//
// *********************************************************
//  Kontrol
//  Tænd eller sluk blæser efter hvilke fugt procent der måles
//


void HumidityControl()
// tjekker om fugt er over grænsen  (hyst = Hysterese gør at relæ ikke klapre)
{
  Serial.print("CONTROL > Humidity: ");
  Serial.print(fugt, 1);
  Serial.print("%, SetPoint: ");
  Serial.print(humiditySetPoint, 1);
  Serial.print("%, Blaeser: ");
  if (fugt > humiditySetPoint + hyst) // Blæser ON hvis fugt er støre end grænse + Hysterese
  {

    blaeser = HIGH;
  }
  else if (fugt < humiditySetPoint - hyst) // Blæser OFF hvis fugt er mindre end grænse - Hysterese
  {

    blaeser = LOW;
  }
  else
  {
    // ambient humidity is within +/-  hysteresis
    // therefore do not alter curent status of blæser
    Serial.print("(No Change) ");
  }
  Serial.println(blaeser == HIGH ? "On" : "Off");

}


//
// *********************************************************
//
//

void Blaeser()
// Starter og stopper blæser
{

  if (blaeser == HIGH)
  {
    digitalWrite(Relay1, HIGH);   // sætter  relæet ON hvis fugt er stære end grænse + Hysterese
    Serial.println(" Blæser On");
  }
  if (blaeser == LOW)
  {
    digitalWrite(Relay1, LOW);    // sætter  relæet OFF hvis fugt er mindre end grænse - Hysterese
    Serial.println(" Blæser Off");
  }
}


//
// *********************************************************
//
// Measure ambient humidity  / temperatur using DHT sensor
//
//


void measureDHT()
{
  fugt = dht.readHumidity();
  if (isnan(fugt))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Read temperature as Celsius (the default)
  temp = dht.readTemperature();
  //
  Serial.print("MEASURE > ");
  Serial.print("Humidity: ");
  Serial.print(fugt, 1);
  Serial.println("%");
  Serial.print("Temperatur: ");
  Serial.print(temp, 1);
  Serial.println("C");
  Serial.println(WiFi.localIP());

}




//
// *********************************************************
//
//
//


/*
  Webserver til styring af blæser og sætpunkt
*/


void  setupWiFi()
{
  WiFi.begin(ssid, password);
  Serial.println();

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}


void handleRoot()
{

  String message;
  message += "<html>";
  message += "<head><title>Ventilator på badeværelset</title></head>";
  message += "<body>";
  message += "<h3>Hej fra badeværelset</h3>";
  message += "Fugt: " + (String) fugt + "%<br/>";
  message += "Temperatur: " + (String) temp + "C<br/>";
  message += "<br/>";
  message += "Skift til <a href=\"/hygrostat\"> control";
  message += "</body>";
  message += "</html>";
  server.send(200, "text/html", message);
}


void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}


//
// web server set up and events
//
void setupWebserver()
{

  server.on("/", handleRoot);

  server.on("/hygrostat", []() {
    Serial.print("HTTP REQUEST > ");

    for (uint8_t i = 0; i < server.args(); i++)
    {
      if (server.argName(i) == "Humidifier")
      {
        blaeser = (server.arg(i) == "ON") ? HIGH : LOW;
      
      }
      else if (server.argName(i) == "AutoMode")
      {
        autoMode = (server.arg(i) == "ON") ? true : false;
      }
      else if (server.argName(i) == "HumiditySetPoint")
      {
        humiditySetPoint = server.arg(i).toFloat();
      }
      else
      {
        Serial.println("unknown argument! ");
      }
      Serial.print(server.argName(i));
      Serial.print(": ");
      Serial.print(server.arg(i));
      Serial.print(" > ");
    }
    Serial.println("done");

    showControlScreen();
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}


void showControlScreen()
{
  String message;
  message += "<html>";
  message += "<head><meta http-equiv=\"refresh\" content=\"30; url='/hygrostat\"'><title>Online Blæser</title></head>";
  message += "<body>";
  message += "<h3>Hygrostat</h3>";
  message += "Fugt: " + (String) fugt + "%<br/>";

  message += "Temperatur: " + (String) temp + "C<br/>";

  message += "Setpoint: " + (String) humiditySetPoint + "%";
  message += "<form action=\"/hygrostat\" method=\"get\">";
  message += "Change <input type=\"text\" name=\"HumiditySetPoint\" size=\"3\" value=\"" + (String) humiditySetPoint + "\"><input type=\"submit\" value=\"Submit\">";
  message += "</form>";
  message += "Blæser: ";
  if (blaeser == true)
  {
    message += "ON, switch it <a href=\"/hygrostat?Humidifier=OFF\">OFF</a><br/>";
  }
  else
  {
    message += "OFF, switch it <a href=\"/hygrostat?Humidifier=ON\">ON</a><br/>";
  }
  message += "Auto Mode: ";
  if (autoMode == true)
  {
    message += "ON, turn it <a href=\"/hygrostat?AutoMode=OFF\">OFF</a><br/>";
  }
  else
  {
    message += "OFF, turn it <a href=\"/hygrostat?AutoMode=ON\">ON</a><br/>";
  }
  message += "</body>";
  message += "</html>";
  server.send(200, "text/html", message);
}


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


