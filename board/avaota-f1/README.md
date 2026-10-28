# Avaota-F1 Buildroot board

## What is built

This board directory connects Buildroot to the self-contained `linux-v821`
repository next to this repository. The build uses Buildroot's internal
RISC-V 32-bit glibc toolchain (`rv32imafdc`, `ilp32d`), the Tina-exported
5.4.220 V821 kernel/BSP, the Avaota-F1 device tree, and a compressed SquashFS
root filesystem. No Tina toolchain or Tina makefiles are required.

The V821 NOR partition exported by Tina allocates 11136 sectors for `rootfs`
and uses a SquashFS image. The external tree therefore selects SquashFS with
xz compression rather than an uncompressed ext4 image.

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
`output/images/avaota-f1.dtb`, `output/images/rootfs.squashfs`, and the
generated `avaota-f1-build-manifest.txt`. The bootloader and final Allwinner
NOR container are intentionally separate from this Buildroot external tree;
the copied `tina-boot/` files record the original board contract for the next
boot/flash integration step.
