AvaotaSBC AvaotaA1
==================

AvaotaSBC AvaotaA1 is a Powerful Open Sourcce SBC with Allwinner T527 Octa-Core A55 SoC

How to build
============

$ make avaota_a1_defconfig
$ make

How to write the SD card
========================

Once the build process is finished you will have an image called "sdcard.img"
in the output/images/ directory.

Copy the bootable "sdcard.img" onto an SD card with "dd":

  $ sudo dd if=output/images/sdcard.img of=/dev/sdX

Connect a TTL UART to the debug connector, insert the microSD card and
plug in a USB-C cable to the PWR connector to boot the system.
