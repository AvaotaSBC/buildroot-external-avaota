# V821 XRLINK Wi-Fi manager

The V821 FMAC driver in `linux-v821` is not a cfg80211 driver. Its control
plane is the Tina V821 XRLINK protocol: userspace sends protocol messages
through netlink protocol 28, while data and events travel through the E907
remote core over RPMSG/RPBUF. The F1 Tina configuration selects this path with
`WMG_PLATFORM_XRLINK`, station mode, AP mode, and the `wifi_test_tool` demo.

This package contains only that selected userspace subset and the Linux
threading/logging support it needs. It deliberately does not copy Tina's
other link-distribution protocols, Android applications, or a generic
`wpa_supplicant`/`iw` control path. The rootfs startup order is:

1. `S40v821-rproc-wifi` starts `remoteproc0` if Boot0/Linux has not already
   started it, then inserts `v821_fmac.ko`.
2. `S50v821-wifidaemon` starts `wifi_daemon`, opens station mode, and enables
   XRLINK auto-reconnect, matching the F1 Tina startup script.

The package is optional in Kconfig, but the Avaota-F1 defconfig enables it.
