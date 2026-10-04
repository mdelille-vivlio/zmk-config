# ZMK Sofle configuration

This repository contains the versioned configuration for a Sofle keyboard. The
Zephyr and ZMK source trees are fetched locally by `west` and are intentionally
not versioned.

The controller documentation is available at
<https://nicekeyboards.com/docs/nice-nano>.

## Edit the keymap

Edit [`config/sofle.keymap`](config/sofle.keymap). The main layers are
`default_layer`, `lower_layer`, `raise_layer`, and `adjust_layer`.

## Set up a fresh checkout

```sh
python3 -m venv .venv
source .venv/bin/activate
python -m pip install west pyelftools
west init -l config
west update
west zephyr-export
python -m pip install -r zephyr/scripts/requirements.txt
```

Install the Zephyr SDK 0.16.x separately and set `ZEPHYR_SDK_INSTALL_DIR` to
its location. This checkout uses Zephyr 3.5.0 and therefore requires CMake 3.x.

## Build firmware

```sh
source .venv/bin/activate

west build -p always -s zmk/app -d build/sofle_left \
  -b nice_nano_v2 \
  -- -DSHIELD=sofle_left -DZMK_CONFIG="$PWD/config"

west build -p always -s zmk/app -d build/sofle_right \
  -b nice_nano_v2 \
  -- -DSHIELD=sofle_right -DZMK_CONFIG="$PWD/config"
```

The resulting firmware files are `build/sofle_left/zephyr/zmk.uf2` and
`build/sofle_right/zephyr/zmk.uf2`. Flash each file to the matching keyboard
half in its nice!nano bootloader drive.
