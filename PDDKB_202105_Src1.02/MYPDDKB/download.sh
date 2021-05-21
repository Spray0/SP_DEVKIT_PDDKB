#!/bin/bash
./stm32flash -w ./build/PDDKB.hex -v -g 0x0 /dev/ttyUSB0
