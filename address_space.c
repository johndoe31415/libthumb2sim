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

//#define DEBUG_MEMORY_ACCESS

static struct address_slice_t *getSlice(struct addressSpace *address_space, uint32_t address) {
	for (int i = 0; i < address_space->sliceCnt; i++) {
		if ((address >= address_space->slices[i].begin) && (address < address_space->slices[i].end)) {
			return address_space->slices + i;
		}
	}
	fprintf(stderr, "Page fault: Ilegal access to 0x%x (no mapped slice).\n", address);
	exit(EXIT_FAILURE);
	return NULL;
}

const uint8_t *getMemoryAt(const struct addressSpace *address_space, uint32_t address) {
	struct address_slice_t *slice = getSlice((struct addressSpace*)address_space, address);
	return slice->data + (address - slice->begin);
}

uint8_t memRead8(struct addressSpace *address_space, uint32_t address) {
	struct address_slice_t *slice = getSlice(address_space, address);
	uint8_t value = slice->data[address - slice->begin];
#ifdef DEBUG_MEMORY_ACCESS
	fprintf(stderr, "Read  8: [0x%x] = 0x%02x\n", address, value);
#endif
	return value;
}

void memWrite8(struct addressSpace *address_space, uint32_t address, uint8_t aValue) {
	struct address_slice_t *slice = getSlice(address_space, address);
	slice->data[address - slice->begin] = aValue;
#ifdef DEBUG_MEMORY_ACCESS
	fprintf(stderr, "Write 8: [0x%x] = 0x%02x\n", address, aValue);
#endif
}

void memWrite16(struct addressSpace *address_space, uint32_t address, uint16_t aValue) {
	struct address_slice_t *sliceBegin = getSlice(address_space, address);
#ifdef DEBUG_MEMORY_ACCESS
	struct address_slice_t *sliceEnd = getSlice(address_space, address + 1);
	if (sliceBegin != sliceEnd) {
		fprintf(stderr, "Out of bounds access for 16 bit read.\n");
		exit(EXIT_FAILURE);
	}
#endif

#ifdef DEBUG_MEMORY_ACCESS
	fprintf(stderr, "Write 16: [0x%x] = 0x%04x\n", address, value);
#endif
	
	uint16_t *location = (uint16_t*)(sliceBegin->data + address - sliceBegin->begin);
	*location = aValue;
}

uint16_t memRead16(struct addressSpace *address_space, uint32_t address) {
	struct address_slice_t *sliceBegin = getSlice(address_space, address);
#ifdef DEBUG_MEMORY_ACCESS
	struct address_slice_t *sliceEnd = getSlice(address_space, address + 1);
	if (sliceBegin != sliceEnd) {
		fprintf(stderr, "Out of bounds access for 16 bit read.\n");
		exit(EXIT_FAILURE);
	}
#endif
	uint32_t value = *((uint16_t*)(sliceBegin->data + address - sliceBegin->begin));

#ifdef DEBUG_MEMORY_ACCESS
	fprintf(stderr, "Read 16: [0x%x] = 0x%04x\n", address, value);
#endif
	return value;
}

uint32_t memRead32(struct addressSpace *address_space, uint32_t address) {
	struct address_slice_t *sliceBegin = getSlice(address_space, address);
#ifdef DEBUG_MEMORY_ACCESS
	struct address_slice_t *sliceEnd = getSlice(address_space, address + 3);
	if (sliceBegin != sliceEnd) {
		fprintf(stderr, "Out of bounds access for 32 bit read.\n");
		exit(EXIT_FAILURE);
	}
#endif
	uint32_t value = *((uint32_t*)(sliceBegin->data + address - sliceBegin->begin));
#ifdef DEBUG_MEMORY_ACCESS
	fprintf(stderr, "Read 32: [0x%x] = 0x%08x\n", address, value);
#endif
	return value;
}

void memWrite32(struct addressSpace *address_space, uint32_t address, uint32_t aValue) {
	struct address_slice_t *sliceBegin = getSlice(address_space, address);
#ifdef DEBUG_MEMORY_ACCESS
	struct address_slice_t *sliceEnd = getSlice(address_space, address);
	if (sliceBegin != sliceEnd) {
		fprintf(stderr, "Out of bounds access for 32 bit read.\n");
		exit(EXIT_FAILURE);
	}
#endif
	//fprintf(stderr, "Write 32: [0x%x] = 0x%08x\n", address, aValue);
#ifdef DEBUG_MEMORY_ACCESS
	fprintf(stderr, "Write 32: [0x%x 0x%x 0x%x 0x%x] = 0x%08x\n", address, address + 1, address + 2, address + 3, aValue);
#endif
	uint32_t *location = (uint32_t*)(sliceBegin->data + address - sliceBegin->begin);
	*location = aValue;
}

void addMemory(struct addressSpace *address_space, uint32_t aStartAddress, uint32_t length, uint8_t *data, bool aReadOnly) {
	if (address_space->sliceCnt == MAX_ADDRESS_SLICES) {
		fprintf(stderr, "Cannot add memory: Slices exhausted\n");
		exit(EXIT_FAILURE);
	}

	int sliceNo = address_space->sliceCnt;
	address_space->sliceCnt++;
	address_space->slices[sliceNo].begin = aStartAddress;
	address_space->slices[sliceNo].end = aStartAddress + length;
	address_space->slices[sliceNo].data = data;
	address_space->slices[sliceNo].readOnly = aReadOnly;
	fprintf(stderr, "Added memory: at 0x%x len %x Readonly %d\n", aStartAddress, length, aReadOnly);
}

void initAddressSpace(struct addressSpace *address_space) {
	memset(address_space, 0, sizeof(struct addressSpace));
	
}
