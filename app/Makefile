.PHONY: all clean gdb test

CFLAGS := -O3 -std=c11 -I../include -Wall -Wmissing-prototypes -Wstrict-prototypes -Werror=implicit-function-declaration -Wimplicit-fallthrough -Wshadow
CFLAGS += -g -D_XOPEN_SOURCE=500

LDFLAGS := -L.. -lthumb2sim

OBJS := thumb2sim.o
TARGETS := thumb2sim

all: $(TARGETS)

clean:
	rm -f $(OBJS) $(TARGETS)

test: all
	./thumb2sim ../cm4test/cm4test.bin

gdb: all
	gdb --args ./thumb2sim ../cm4test/cm4test.bin

thumb2sim: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<
