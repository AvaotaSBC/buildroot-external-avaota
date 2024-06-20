################################################################################
#
# AIC8800 FIRMWARE
#
################################################################################

AIC8800_FIRMWARE_SITE_METHOD = local
AIC8800_FIRMWARE_SITE = $(BR2_EXTERNAL_AVAOTASBC_PATH)/package/aic8800-firmware
AIC8800_FIRMWARE_LICENSE = GPLv2+, GPLv3+
AIC8800_FIRMWARE_LICENSE_FILES = Copyright COPYING
FIRMWARE_DIR := $(TARGET_DIR)/lib/firmware

define AIC8800_FIRMWARE_INSTALL_TARGET_CMDS
	$(INSTALL) -d -m 0755 $(FIRMWARE_DIR)/
	cp -rv $(AIC8800_FIRMWARE_SITE)/firmware/* $(FIRMWARE_DIR);
endef

$(eval $(generic-package))
