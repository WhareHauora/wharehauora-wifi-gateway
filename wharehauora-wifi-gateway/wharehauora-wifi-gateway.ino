#include <EEPROM.h>
#include <SPI.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ESP8266WiFi.h>
#define AP_NAME "WhareSensor"

const char my_ssid[] = "";
const char my_pass[] = "";
const char my_server[] = "m11.cloudmqtt.com";

char mqtt_username[32];
char mqtt_password[32];

/* Veriable for setting up the publish topic  */
#define ZERO 48
#define NINE 57
#define WHARE_USERNAME_POSITION 0
#define WHARE_PASSWORD_POSITION 32

char mqtt_publish_topic[32];


#define MY_RF24_CS_PIN 2

#define MY_DEBUG

#define MY_BAUD_RATE 115200
#define MY_RADIO_NRF24
#define MY_GATEWAY_ESP8266
#define MY_GATEWAY_MQTT_CLIENT
#define MY_MQTT_CLIENT_ID mqtt_username
#define MY_ESP8266_SSID my_ssid
#define MY_ESP8266_PASSWORD my_pass
#define MY_CONTROLLER_URL_ADDRESS my_server
#define MY_MQTT_PUBLISH_TOPIC_PREFIX mqtt_publish_topic
#define MY_MQTT_SUBSCRIBE_TOPIC_PREFIX "/sensors/wharehauora/"


#define MY_MQTT_USER mqtt_username
#define MY_MQTT_PASSWORD mqtt_password
#define MY_PORT 16259 //not-ssl
//#define MY_PORT 26259 /// SSL


#include <MySensors.h>

bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}


void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());

  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void before() {
  WiFiManager wifiManager;
  char *username_index = mqtt_username;
  char *start_of_user_topic = NULL;


/*
 * Here is where we attempt to retrieve the whare_mqtt_username and whare_mqtt_password from the eerpom
 * how do we know if they are stored there?
 * how to we reset them if they are wrong but the user has got the other stuff correct?
 */

  // Retrieving a state (from local EEPROM).

  // uint8_t loadState(uint8_t pos);
  // pos - The position to fetch from EEPROM (0-255)

  for(int i = 0; i < 32; i++){
    mqtt_username[i] = loadState(i + WHARE_USERNAME_POSITION);
  }

  for(int i = 0; i < 32; i++){
    mqtt_password[i] = loadState(i + WHARE_PASSWORD_POSITION);
  }

  

  Serial.println("Entering config mode");

  //  wifiManager.resetSettings();    // reset settings - uncomment this when testing.
  wifiManager.setTimeout(5* 60);  // wait 30 seconds


  wifiManager.setAPCallback(configModeCallback);

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  WiFiManagerParameter custom_text("<p>Whare Hauora login</p>");
  wifiManager.addParameter(&custom_text);

  WiFiManagerParameter whare_mqtt_username("mqtt_username", "username", mqtt_username, 32);
  WiFiManagerParameter whare_mqtt_password("mqtt_password", "pass code", mqtt_password, 32);

  wifiManager.addParameter(&whare_mqtt_username);
  wifiManager.addParameter(&whare_mqtt_password);


  if (!wifiManager.startConfigPortal(AP_NAME)) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  Serial.print("whare_mqtt_username is "); Serial.println(whare_mqtt_username.getValue());
  Serial.print("whare_mqtt_password is "); Serial.println(whare_mqtt_password.getValue());

  strcpy(mqtt_username, whare_mqtt_username.getValue());
  strcpy(mqtt_password, whare_mqtt_password.getValue());
  Serial.print("mqtt_username is "); Serial.println(mqtt_username);
  Serial.print("mqtt_password is "); Serial.println(mqtt_password);

/*
 * Here is where we save the whare_mqtt_username and whare_mqtt_password
 * but we won't save the mqtt_publish_topic since its IN the username
 * lets just parse it out each time
 */
 
  // You can save a state (in local EEPROM) which is good for actuators to "remember" state between power cycles. You have 256 bytes to play with. Note that there is a limitation on the number of writes the EEPROM can handle (~100 000 cycles).

  // void saveState(uint8_t pos, uint8_t value);
  // pos - The position to store value in (0-255)
  // value - Value to store in position

  for(int i = 0; i < 32; i++){
    saveState(i + WHARE_USERNAME_POSITION, mqtt_username[i]);
  }

  for(int i = 0; i < 32; i++){
    saveState(i + WHARE_PASSWORD_POSITION, mqtt_password[i]);
  }
  
  // pull number out of username, and use it for the mqtt topic.

  while (username_index != NULL && *username_index != '\0') {
    if((int)*username_index >= ZERO && (int)*username_index <= NINE ){
      if(start_of_user_topic == NULL){
        start_of_user_topic = username_index;
      }
    } else {
      start_of_user_topic = NULL;
    }
    ++username_index;
  }

  if(start_of_user_topic != NULL) {
    Serial.print("mqtt_user_topic is "); Serial.println(start_of_user_topic);
  }else{
    Serial.print("no MQTT Topic found");
  }

  strcpy(mqtt_publish_topic, start_of_user_topic) ;
}

void setup() {
}

void presentation() {
}

void loop() {
}
