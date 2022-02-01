.PHONY: all clean install uninstall disas test gdb app gdbserver

CFLAGS := -O3 -shared -fPIC -std=c11 -Iinclude -Wall -Wmissing-prototypes -Wstrict-prototypes -Werror=implicit-function-declaration -Wimplicit-fallthrough -Wshadow
CFLAGS += -g -D_XOPEN_SOURCE=500

LDFLAGS :=

OBJS := address_space.o cpu_cm3.o decoder.o hexdump.o impl_disassembly.o impl_emulation.o convenience.o
TARGETS := libthumb2sim.so

INSTALL_LIB_DIR := /usr/local/lib
INSTALL_INC_DIR := /usr/local/include
VERSION := 0.0.1

all: $(TARGETS) app gdbserver

app:
	make -C app

gdbserver:
	make -C gdbserver

clean:
	rm -f $(OBJS) $(TARGETS)
	make -C app clean
	make -C gdbserver clean

install: libthumb2sim.so
	cp libthumb2sim.so "$(INSTALL_LIB_DIR)/libthumb2sim.so.$(VERSION)"
	chown root:root "$(INSTALL_LIB_DIR)/libthumb2sim.so.$(VERSION)"
	chmod 755 "$(INSTALL_LIB_DIR)/libthumb2sim.so.$(VERSION)"
	strip "$(INSTALL_LIB_DIR)/libthumb2sim.so.$(VERSION)"
	ln -sf "libthumb2sim.so.$(VERSION)" "$(INSTALL_LIB_DIR)/libthumb2sim.so"
	chown -h root:root "$(INSTALL_LIB_DIR)/libthumb2sim.so"
	ldconfig
	cp -r include/. "$(INSTALL_INC_DIR)/thumb2sim"
	chmod 755 "$(INSTALL_INC_DIR)/thumb2sim"
	chmod 644 "$(INSTALL_INC_DIR)/thumb2sim/"*.h

uninstall:
	rm -fr "$(INSTALL_INC_DIR)/thumb2sim"
	rm -f "$(INSTALL_LIB_DIR)/libthumb2sim.so.$(VERSION)"
	rm -f "$(INSTALL_LIB_DIR)/libthumb2sim.so"

disas:
	./objdump -D rom.bin -M force-thumb -b binary -m arm

test: all
	make -C app test

gdb: all
	make -C app gdb

libthumb2sim.so: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<
