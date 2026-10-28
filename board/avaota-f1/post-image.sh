#!/bin/sh

set -eu

: "${BINARIES_DIR:?BINARIES_DIR is required by Buildroot}"

rootfs="${BINARIES_DIR}/rootfs.squashfs"
if [ ! -f "${rootfs}" ]; then
	echo "error: Buildroot did not produce ${rootfs}" >&2
	exit 1
fi

external_dir=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
image_tools_dir="${AVAOTA_IMAGE_TOOLS_DIR:-${external_dir}/../avaota-f1-image-tools}"
uboot_dir="${AVAOTA_UBOOT_DIR:-${external_dir}/../u-boot-v821}"
host_dir="${HOST_DIR:-$(CDPATH= cd -- "${BINARIES_DIR}/../host" && pwd)}"
build_dir="${BUILD_DIR:-${BINARIES_DIR}/../build}"
toolchain_prefix="${host_dir}/bin/riscv32-buildroot-linux-gnu-"

if [ ! -x "${image_tools_dir}/pack-avaota-f1-images.sh" ]; then
	echo "error: Avaota-F1 image-tools repository not found: ${image_tools_dir}" >&2
	exit 1
fi
if [ ! -x "${uboot_dir}/build-u-boot-v821.sh" ]; then
	echo "error: V821 U-Boot repository not found: ${uboot_dir}" >&2
	exit 1
fi

"${image_tools_dir}/pack-avaota-f1-images.sh" \
	--binaries-dir "${BINARIES_DIR}" \
	--build-dir "${build_dir}/avaota-f1-image-tools" \
	--output-dir "${BINARIES_DIR}" \
	--u-boot-dir "${uboot_dir}" \
	--toolchain-prefix "${toolchain_prefix}"

# The F1 NOR partition table calls this partition rootfs_nor.fex. The image is
# a standard SquashFS image and can be handed to the boot/flash packer.
ln -sf rootfs.squashfs "${BINARIES_DIR}/avaota-f1-rootfs-nor.squashfs"

{
	echo "board=Avaota-F1"
	echo "soc=Allwinner V821 / sun300iw1p1"
	echo "kernel=Image.gz"
	echo "dtb=avaota-f1.dtb"
	echo "rootfs=rootfs.squashfs"
	echo "rootfs_format=SquashFS xz"
	echo "spinor_image=avaota-f1-spinor.raw"
	echo "sd_image=avaota-f1-sd.img"
} > "${BINARIES_DIR}/avaota-f1-build-manifest.txt"
