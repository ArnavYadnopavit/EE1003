#!/bin/bash
avr-gcc -DF_CPU=16000000UL -mmcu=atmega328p -Os -Wall -o calculator.elf calculator.c -lm
avr-objcopy -O ihex calculator.elf calculator.hex
avrdude -p atmega328p -c arduino -P /dev/ttyACM0 -b 57600 -U flash:w:calculator.hex
