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
#ifndef MQTT_TOPIC
    #define MQTT_TOPIC "/*"
#endif
#ifndef MQTT_DEVICE_ID
    #define MQTT_DEVICE_ID "ESP32_lobbyStatusLight"
#endif

unsigned long previousMillis = 0;
unsigned long interval = 30000;

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
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (mqttClient.connect(MQTT_DEVICE_ID)) {
      Serial.println("connected");
      // ... and resubscribe
      mqttClient.subscribe(MQTT_TOPIC, 1);

    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
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

void setup(){
    Serial.begin(9600);
    delay(1000);

    initWiFi();
    mqttClient.setServer(MQTT_BROKER_URL, MQTT_BROKER_PORT);
    mqttClient.setCallback(mqttCallback);
    initMqtt();
}

void loop()
{
    if (!mqttClient.connected())
    {
        reconnect();
    }
    mqttClient.loop();
    delay(100);

}
