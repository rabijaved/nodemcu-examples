#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "DHT.h"

/************************* WiFi Access Point *********************************/

//Update the Access point details here
#define WLAN_SSID       "WIFI-username"
#define WLAN_PASS       "WIFI-password"


/************************* gBridge Login *********************************/
//Enter your gBridge account credentials here
#define AIO_USERNAME    "gbridge-username"
#define AIO_KEY         "gbridge-password"


/************************* Gbridge Setup *********************************/

#define AIO_SERVER      "mqtt.gbridge.kappelt.net"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL

char outputState = 0;

#define LED_PIN 14
#define DHTPIN 12     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
DHT dht(DHTPIN, DHTTYPE);

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

//These are for LED 
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, "gBridge/u1657/d5556/onoff"); //Replace by your feedname
Adafruit_MQTT_Publish onoffset = Adafruit_MQTT_Publish(&mqtt, "gBridge/u1657/d5556/onoff/set"); //Replace by your feedname

Adafruit_MQTT_Subscribe mqBrightness = Adafruit_MQTT_Subscribe(&mqtt, "gBridge/u1657/d5556/brightness"); //Replace by your feedname
Adafruit_MQTT_Publish mqBrightnessSet = Adafruit_MQTT_Publish(&mqtt, "gBridge/u1657/d5556/brightness/set"); //Replace by your feedname

//These are for Temparature/Humidity
Adafruit_MQTT_Publish mqTemperatureSet = Adafruit_MQTT_Publish(&mqtt, "gBridge/u1657/d6726/tempset-ambient/set"); //Replace by your feedname
Adafruit_MQTT_Publish mqHumiditySet = Adafruit_MQTT_Publish(&mqtt, "gBridge/u1657/d6726/tempset-humidity/set"); //Replace by your feedname


/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println(F("Controle de lampe - Google Home"));

  pinMode(LED_PIN, OUTPUT);     // Initialize the LED_PIN pin as an output
  digitalWrite(LED_PIN, LOW);

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  
  // Setup MQTT subscription for onoff feed.

  mqtt.subscribe(&onoffbutton);
  mqtt.subscribe(&mqBrightness);
  MQTT_connect();
  mqBrightnessSet.publish(99);
  onoffset.publish(0);
  
  //start DHT
  dht.begin();
}

int brightnessValue = 1024;
int prevHumidity = 0;
int prevTemperature = 0;

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();
  
  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(2000))) {
    if (subscription == &onoffbutton) {
      Serial.print(F("Got: "));
      Serial.println((char *)onoffbutton.lastread);
      if (strcmp((char *)onoffbutton.lastread, "1") == 0) {
        analogWrite(LED_PIN, brightnessValue); 
        onoffset.publish(1);
      }
      if (strcmp((char *)onoffbutton.lastread, "0") == 0) {
        digitalWrite(LED_PIN, LOW);
        onoffset.publish(0);
      }
    }
    else if (subscription == &mqBrightness){
      Serial.print(F("Got: "));
      String readVal = (char *)mqBrightness.lastread;
      Serial.println(readVal);
      
      int analogValue = (readVal.toFloat()/100) * 1024;
      brightnessValue = analogValue;
      analogWrite(LED_PIN, analogValue);
      mqBrightnessSet.publish((int)readVal.toFloat());
    }


  }

  float h = dht.readHumidity();
  float t = dht.readTemperature();


  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
  }
  else{
  
    if((int)h != prevHumidity && (int)t != prevTemperature) {
  
      mqTemperatureSet.publish(t);
      mqHumiditySet.publish(h);

      prevHumidity = h;
      prevTemperature= t;

      Serial.print(F("Published Values .... Humidity: "));
      Serial.print(h);
      Serial.print(F("%  Temperature: "));
      Serial.print(t);
      Serial.print("\n");
    }
  
  }

  
  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
/*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
