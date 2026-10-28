#!/bin/sh

set -eu

# The V821 boot environment passes init=/init. Keep that board-level contract
# while using Buildroot's normal BusyBox init implementation.
if [ ! -e "${TARGET_DIR}/init" ]; then
	ln -s /sbin/init "${TARGET_DIR}/init"
fi

# The F1 boot environment uses ttyAS0 at 1500000 baud. Buildroot 2024.02.3
# only offers up to 115200 in Kconfig, so correct the generated BusyBox getty
# entry here and fail loudly if the expected serial console was not generated.
inittab="${TARGET_DIR}/etc/inittab"
if ! grep -q '^ttyAS0::respawn:/sbin/getty' "${inittab}"; then
	echo "error: Buildroot did not generate the ttyAS0 getty entry" >&2
	exit 1
fi
sed -i -E '/^ttyAS0::respawn:\/sbin\/getty/ s/[[:space:]][0-9]+([[:space:]]+vt100)/ 1500000\1/' "${inittab}"
if ! grep -q '^ttyAS0::respawn:/sbin/getty.*1500000' "${inittab}"; then
	echo "error: failed to set ttyAS0 getty baudrate to 1500000" >&2
	exit 1
fi
