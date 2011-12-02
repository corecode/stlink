# make ... for both stlink v1 and stlink v2 support
##
VPATH=src

SOURCES_LIB=stlink-common.c stlink-usb.c stlink-sg.c uglylogging.c
OBJS_LIB=$(SOURCES_LIB:.c=.o)
TEST_PROGRAMS=test_usb test_sg
LDFLAGS=$(ARCH) -L. -lstlink -lusb-1.0

CFLAGS+=$(ARCH)
CFLAGS+=-g
CFLAGS+=-DDEBUG=1
CFLAGS+=-std=gnu99
CFLAGS+=-Wall -Wextra

ifeq ("$(CONFIG_WIN32)","1")
	CFLAGS+=-DCONFIG_WIN32=$(CONFIG_WIN32)
endif

LIBRARY=libstlink.a

all:  $(LIBRARY) flash gdbserver $(TEST_PROGRAMS)

$(LIBRARY): $(OBJS_LIB)
	@echo "objs are $(OBJS_LIB)"
	$(AR) -cr $@ $^
	@echo "done making library"


test_sg: test_sg.o $(LIBRARY)
	@echo "building test_sg"
	$(CC)  test_sg.o $(LDFLAGS) -o $@

test_usb: test_usb.o $(LIBRARY)
	@echo "building test_usb"
	$(CC) test_usb.o $(LDFLAGS) -o $@
	@echo "done linking"

%.o: %.c
	@echo "building $^ into $@"
	$(CC) $(CFLAGS) -c $^ -o $@
	@echo "done compiling"

clean:
	rm -rf $(OBJS_LIB)
	rm -rf $(LIBRARY)
	rm -rf test_usb*
	rm -rf test_sg*
	$(MAKE) -C flash clean
	$(MAKE) -C gdbserver clean

flash:
	$(MAKE) -C flash CONFIG_WIN32="$(CONFIG_WIN32)" ARCH=$(ARCH)

gdbserver:
	$(MAKE) -C gdbserver CONFIG_USE_LIBSG="$(CONFIG_USE_LIBSG)" CONFIG_WIN32="$(CONFIG_WIN32)" ARCH=$(ARCH)

.PHONY: clean all flash gdbserver
