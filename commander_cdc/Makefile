.PHONY: $(MAKECMDGOALS)

SHELL := /bin/bash

PROJECT_NAME = commander-cdc
ROOT_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
BUILD_DIR = $(ROOT_DIR)/build
SRC = $(ROOT_DIR)/src

ID_VENDOR = 1902
ID_PRODUCT = 1992
SERIAL_NUMBER = E0C9125B0D9B

SERVICE_FILE = $(PROJECT_NAME)@.service
SERVICE_DST  = /etc/systemd/system/$(SERVICE_FILE)
BINARY       = /media/xxx/DATA_EXT4/xxx/Git/x-syaifullah-x/raspberry_pi_pico_commander/build/commander_cdc/commander_cdc --vid=$(ID_VENDOR) --pid=$(ID_PRODUCT) --serial=$(SERIAL_NUMBER)

RULES_FILE = 99-$(PROJECT_NAME).rules
RULES_DST  = /etc/udev/rules.d/$(RULES_FILE)

TTYACM ?= /dev/ttyACM0

build:
	go build -buildvcs=false -ldflags='-s -w' -trimpath -o build/commander_cdc/commander_cdc $(SRC)

run:
	go run -ldflags='-s -w' -trimpath $(SRC) --vid=2e8a --pid=0003 --serial='E0C9125B0D9B'

clean:
	rm -rfv build/commander_cdc

reboot_bootsel:
	printf '\xFF\x01\x02' > $(TTYACM)

monitor:
	stty -F $(TTYACM) raw -echo
	cat $(TTYACM)

install: install-service
	@echo "Installing udev rules..."
	@printf 'ACTION=="add", SUBSYSTEM=="tty", ATTRS{idVendor}=="$(ID_VENDOR)", ATTRS{idProduct}=="$(ID_PRODUCT)", ATTRS{serial}=="$(SERIAL_NUMBER)", MODE="0666", TAG+="systemd", ENV{SYSTEMD_WANTS}+="$(PROJECT_NAME)@%%k.service"' | tee $(RULES_DST)
	udevadm control --reload-rules && udevadm trigger --subsystem-match=tty
	@echo "Done. Verifying..."

install-service:
	@echo "Installing systemd service..."
	@printf '[Unit]\nDescription=Commander CDC (%%i)\nAfter=dev-%%i.device\nBindsTo=dev-%%i.device\n\n[Service]\nType=simple\nExecStart=$(BINARY)\nRestart=no\n\n[Install]\nWantedBy=multi-user.target\n' | tee $(SERVICE_DST)
	systemctl daemon-reload
	@echo "Service installed."

uninstall: uninstall-service
	rm -f $(RULES_DST)
	udevadm control --reload-rules && udevadm trigger --subsystem-match=tty
	@echo "Rules removed."

uninstall-service:
	systemctl stop $(PROJECT_NAME)@ttyACM0.service || true
	rm -f $(SERVICE_DST)
	systemctl daemon-reload
	@echo "Service removed."