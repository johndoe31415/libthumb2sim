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

#ifndef __ADDRESS_SPACE_H__
#define __ADDRESS_SPACE_H__

#include <stdint.h>
#include <stdbool.h>

#define MAX_ADDRESS_SLICES		4

struct address_slice_t {
	uint32_t begin;
	uint32_t end;
	uint8_t *data;
	bool readOnly;
};

struct addressSpace {
	int sliceCnt;
	struct address_slice_t slices[MAX_ADDRESS_SLICES];
};

/*************** AUTO GENERATED SECTION FOLLOWS ***************/
const uint8_t *getMemoryAt(const struct addressSpace *address_space, uint32_t address);
uint8_t memRead8(struct addressSpace *address_space, uint32_t address);
void memWrite8(struct addressSpace *address_space, uint32_t address, uint8_t aValue);
void memWrite16(struct addressSpace *address_space, uint32_t address, uint16_t aValue);
uint16_t memRead16(struct addressSpace *address_space, uint32_t address);
uint32_t memRead32(struct addressSpace *address_space, uint32_t address);
void memWrite32(struct addressSpace *address_space, uint32_t address, uint32_t aValue);
void addMemory(struct addressSpace *address_space, uint32_t aStartAddress, uint32_t length, uint8_t *data, bool aReadOnly);
void initAddressSpace(struct addressSpace *address_space);
/***************  AUTO GENERATED SECTION ENDS   ***************/

#endif
