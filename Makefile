.PHONY: all clean

CC := gcc
CFLAGS := -Wall -O3 -pthread -std=c11 -g -D_XOPEN_SOURCE=500
LDFLAGS :=

OBJS := address_space.o app_initialization.o cpu_cm3.o hexdump.o MD5.o decoder.o impl_disassembly.o impl_emulation.o rdtsc.o

all: simulator

clean:
	rm -f $(OBJS) simulator disassembler

disas:
	./objdump -D rom.bin -M force-thumb -b binary -m arm

test: all
	./simulator simulation_rom.bin -

#	./simulator ../cm3sim/testdata/trc03_aes128/rom.bin - >dumpfile_verbose 2>&1
#	./trace_compare ../cm3sim/testdata/trc03_aes128/trace.txt trace.txt

simulator: $(OBJS) simulator.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) simulator.c

disassembler: $(OBJS) disassembler.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) disassembler.c

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<
