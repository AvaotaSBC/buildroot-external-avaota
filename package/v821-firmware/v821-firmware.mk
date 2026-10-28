################################################################################
#
# v821-firmware
#
################################################################################

V821_FIRMWARE_VERSION = 1.0
V821_FIRMWARE_SITE = $(BR2_EXTERNAL_AVAOTAF1_PATH)/package/v821-firmware/firmware
V821_FIRMWARE_SITE_METHOD = local

define V821_FIRMWARE_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0644 $(@D)/boot_v821.bin $(TARGET_DIR)/lib/firmware/boot_v821.bin
	$(INSTALL) -D -m 0644 $(@D)/etf_v821.bin $(TARGET_DIR)/lib/firmware/etf_v821.bin
	$(INSTALL) -D -m 0644 $(@D)/fw_mac_v821.bin $(TARGET_DIR)/lib/firmware/fw_mac_v821.bin
	$(INSTALL) -D -m 0644 $(@D)/sdd_v821.bin $(TARGET_DIR)/lib/firmware/sdd_v821.bin
endef

$(eval $(generic-package))
