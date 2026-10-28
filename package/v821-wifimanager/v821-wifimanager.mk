################################################################################
#
# v821-wifimanager
#
################################################################################

V821_WIFIMANAGER_VERSION = 2.0
V821_WIFIMANAGER_SITE = $(BR2_EXTERNAL_AVAOTAF1_PATH)/package/v821-wifimanager/src
V821_WIFIMANAGER_SITE_METHOD = local
V821_WIFIMANAGER_DEPENDENCIES = v821-firmware

define V821_WIFIMANAGER_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define V821_WIFIMANAGER_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/libwirelesscom.so \
		$(TARGET_DIR)/lib/libwirelesscom.so
	$(INSTALL) -D -m 0755 $(@D)/libwifimg-v2.0.so \
		$(TARGET_DIR)/lib/libwifimg-v2.0.so
	$(INSTALL) -D -m 0755 $(@D)/wifi \
		$(TARGET_DIR)/bin/wifi
	$(INSTALL) -D -m 0755 $(@D)/wifi_daemon \
		$(TARGET_DIR)/bin/wifi_daemon
	$(INSTALL) -D -m 0644 $(@D)/wifimg.config \
		$(TARGET_DIR)/etc/wifi/wifimg.config
	$(INSTALL) -D -m 0644 $(@D)/wifimg.config \
		$(TARGET_DIR)/etc/wifi/wifimg.def
endef

$(eval $(generic-package))
