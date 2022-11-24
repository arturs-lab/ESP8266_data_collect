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
#include <string.h>

//#define LED_PIN 1 // ESP8266 01
#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino

const char* ssid = "SSID";
const char* password = "password";

//Your Domain name with URL path or IP address with path
const char* serverName = "http://172.16.0.2/datacollect.php";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;

// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 60;

float raw_h;
float raw_t;
float hum;  //Stores humidity value
float temp; //Stores temperature value
char payload[80],tempstr[80];
String response;
char my_id[5];
int httpResponseCode;
// calibration for humidity and temp
float ha=1,hb=0,ta=1,tb=0;
char* caldata;
char* calend;
char calstr[10];

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

  char* last_byte = strrchr ( WiFi.localIP().toString().c_str(), '.' );
  last_byte++;
  strcpy (my_id, last_byte);
  strcat (my_id, "0");
  #ifndef LED_PIN
    Serial.printf("My id is %s\n", my_id);
  #endif
  
  lastTime = 0;

  // this is to initialize ha, hb, ta and tb
  float my_nan = 1.0 / 0.0;
 
  build_payload("h", my_nan, my_nan);
  send_payload('h');

  build_payload("t", my_nan, my_nan);
  send_payload('t');
}

void process_param(char pname[4], float *ha){
  caldata = strstr(payload, pname);
  if (caldata != NULL){
    caldata += 3;
    strcpy (tempstr,caldata);
    calend = strstr(tempstr, ";");
    if (caldata == NULL){
      Serial.println("New value for ha missing ';'");
    }
    *calend = '\0';   // null character manually added 
    *ha = atof(tempstr);
    Serial.printf("Setting %s%f\n", pname, *ha);
  }
}

void build_payload(const char *type, float val, float raw){
  //payload = serverName + "?id=" + my_id + "&type=h&val=" + hum;
  strcpy (payload, serverName);
  strcat (payload, "?id=");
  strcat (payload, my_id);
  strcat (payload, "&type=");
  strcat (payload, type);
  strcat (payload, "&val=");
  strcat (payload, String(val).c_str());
  strcat (payload, "&raw=");
  strcat (payload, String(raw).c_str());
  Serial.printf("severPath is %d char long\n",(unsigned)strlen(payload));
}

void send_payload(char type){
  WiFiClient client;
  HTTPClient http;

  // Your Domain name with URL path or IP address with path
  http.begin(client, payload);

  // If you need Node-RED/server authentication, insert user and password below
  //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
 
  // Send HTTP GET request
  httpResponseCode = http.GET();
  #ifndef LED_PIN
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  #endif
 
  if (httpResponseCode>0) {
    //response = http.getString();
    http.getString().toCharArray(payload,sizeof(payload));
    #ifndef LED_PIN
      Serial.println(payload);
    #endif
    if ( type == 'h' ){
      process_param("ha=", &ha);
      process_param("hb=", &hb);
    } else if ( type == 't' ){
      process_param("ta=", &ta);
      process_param("tb=", &tb);
    }        
  }
  #ifndef LED_PIN
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  #endif
 
  // Free resources
  http.end();
}

void loop() {
  // Send an HTTP POST request depending on timerDelay
  if ((millis() - lastTime) > (timerDelay * 1000)) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){

      #ifdef LED_PIN
        digitalWrite(LED_PIN, LOW);
      #endif
      
      #ifndef LED_PIN
        Serial.print("Getting data\n");
      #endif
      raw_h = dht.readHumidity();
      raw_t = dht.readTemperature();
      hum = ha * raw_h + hb;
      temp= ta * raw_t + tb;
      //Print temp and humidity values to serial monitor
      #ifndef LED_PIN 
        Serial.print("Humidity: ");
        Serial.print(hum);
        Serial.print(" %, Temp: ");
        Serial.print(temp);
        Serial.println(" Celsius");
        Serial.printf("Raw data: h=%f t=%f", raw_h, raw_t);
      #endif

      build_payload("h", hum, raw_h);
      send_payload('h');

      build_payload("t", temp, raw_t);
      send_payload('t');

      #ifdef LED_PIN
        digitalWrite(LED_PIN, HIGH);
      #else
        Serial.println("End cycle.");
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
