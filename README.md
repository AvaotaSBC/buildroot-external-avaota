# Buildroot external tree for Avaota-F1 / Allwinner V821

This repository is a Buildroot external tree for Avaota-F1. It is based on the
board facts in Allwinner V821 Tina and follows the external-tree structure of
the Avaota Buildroot reference repository, but it does not reuse that
repository's AArch64 Avaota-A1 configuration.

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

## How the Tina sources were converted

The conversion keeps the board-specific source boundary explicit:

1. `kernel/linux-5.4-ansc` and the V821 `bsp/` tree were combined into
   `linux-v821`. The BSP is a real directory in that repository, so its
   original Tina symlink is not needed.
2. The V821 kernel configuration was exported as
   `configs/avaota_f1_defconfig`; the Avaota-F1 DTS and its V821 SoC include
   files are built as `avaota-f1.dtb`.
3. Tina's generated `bsp/ramfs` was not copied. Buildroot owns the root
   filesystem through the rootfs overlay, the `v821-firmware` package, and
   SquashFS generation.
4. The F1 NOR FEX files and board U-Boot DTS were copied as reference inputs
   under `board/avaota-f1/tina-boot/`. The executable boot/flash integration
   lives in the separate `bootloader-v821` repository, so this external tree
   does not need Tina's proprietary pack flow.

The Linux extension file supplies the two variables that Tina normally adds
implicitly:

```make
LINUX_MAKE_FLAGS += BSP_TOP=bsp/
LINUX_MAKE_FLAGS += KERNEL_SRC_DIR=$(LINUX_DIR)
```

The first points the BSP Kconfig at the standalone BSP directory. The second
lets the legacy V821 GPU/NAND makefiles resolve the in-tree kernel without a
Tina output directory. The few legacy include paths that assumed Tina's
layout are fixed in `linux-v821` so the same source builds from its Git
checkout.

## Toolchain and architecture

- Buildroot internal toolchain, not the Tina toolchain.
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
