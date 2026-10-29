# Avaota-F1 Buildroot board

## What is built

This board directory connects Buildroot to the self-contained `linux-v821`
repository next to this repository. The build uses Buildroot's internal
RISC-V 32-bit glibc toolchain (`rv32imafdc`, `ilp32d`), the standalone
5.4.220 V821 kernel/BSP, the Avaota-F1 device tree, and a compressed SquashFS
root filesystem. No Tina checkout, toolchain, or makefile is required.

The V821 F1 NOR map allocates 11136 sectors for `rootfs` and uses a SquashFS
image. The external tree therefore selects SquashFS with xz compression rather
than an uncompressed ext4 image. The F1 DTS disables the GMAC, so this board
configuration does not request Buildroot's automatic wired DHCP setup.

The V821 FMAC path is enabled with the standalone `v821-wifimanager` package.
Its init scripts start `remoteproc0`, load `v821_fmac.ko`, and then start the
XRLINK STA/AP manager; this matches the F1 Tina flow without requiring a Tina
checkout.

ADB follows Tina's V821 FunctionFS flow. Buildroot's `android-tools` package
provides `/usr/bin/adbd`; `S30adbd` mounts configfs and
`/dev/usb-ffs/adb`, creates `ffs.adb`, starts `adbd -D`, and binds the first
Sunxi UDC. The kernel configuration contains the required configfs,
FunctionFS, and Sunxi UDC options. The legacy Tina `setusbconfig` executable
and old Android 4.2 adbd source are intentionally not copied into this
Buildroot tree.

## Build

Keep the external tree, kernel, U-Boot, and image-tools repositories as
siblings, alongside the upstream Buildroot checkout:

```sh
git clone <buildroot-external-avaota-url> buildroot-external-avaota
git clone <linux-v821-url> linux-v821
git clone <u-boot-v821-url> u-boot-v821
git clone <avaota-image-tools-url> avaota-image-tools
git clone https://github.com/buildroot/buildroot.git buildroot
cd buildroot
git checkout 2024.02.3
make BR2_EXTERNAL=../buildroot-external-avaota \
     O=../output avaota_f1_defconfig
make O=../output
```

The important files are `output/images/Image.gz`,
`output/images/avaota-f1.dtb`, `output/images/rootfs.squashfs`,
`output/images/avaota-f1-flash.img`, `output/images/avaota-f1-spinor.raw`,
and `output/images/avaota-f1-sd.raw`.
The post-image hook invokes the sibling `avaota-image-tools` repository,
which in turn builds U-Boot from the sibling `u-boot-v821` repository, to
create the common Allwinner Phoenix/Dragon image plus independent raw
SPI-NOR and SD images.

`avaota-f1-sd.raw` is a real DOS/MBR disk image. Its four primary entries are
`boot` at LBA52352, `rootfs` at LBA67712 (`/dev/mmcblk0p2`), `rootfs_data` at
LBA87808, and `UDISK` at LBA98048 through the end. The V821 Boot0 copies
remain at LBA16/LBA256, the primary and backup boot packages at
LBA32800/LBA24576, and the Allwinner logical partition area starts at
LBA40960. It can be inspected with `fdisk -l`/`sfdisk --dump` and written
directly to an SD card; the default file size is 64 MiB. SPI-NOR uses the
Allwinner MTD map and boots its root filesystem from `/dev/mtdblock6`
(`rootfs`; `rootfs_data` is mtd7).
