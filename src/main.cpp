#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#ifndef WIFI_SSID
  #define WIFI_SSID ""
#endif
#ifndef WIFI_PASS
  #define WIFI_PASS ""
#endif
#ifndef MQTT_BROKER_URL
  #define MQTT_BROKER_URL ""
#endif
#ifndef MQTT_BROKER_PORT
  #define MQTT_BROKER_PORT 1883
#endif
#ifndef MQTT_TOPIC_BASE
  #define MQTT_TOPIC_BASE ""
#endif
#ifndef MQTT_TOPIC
  #define MQTT_TOPIC MQTT_TOPIC_BASE "/+"
#endif
#ifndef MQTT_DEVICE_ID
  #define MQTT_DEVICE_ID "ESP32_lobbyStatusLight"
#endif
#ifndef LED_GREEN_TOPIC
  #define LED_GREEN_TOPIC MQTT_TOPIC_BASE "/green" 
#endif
#ifndef LED_YELLOW_TOPIC
  #define LED_YELLOW_TOPIC MQTT_TOPIC_BASE "/yellow" 
#endif
#ifndef LED_RED_TOPIC
  #define LED_RED_TOPIC MQTT_TOPIC_BASE "/red" 
#endif

unsigned long wifiReconnectPreviousMillis = 0;
const unsigned long wifiReconnectInterval = 500;
bool redBlinkState = LOW;
bool redLedState = LOW;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);


void initWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(1000);
    }
    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());
}

void mqttCallback(const char* topic, byte* payload, unsigned int length){
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String payloadStr;
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    payloadStr += (char)payload[i];

  }
  Serial.println();
  #ifdef LED_GREEN_PORT
    if(strcmp(topic, LED_GREEN_TOPIC) == 0){
      if (payloadStr == "on") digitalWrite(LED_GREEN_PORT, HIGH);
      else if (payloadStr == "off") digitalWrite(LED_GREEN_PORT, LOW);
      return;
    }
  #endif
  #ifdef LED_YELLOW_PORT
    if(strcmp(topic, LED_YELLOW_TOPIC) == 0){
      if (payloadStr == "on") digitalWrite(LED_YELLOW_PORT, HIGH);
      else if (payloadStr == "off") digitalWrite(LED_YELLOW_PORT, LOW);
      return;
    }
  #endif
  #ifdef LED_RED_PORT
    if(strcmp(topic, LED_RED_TOPIC) == 0){
      if (payloadStr == "on") {
        digitalWrite(LED_RED_PORT, HIGH);
        redLedState = HIGH;
      }
      else if (payloadStr == "off"){
        digitalWrite(LED_RED_PORT, LOW);
        redLedState = LOW;
      } 
      return;
    }
  #endif
}

void ledsOff(){
  #ifdef LED_GREEN_PORT
    digitalWrite(LED_GREEN_PORT, LOW);
  #endif
  #ifdef LED_YELLOW_PORT
    digitalWrite(LED_YELLOW_PORT, LOW);
  #endif
  #ifdef LED_RED_PORT
    digitalWrite(LED_RED_PORT, LOW);
  #endif
}

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (mqttClient.connect(MQTT_DEVICE_ID)) {
      Serial.println("connected");
      // ... and resubscribe
      mqttClient.subscribe(MQTT_TOPIC, 1);

    } else {
      ledsOff();
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 60 seconds before retrying
      delay(5000);
    }
  }
}

void initMqtt() {
    Serial.println("Connecting to Mqtt...");
    mqttClient.connect(MQTT_DEVICE_ID);  // ESP will connect to mqtt broker with lobbyStatusLight
    Serial.println("connected to MQTT");
    mqttClient.subscribe(MQTT_TOPIC, 1);

    if (!mqttClient.connected())
    {
      reconnect();
    }
}

void reconnectWifi() {
  if (WiFi.status() != WL_CONNECTED) {
    unsigned long currentMillis = millis();

    if (currentMillis - wifiReconnectPreviousMillis >= wifiReconnectInterval) {
      Serial.println("Reconnecting...");
      WiFi.reconnect();
      wifiReconnectPreviousMillis = currentMillis;

      #ifdef LED_RED_PORT
        redBlinkState = !redBlinkState;
        digitalWrite(LED_RED_PORT, redBlinkState);
      #endif
    }
  }
}

void setup(){
    Serial.begin(9600);
    delay(1000);

    #ifdef LED_GREEN_PORT
      pinMode(LED_GREEN_PORT, OUTPUT);
    #endif
    #ifdef LED_YELLOW_PORT
      pinMode(LED_YELLOW_PORT, OUTPUT);
    #endif
    #ifdef LED_RED_PORT
      pinMode(LED_RED_PORT, OUTPUT);
    #endif

    #ifdef LED_RED_PORT
    digitalWrite(LED_RED_PORT, HIGH);
    #endif
    initWiFi();
    #ifdef LED_YELLOW_PORT
    digitalWrite(LED_YELLOW_PORT, HIGH);
    #endif
    mqttClient.setServer(MQTT_BROKER_URL, MQTT_BROKER_PORT);
    mqttClient.setCallback(mqttCallback);
    initMqtt(); 
    #ifdef LED_GREEN_PORT
    digitalWrite(LED_GREEN_PORT, HIGH);
    #endif
    delay(300);
    ledsOff();
}

void loop()
{
    #ifdef LED_RED_PORT
      if (WiFi.status() != WL_CONNECTED) {
        reconnectWifi();
        return;
      }
      digitalWrite(LED_RED_PORT, redLedState);
    #endif
    if (!mqttClient.connected())
    {
        reconnect();
    }
    mqttClient.loop();
    delay(100);

}
