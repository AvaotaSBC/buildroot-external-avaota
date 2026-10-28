CC ?= cc
CFLAGS ?=
LDFLAGS ?=

COMMON_CPPFLAGS := -DOS_NET_LINUX_OS
CORE_CPPFLAGS := -D_GNU_SOURCE=1 -DOS_NET_XRLINK_OS \
	-DDEFAULT_DEBUG_LV_INFO -DSUPPORT_STA_MODE -DSUPPORT_AP_MODE

COMMON_INCLUDES := \
	-Icommon/src/api_action \
	-Icommon/src/os \
	-Icommon/src/os/linux \
	-Icommon/src/tracing
CORE_INCLUDES := \
	-Icore/include \
	-Icore/include/os \
	-Icore/include/os/xrlink \
	$(COMMON_INCLUDES)

COMMON_SRCS := $(wildcard common/src/api_action/*.c) \
	$(wildcard common/src/os/linux/*.c) \
	$(wildcard common/src/tracing/*.c)
CORE_SRCS := \
	core/src/wifimg.c \
	core/src/wmg_common.c \
	core/src/wmg_sta.c \
	core/src/wmg_ap.c \
	core/src/config/wmg_get_config.c \
	core/src/log/wifi_log.c \
	core/src/os/xrlink/msg_netlink.c \
	core/src/os/xrlink/xrlink_common.c \
	core/src/os/xrlink/xrlink_get_config.c \
	core/src/os/xrlink/xrlink_sta.c \
	core/src/os/xrlink/xrlink_ap.c

COMMON_OBJS := $(COMMON_SRCS:%.c=build/%.o)
CORE_OBJS := $(CORE_SRCS:%.c=build/%.o)

COMMON_LIBS := -lpthread -lrt -ldl -lm

.PHONY: all clean

all: libwirelesscom.so libwifimg-v2.0.so wifi wifi_daemon

build/common/src/%.o: common/src/%.c
	mkdir -p $(dir $@)
	$(CC) $(COMMON_CPPFLAGS) $(CPPFLAGS) $(CFLAGS) $(COMMON_INCLUDES) \
		-fPIC -c $< -o $@

build/core/src/%.o: core/src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CORE_CPPFLAGS) $(CPPFLAGS) $(CFLAGS) $(CORE_INCLUDES) \
		-fPIC -c $< -o $@

libwirelesscom.so: $(COMMON_OBJS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,libwirelesscom.so \
		-o $@ $^ $(COMMON_LIBS)

libwifimg-v2.0.so: $(CORE_OBJS) libwirelesscom.so
	$(CC) $(LDFLAGS) -shared -Wl,-soname,libwifimg-v2.0.so \
		-o $@ $(CORE_OBJS) ./libwirelesscom.so $(COMMON_LIBS)

wifi: wifi.c libwifimg-v2.0.so libwirelesscom.so
	$(CC) $(CORE_CPPFLAGS) $(CPPFLAGS) $(CFLAGS) $(CORE_INCLUDES) \
		$(LDFLAGS) -o $@ $< ./libwifimg-v2.0.so ./libwirelesscom.so \
		$(COMMON_LIBS)

wifi_daemon: wifi_daemon.c libwifimg-v2.0.so libwirelesscom.so
	$(CC) $(CORE_CPPFLAGS) $(CPPFLAGS) $(CFLAGS) $(CORE_INCLUDES) \
		$(LDFLAGS) -o $@ $< ./libwifimg-v2.0.so ./libwirelesscom.so \
		$(COMMON_LIBS)

clean:
	rm -rf build libwirelesscom.so libwifimg-v2.0.so wifi wifi_daemon
