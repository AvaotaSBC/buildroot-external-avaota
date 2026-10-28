# Tina boot/flash reference assets

The FEX/DTS/configuration files in this directory are copied from the V821
Avaota-F1 board configuration in Tina. They document the NOR partition layout,
UART0 and SPI-NOR pin configuration, and the 2018 U-Boot board description.

The Buildroot build uses the independent `linux-v821` repository and produces
`Image.gz`, `avaota-f1.dtb`, and `rootfs.squashfs`. It does not invoke Tina's
`pack` scripts. A separate Allwinner-compatible boot/flash packer can consume
these reference assets together with a bootloader and the Buildroot images.
