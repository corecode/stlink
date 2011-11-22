PRG := st-util
OBJS = gdb-remote.o gdb-server.o

CFLAGS+=-g -Wall -Werror -std=gnu99 -I../src -DCONFIG_WIN32=$(CONFIG_WIN32)
LDFLAGS=-L.. -lstlink -lusb-1.0

ifeq ($(CONFIG_WIN32), 1)
  LDFLAGS+= -lws2_32 -lwsock32
endif

all: $(PRG)

$(PRG): $(OBJS)
	$(CC) -o $@  $^ $(LDFLAGS)

clean:
	rm -rf $(OBJS)
	rm -rf $(PRG)

.PHONY: clean all
