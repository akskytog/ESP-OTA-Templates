#include <TelnetStream.h>  /* Telnet debugging */
#include "OTA.h"
unsigned long entry;

IPAddress staticIP(192, 168, 1, 199);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);


// Test OTA with LED blink enable  
// (ESP32 Dev Module: tried onboard pin 2, no luck.  Also tried onboard pin 6 - no luck  [38-Pin version of the ESP Dev Mobule])
//                    External pin 16 - LOW = off, HIGH = on - worked OK
// (WMOS LOLIN32:  tried onboard pin 5.  LOW = on,  HIGH = off - worked OK)

int LED_PIN = 16;

//#define ESP32_RTOS  // Uncomment this line if you want to use the code with freertos (only works on the ESP32)

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");

  setupOTA("OTA_Template_Sketch");  /* hostname prefix */

  WiFi.config(staticIP, gateway, subnet);

  // your code
  pinMode (LED_PIN, OUTPUT);

  TelnetStream.begin();    /* Telnet debugging */

}

void loop() {
#ifndef ESP32_RTOS
  ArduinoOTA.handle();
#endif

  delay(20);

  TelnetStream.read();                   /* Telnet debugging (without the read, the println does nothing!) */
  TelnetStream.print("WiFi Info:  IP: ");  /* Telnet debugging */
  TelnetStream.print(WiFi.localIP());
  TelnetStream.print("    macAddress: ");
  TelnetStream.print(WiFi.macAddress());
  TelnetStream.println();
  
//  TelnetStream.flush();
//      TelnetStream.stop();

  // your code here
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(2000);

}
