# ESP-OTA-Templates
Repository for ESP8266 and ESP32 OTA (Over The Air) firmware update support

## Purpose
Establish OTA templates for ESP8266 and ESP32 based projects

Based on the work of Chris Joyce - https://github.com/chrisjoyce911/esp32FOTA


## Proposed OTA Features (templates for each to be developed & tested)
### OTA minimal - Minimal OTA solution based on esp32FOTA   
*(refer to sketch ESP-OTA-minimal.ino)*
  - simple versioning support
  - via HTTP webserver
  - for boards
    - online all the time
    - in deep sleep 


### OTA fleet - a solution supporting a "fleet" of boards
  - OTA triggered by new .bin & fota_fleet.json ref update (in webserver)

  - reference devices by ChipID 
  [decided not to use ChipID.  Simple board number or MAC address may be better.  For now, will use a simple board number]

        String esp32FOTA::getDeviceID()
        {
            char deviceid[21];
            uint64_t chipid;
            chipid = ESP.getEfuseMac();
            sprintf(deviceid, "%" PRIu64, chipid);
            String thisID(deviceid);
            return thisID;
        }



### OTA update - alternative triggering options
#### Online boards
  1. Trigger by periodic checks (eg. every 5 minutes) using timer interrupts  (refer to tutorial: https://techtutorialsx.com/2017/10/07/esp32-arduino-timer-interrupts/)  
    *(refer to sketch ```ESP-OTA-timer-interrupt.ino```)*        
    - tested with interrupt          
      - every 15 seconds - OK: checked for OTA firmware update when timer trigger occurred          
      - every 300 seconds (5 minutes) - OK: checked for OTA firmware update when timer trigger occurred
  2. Trigger via NodeRed flow MQTT push message to flag to the sketch to "check for update"  
    *not tested*


#### Deep sleep boards

  - check for OTA on wakeup (no change to minimal OTA approach)


### OTA update logging

  1. Logging from updates detected by the sketch  
  *(refer to sketch ```ESP-OTA-log-firmware-info.ino```)*  
  
   - uses MQTT to communicate status to NodeRed      
    (note: to avoid MQTT timeout, need to call *client.loop();* about every 15 seconds or less, otherwise connection to MQTT broker is lost)  
    
   - NodeRed formats JSON output and logs firmware status changes to a text file - timestamp, board, (sketch?), version, update request or current version  

     ** Question:  is there value in logging the name of the sketch running on the board also?  Probably useful **  

   - on OTA update request: log to file  (event trigger TBC - eg. when .bin & JSON files are updated and copied to webserver FOTA directory)  
    
   - on OTA update completion: log to file via MQTT push messaging   
    
    *just need to filter out duplicate "no update needed" log messages using NodeRed "filter" node (AKA "RBE" node):*   
    ```"filter node" AKA "Report by Exception (RBE) node" - only passes on data if the payload has changed```  
       
   - In NodeRed, log version update requests and confirmations (timestamped) for all boards (or by board) to a file on master server (RaspberryPi)  

  2. Logging triggered by VSCode sketch update and complile (fully auto or with manual confirmation)  
     - establish post-compile script after successful script compile to prepare JSON file, .bin file and copy to required webserver directory 
      (staging or production)


### Semantic versioning support   

### Increased security via signature checking

### HTTPS webserver support 

### Update notifications via Telegram


## Test environment
- ESP32 Dev Module (30-pin)  
- RPi4 (rp4sep2021) 
    - docker containers
        - nginx (with volumes mapping for FOTA)
        - NodeRed
        - MQTT

        
### Tests

#### Minimal OTA
Tested 2nd Mar 22

##### 1) [Pass] Recovery from Reset (EN) button push  
Online board case:  when reset (EN) button pushed, board goes into this state  
  *rst:0x1 (POWERON_RESET),boot:0x3 (DOWNLOAD_BOOT(UART0/UART1/SDIO_REI_REO_V2))*  
  *waiting for download*

It doesn't recover from this state unless a new sketch is uploaded (using a cable).

**Retested 3rd Mar 2022:  Worked OK**
```rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
flash read err, 1000
ets_main.c 371
ets Jun  8 2016 00:22:57

rst:0x10 (RTCWDT_RTC_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0018,len:4
load:0x3fff001c,len:1216
ho 0 tail 12 room 4
load:0x40078000,len:9720
ho 0 tail 12 room 4
load:0x40080400,len:6352
entry 0x400806b8

Connecting to MY-WIFI
.
WiFi connected
IP address:
192.168.1.3
Getting HTTP
http://192.168.1.72:8080/fota/fota.json
```


##### 2) [Pass] Recovery from power off/on.
Works OK when powered off & on (disconnected & reconnected to a power bank)

##### 3) [Pass] Switch off webserver
When webserver is powered off, the ESP32 HTTP get slows (until timeout)
*therefore time to loop is extended to HTTP get timeout - approx. 15 seconds*

##### 4) [Pass] Switch on webserver
When webserver is powered on, the ESP32 starts running normally


#### Fleet OTA
Tested 8th Mar 2022

Simplied URL version of fota-fleet.json works OK
[FAIL] - Multible firmware types in the json file doesn't work (suspect bug)  
      - perhaps issue with length of fw type name???
         - failed with this "type":"ESP32-Dev-Module-30pin-23"  
            - so try with "type":"ESP32-23"  - also failed




## Alternatives
In addition to esp32FOTA, other OTA solutions considered were:
- AsyncElegantOTA
  - Refer https://randomnerdtutorials.com/esp32-ota-over-the-air-arduino/
  - requires a webserver on the ESP32
  - logs onto the LAN via WiFi in AP mode, but starts the webserver (how?)
  - requires manual "push" via a browser session on the local LAN

- ???  (others?)  

