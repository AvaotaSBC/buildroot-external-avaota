target = libwirelesscom.so

SRC_ROOT=$(PKG_BUILD_DIR)

INCLUDES += \
	-I $(SRC_ROOT)/api_action -I $(SRC_ROOT)/os \
	-I $(SRC_ROOT)/os/linux -I $(SRC_ROOT)/tracing \

SRC += $(wildcard $(SRC_ROOT)/api_action/*.c)
SRC += $(wildcard $(SRC_ROOT)/tracing/*.c)
SRC += $(wildcard $(SRC_ROOT)/os/linux/*.c)
OBJ=$(patsubst %.c,%.o,$(SRC))

####################################################################
install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/lib
	-@mkdir -p $(CONFIG_PREFIX)/usr/include
	@cp $(SRC_ROOT)/tracing/log_core.h  $(CONFIG_PREFIX)/usr/include
	@cp $(SRC_ROOT)/api_action/api_action.h $(CONFIG_PREFIX)/usr/include
	@cp $(SRC_ROOT)/os/*.h $(CONFIG_PREFIX)/usr/include
	@cp $(SRC_ROOT)/os/linux/*.h $(CONFIG_PREFIX)/usr/include

all: install $(target)

$(target):$(SRC)
	 $(CC) -fPIC -g -shared -DOS_NET_LINUX_OS $(CFLAGS) $(INCLUDES) $(CPPFLAGS) $(LDFLAGS) $(LIBS) $^ -o $@
	 cp $@ $(CONFIG_PREFIX)/usr/lib

.PHONY:clean all install
clean:
	rm -rf $(target)
