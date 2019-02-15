/*
	libthumb2sim - Emulator for the Thumb-2 ISA (Cortex-M)
	Copyright (C) 2014-2019 Johannes Bauer

	This file is part of libthumb2sim.

	libthumb2sim is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; this program is ONLY licensed under
	version 3 of the License, later versions are explicitly excluded.

	libthumb2sim is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with libthumb2sim; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	Johannes Bauer <JohannesBauer@gmx.de>
*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "address_space.h"
#include "cpu_cm3.h"

struct loadedFile {
	uint8_t *data;
	uint32_t length;
};

static struct loadedFile loadFile(const char *filename, uint32_t max_size, bool aAllocMax) {
	struct loadedFile lfile;
	memset(&lfile, 0, sizeof(lfile));

	struct stat sstat;
	if (stat(filename, &sstat) != 0) {
		fprintf(stderr, "Cannot open %s: %s\n", filename, strerror(errno));
		return lfile;
	}
	if (sstat.st_size > max_size) {
		fprintf(stderr, "File size of %s is greater than maximum permissible (0x%x).\n", filename, max_size);
		return lfile;
	}
	
	int allocLen = aAllocMax ? max_size : sstat.st_size;
	uint8_t *data = malloc(allocLen);
	if (!data) {
		fprintf(stderr, "Memory alloc error when reading %s (tried to alloc 0x%x bytes).\n", filename, allocLen);
		return lfile;
	}

	memset(data, 0, allocLen);
	FILE *f = fopen(filename, "rb");
	if (!f) {
		fprintf(stderr, "Cannot open %s: %s\n", filename, strerror(errno));
		free(data);
		return lfile;
	}

	if (fread(data, sstat.st_size, 1, f) != 1) {
		fprintf(stderr, "Error reading %s: %s\n", filename, strerror(errno));
		free(data);
		return lfile;
	}

	lfile.data = data;
	lfile.length = allocLen;
	return lfile;
}

#define ROM_SIZE	(128 * 1024)
#define RAM_SIZE	(128 * 1024)

int main(int argc, char **argv) {
	if (argc < 3) {
		fprintf(stderr, "%s [ROM-Image] [RAM-Image]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Load file images */
	struct loadedFile ram, rom;
	rom = loadFile(argv[1], ROM_SIZE, false);
	if (strcmp(argv[2], "-")) {
		ram = loadFile(argv[2], RAM_SIZE, true);	
	} else {
		ram.data = calloc(RAM_SIZE, 1);
		if (!ram.data) {
			fprintf(stderr, "Cannot alloc RAM\n");
			exit(EXIT_FAILURE);
		}
		ram.length = RAM_SIZE;
	}

	if (rom.length == 0) {
		fprintf(stderr, "Could not init ROM!\n");
		exit(EXIT_FAILURE);
	}
	if (ram.length == 0) {
		fprintf(stderr, "Could not init RAM!\n");
		exit(EXIT_FAILURE);
	}

	fprintf(stderr, "RAM at %p (0x%x bytes), ROM at %p (0x%x bytes)\n", ram.data, ram.length, rom.data, rom.length);

#if 0	
	struct cm3_cpu_state_t cpuState;
	memset(&cpuState, 0, sizeof(struct cm3_cpu_state_t));
	addrspace_init(&cpuState.addr_space);
	addrspace_add_region(&cpuState.addr_space, 0x0, rom.length, rom.data, true);
	addrspace_add_region(&cpuState.addr_space, 0x08000000, rom.length, rom.data, true);
	addrspace_add_region(&cpuState.addr_space, 0x20000000, ram.length, ram.data, false);
	fprintf(stderr, "Address space initialized.\n");

//	void *appContext = appInitContext();

	
	int runs = 50000;
	if (argc >= 4) {
		runs = atoi(argv[3]);
	}
	for (int i = 0; i < runs; i++) {
		cpu_reset(&cpuState);
//		fprintf(stderr, "CPU initialized and reset: SP = 0x%x, PC = 0x%x.\n", cpuState.reg[REG_SP], cpuState.reg[REG_PC]);

//		fprintf(stderr, "Application prepared for first run: SP = 0x%x, PC = 0x%x.\n", cpuState.reg[REG_SP], cpuState.reg[REG_PC]);
	
		cpu_run(&cpuState, NULL, false);
//		cpu_run(&cpuState, "tracefile.txt", true);
	}
#endif
	return 0;
}

