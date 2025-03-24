#!/bin/bash
avr-gcc -Wall -Os -DF_CPU=16000000UL -mmcu=atmega328p -o clock.elf clock.c
avr-objcopy -O ihex -R .eeprom clock.elf clock.hex
avrdude -c arduino -p atmega328p -P /dev/ttyACM0 -b 115200 -U flash:w:clock.hex:i

