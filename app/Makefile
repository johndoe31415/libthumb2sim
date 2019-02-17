.PHONY: all clean gdb test

CFLAGS := -O3 -std=c11 -I../include -Wall -Wmissing-prototypes -Wstrict-prototypes -Werror=implicit-function-declaration -Wimplicit-fallthrough -Wshadow
CFLAGS += -g -D_XOPEN_SOURCE=500

LDFLAGS := -L.. -lthumb2sim

OBJS := thumb2sim.o
TARGETS := thumb2sim
TEST_IMG := ../cm4test/simple_test.bin
#TEST_IMG := ../cm4test/md5_test.bin

all: $(TARGETS)

clean:
	rm -f $(OBJS) $(TARGETS)

test: all
	./thumb2sim $(TEST_IMG)

gdb: all
	gdb --args ./thumb2sim $(TEST_IMG)

thumb2sim: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<
