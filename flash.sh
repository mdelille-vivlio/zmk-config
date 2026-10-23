#!/usr/bin/env bash
# Flash both Sofle halves: for each half, wait for its NICENANO bootloader
# drive to mount, copy the firmware, and wait for the drive to go away.
set -euo pipefail

cd "$(dirname "$0")"

if [ "$(uname)" = Darwin ]; then
    NICENANO=/Volumes/NICENANO
    CP="cp -X"
else
    NICENANO=/media/$USER/NICENANO
    CP=cp
fi

flash() {
    local side=$1
    local uf2=build/sofle_$side/zephyr/zmk.uf2

    if [ ! -f "$uf2" ]; then
        echo "Missing $uf2, run 'make $side' first." >&2
        exit 1
    fi

    if [ -d "$NICENANO" ]; then
        echo "Waiting for $NICENANO to unmount first..."
        while [ -d "$NICENANO" ]; do sleep 0.5; done
    fi

    echo "Plug in the $side half, then hold Lower + Raise and press its outer Shift corner"
    echo "(or double-tap its reset button)."
    printf "Waiting for %s" "$NICENANO"
    until [ -d "$NICENANO" ]; do
        printf .
        sleep 0.5
    done
    echo
    sleep 1 # let the drive finish mounting

    echo "Copying $uf2..."
    # The board reboots as soon as the copy ends, which can make cp report an error.
    $CP "$uf2" "$NICENANO" || true

    while [ -d "$NICENANO" ]; do sleep 0.5; done
    echo "The $side half is flashed."
    echo
}

sides=("$@")
[ ${#sides[@]} -eq 0 ] && sides=(left right)

for side in "${sides[@]}"; do
    flash "$side"
done
