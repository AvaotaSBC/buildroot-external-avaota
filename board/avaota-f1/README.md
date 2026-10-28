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
configuration does not request Buildroot's `eth0` DHCP setup.

## Build

Keep the two repositories as siblings:

```sh
git clone <buildroot-external-avaota-url> buildroot-external-avaota
git clone <linux-v821-url> linux-v821
git clone https://github.com/buildroot/buildroot.git buildroot
cd buildroot
git checkout 2024.02.3
make BR2_EXTERNAL=../buildroot-external-avaota \
     O=../output avaota_f1_defconfig
make O=../output
```

The important files are `output/images/Image.gz`,
`output/images/avaota-f1.dtb`, `output/images/rootfs.squashfs`,
`output/images/avaota-f1-spinor.raw`, and `output/images/avaota-f1-sd.img`.
The post-image hook invokes the sibling `bootloader-v821` repository to create
the raw SPI-NOR image and the Allwinner Phoenix/Dragon SD image.
