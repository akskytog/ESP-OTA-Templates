/* Test sketch for OTA using HTTP
   Date: 14th Oct 2020, updated 26th Feb 2022
   Board: ESP32 Dev Module - 30pin with onboard LED (was LOLIN D32)
   Board Id: 22
*/

/*
   esp32 firmware OTA

   Purpose: Perform an OTA update from a bin located on a webserver (HTTP Only)
   Steps:
   In the code:
     Step 1 : Confirm/Set your WiFi (ssid & password)
     Step 2 : set my_esp32FOTA("the type of device - device number", version-number)
                  eg. esp32FOTA my_esp32FOTA("ESP32-Dev-Module-30pin-22", 2);        
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

/* 1 - #define statements          */
#define LED_BUILTIN 2 // // works on ESP32 Dev Module 30-pin, but 38-pin version doesn't have onboard addressable LED

/* 2 - Required utility libraries  */
#include <esp32fota.h> // from https://github.com/chrisjoyce911/esp32FOTA

/* 3 - Wifi Credentials & library */
#include <WiFi.h>
#include <myCreds.h> // my wifi credentials

// why are these defined?? (IPAddress settings below)
// IPAddress ip(192,168,0,105);
// IPAddress dns(192,168,1,1);
// IPAddress gateway(192,168,1,1);
// IPAddress subnet(255,255,255,0);

/* 4 - Firmware version setup */
// format:
//          esp32FOTA esp32FOTA("<Type of Firmware for this device>", <this version>);
esp32FOTA my_esp32FOTA("ESP32-Dev-Module-30pin-22", 3);   
//--------------------------------------------------------------------------------------
// Version Log (update with each revision):
// Version  - description
// 1        - initial (LED flash every 1000ms)
// 2        - faster blink duration (every 100ms) with 1000ms pause between blinks
// 3        - change reference to fota_fleet.json file (instead of fota.json) 
//             and change blink duration to 2000ms, with 500ms pause between blinks
//--------------------------------------------------------------------------------------

/* 5 - The delay set for the onboard LED Pin to indicate changes in FW version */
int LED_TIME_ON = 2000;    // milliseconds (update between versions)
int LED_TIME_OFF = 500;  // milliseconds 

void connectToWiFi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(mySSID);

  WiFi.mode(WIFI_STA);
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
  Serial.println(">> start ESP-OTA-minimal sketch");

  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

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
}

/**************** LOOP *******************/
void loop()
{

  // OTA Code:
  bool updatedNeeded = my_esp32FOTA.execHTTPcheck();
  if (updatedNeeded)
  {
    Serial.println("  OTA firmware update required");
    my_esp32FOTA.execOTA();
  }
  else
  {
    Serial.println("  OTA firmware update NOT required");
  }

  // put your main code here, to run repeatedly:
  digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
  delay(LED_TIME_ON);
  digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
  delay(LED_TIME_OFF);
}