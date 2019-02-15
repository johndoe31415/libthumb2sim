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

#ifndef __LIBTHUMB2SIM_H__
#define __LIBTHUMB2SIM_H__

#include <stdint.h>
#include <stdbool.h>

#define MAX_ADDRESS_SLICES		4

struct address_slice_t {
	uint32_t begin;
	uint32_t end;
	uint8_t *data;
	bool readOnly;
};

struct addrspace_t {
	int sliceCnt;
	struct address_slice_t slices[MAX_ADDRESS_SLICES];
};

#define REG_SP		13
#define REG_LR		14
#define REG_PC		15

#define FLAG_NEGATIVE					(1 << 31)		// 0x80 00 00 00
#define FLAG_ZERO						(1 << 30)		// 0x40 00 00 00
#define FLAG_CARRY						(1 << 29)		// 0x20 00 00 00
#define FLAG_OVERFLOW					(1 << 28)		// 0x10 00 00 00
#define FLAG_SATURATION					(1 << 27)
#define FLAG_MASK						(FLAG_NEGATIVE | FLAG_ZERO | FLAG_CARRY | FLAG_OVERFLOW | FLAG_SATURATION)

#define TYPE_LSL		0
#define TYPE_LSR		1
#define TYPE_ASR		2
#define TYPE_ROR		3

enum it_cond_t {
	CONDITION_EQ = 0,		// Equal (==)						Z == 1
	CONDITION_NE = 1,		// Not equal (!=)					Z == 0
	CONDITION_CS = 2,		// Carry set (>= or unordered)		C == 1
	CONDITION_CC = 3,		// Carry clear (<)					C == 0
	CONDITION_MI = 4,		// Minus (<)						N == 1
	CONDITION_PL = 5,		// Plus (>= or unordered)			N == 0
	CONDITION_VS = 6,		// Overflow (unordered)				V == 1
	CONDITION_VC = 7,		// No overflow (not unordered)		V == 0
	CONDITION_HI = 8,		// Unsigned higher (> or unordered)	(C == 1) and (Z == 0)
	CONDITION_LS = 9,		// Unsigned lower or same (<=)		(C == 0) or (Z == 1)
	CONDITION_GE = 10,		// Signed >=						N == V
	CONDITION_LT = 11,		// Signed <							N != V
	CONDITION_GT = 12,		// Signed >							(Z == 0) and (N == V)
	CONDITION_LE = 13,		// Signed <=						(Z == 1) or (N != V)
	CONDITION_AL = 14,		// Always
};

enum it_state_t {
	IT_NONE = 0,
	IT_THEN = 1,
	IT_ELSE = 2
};

struct cm3_cpu_state_t {
	uint32_t reg[16];
	uint32_t psr;
	uint32_t clockcycle;
	uint8_t it_cond;
	uint8_t it_state;
};

struct emu_ctx_t;

typedef void (*bkpt_callback_t)(struct emu_ctx_t *emu_ctx, uint8_t bkpt_number);

struct emu_ctx_t {
	struct cm3_cpu_state_t cpu;
	struct addrspace_t addr_space;
	uint32_t ivt_base_address;
	bkpt_callback_t bkpt_callback;
	void *user;
	//uint32_t registerCopy[16];
};

#endif
