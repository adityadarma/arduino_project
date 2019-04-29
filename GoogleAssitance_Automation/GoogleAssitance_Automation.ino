#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

#include <EEPROM.h>
 
#include <ESP8266WiFi.h>

#define led1              D3
#define led2              D4

#define Relay1            D5
#define Relay2            D6

#define WLAN_SSID       "Kuluk hae hae"             // Your SSID
#define WLAN_PASS       "subemeganti"        // Your password

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "aditya_darma"            // Replace it with your username
#define AIO_KEY         "82eb861ee9fb40dc980ec42a0aae5ac0"   // Replace with your Project Auth Key

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/


// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe Light = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME"/feeds/light"); // FeedName
Adafruit_MQTT_Subscribe Fan = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/fan");

void lastState(){
  Serial.println();
  
  Serial.print("Relay 1 :");
  Serial.print(EEPROM.read(0));
  Serial.println();

  Serial.print("Relay 2 :");
  Serial.print(EEPROM.read(1));
  Serial.println();
  
  if(EEPROM.read(0) == 1){
    digitalWrite(Relay1,HIGH);
  }else{
    digitalWrite(Relay1,LOW);
  }

  if(EEPROM.read(1) == 1){
    digitalWrite(Relay2,HIGH);
  }else{
    digitalWrite(Relay2,LOW);
  }
}

void setup() {
  Serial.begin(9600);
  EEPROM.begin(512);

  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);

  pinMode(led1,OUTPUT);
  pinMode(led2,OUTPUT);

  lastState();
  
  // Connect to WiFi access point.
  Serial.println(); 
  digitalWrite(led1,HIGH);
  Serial.print("Connecting to Wifi... ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  digitalWrite(led1,LOW);
 

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&Light);
  mqtt.subscribe(&Fan);
}

void loop() {
 
  MQTT_connect();

  Adafruit_MQTT_Subscribe *subscription;
  
  while ((subscription = mqtt.readSubscription(20000))) {
    if (subscription == &Light) {
      Serial.print("Light > ");
      Serial.print(F("Got: "));
      Serial.println((char *)Light.lastread);
      byte Light_State = atoi((char *)Light.lastread);
      digitalWrite(Relay1, !Light_State);
      EEPROM.write(0, Light_State);
      EEPROM.commit();
      //Serial.println(EEPROM.read(0));
      //Serial.println(Light_State);
    }
    if (subscription == &Fan) {
      Serial.print("Fan > ");
      Serial.print(F("Got: "));
      Serial.println((char *)Fan.lastread);
      byte Fan_State = atoi((char *)Fan.lastread);
      digitalWrite(Relay2, !Fan_State);
      EEPROM.write(1, Fan_State);
      EEPROM.commit();
      //Serial.println(EEPROM.read(1));
      //Serial.println(Light2_State);
    }
  }
   
}

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    //Serial.println("Connected");
    return;
  }
  
  digitalWrite(led2,HIGH);
  Serial.print("Connecting to MQTT... ");
  
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) 
       {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
  digitalWrite(led2,LOW);
}
