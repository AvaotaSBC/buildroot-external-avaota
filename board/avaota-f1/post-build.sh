#!/bin/sh

set -eu

# Tina's V821 command line uses init=/init. Keep that board-level contract
# while using Buildroot's normal BusyBox init implementation.
if [ ! -e "${TARGET_DIR}/init" ]; then
	ln -s /sbin/init "${TARGET_DIR}/init"
fi
