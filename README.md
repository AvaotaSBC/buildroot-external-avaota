# Buildroot external tree for Avaota-F1 / Allwinner V821

This repository contains only the Buildroot external tree for Avaota-F1. It is
not a distribution framework and it does not contain an AvaotaOS build system,
an AArch64 board configuration, a Mesa/Panfrost graphics stack, or unrelated
board patches. The external tree was checked against the V821 Avaota-F1 board
files and the RISC-V32 kernel configuration.

## Repository split

| Repository | Contents |
| --- | --- |
| `buildroot-external-avaota` | Buildroot board configuration, package, firmware and rootfs/image hooks |
| `linux-v821` | Linux 5.4.220 plus the V821 BSP and Avaota-F1 DTS/configuration |
| `bootloader-v821` | Standalone V821 U-Boot build, Allwinner boot stages and NOR/SD image packer |

The three repositories are siblings in the verification workspace and are
intended to become three independent repositories. The external tree refers to
the kernel through a local Git URL so the exact source is reproducible without
Tina. After publishing `linux-v821`, change the one repository URL in
`configs/avaota_f1_defconfig` to the published Git URL.

## Source boundary

The final build has three source repositories and one upstream Buildroot
checkout:

- `buildroot-external-avaota` (this repository) owns Buildroot configuration,
  rootfs policy, board hooks, and the optional V821 runtime firmware package.
- `linux-v821` owns Linux 5.4.220, the V821 BSP, and the F1 DTS/configuration.
  The kernel and BSP source were exported from the local V821 BSP into that
  repository; no Tina checkout is read during a build.
- `bootloader-v821` owns U-Boot, board FEX files, boot-stage assets, Allwinner
  packing tools, and NOR/SD image assembly. The bootloader repository contains
  the board partition and environment inputs; this external tree deliberately
  does not duplicate them.
- `buildroot` is the upstream Buildroot 2024.02.3 checkout.

Tina's generated rootfs, Tina makefiles, and Tina toolchain are not build
inputs. Buildroot supplies the userspace, its internal RISC-V32 toolchain, and
the final SquashFS image. The four small V821 FMAC/remote-core firmware files
are shipped as board runtime assets by the optional `v821-firmware` package.

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
- `rootfs.squashfs` (xz compressed, suitable for the Tina NOR rootfs slot)
- `avaota-f1-rootfs-nor.squashfs` and a build manifest
- `avaota-f1-spinor.raw` (raw SPI-NOR image)
- `avaota-f1-sd.img` (Allwinner Phoenix/Dragon SD-card image)
- `avaota-f1-nor-env.fex` and `avaota-f1-sd-env.fex`

The `post-image.sh` hook calls `../bootloader-v821/pack-v821.sh` after
Buildroot has produced the kernel, DTB and rootfs. The packer rebuilds U-Boot
with Buildroot's `riscv32-buildroot-linux-gnu-` compiler, creates the Android
header-v2 `boot.fex`, and assembles the two final images. Set
`AVAOTA_BOOTLOADER_DIR` if the bootloader repository is not the default
sibling directory.

The F1 board-specific decisions are intentional:

- The kernel is RV32 (`rv32imafdc`, `ilp32d`) and does not enable DRM, Mali,
  Panfrost, Lima, or Mesa.
- The F1 DTS disables `gmac0` and `mdio0`; the Buildroot image therefore does
  not install an automatic `eth0` DHCP policy.
- The boot environment uses `ttyAS0` at 1500000 baud. The post-build hook
  adjusts BusyBox getty to the same rate because Buildroot 2024.02.3 does not
  expose a 1500000 baud Kconfig choice.
- The kernel command line keeps the board's `/init` contract while the target
  rootfs remains Buildroot's normal BusyBox init.

The V821 Boot0/FES/PMBoot binaries remain board-specific prebuilt assets:
their DRAM code uses Andes instructions and vendor relocation types that the
generic Buildroot binutils cannot link. U-Boot, Linux, DTB, rootfs and image
layout/packaging are built independently. The bootloader repository contains
the required Allwinner host utilities and Dragon plugin files, but the host
still needs a compatible 32-bit glibc/libstdc++ runtime for the Phoenix tool.

## Reproducible verification

From a workspace containing the four sibling directories `buildroot`,
`buildroot-external-avaota`, `linux-v821`, and `bootloader-v821`:

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
images, manifests, and the NOR-rootfs symlink.
