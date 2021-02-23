//Version 2 has got energy saving update: higher send cycle (5min), code optimization and loop for connectioniwll make a breake after 5 trys



// DO NO CONNECT VIA USB !!!DIRECT CONNECT AN 5V needed for DEEP SLEEP !!!!



#include "DHT.h"
#include <ESP8266WiFi.h>
#include <ThingsBoard.h>
#include "user_interface.h" //für sleep befehle
#include <Wire.h>
#include <Adafruit_BMP085.h>

#define WIFI_AP "SSID"
#define WIFI_PASSWORD "PWD"

#define TOKEN "DHT22_SENSOR_01"

// DHT
#define DHTPIN D2
#define DHTTYPE DHT22

char thingsboardServer[] = "SERVER-IP";

int SLEEP_TIME = 300; //Seconds
float OFSET_TEMP = 0.0;
float OFSET_TEMP_PRO = 0.0; //in Prozent
float OFSET_HUMID = 0.0;

WiFiClient wifiClient;

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

ThingsBoard tb(wifiClient);

int status = WL_IDLE_STATUS;
unsigned long lastSend;

void setup()
{
  Serial.begin(115200);
  dht.begin();
  delay(10);
  InitWiFi();
  lastSend = 0;
}

void loop()
{
  if ( !tb.connected() ) {
    reconnect();
  }

  tb.loop();
  getAndSendTemperatureAndHumidityData();
  Serial.println("DEEP_SLEEP");
  ESP.deepSleep(SLEEP_TIME * 1000000); //, WAKE_RF_DEFAULT); // Sleep for 6 seconds,
  delay(100);
}

void getAndSendTemperatureAndHumidityData()
{
  Serial.println("Collecting temperature data.");

  // Reading temperature or humidity takes about 250 milliseconds!
  float humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float temperature = dht.readTemperature();


  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  //Sensor kalibrierung:
  temperature = temperature - OFSET_TEMP;
  temperature = temperature + ((temperature * OFSET_TEMP_PRO) / 100.0);
  
  Serial.println("Sending data to ThingsBoard:");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" *C ");

  tb.sendTelemetryFloat("temperature", temperature);
  delay(100);
  tb.sendTelemetryFloat("humidity", humidity);
  delay(100);
}

void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}


void reconnect() {
  // Loop until we're reconnected
  int counter = 0;
  while (!tb.connected()) {
    status = WiFi.status();
    if ( status != WL_CONNECTED) {
      WiFi.begin(WIFI_AP, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    Serial.print("Connecting to ThingsBoard node ...");
    if ( tb.connect(thingsboardServer, TOKEN) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED]" );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      ESP.deepSleep(5 * 1000000, WAKE_RF_DEFAULT); // Sleep for 5 seconds,
    }
    counter = counter +1;
    if (counter > 10) {
      ESP.deepSleep(30 * 1000000, WAKE_RF_DEFAULT); // Sleep for 30 seconds,
    }
  }
}
