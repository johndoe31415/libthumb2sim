.PHONY: all 

CC := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
CFLAGS := -O3 -std=c11 -I../include -Wall -Wmissing-prototypes -Wstrict-prototypes -Werror=implicit-function-declaration -Wshadow
CFLAGS += -g -D_XOPEN_SOURCE=500

LDFLAGS := -Tstm32f407.ld

OBJS := ivt.o emu_syscall.o md5.o boilerplate.o
TARGETS := simple_test.bin md5_test.bin

all: $(TARGETS)

clean:
	rm -f $(OBJS) $(TARGETS)
	rm -f simple_test md5_test

simple_test: $(OBJS) simple_test.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

simple_test.bin: simple_test
	$(OBJCOPY) -O binary $< $@

md5_test: $(OBJS) md5_test.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

md5_test.bin: md5_test
	$(OBJCOPY) -O binary $< $@

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

.s.o:
	$(CC) $(CFLAGS) -c -o $@ $<
