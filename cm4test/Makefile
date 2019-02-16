.PHONY: all 

CC := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
CFLAGS := -O3 -std=c11 -Iinclude -Wall -Wmissing-prototypes -Wstrict-prototypes -Werror=implicit-function-declaration -Wshadow
CFLAGS += -g -D_XOPEN_SOURCE=500

LDFLAGS := -Tstm32f407.ld

OBJS := cm4test.o ivt.o
TARGETS := cm4test cm4test.bin

all: $(TARGETS)

clean:
	rm -f $(OBJS) $(TARGETS)

cm4test: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

cm4test.bin: cm4test
	$(OBJCOPY) -O binary $< $@

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

.s.o:
	$(CC) $(CFLAGS) -c -o $@ $<
