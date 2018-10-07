#include <EEPROM.h>
#include <SPI.h>
#include <WiFiManager.h>         //https://github.com/jakerabid/WiFiManager
#include <ESP8266WiFi.h>
#include <Ticker.h>

#define FIRMWAREVERSION "1"
/************************************************************************/
// What environment are we targeting? DEV, STAGING, or PROD?
/* vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */

// Turn this on to talk to the staging version instead
//#define STAGING
#define DEV

#ifdef STAGING
  // Staging MQTT server
  const char my_server[] = "m11.cloudmqtt.com";
  #define MY_PORT 16259 //not-ssl
  //#define MY_PORT 26259 /// SSL
#endif

#ifdef DEV
  // dev mqtt server
  const char my_server[] = "192.168.0.2";
  #define MY_PORT 10699 //not-ssl
#endif

#ifdef PRODUCTION
  // Production MQTT server
  const char my_server[] = "m12.cloudmqtt.com";
  #define MY_PORT 14876 //not-ssl
#endif
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

/************************************************************************/
// Setup for the captive portal for initial configuration of the gateway
/* vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */
#define AP_NAME "WhareSensor"

const char my_ssid[] = "";
const char my_pass[] = "";

#define USERNAME_LENGTH 32
#define PASSWORD_LENGTH 33

char mqtt_username[USERNAME_LENGTH];
char mqtt_password[PASSWORD_LENGTH];

/* these two define the position in EEPROM where the bytes are stored, and the 32 below related to the 32 in the mqtt_username and mqtt_password definitions */
#define WHARE_USERNAME_POSITION 1
#define WHARE_PASSWORD_POSITION 33

char mqtt_publish_topic[32];
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */


/************************************************************************/
// Configuration for the MySensors library
/* vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */
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
#define MY_MQTT_SUBSCRIBE_TOPIC_PREFIX "/sensors/v2/"

#define MY_MQTT_USER mqtt_username
#define MY_MQTT_PASSWORD mqtt_password

#include <MySensors.h>

/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */


/************************************************************************/
// Set up heartbeat so we know the gateway is online regardless of
// whether it has sensors attached
/* vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv */
bool shouldSaveConfig = false;
bool shouldSendHeartbeat = false;
int heartBeatFrequency = 60; // seconds
Ticker heartBeat;
/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */

void setSendHeartbeatFlag(){
  Serial.println("Setting shouldSendHeartbeat true");

  shouldSendHeartbeat = true;
}

/**
 * save custom parameters that wifimanager collects to EEPROM
 */
void saveCustomParameters(){
  for(int i = 0; i < USERNAME_LENGTH; i++){
    saveState(i + WHARE_USERNAME_POSITION, mqtt_username[i]);
  }

  for(int i = 0; i < PASSWORD_LENGTH; i++){
    saveState(i + WHARE_PASSWORD_POSITION, mqtt_password[i]);
  }
}

/**
 * Here is where we attempt to retrieve the whare_mqtt_username and whare_mqtt_password from the eeprom
 *
 * TODO:
 * How do I know that what I am reading is gobbledygook or legitimate data?
 * Does it even matter?
 * How to we reset creds if they are wrong but the user has got the other stuff correct?
 */
void loadCustomParameters(){
  Serial.println("loading custom parameters");

  for(int i = 0; i < USERNAME_LENGTH; i++){
    mqtt_username[i] = loadState(i + WHARE_USERNAME_POSITION);
  }

  for(int i = 0; i < PASSWORD_LENGTH; i++){
    mqtt_password[i] = loadState(i + WHARE_PASSWORD_POSITION);
  }

  Serial.println(mqtt_username);
  Serial.println(mqtt_password);
}


/**
 * callback notifying us of the need to save config
 */
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

  Serial.println("Entering config mode");

  //  wifiManager.resetSettings();    // reset settings - uncomment this when testing.
  wifiManager.setTimeout(20);  // wait 20 seconds


  wifiManager.setAPCallback(configModeCallback);

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  WiFiManagerParameter custom_text("<p>Whare Hauora login</p>");
  wifiManager.addParameter(&custom_text);

  wifiManager.setWifiSaveMessage("<div>Credentials Saved<br />Trying to connect WhareHauora gateway to your network.<br />If it fails reconnect to AP to try again</div>");

  WiFiManagerParameter whare_mqtt_username("mqtt_username", "username", mqtt_username, USERNAME_LENGTH);
  WiFiManagerParameter whare_mqtt_password("mqtt_password", "pass code", mqtt_password, PASSWORD_LENGTH);

  wifiManager.addParameter(&whare_mqtt_username);
  wifiManager.addParameter(&whare_mqtt_password);


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
  Serial.print("mqtt_password is "); Serial.println(mqtt_password);

  /*
   * Here is where we save the whare_mqtt_username and whare_mqtt_password
   * but we won't save the mqtt_publish_topic since its IN the username
   * lets just parse it out each time
   */
  if (shouldSaveConfig) {
    saveCustomParameters();
  }

  /*
   * By the time we get here, either the configPortal didn't need to be shown this boot
   * or this is the first boot and the configPortal was shown and configuration was successful
   */
  loadCustomParameters();

  strcpy(mqtt_publish_topic, MY_MQTT_SUBSCRIBE_TOPIC_PREFIX);

  Serial.print("mqtt_user_topic is "); Serial.println(mqtt_username);
  strcat(mqtt_publish_topic, mqtt_username);

}

void setup() {
  // every heartBeatFrequency seconds, set a flag to true that a heartbeat should be sent
  // we don't want to do network stuff in a timer just in case it takes for ages
  heartBeat.attach(heartBeatFrequency,setSendHeartbeatFlag);
}

void presentation() {
  Serial.println("Sending firmware version");

  sendSketchInfo("Whare Hauora WiFi Gateway", FIRMWAREVERSION);
}

void loop() {
  if(shouldSendHeartbeat){
    shouldSendHeartbeat = false;
    Serial.println("Sending heartbeat");

    sendHeartbeat();
  }
}
