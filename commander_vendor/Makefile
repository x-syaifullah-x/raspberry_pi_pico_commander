.PHONY: $(MAKECMDGOALS)

SHELL := /bin/bash

PROJECT_NAME = commander-vendor
ROOT_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
BUILD_DIR = $(ROOT_DIR)/build
SRC = $(ROOT_DIR)/src

ID_VENDOR = 1902
ID_PRODUCT = 1992
SERIAL_NUMBER = E0C9125B0D9B
USER = xxx

BINARY = $(BUILD_DIR)/$(PROJECT_NAME) --vid=0x$(ID_VENDOR) --pid=0x$(ID_PRODUCT)

RULES_FILE = 99-$(PROJECT_NAME).rules
RULES_DST  = /etc/udev/rules.d/$(RULES_FILE)

dependencies:
	apt install --no-install-suggests --no-install-recommends ca-certificates gcc libc-dev pkg-config libusb-1.0-0-dev

rules_setup:
	@echo "Installing udev rules..."
	@printf 'ACTION=="add", SUBSYSTEM=="usb", ATTRS{idVendor}=="$(ID_VENDOR)", ATTRS{idProduct}=="$(ID_PRODUCT)", ATTRS{serial}=="$(SERIAL_NUMBER)", MODE="0666", GROUP="$(USER)", RUN+="/bin/systemd-run --unit=$(PROJECT_NAME) $(BINARY)"' | tee $(RULES_DST)
	udevadm control --reload-rules && udevadm trigger --subsystem-match=usb
	@echo "Done. Verifying..."

rules_remove:
	rm -f $(RULES_DST)
	udevadm control --reload-rules && udevadm trigger --subsystem-match=usb
	@echo "Rules removed."

build:
	CGO_ENABLED=1 CC=gcc go build -buildvcs=false -ldflags='-s -w' -trimpath -o $(BUILD_DIR)/$(PROJECT_NAME)  $(SRC)

run:
	CGO_ENABLED=1 CC=gcc go run -ldflags='-s -w' -trimpath $(SRC) --vid=0x$(ID_VENDOR) --pid=0x$(ID_PRODUCT)

clean:
	rm -rfv $(BUILD_DIR)