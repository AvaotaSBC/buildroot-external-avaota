# Buildroot external tree for Avaota-F1 / Allwinner V821

This repository is a Buildroot external tree for Avaota-F1. It is based on the
board facts in Allwinner V821 Tina and follows the external-tree structure of
the Avaota Buildroot reference repository, but it does not reuse that
repository's AArch64 Avaota-A1 configuration.

## Repository split

| Repository | Contents |
| --- | --- |
| `buildroot-external-avaota` | Buildroot board configuration, package, firmware, rootfs hooks and boot/flash reference assets |
| `linux-v821` | Linux 5.4.220 plus the V821 BSP and Avaota-F1 DTS/configuration |

The two directories are siblings in the verification workspace and are
intended to become two independent repositories. The external tree refers to
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
   under `board/avaota-f1/tina-boot/`. They are deliberately not executed by
   Buildroot because the proprietary Tina pack flow is not part of this
   repository.

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

Buildroot does not build Tina's proprietary Allwinner pack scripts. The
copied files under `board/avaota-f1/tina-boot/` preserve the original NOR
partition and board configuration for a later, independent bootloader/flash
integration.

## Reproducible verification

From a workspace containing the three sibling directories `buildroot`,
`buildroot-external-avaota`, and `linux-v821`:

```sh
make -C buildroot \
  BR2_EXTERNAL="$PWD/buildroot-external-avaota" \
  O="$PWD/output" avaota_f1_defconfig
make -C buildroot O="$PWD/output" -j"$(nproc)"
```

The verified configuration uses Buildroot's internal RISC-V32 glibc
toolchain (`riscv32-buildroot-linux-gnu-`), not a Tina compiler. The kernel
build is `ARCH=riscv`, uses the `ilp32d` ABI, and produces Linux 5.4.220.
The expected images are `Image.gz`, `avaota-f1.dtb`, and an xz-compressed
`rootfs.squashfs`; `post-image.sh` also writes a manifest and the NOR-rootfs
symlink.
