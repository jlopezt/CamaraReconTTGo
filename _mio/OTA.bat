@echo off

set IP=%1


curl http://%IP%/setNextBoot?p=Loader
curl http://%IP%/restart
curl http://%IP%/test

REM <path espota.exe>/espota.exe -i <ip del dispositivo> -p <puerto> --auth=<calve OTA> -f <path del fichero bin/fichero.bin>

C:\Users\jlopezt\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.4\tools\espota.exe -i %IP% -p 3232 --auth=88716 -f C:\Users\jlopezt\Documents\PlatformIO\Projects\Camara\.pio\build\esp32dev\firmware.bin