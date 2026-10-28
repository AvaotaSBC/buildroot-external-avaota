#!/bin/sh

set -eu

# The V821 boot environment passes init=/init. Keep that board-level contract
# while using Buildroot's normal BusyBox init implementation.
if [ ! -e "${TARGET_DIR}/init" ]; then
	ln -s /sbin/init "${TARGET_DIR}/init"
fi

# The V821 uart-ng driver is built without the 8250 compatibility layer, so
# the serial console is ttyS0. Keep the generated BusyBox getty entry aligned
# with the U-Boot bootargs at 115200 baud.
inittab="${TARGET_DIR}/etc/inittab"
if ! grep -q '^ttyS0::respawn:/sbin/getty' "${inittab}"; then
	echo "error: Buildroot did not generate the ttyS0 getty entry" >&2
	exit 1
fi
sed -i -E '/^ttyS0::respawn:\/sbin\/getty/ s/[[:space:]][0-9]+([[:space:]]+vt100)/ 115200\1/' "${inittab}"
if ! grep -q '^ttyS0::respawn:/sbin/getty.*115200' "${inittab}"; then
	echo "error: failed to set ttyS0 getty baudrate to 115200" >&2
	exit 1
fi
