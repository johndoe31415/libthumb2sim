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

static struct addressSlice *getSlice(struct addressSpace *aAddressSpace, uint32_t aAddress) {
	for (int i = 0; i < aAddressSpace->sliceCnt; i++) {
		if ((aAddress >= aAddressSpace->slices[i].begin) && (aAddress < aAddressSpace->slices[i].end)) {
			return aAddressSpace->slices + i;
		}
	}
	fprintf(stderr, "Page fault: Ilegal access to 0x%x (no mapped slice).\n", aAddress);
	exit(EXIT_FAILURE);
	return NULL;
}

const uint8_t *getMemoryAt(const struct addressSpace *aAddressSpace, uint32_t aAddress) {
	struct addressSlice *slice = getSlice((struct addressSpace*)aAddressSpace, aAddress);
	return slice->data + (aAddress - slice->begin);
}

uint8_t memRead8(struct addressSpace *aAddressSpace, uint32_t aAddress) {
	struct addressSlice *slice = getSlice(aAddressSpace, aAddress);
	uint8_t value = slice->data[aAddress - slice->begin];
#ifdef DEBUG_MEMORY_ACCESS
	fprintf(stderr, "Read  8: [0x%x] = 0x%02x\n", aAddress, value);
#endif
	return value;
}

void memWrite8(struct addressSpace *aAddressSpace, uint32_t aAddress, uint8_t aValue) {
	struct addressSlice *slice = getSlice(aAddressSpace, aAddress);
	slice->data[aAddress - slice->begin] = aValue;
#ifdef DEBUG_MEMORY_ACCESS
	fprintf(stderr, "Write 8: [0x%x] = 0x%02x\n", aAddress, aValue);
#endif
}

void memWrite16(struct addressSpace *aAddressSpace, uint32_t aAddress, uint16_t aValue) {
	struct addressSlice *sliceBegin = getSlice(aAddressSpace, aAddress);
#ifdef DEBUG_MEMORY_ACCESS
	struct addressSlice *sliceEnd = getSlice(aAddressSpace, aAddress + 1);
	if (sliceBegin != sliceEnd) {
		fprintf(stderr, "Out of bounds access for 16 bit read.\n");
		exit(EXIT_FAILURE);
	}
#endif

#ifdef DEBUG_MEMORY_ACCESS
	fprintf(stderr, "Write 16: [0x%x] = 0x%04x\n", aAddress, value);
#endif
	
	uint16_t *location = (uint16_t*)(sliceBegin->data + aAddress - sliceBegin->begin);
	*location = aValue;
}

uint16_t memRead16(struct addressSpace *aAddressSpace, uint32_t aAddress) {
	struct addressSlice *sliceBegin = getSlice(aAddressSpace, aAddress);
#ifdef DEBUG_MEMORY_ACCESS
	struct addressSlice *sliceEnd = getSlice(aAddressSpace, aAddress + 1);
	if (sliceBegin != sliceEnd) {
		fprintf(stderr, "Out of bounds access for 16 bit read.\n");
		exit(EXIT_FAILURE);
	}
#endif
	uint32_t value = *((uint16_t*)(sliceBegin->data + aAddress - sliceBegin->begin));

#ifdef DEBUG_MEMORY_ACCESS
	fprintf(stderr, "Read 16: [0x%x] = 0x%04x\n", aAddress, value);
#endif
	return value;
}

uint32_t memRead32(struct addressSpace *aAddressSpace, uint32_t aAddress) {
	struct addressSlice *sliceBegin = getSlice(aAddressSpace, aAddress);
#ifdef DEBUG_MEMORY_ACCESS
	struct addressSlice *sliceEnd = getSlice(aAddressSpace, aAddress + 3);
	if (sliceBegin != sliceEnd) {
		fprintf(stderr, "Out of bounds access for 32 bit read.\n");
		exit(EXIT_FAILURE);
	}
#endif
	uint32_t value = *((uint32_t*)(sliceBegin->data + aAddress - sliceBegin->begin));
#ifdef DEBUG_MEMORY_ACCESS
	fprintf(stderr, "Read 32: [0x%x] = 0x%08x\n", aAddress, value);
#endif
	return value;
}

void memWrite32(struct addressSpace *aAddressSpace, uint32_t aAddress, uint32_t aValue) {
	struct addressSlice *sliceBegin = getSlice(aAddressSpace, aAddress);
#ifdef DEBUG_MEMORY_ACCESS
	struct addressSlice *sliceEnd = getSlice(aAddressSpace, aAddress);
	if (sliceBegin != sliceEnd) {
		fprintf(stderr, "Out of bounds access for 32 bit read.\n");
		exit(EXIT_FAILURE);
	}
#endif
	//fprintf(stderr, "Write 32: [0x%x] = 0x%08x\n", aAddress, aValue);
#ifdef DEBUG_MEMORY_ACCESS
	fprintf(stderr, "Write 32: [0x%x 0x%x 0x%x 0x%x] = 0x%08x\n", aAddress, aAddress + 1, aAddress + 2, aAddress + 3, aValue);
#endif
	uint32_t *location = (uint32_t*)(sliceBegin->data + aAddress - sliceBegin->begin);
	*location = aValue;
}

void addMemory(struct addressSpace *aAddressSpace, uint32_t aStartAddress, uint32_t aLength, uint8_t *aData, bool aReadOnly) {
	if (aAddressSpace->sliceCnt == MAX_ADDRESS_SLICES) {
		fprintf(stderr, "Cannot add memory: Slices exhausted\n");
		exit(EXIT_FAILURE);
	}

	int sliceNo = aAddressSpace->sliceCnt;
	aAddressSpace->sliceCnt++;
	aAddressSpace->slices[sliceNo].begin = aStartAddress;
	aAddressSpace->slices[sliceNo].end = aStartAddress + aLength;
	aAddressSpace->slices[sliceNo].data = aData;
	aAddressSpace->slices[sliceNo].readOnly = aReadOnly;
	fprintf(stderr, "Added memory: at 0x%x len %x Readonly %d\n", aStartAddress, aLength, aReadOnly);
}

void initAddressSpace(struct addressSpace *aAddressSpace) {
	memset(aAddressSpace, 0, sizeof(struct addressSpace));
	
}
