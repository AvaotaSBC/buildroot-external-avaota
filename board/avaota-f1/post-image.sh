#!/bin/sh

set -eu

: "${BINARIES_DIR:?BINARIES_DIR is required by Buildroot}"

rootfs="${BINARIES_DIR}/rootfs.squashfs"
if [ ! -f "${rootfs}" ]; then
	echo "error: Buildroot did not produce ${rootfs}" >&2
	exit 1
fi

# The Tina NOR layout calls this partition rootfs_nor.fex. The image is a
# standard SquashFS image and can be handed to a separate boot/flash packer.
ln -sf rootfs.squashfs "${BINARIES_DIR}/avaota-f1-rootfs-nor.squashfs"

{
	echo "board=Avaota-F1"
	echo "soc=Allwinner V821 / sun300iw1p1"
	echo "kernel=Image.gz"
	echo "dtb=avaota-f1.dtb"
	echo "rootfs=rootfs.squashfs"
	echo "rootfs_format=SquashFS xz"
} > "${BINARIES_DIR}/avaota-f1-build-manifest.txt"
