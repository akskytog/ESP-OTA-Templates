/* Test sketch for OTA using HTTP - with check for firmware update on timer interrupt
   Date: 4th March 2022
   Board: ESP32 Dev Module - 30pin with onboard LED
   Board Id: 24

   Modified version of sketch "ESP32-OTA-fleet.ino" to test logging of firmware updates via MQTT messaging to NodeRed
   Tutorial for timer interrupts:  https://techtutorialsx.com/2017/10/07/esp32-arduino-timer-interrupts/
*/

/*
   esp32 firmware OTA

   Purpose: Perform an OTA update from a bin located on a webserver (HTTP Only)
   Steps:
   In the code:
     Step 1 : Confirm/Set your WiFi (ssid & password)
     Step 2 : set my_esp32FOTA("the type of device - device number", version-number)
                  eg. esp32FOTA my_esp32FOTA("ESP32-Dev-Module-30pin-23", 1);
     Step 3 : describe the version change
     Step 4 : make the required code changes

   In the IDE:
   Step 5 : complile
             (in Arduino IDE: Menu > Sketch > Export Compiled Library. The bin file will be saved in the sketch folder (Menu > Sketch > Show Sketch folder))
             (in VSCode, use Verify (without upload) )

   Update and copy files
   Step 6 : Copy the generated .bin file to the webserver /fota/ directory
   Step 7 : Check/Update the fota.json file in the webserver /fota/ directory to reference the "type of device - device number"
   Step 8 : Update the version number in the fota.json file in the webserver /fota/ directory
*/

/* notes (can be removed later)
#define is a useful C++ component that allows the programmer to give a name to a constant value before the program is compiled.
Defined constants in arduino don’t take up any program memory space on the chip.
The compiler will replace references to these constants with the defined value at compile time
*/

/* 010 - #define statements          */
#define LED_BUILTIN 2 // // works on ESP32 Dev Module 30-pin, but 38-pin version doesn't have onboard addressable LED

/* 020 -  global variables */
int check_for_firmware_update_flag = 0; // Flag to indicate whether MQTT subscription indicates that a firmware update is requested

/* 030 - Required utility libraries  */
#include <esp32fota.h> // from https://github.com/chrisjoyce911/esp32FOTA
#include "./mqtt.h"    // added file into this sketch folder
#include <ArduinoJson.h>
//#include <String.h> // for string function

/* 040 - Wifi Credentials & library */
#include <WiFi.h>
#include <myCreds.h> // my wifi credentials

/* 050 - Firmware version setup */
// format:
//          esp32FOTA esp32FOTA("<Type of Firmware for this device>", <this version>);
#define FW_SKETCH_NAME "ESP32-OTA-log.ino"  //<<<<<<<<<<<<<<<<<<<<<<<<< not required for OTA, but for logging OTA updates
#define FW_TYPE "ESP32-Dev-Module-30pin-24" //<<<<<<<<<<<<<<<<<<<<<<<<< set target device
#define FW_VERSION 4                        //<<<<<<<<<<<<<<<<<<<<<<<<< define version number of this sketch
esp32FOTA my_esp32FOTA(FW_TYPE, FW_VERSION);
//--------------------------------------------------------------------------------------
// Version Log (update with each revision):
// Version  - description
// 1        - LED on for 100ms, off for 1000ms
// 2        - LED on for 50ms, off for 2000ms
// 3        - LED on for 25ms, off for 5000ms
// 4        - LED on for 25ms, off for 5000ms,  added FW_SKETCH_NAME to allow logging of the name of the sketch
//--------------------------------------------------------------------------------------

/* 060 - Timer Interrupt variables */
/*     Time to wait between each OTA Firmware update check (seconds) */
int FOTA_CHECK_INTERVAL = 120;
/*     Other required variables  */
volatile int interruptCounter;
int totalInterruptCounter;
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

/* 070 - set up JsonDocument for serialised data */
StaticJsonDocument<512> doc;
char jsonString[400];
JsonObject firmware_info = doc.createNestedObject("firmware_info");

/* 080 - The delay set for the onboard LED Pin to indicate changes in FW version */
int LED_TIME_ON = 25;    // milliseconds (update between versions)
int LED_TIME_OFF = 5000; // milliseconds

void IRAM_ATTR onTimer() // triggered each time an interrupt occurs
{
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void connectToWiFi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(mySSID);

  WiFi.mode(WIFI_MODE_STA);
  //  WiFi.config(ip);                   // set static IP Addr
  //  WiFi.config(ip, gateway, subnet, dns);
  WiFi.begin(mySSID, myPASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

/**************** SETUP ********************/
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.print(">>>> start ");
  Serial.println(FW_SKETCH_NAME);

  // Timer setup
  timer = timerBegin(0, 80, true); // start at zero, assume clock speed 80MHz.  Prescaler of 80 means one count every CPU cycle, count up = true
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, FOTA_CHECK_INTERVAL * 1000000, true); // alarm is triggered very FOTA_CHECK_INTERVAL*1000000 ticks (ie. every FOTA_CHECK_INTERVAL seconds)
  timerAlarmEnable(timer);

  /* OTA setup:
         - URL of fota.json file
                "http://(ip_addr_of_rpi)/fota/(json-file-name)"     [for rp4sep2021: 192.168.1.72]
         - full path on RPi (nginx in a docker container - using port 8080):
                /home/pi/docker-nginx/volumes/nginx/html/fota
  */
  my_esp32FOTA.checkURL = "http://192.168.1.72:8080/fota/fota_fleet.json";
  /**** best-practise-question:  should the URL be set as a #define statement ??? ****/

  // connect to wifi:
  connectToWiFi();

  // Connect to MQTT Server
  connectMQTT();
  delay(1000);
  yield(); /*  what is this??? */
}

/**************** LOOP *******************/
void loop()
{

  // Interrupt handling code:
  if (interruptCounter > 0)
  {
    portENTER_CRITICAL(&timerMux);
    interruptCounter--;
    portEXIT_CRITICAL(&timerMux);
    totalInterruptCounter++;
    Serial.print("An interrupt as occurred. Total number: ");
    Serial.println(totalInterruptCounter);
    // (put the code logic that you want to run each time the interrupt occurs here)

    // OTA Code:
    bool updatedNeeded = my_esp32FOTA.execHTTPcheck();

    // Set up info to be published via MQTT using ArduinoJson
    firmware_info["fw_type"] = FW_TYPE;
    String my_FW_VERSION = String(FW_VERSION);
    firmware_info["fw_version"] = my_FW_VERSION;
    String my_updatedNeeded = String(updatedNeeded);
    firmware_info["updateNeeded"] = my_updatedNeeded;
    firmware_info["sketch_name"] = FW_SKETCH_NAME; 

    // print serialised data to Serial port (for debugging)
    serializeJson(doc, Serial);
    Serial.println();

    // Publish jsonString (built from firware_info)
    serializeJson(doc, jsonString);
    if (client.publish("node/firmware/update/ESP32-24", jsonString) == 1)
    {
      Serial.println("Successfully published json string");
      //client.publish("outTopic", "MQTT outTopic after publishing json string");
    }
    else
    {
      Serial.println("!!!!! Not published. Please add #define MQTT_MAX_PACKET_SIZE 2048 at the beginning of PubSubClient.h file");
    }

    if (updatedNeeded)
    {
      Serial.println("  OTA firmware update required");
      my_esp32FOTA.execOTA();
    }
    else
    {
      Serial.println("  OTA firmware update NOT required");
    }
  }

  client.loop();

  // Sketch application logic code::
  digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
  delay(LED_TIME_ON);
  digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
  delay(LED_TIME_OFF);
}