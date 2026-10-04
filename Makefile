.PHONY: default install left right

default: left right

install:
	python3 -m venv .venv
	source .venv/bin/activate
	python -m pip install west pyelftools
	west init -l config
	west update
	west zephyr-export
	python -m pip install -r zephyr/scripts/requirements.txt

left:
	west build -p always -s zmk/app -d build/sofle_left \
		-b nice_nano_v2 \
		-- -DSHIELD=sofle_left -DZMK_CONFIG="$(CURDIR)/config"

right:
	west build -p always -s zmk/app -d build/sofle_right \
		-b nice_nano_v2 \
		-- -DSHIELD=sofle_right -DZMK_CONFIG="$(CURDIR)/config"

ileft:
	cp -X build/sofle_left/zephyr/zmk.uf2 /Volumes/NICENANO

iright:
	cp -X build/sofle_right/zephyr/zmk.uf2 /Volumes/NICENANO
