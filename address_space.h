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
#include <thumb2sim.h>

/*************** AUTO GENERATED SECTION FOLLOWS ***************/
const uint8_t *addrspace_memptr(const struct addrspace_t *address_space, uint32_t address);
uint8_t addrspace_read8(struct addrspace_t *address_space, uint32_t address);
void addrspace_write8(struct addrspace_t *address_space, uint32_t address, uint8_t value);
void addrspace_write16(struct addrspace_t *address_space, uint32_t address, uint16_t value);
uint16_t addrspace_read16(struct addrspace_t *address_space, uint32_t address);
uint32_t addrspace_read32(struct addrspace_t *address_space, uint32_t address);
void addrspace_write32(struct addrspace_t *address_space, uint32_t address, uint32_t value);
void addrspace_add_region(struct addrspace_t *address_space, uint32_t start_addr, uint32_t length, uint8_t *data, bool read_only);
void addrspace_init(struct addrspace_t *address_space);
/***************  AUTO GENERATED SECTION ENDS   ***************/

#endif
