.PHONY: all clean gdb test client

CFLAGS := -O3 -std=c11 -I../include -Wall -Wmissing-prototypes -Wstrict-prototypes -Werror=implicit-function-declaration -Wimplicit-fallthrough -Wshadow
CFLAGS += -g -D_XOPEN_SOURCE=500

LDFLAGS := -L.. -lthumb2sim

OBJS := gdbserver.o
TARGETS := t2sim-gdbserver
TEST_IMG := ../qemu/qemu_test.bin

all: $(TARGETS)

clean:
	rm -f $(OBJS) $(TARGETS)

test: all
	./t2sim-gdbserver $(TEST_IMG) test_socket

client:
	rm -f test_socket
	socat - unix-listen:test_socket

gdb: all
	gdb --args ./t2sim-gdbserver $(TEST_IMG) test_socket

t2sim-gdbserver: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<
