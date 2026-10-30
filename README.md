# Buildroot external tree for Avaota-F1

Buildroot-only support for the Allwinner V821 Avaota-F1. A Tina checkout and
the Tina toolchain are not build inputs.

## Sibling repositories

| Repository | Branch | Directory |
| --- | --- | --- |
| [buildroot-external-avaota](https://github.com/AvaotaSBC/buildroot-external-avaota) | `v821-avaota-f1` | `buildroot-external-avaota` |
| [linux-v8xx](https://github.com/AvaotaSBC/linux-v8xx) | `v821-avaota-f1` | `linux-v821` |
| [u-boot-v8xx](https://github.com/AvaotaSBC/u-boot-v8xx) | `v821-avaota-f1` | `u-boot-v821` |
| [avaota-image-tools](https://github.com/AvaotaSBC/avaota-image-tools) | `main` | `avaota-image-tools` |

Use Buildroot 2024.02.3 in a fifth sibling directory:

```sh
git clone --branch 2024.02.3 https://github.com/buildroot/buildroot.git buildroot
```

## Build

```sh
make -C buildroot \
  BR2_EXTERNAL="$PWD/buildroot-external-avaota" \
  O="$PWD/output" avaota_f1_defconfig
make -C buildroot O="$PWD/output" -j"$(nproc)"
```

Buildroot supplies the RISC-V32 glibc toolchain. The post-image hook calls
`avaota-image-tools`, which builds the sibling U-Boot variants and creates:

- `output/images/avaota-f1-spinor.img`: Allwinner `IMAGEWTY` image for
  SPI-NOR/PhoenixSuit;
- `output/images/avaota-f1-spinor.raw`: raw SPI-NOR image;
- `output/images/avaota-f1-sd.raw`: raw DOS/MBR SD image, written directly to
  the card, with rootfs at `/dev/mmcblk0p2`.

SPI-NOR boots `/dev/mtdblock6`. The serial console is `ttyS0` at 115200 baud;
USB `adbd` is enabled by default. V821 remoteproc and the board Wi-Fi manager
are included by the F1 configuration.
