PRG := st-util
OBJS = gdb-remote.o gdb-server.o

CFLAGS+=$(ARCH) -g -Wall -Werror -std=gnu99 -I../src -DCONFIG_WIN32=$(CONFIG_WIN32)
LDFLAGS=$(ARCH) -L.. -lstlink

ifeq ($(CONFIG_WIN32), 1)
  LDFLAGS+= -lws2_32 -lwsock32
endif

# libusb location
LDFLAGS+=`pkg-config --libs libusb-1.0`
CFLAGS+=`pkg-config --cflags libusb-1.0`

all: $(PRG)

$(PRG): $(OBJS)  ../libstlink.a
	$(CC) -o $@  $^ $(LDFLAGS)

clean:
	rm -rf $(OBJS)
	rm -rf $(PRG)

.PHONY: clean all
