// Updated by AK - 6/3/2022 - updated to use mqttServer as IP Address of RaspberryPi rp4sep2021 (test environment):  192.168.1.72

#include <PubSubClient.h>
#define MQTT_MAX_PACKET_SIZE 4096

WiFiClient espClient;
PubSubClient client(espClient);

const char *mqttServer = "192.168.1.72";

void callback(char *topic, byte *payload, unsigned int length)
{
  // push message from NodeRed.... to trigger firmware update (not used - firmware update check based on time interval)
  Serial.print("Message arrived in topic: [");
  Serial.print(topic);
  check_for_firmware_update_flag = 0;
  if (strcmp(topic, "node/firmware/update/ESP32-24/check-for-update") == 0)
  { /*** UPDATE IF REQUIRED ***/
    check_for_firmware_update_flag = 1;   // this variable is for triggering check for firmware update when indicated by NodeRed-originated message
  }
  Serial.print("] ");
  Serial.print("Message: ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  //Serial.println("  (not in use) check_for_firmware_update_flag = ", check_for_firmware_update_flag);
  Serial.println("--------------------------------");
  yield();
}

void connectMQTT()
{

  client.setServer(mqttServer, 1883);
  client.setCallback(callback);

  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ES32-24-Client"))
    { /*** UPDATE IF REQUIRED ***/
      Serial.print("connected MQTT server at ");
      Serial.println(mqttServer);
      // Once connected, publish an announcement...
      client.publish("outTopic", "MQTT outTopic on connection:  hello world");
      // ... and resubscribe
      client.subscribe("node/firmware/update/ESP32-24"); /*** UPDATE IF REQUIRED ***/
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
    }
    delay(100);
  }
  yield(); // is this in the correct place?  What does it do?
}