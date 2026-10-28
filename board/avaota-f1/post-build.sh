#!/bin/sh

set -eu

# The V821 boot environment passes init=/init. Keep that board-level contract
# while using Buildroot's normal BusyBox init implementation.
if [ ! -e "${TARGET_DIR}/init" ]; then
	ln -s /sbin/init "${TARGET_DIR}/init"
fi

# The F1 boot environment and the board's documented serial console use
# ttyAS0 at 115200 baud. Keep the generated BusyBox getty entry aligned with
# the U-Boot bootargs and fail loudly if the expected console was not generated.
inittab="${TARGET_DIR}/etc/inittab"
if ! grep -q '^ttyAS0::respawn:/sbin/getty' "${inittab}"; then
	echo "error: Buildroot did not generate the ttyAS0 getty entry" >&2
	exit 1
fi
sed -i -E '/^ttyAS0::respawn:\/sbin\/getty/ s/[[:space:]][0-9]+([[:space:]]+vt100)/ 115200\1/' "${inittab}"
if ! grep -q '^ttyAS0::respawn:/sbin/getty.*115200' "${inittab}"; then
	echo "error: failed to set ttyAS0 getty baudrate to 115200" >&2
	exit 1
fi
