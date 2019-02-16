.PHONY: all clean

CFLAGS := -O3 -std=c11 -Iinclude -Wall -Wmissing-prototypes -Wstrict-prototypes -Werror=implicit-function-declaration -Wimplicit-fallthrough -Wshadow
CFLAGS += -g -D_XOPEN_SOURCE=500

INSTALL_DIR := /usr/local/lib
VERSION := 0.1

SO_CFLAGS := -fPIC -shared 
SO_LDFLAGS :=

APP_CFLAGS := 
APP_LDFLAGS := -L. -L$(INSTALL_DIR) -lthumb2sim

SO_OBJS := address_space.o cpu_cm3.o hexdump.o decoder.o impl_disassembly.o impl_emulation.o rdtsc.o
APP_OBJS := app/thumb2sim.o

ALL_TARGETS := libthumb2sim.so thumb2sim

all: $(ALL_TARGETS)

clean:
	rm -f $(SO_OBJS) $(APP_OBJS) $(ALL_TARGETS)

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
	./simulator simulation_rom.bin -

libthumb2sim.so: $(SO_OBJS)
	$(CC) $(CFLAGS) $(SO_CFLAGS) $(SO_LDFLAGS) -o $@ $^

thumb2sim: $(APP_OBJS)
	$(CC) $(CFLAGS) $(SO_CFLAGS) $(SO_LDFLAGS) -o $@ $^

.c.o: $(SO_OBJS)
	$(CC) $(CFLAGS) $(SO_CFLAGS) -c -o $@ $<

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<
