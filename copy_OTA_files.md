# ESP32 OTA - How to copy .bin and JSON files 

## Background
When a sketch is updated, a new .bin file is created (in USER/Documents/MySoftwareProjects/ArduinoOutput/)   and
the JSON file to indicate a new version and to point to the new executable .bin file is updated 
(in USER/Documents/MySoftwareProjects/PROJECT-NAME/src/FOTA/) - usually names fota_fleet.json

Once updated or created, these exist on the machine running VSCode (usually a Windows Desktop computer).
These need to be copied to the webserver running on the Raspberry Pi that is accessed by the ESP32 so that the new
version can be discovered and copied to the ESP32.

## Webserver
To aid management and simplicity, the webserver on the Raspberry Pi is nginx running in a Docker container.
The volumes mapping for the webserver for OTA is:
/home/pi/docker-iot/volumes/nginx/html/fota/

When the Docker container is built (using a ```docker-compose up -d nginx``` command), the directory and any new files 
get a user and group of root:root by default.

In order to use WinSCP or Windows PowerShell to copy from the Windows Desktop to the fota/ directory, permissions need to be changed.
Repository for ESP8266 and ESP32 OTA (Over The Air) firmware update support

## Permission Changes Required 
For a new nginx Docker Container, an initial permission change will be required to allow 
SCP (Secure Copy Protocol) to be used with user "pi".

To do this, on the Raspberry Pi, go to the ```/home/pi/docker-iot/volumes/nginx/html/``` directory and issue command
```sudo chown pi:pi fota``` to change the user and group owner of the fota directory to pi:pi

After that change, SCP (either using a Windows PowerShell command like ```scp fota_fleet.json pi@192.168.1.72:/home/pi/docker-iot/volumes/nginx/html/fota/``` 
or WinSCP) can be used to copy the update JSON and .bin files to the webserver.
Once copied, the ESP32 will detect a new firmware version and it will be uploaded over the air.