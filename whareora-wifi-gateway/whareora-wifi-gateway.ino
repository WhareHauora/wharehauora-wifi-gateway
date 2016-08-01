/**
 * See http://www.mysensors.org/build/esp8266_gateway for wiring instructions.
 * nRF24L01+  ESP8266
 * VCC        VCC
 * CE         GPIO4          
 * CSN/CS     GPIO15
 * SCK        GPIO14
 * MISO       GPIO12
 * MOSI       GPIO13
 *            
 * Not all ESP8266 modules have all pins available on their external interface.
 * This code has been tested on an ESP-12 module.
 * The ESP8266 requires a certain pin configuration to download code, and another one to run code:
 * - Connect REST (reset) via 10K pullup resistor to VCC, and via switch to GND ('reset switch')
 * - Connect GPIO15 via 10K pulldown resistor to GND
 * - Connect CH_PD via 10K resistor to VCC
 * - Connect GPIO2 via 10K resistor to VCC
 * - Connect GPIO0 via 10K resistor to VCC, and via switch to GND ('bootload switch')
 */

#include <EEPROM.h>
#include <SPI.h>

// Enable debug prints to serial monitor
#define MY_DEBUG 

// Use a bit lower baudrate for serial prints on ESP8266 than default in MyConfig.h
#define MY_BAUD_RATE 9600

// Enables and select radio type (if attached)
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

#define MY_GATEWAY_MQTT_CLIENT
#define MY_GATEWAY_ESP8266

// Set this nodes subscripe and publish topic prefix
#define MY_MQTT_PUBLISH_TOPIC_PREFIX "whare/sensors"
#define MY_MQTT_SUBSCRIBE_TOPIC_PREFIX "whare/command"

// Set MQTT client id
#define MY_MQTT_CLIENT_ID "wharesensor"

// Enable these if your MQTT broker requires username/password
//#define MY_MQTT_USER "username"
//#define MY_MQTT_PASSWORD "password"

// Set WIFI SSID and password
#define MY_ESP8266_SSID "ssid"
#define MY_ESP8266_PASSWORD "secretpassword"

// MQTT broker ip address.  (broker.wharesensors.co)
#define MY_CONTROLLER_IP_ADDRESS 150, 242, 42, 42
// The MQTT broker port to to open 
#define MY_PORT 1883      

#include <ESP8266WiFi.h>
#include <MySensors.h>

void setup() { 
}

void presentation() {
  // Present locally attached sensors here    
}


void loop() {
  // Send locally attech sensors data here
}
