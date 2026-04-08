.PHONY: $(MAKECMDGOALS)

SHELL := /bin/bash

PROJECT_NAME := $(notdir $(CURDIR))
ROOT_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
BUILD_DIR = $(ROOT_DIR)/build

dependencies:
	apt install --no-install-suggests --no-install-recommends cmake gcc-arm-none-eabi picotool libstdc++-arm-none-eabi-newlib

configure:
	cmake -S $(ROOT_DIR) -B $(BUILD_DIR)

build:
	cmake --build $(BUILD_DIR)

flash:
	cp -rfv $(BUILD_DIR)/firmware.uf2 /media/$$USER/RPI-RP2/

clean:
	rm -rfv $(BUILD_DIR)