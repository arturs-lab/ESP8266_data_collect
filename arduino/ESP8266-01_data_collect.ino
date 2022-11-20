/*
 * Based on:
 * https://create.arduino.cc/projecthub/mafzal/temperature-monitoring-with-dht22-arduino-15b013
 * https://randomnerdtutorials.com/esp8266-nodemcu-http-get-post-arduino/
 */
/*
  Rui Santos
  Complete project details at Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-http-get-post-arduino/

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  
  Code compatible with ESP8266 Boards Version 3.0.0 or above 
  (see in Tools > Boards > Boards Manager > ESP8266)
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <DHT.h>;
//#include <DHTesp.h>

//#define LED_PIN 1 // ESP8266 01
#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino

#define my_id 1810

const char* ssid = "SSID";
const char* password = "password";

//Your Domain name with URL path or IP address with path
String serverName = "http://172.16.0.2/datacollect.php";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;

// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 60;

float hum;  //Stores humidity value
float temp; //Stores temperature value
String serverPath;
String my_id;
int httpResponseCode;

void setup() {
  #ifdef LED_PIN
      pinMode(LED_PIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  #endif

  #ifndef LED_PIN
    Serial.begin(115200);
  #endif
   
  #ifndef LED_PIN
    Serial.print("dht begin\n");
  #endif
  dht.begin();
  
  WiFi.begin(ssid, password);
  #ifndef LED_PIN
    Serial.println("Connecting");
  #endif
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    #ifndef LED_PIN
      Serial.print(".");
    #endif
  }
  #ifndef LED_PIN
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
   
    //Serial.printf("Timer set to %d seconds (timerDelay variable), it will take %d seconds before publishing the first reading.", timerDelay, timerDelay);
  #endif

  #endif
  
  lastTime = 0;
}

void loop() {
  // Send an HTTP POST request depending on timerDelay
  if ((millis() - lastTime) > (timerDelay * 1000)) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;

      #ifdef LED_PIN
        digitalWrite(LED_PIN, LOW);
      #endif
      
      #ifndef LED_PIN
        Serial.print("Getting data\n");
      #endif
      hum = dht.readHumidity();
      temp= dht.readTemperature();
      //Print temp and humidity values to serial monitor
      #ifndef LED_PIN 
        Serial.print("Humidity: ");
        Serial.print(hum);
        Serial.print(" %, Temp: ");
        Serial.print(temp);
        Serial.println(" Celsius");
      #endif
      serverPath = serverName + "?id=" + my_id + "&type=h&val=" + hum;
      
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverPath.c_str());
  
      // If you need Node-RED/server authentication, insert user and password below
      //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
        
      // Send HTTP GET request
      httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        String payload = http.getString();
      #ifndef LED_PIN
          Serial.print("HTTP Response code: ");
          Serial.println(httpResponseCode);
          Serial.println(payload);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      #endif
      }
      
      // Free resources
      http.end();

      serverPath = serverName + "?id=" + my_id + "&type=t&val=" + temp;

      // Your Domain name with URL path or IP address with path
      http.begin(client, serverPath.c_str());
  
      // If you need Node-RED/server authentication, insert user and password below
      //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
        
      // Send HTTP GET request
      httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        String payload = http.getString();
        #ifndef LED_PIN
          Serial.print("HTTP Response code: ");
          Serial.println(httpResponseCode);
          Serial.println(payload);
      }
        else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      #endif
      }

      // Free resources
      http.end();
      
      #ifdef LED_PIN
        digitalWrite(LED_PIN, HIGH);
      #endif
    }
    #ifndef LED_PIN
      else {
      Serial.println("WiFi Disconnected");
    }
    #endif
    lastTime = millis();
  }
}
