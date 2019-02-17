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

#include "address_space.h"

static struct address_slice_t *addrspace_getslice(struct addrspace_t *address_space, uint32_t address, unsigned int length) {
	for (int i = 0; i < address_space->slice_cnt; i++) {
		if ((address >= address_space->slices[i].begin) && (address + length - 1 < address_space->slices[i].end)) {
			return &address_space->slices[i];
		}
	}
	return NULL;
}

uint8_t *addrspace_memptr(struct addrspace_t *address_space, uint32_t address, unsigned int length) {
	struct address_slice_t *slice = addrspace_getslice((struct addrspace_t*)address_space, address, length);
	if (slice) {
		return slice->data + (address - slice->begin);
	} else {
		return NULL;
	}
}

static uint32_t addrspace_read_n(struct addrspace_t *address_space, uint32_t address, unsigned int length) {
	struct address_slice_t *slice = addrspace_getslice(address_space, address, length);
	uint32_t value = 0;
	if (slice) {
		if (length == 1) {
			value = *(uint8_t*)(slice->data + address - slice->begin);
		} else if (length == 2) {
			value = *(uint16_t*)(slice->data + address - slice->begin);
		} else if (length == 4) {
			value = *(uint32_t*)(slice->data + address - slice->begin);
		}
	} else {
		fprintf(stderr, "Read %2d ERR: [0x%08x]\n", 8 * length, address);
	}
	return value;
}

static void addrspace_write_n(struct addrspace_t *address_space, uint32_t address, unsigned int length, uint32_t value) {
	struct address_slice_t *slice = addrspace_getslice(address_space, address, length);
	if (slice) {
		if (!slice->read_only) {
			if (length == 1) {
				*(uint8_t*)(slice->data + address - slice->begin) = value;
			} else if (length == 2) {
				*(uint16_t*)(slice->data + address - slice->begin) = value;
			} else if (length == 4) {
				*(uint32_t*)(slice->data + address - slice->begin) = value;
			}
		} else {
			fprintf(stderr, "Write %2d RO: [0x%08x] = 0x%x\n", 8 * length, address, value);
		}
	} else {
		fprintf(stderr, "Write %2d ERR: [0x%08x] = 0x%x\n", 8 * length, address, value);
	}
}

uint8_t addrspace_read8(struct addrspace_t *address_space, uint32_t address) {
	return addrspace_read_n(address_space, address, 1);
}

void addrspace_write8(struct addrspace_t *address_space, uint32_t address, uint8_t value) {
	addrspace_write_n(address_space, address, 1, value);
}

uint16_t addrspace_read16(struct addrspace_t *address_space, uint32_t address) {
	return addrspace_read_n(address_space, address, 2);
}

void addrspace_write16(struct addrspace_t *address_space, uint32_t address, uint16_t value) {
	addrspace_write_n(address_space, address, 2, value);
}

uint32_t addrspace_read32(struct addrspace_t *address_space, uint32_t address) {
	return addrspace_read_n(address_space, address, 4);
}

void addrspace_write32(struct addrspace_t *address_space, uint32_t address, uint32_t value) {
	addrspace_write_n(address_space, address, 4, value);
}

void addrspace_add_region(struct addrspace_t *address_space, const char *name, uint32_t start_addr, uint32_t length, void *data, bool read_only, bool shadow_mapping) {
	if (address_space->slice_cnt == MAX_ADDRESS_SLICES) {
		fprintf(stderr, "Cannot add memory region: slices exhausted (%d). Increase MAX_ADDRESS_SLICES at compile time.\n", MAX_ADDRESS_SLICES);
		exit(EXIT_FAILURE);
	}

	int slice_no = address_space->slice_cnt;
	address_space->slice_cnt++;
	address_space->slices[slice_no].begin = start_addr;
	address_space->slices[slice_no].end = start_addr + length;
	address_space->slices[slice_no].data = data;
	address_space->slices[slice_no].read_only = read_only;
	address_space->slices[slice_no].shadow_mapping = shadow_mapping;
	address_space->slices[slice_no].name = name;
	fprintf(stderr, "Added memory \"%s\": at 0x%x len 0x%x read_only=%s shadow_mapping=%s\n", name, start_addr, length, read_only ? "true" : "false",  shadow_mapping ? "true" : "false");
}

void addrspace_init(struct addrspace_t *address_space) {
	memset(address_space, 0, sizeof(struct addrspace_t));
}
