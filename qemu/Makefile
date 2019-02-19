.PHONY: all qemu

CC := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
CFLAGS := -O3 -std=c11 -I../include -Wall -Wmissing-prototypes -Wstrict-prototypes -Werror=implicit-function-declaration -Wshadow -nostdlib -DNO_DATA_SECTION -DNO_BSS_SECTION
CFLAGS += -g -D_XOPEN_SOURCE=500

LDFLAGS := -Tqemu.ld

TARGETS := qemu_test.bin
OBJS := ivt.o qemu_test.o

all: $(TARGETS)

clean:
	rm -f $(OBJS) $(TARGETS)

qemu: qemu_test.bin
	qemu-system-arm -S -machine lm3s6965evb -display none -monitor stdio -gdb tcp::1234 -kernel qemu_test.bin

gdb:
	arm-none-eabi-gdb -q -ex 'target remote :1234' -ex 'source example.py' qemu_test

qemu_test: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

qemu_test.bin: qemu_test
	$(OBJCOPY) -O binary $< $@

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

.s.o:
	$(CC) $(CFLAGS) -c -o $@ $<
