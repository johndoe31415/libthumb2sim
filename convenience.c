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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <thumb2sim.h>

#include "address_space.h"
#include "convenience.h"

const struct hardware_params_t default_parameters = {
	.rom_size_bytes = 1024 * 1024,
	.ram_size_bytes = 128 * 1024,
	.ivt_base_address = 0x0,
	.rom_base_address = 0x0,
	.ram_base_address = 0x20000000,
	.rom_image_filename = NULL,
	.ram_image_filename = NULL,
};

static bool load_file(const char *img_name, const char *filename, void *memory, unsigned int max_length) {
	bool success = true;
	if (!filename) {
		return success;
	}
	struct stat statbuf;
	if (stat(filename, &statbuf)) {
		fprintf(stderr, "%s image / file %s -- stat failed: %s\n", img_name, filename, strerror(errno));
		return false;
	}

	if (statbuf.st_size > max_length) {
		fprintf(stderr, "%s image / file %s: file size is %lu bytes, but memory region only %u bytes. Truncated memory.\n", img_name, filename, statbuf.st_size, max_length);
		success = false;
	}

	FILE *f = fopen(filename, "r");
	if (!f) {
		fprintf(stderr, "%s image / file %s -- fopen failed: %s\n", img_name, filename, strerror(errno));
		return false;
	}

	unsigned int read_length = (statbuf.st_size < max_length) ? statbuf.st_size : max_length;
	if (fread(memory, read_length, 1, f) != 1) {
		fprintf(stderr, "%s image / file %s -- short fread: %s\n", img_name, filename, strerror(errno));
		success = false;
	}

	fclose(f);
	return success;
}

struct emu_ctx_t* init_cortexm(const struct hardware_params_t *hwparams) {
	if (!hwparams) {
		hwparams = &default_parameters;
	}

	struct emu_ctx_t *emu_ctx = calloc(sizeof(struct emu_ctx_t), 1);
	void *rom = calloc(hwparams->rom_size_bytes, 1);
	void *ram = calloc(hwparams->ram_size_bytes, 1);
	if (!emu_ctx || !rom || !ram) {
		perror("calloc");
		free(emu_ctx);
		free(rom);
		free(ram);
		return NULL;
	}

	emu_ctx->ivt_base_address = hwparams->ivt_base_address;
	addrspace_init(&emu_ctx->addr_space);
	addrspace_add_region(&emu_ctx->addr_space, "rom", hwparams->rom_base_address, hwparams->rom_size_bytes, rom, true, false);
	addrspace_add_region(&emu_ctx->addr_space, "ram", hwparams->ram_base_address, hwparams->ram_size_bytes, ram, false, false);

	/* Now load images if they're available */
	load_file("rom", hwparams->rom_image_filename, rom, hwparams->rom_size_bytes);
	load_file("ram", hwparams->ram_image_filename, ram, hwparams->ram_size_bytes);

	cpu_reset(emu_ctx);
	return emu_ctx;
}

void free_cortexm(struct emu_ctx_t *emu_ctx) {
	free(emu_ctx->addr_space.slices[0].data);
	free(emu_ctx->addr_space.slices[1].data);
	free(emu_ctx);
}
