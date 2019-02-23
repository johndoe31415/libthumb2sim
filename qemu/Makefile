.PHONY: all

CC := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
CFLAGS := -O3 -std=c11 -I../include -Wall -Wmissing-prototypes -Wstrict-prototypes -Werror=implicit-function-declaration -Wshadow -nostdlib

LDFLAGS := -Ttestcase.ld

TARGETS := testcase.bin
OBJS := ivt.o testcase.o
TRACE_OPTS := --pretty-json -v

all: $(TARGETS)

clean:
	rm -f $(OBJS) $(TARGETS) qemu_trace.json

gdb:
	arm-none-eabi-gdb -q -ex 'target remote :1234' -ex 'source example.py' testcase

qemu: testcase.bin
	qemu-system-arm -S -machine lm3s6965evb -display none -monitor stdio -gdb tcp::1234 -kernel testcase.bin

qemu_trace: testcase.bin
	./run_testcase.py -e qemu $(TRACE_OPTS) testcase.bin qemu_trace.json

t2sim_trace: testcase.bin
	./run_testcase.py -e t2sim $(TRACE_OPTS) testcase.bin t2sim_trace.json

user_trace: testcase.bin
	rm -f dbg_socket
	./run_testcase.py -e user --unix-socket dbg_socket $(TRACE_OPTS) testcase.bin t2sim_trace.json

testcase: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

testcase.bin: testcase
	$(OBJCOPY) -O binary $< $@

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

.s.o:
	$(CC) $(CFLAGS) -c -o $@ $<
