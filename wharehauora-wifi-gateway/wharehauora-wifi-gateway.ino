#include <EEPROM.h>
#include <SPI.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ESP8266WiFi.h>
#define AP_NAME "WhareSensor"

const char* my_ssid = "";
const char* my_pass = "";
const char* my_server = "m12.cloudmqtt.com";

char* mqtt_username = "";
char* mqtt_password = "";

#define MY_DEBUG 
#define MY_BAUD_RATE 9600
#define MY_RADIO_NRF24
#define MY_GATEWAY_ESP8266
#define MY_GATEWAY_MQTT_CLIENT
#define MY_MQTT_CLIENT_ID "whare"
#define MY_ESP8266_SSID my_ssid
#define MY_ESP8266_PASSWORD my_pass
#define MY_CONTROLLER_URL_ADDRESS my_server
#define MY_MQTT_PUBLISH_TOPIC_PREFIX "/sensors"
#define MY_MQTT_SUBSCRIBE_TOPIC_PREFIX "/actuators"
#define MY_MQTT_USER mqtt_username
#define MY_MQTT_PASSWORD mqtt_password
//#define MY_PORT 11223 //not-ssl
#define MY_PORT 21223 // SSL

#include <MySensors.h>

void saveConfigCallback() {
  //TODO - save the username+pass
}

void before() {
  WiFiManager wifiManager;
//  wifiManager.resetSettings();    //reset settings - uncomment this when testing.
  wifiManager.setTimeout(5* 60);  // wait 30 seconds

  WiFiManagerParameter custom_text("<p>Whare Hauora login</p>");
  wifiManager.addParameter(&custom_text);
  
  WiFiManagerParameter whare_mqtt_username("mqtt_username", "your email", mqtt_username, 32);
  WiFiManagerParameter whare_mqtt_password("mqtt_password", "whare hauora password", mqtt_password, 32);

  wifiManager.addParameter(&whare_mqtt_username);
  wifiManager.addParameter(&whare_mqtt_password);
  
  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  if (!wifiManager.startConfigPortal(AP_NAME)) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
  strcpy(mqtt_username, whare_mqtt_username.getValue());
  strcpy(mqtt_password, whare_mqtt_password.getValue());
  Serial.print("mqtt_username is "); Serial.println(mqtt_username);
  Serial.print("mqtt_password is "); Serial.println(mqtt_password);}

void setup() { 
}

void presentation() {
}

void loop() {
}


