.PHONY: all clean install disas test gdb app

CFLAGS := -O3 -shared -fPIC -std=c11 -Iinclude -Wall -Wmissing-prototypes -Wstrict-prototypes -Werror=implicit-function-declaration -Wimplicit-fallthrough -Wshadow
CFLAGS += -g -D_XOPEN_SOURCE=500

LDFLAGS :=

OBJS := address_space.o cpu_cm3.o decoder.o hexdump.o impl_disassembly.o impl_emulation.o convenience.o
TARGETS := libthumb2sim.so

all: $(TARGETS) app

app:
	make -C app

clean:
	rm -f $(OBJS) $(TARGETS)
	make -C app clean

install: libthumb2sim.so
	cp libthumb2sim.so "$(INSTALL_DIR)/libthumb2sim.so.$(VERSION)"
	chown root:root "$(INSTALL_DIR)/libthumb2sim.so.$(VERSION)"
	chmod 755 "$(INSTALL_DIR)/libthumb2sim.so.$(VERSION)"
	strip "$(INSTALL_DIR)/libthumb2sim.so.$(VERSION)"
	ln -sf "libthumb2sim.so.$(VERSION)" "$(INSTALL_DIR)/libthumb2sim.so"
	chown -h root:root "$(INSTALL_DIR)/libthumb2sim.so"
	ldconfig

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
