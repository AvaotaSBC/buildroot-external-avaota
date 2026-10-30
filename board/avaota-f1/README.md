# Avaota-F1 Buildroot board

`avaota_f1_defconfig` selects the standalone V821 Linux 5.4.220 tree, the F1
device tree, Buildroot's RISC-V32 glibc toolchain, SquashFS/xz, V821
remoteproc/Wi-Fi support, and USB `adbd`.

Keep `buildroot-external-avaota`, `linux-v821`, `u-boot-v821`, and
`avaota-image-tools` next to Buildroot, then run:

```sh
make -C buildroot BR2_EXTERNAL="$PWD/buildroot-external-avaota" \
  O="$PWD/output" avaota_f1_defconfig
make -C buildroot O="$PWD/output" -j"$(nproc)"
```

The resulting files are `output/images/avaota-f1-spinor.img`,
`avaota-f1-spinor.raw`, and `avaota-f1-sd.raw`. Use the IMAGEWTY image with
PhoenixSuit for SPI-NOR. Write the raw SD image directly to the card; it has
a DOS/MBR partition table and uses `/dev/mmcblk0p2`. SPI-NOR uses
`/dev/mtdblock6`.

The console is `ttyS0,115200`. `S80adbd` starts FunctionFS/configfs ADB by
default; the V821 E907 remoteproc and XRLINK Wi-Fi service are started by the
board rootfs overlay.
