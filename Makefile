.PHONY: default install sdk left right

VENV := $(CURDIR)/.venv
export VIRTUAL_ENV := $(VENV)
export PATH := $(VENV)/bin:$(PATH)

SDK_VERSION := 0.16.8
SDK_DIR := $(HOME)/zephyr-sdk-$(SDK_VERSION)
SDK_URL := https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v$(SDK_VERSION)/zephyr-sdk-$(SDK_VERSION)_linux-x86_64_minimal.tar.xz

default: left right

install:
	python3 -m venv $(VENV)
	python -m pip install west pyelftools
	test -d .west || west init -l config
	west update
	west zephyr-export
	python -m pip install -r zephyr/scripts/requirements.txt

sdk:
	test -d $(SDK_DIR) || (curl -L $(SDK_URL) | tar -xJ -C $(HOME))
	cd $(SDK_DIR) && ./setup.sh -t arm-zephyr-eabi -c

left:
	west build -p always -s zmk/app -d build/sofle_left \
		-b nice_nano_v2 \
		-- -DSHIELD=sofle_left -DZMK_CONFIG="$(CURDIR)/config"

right:
	west build -p always -s zmk/app -d build/sofle_right \
		-b nice_nano_v2 \
		-- -DSHIELD=sofle_right -DZMK_CONFIG="$(CURDIR)/config"

ifeq ($(shell uname),Darwin)
NICENANO := /Volumes/NICENANO
CP := cp -X
else
NICENANO := /media/$(USER)/NICENANO
CP := cp
endif

ileft:
	$(CP) build/sofle_left/zephyr/zmk.uf2 $(NICENANO)

iright:
	$(CP) build/sofle_right/zephyr/zmk.uf2 $(NICENANO)
