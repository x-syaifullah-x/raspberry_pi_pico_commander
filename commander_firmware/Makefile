.PHONY: $(MAKECMDGOALS)

SHELL := /bin/bash

PROJECT_NAME := $(notdir $(CURDIR))
ROOT_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
BUILD_DIR = $(ROOT_DIR)/build
FIRMWARE_UF2 = $(BUILD_DIR)/firmware.uf2

dependencies:
	apt install --no-install-suggests --no-install-recommends -y cmake gcc-arm-none-eabi picotool libstdc++-arm-none-eabi-newlib python3

configure:
	cmake -S $(ROOT_DIR) -B $(BUILD_DIR)

build:
	cmake --build $(BUILD_DIR)

# flash:
# 	cp -rfv $(FIRMWARE_UF2) /media/$$USER/RPI-RP2/

flash:
	picotool load $(FIRMWARE_UF2)
	picotool info -a

flash_and_reboot: flash
	picotool reboot

clean:
	rm -rfv $(BUILD_DIR)