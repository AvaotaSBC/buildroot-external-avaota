# V821 firmware

`BR2_PACKAGE_V821_FIRMWARE` installs the board runtime assets under
`/lib/firmware`:

- `amp_rv0.bin` is the E907 remote-core firmware. Boot0 normally loads the
  same bytes from the `riscv0` boot-package partition; the filesystem copy is
  the Linux remoteproc fallback and is required for a standalone Buildroot
  root filesystem.
- `boot_v821.bin`, `fw_mac_v821.bin`, and `sdd_v821.bin` are the V821 FMAC
  files requested by the kernel driver.
- The corresponding `*_v821b.bin` files cover the V821B chip revision. The
  `etf_*` files are retained for the vendor FMAC RF-test path.

All files are board runtime assets exported from the V821 BSP. They are
versioned in this repository so the Buildroot build does not read Tina.
