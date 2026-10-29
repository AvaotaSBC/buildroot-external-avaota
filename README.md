# Buildroot external tree for Avaota-F1 / Allwinner V821

This repository contains only the Buildroot external tree for Avaota-F1. It is
not a distribution framework and it does not contain unrelated board
configurations, graphics userspace, or distribution-specific patches. The
external tree was checked against the V821 Avaota-F1 board files and the
RISC-V32 kernel configuration.

## Repository split

| Repository | Contents |
| --- | --- |
| `buildroot-external-avaota` | Buildroot board configuration, package, firmware and rootfs/image hooks |
| `linux-v821` | Linux 5.4.220 plus the V821 BSP and Avaota-F1 DTS/configuration |
| `u-boot-v821` | Standalone V821 U-Boot source and Avaota-F1 NOR/SD U-Boot build wrapper |
| `avaota-image-tools` | Reusable Allwinner image assembly; currently the `boards/avaota-f1` category |

The four repositories are siblings in the verification workspace and are
intended to become four independent repositories. The external tree refers to
the kernel through a local Git URL so the exact source is reproducible without
Tina. After publishing `linux-v821`, change the one repository URL in
`configs/avaota_f1_defconfig` to the published Git URL.

## Source boundary

The final build has four source repositories and one upstream Buildroot
checkout:

- `buildroot-external-avaota` (this repository) owns Buildroot configuration,
  rootfs policy, board hooks, and the optional V821 runtime firmware package.
- `linux-v821` owns Linux 5.4.220, the V821 BSP, and the F1 DTS/configuration.
  The kernel and BSP source were exported from the local V821 BSP into that
  repository; no Tina checkout is read during a build.
- `u-boot-v821` owns the V821 U-Boot source, the two F1 storage-specific board
  DTS inputs, and the out-of-tree build wrapper. It follows the Buildroot
  bootloader boundary: U-Boot is built from source with the Buildroot
  RISC-V32 compiler.
- `avaota-image-tools` owns the reusable Allwinner host tools and image
  assembly. Its current `boards/avaota-f1` category contains the FEX
  partition/environment inputs and board-specific prebuilt boot stages. It
  calls the sibling `u-boot-v821` wrapper and
  does not contain a second copy of U-Boot.
- `buildroot` is the upstream Buildroot 2024.02.3 checkout.

Vendor-generated rootfs/makefiles and the vendor toolchain are not build
inputs. Buildroot supplies the userspace, its internal RISC-V32 toolchain, and
the final SquashFS image. The V821 E907/FMAC runtime assets are shipped by the
optional `v821-firmware` package. The optional `v821-wifimanager` package
contains the F1 Tina-selected XRLINK STA/AP control path, including
`wifi_daemon` and `wifi`; it is a source package in this repository and does
not read Tina at build time.

## ADB / adbd

Tina's V821 implementation was checked against
`platform/allwinner/usb/adbd` and `platform/allwinner/usb/setusbconfig`. Its
runtime contract is FunctionFS, not the obsolete `android_usb` sysfs API:
`ffs.adb` is linked into a configfs gadget, `/dev/usb-ffs/adb` is mounted, and
`adbd` publishes its descriptors before the Sunxi UDC is bound. Tina starts
the daemon with `adbd -D` and uses `/bin/sh` for shell services.

This external tree selects Buildroot's native `android-tools` `adbd` package
(`BR2_PACKAGE_ANDROID_TOOLS_ADBD`) and keeps the same V821 contract in
`board/avaota-f1/rootfs-overlay/etc/init.d/S30adbd`. The service creates the
F1 gadget with VID:PID `1f3a:1002`, derives the serial number from
`androidboot.serialno`/`snum`, mounts FunctionFS, starts `/usr/bin/adbd -D`,
and binds the first UDC exported by the kernel. Buildroot's package patch
uses `ADBD_NON_ANDROID`, so an ADB shell executes the Buildroot `/bin/sh`
instead of Tina's `/system/bin/sh`.

The standalone kernel configuration explicitly includes
`CONFIG_USB_CONFIGFS_F_FS`, `CONFIG_USB_F_FS`, and `CONFIG_CONFIGFS_FS` in
addition to the V821 Sunxi UDC options. No Tina `adbd` binary, old Android
source tree, `setusbconfig` package, or vendor toolchain is a Buildroot input.

The Linux extension file supplies the two variables that the legacy V821 BSP
makefiles need:

```make
LINUX_MAKE_FLAGS += BSP_TOP=bsp/
LINUX_MAKE_FLAGS += KERNEL_SRC_DIR=$(LINUX_DIR)
```

The first points the BSP Kconfig at the standalone BSP directory. The second
lets the legacy V821 NAND makefile resolve the in-tree kernel without a Tina
output directory. The few legacy include paths that assumed Tina's layout are
fixed in `linux-v821` so the same source builds from its Git checkout.

## Toolchain and architecture

- Buildroot internal toolchain, not a Tina toolchain.
- RISC-V 32-bit, `rv32imafdc`, ABI `ilp32d`.
- glibc and Linux 5.4 headers.
- The kernel BSP's `BSP_TOP=bsp/` requirement is supplied by
  `linux/linux-ext-v821.mk`.

## Outputs

The verified build emits:

- `Image.gz`
- `avaota-f1.dtb`
- `rootfs.squashfs` (xz compressed, suitable for the F1 NOR rootfs slot)
- build and image manifests
- `avaota-f1-flash.img` (common Allwinner Phoenix/Dragon image usable for
  SPI-NOR and SD flashing)
- `avaota-f1-spinor.raw` (raw SPI-NOR image)
- `avaota-f1-sd.raw` (DOS/MBR-partitioned, bootable raw SD image; root is p2)

The `post-image.sh` hook calls
`../avaota-image-tools/pack-avaota-f1-images.sh` after Buildroot has
produced the kernel, DTB and rootfs. The image-tools script calls
`../u-boot-v821/scripts/build-u-boot-v821.sh` for both storage variants, creates the
Android header-v2 `boot.fex`, and assembles the common Allwinner image plus
the two raw images. The common image follows Tina's packaging model: it
contains both storage-specific boot files and uses the NOR partition map so
the same Phoenix/Dragon file can be used for SPI-NOR or SD flashing. Set
`AVAOTA_IMAGE_TOOLS_DIR` and/or `AVAOTA_UBOOT_DIR` if these repositories are
not the default sibling directories.

The F1 board-specific decisions are intentional:

- The kernel is RV32 (`rv32imafdc`, `ilp32d`) and does not enable a board GPU
  or DRM userspace.
- The F1 DTS disables `gmac0` and `mdio0`; the Buildroot image therefore does
  not install an automatic wired DHCP policy.
- The boot environment and BusyBox getty use the V821 `uart-ng` device
  `ttyS0` at 115200 baud. (`ttyAS0` is only selected when the kernel is built
  with its 8250 compatibility layer, which this configuration does not use.)
- The kernel command line keeps the board's `/init` contract while the target
  rootfs remains Buildroot's normal BusyBox init.
- V821 Wi-Fi starts through the E907 remoteproc and `v821_fmac` module before
  the XRLINK `wifi_daemon`. This is the vendor V821 control protocol, not a
  generic cfg80211/wpa_supplicant substitution.

The V821 Boot0/FES/PMBoot binaries remain board-specific prebuilt assets:
their DRAM code uses Andes instructions and vendor relocation types that the
generic Buildroot binutils cannot link. U-Boot, Linux, DTB, rootfs and image
layout/packaging are built independently. The image-tools repository contains
the required Allwinner host utilities and Dragon plugin files, but the host
still needs a compatible 32-bit glibc/libstdc++ runtime for the Phoenix tool.

## Reproducible verification

From a workspace containing the five sibling directories `buildroot`,
`buildroot-external-avaota`, `linux-v821`, `u-boot-v821`, and
`avaota-image-tools`:

```sh
make -C buildroot \
  BR2_EXTERNAL="$PWD/buildroot-external-avaota" \
  O="$PWD/output" avaota_f1_defconfig
make -C buildroot O="$PWD/output" -j"$(nproc)"
```

The final images are written to `output/images/`. To run only the image
integration again after the normal Buildroot build:

```sh
make -C buildroot O="$PWD/output" target-post-image
```

The verified configuration uses Buildroot's internal RISC-V32 glibc
toolchain (`riscv32-buildroot-linux-gnu-`), not a Tina compiler. The kernel
build is `ARCH=riscv`, uses the `ilp32d` ABI, and produces Linux 5.4.220.
The expected intermediate images are `Image.gz`, `avaota-f1.dtb`, and an
xz-compressed `rootfs.squashfs`; `post-image.sh` also writes the two flashable
images, manifests, and the raw SD image with a standard DOS/MBR plus the
Allwinner SD boot areas. The SD root is `/dev/mmcblk0p2`; the SPI-NOR root is
`/dev/mtdblock6` (`rootfs`; `rootfs_data` is mtd7) with the V821 NOR logical
offset supplied by U-Boot.
