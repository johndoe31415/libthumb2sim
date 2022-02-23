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

enum emu_dump_t {
	MULTI_FILE_RW_MEMORY
};

struct address_slice_t {
	uint32_t begin;
	uint32_t end;
	uint8_t *data;
	bool read_only;
	bool shadow_mapping;
	const char *name;
};

struct addrspace_t {
	int slice_cnt;
	struct address_slice_t slices[MAX_ADDRESS_SLICES];
};

#define REG_R0		0
#define REG_R1		1
#define REG_R2		2
#define REG_R3		3
#define REG_R4		4
#define REG_R5		5
#define REG_R6		6
#define REG_R7		7
#define REG_R8		8
#define REG_R9		9
#define REG_R10		10
#define REG_R11		11
#define REG_R12		12
#define REG_R13		13
#define REG_SP		13
#define REG_R14		14
#define REG_LR		14
#define REG_R15		15
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
	uint32_t insn_ctr;
	uint8_t it_cond;
	uint8_t it_state;
};

struct emu_ctx_t;

/* Called when a breakpoint is emulated */
typedef void (*bkpt_callback_t)(struct emu_ctx_t *emu_ctx, uint8_t bkpt_number);

/* Called when running the CPU so the user can customize conditions at which to
 * abort emulation */
typedef bool (*end_emulation_callback_t)(struct emu_ctx_t *emu_ctx);

/* Called when single stepped operation is requested */
typedef void (*post_step_callback_t)(struct emu_ctx_t *emu_ctx);

/* Syscalls for communication between guest/host */
typedef uint32_t (*syscall_read_t)(struct emu_ctx_t *emu_ctx, void *data, uint32_t max_length);
typedef void (*syscall_write_t)(struct emu_ctx_t *emu_ctx, const void *data, uint32_t length);
typedef void (*syscall_puts_t)(struct emu_ctx_t *emu_ctx, const char *msg);
typedef void (*syscall_exit_t)(struct emu_ctx_t *emu_ctx, uint32_t status);

struct emu_ctx_t {
	struct cm3_cpu_state_t cpu;
	struct addrspace_t addr_space;
	uint32_t ivt_base_address;
	bkpt_callback_t bkpt_callback;
	end_emulation_callback_t end_emulation_callback;
	post_step_callback_t post_step_callback;
	syscall_read_t emulator_syscall_read;
	syscall_write_t emulator_syscall_write;
	syscall_puts_t emulator_syscall_puts;
	syscall_exit_t emulator_syscall_exit;
	bool print_disassembly;
	bool debug_instruction_decoding;
	void *user;
};

struct hardware_params_t {
	unsigned int rom_size_bytes;
	unsigned int ram_size_bytes;
	uint32_t ivt_base_address;
	uint32_t rom_base_address;
	uint32_t ram_base_address;
	const char *rom_image_filename;
	const char *ram_image_filename;
};

struct emu_ctx_t* init_cortexm(const struct hardware_params_t *hwparams);
void free_cortexm(struct emu_ctx_t *emu_ctx);

uint8_t *addrspace_memptr(struct addrspace_t *address_space, uint32_t address, unsigned int length);
uint8_t addrspace_read8(struct addrspace_t *address_space, uint32_t address);
void addrspace_write8(struct addrspace_t *address_space, uint32_t address, uint8_t value);
void addrspace_write16(struct addrspace_t *address_space, uint32_t address, uint16_t value);
uint16_t addrspace_read16(struct addrspace_t *address_space, uint32_t address);
uint32_t addrspace_read32(struct addrspace_t *address_space, uint32_t address);
void addrspace_write32(struct addrspace_t *address_space, uint32_t address, uint32_t value);
void addrspace_add_region(struct addrspace_t *address_space, const char *name, uint32_t start_addr, uint32_t length, void *data, bool read_only, bool shadow_mapping);
void addrspace_init(struct addrspace_t *address_space);

void cpu_print_state(const struct emu_ctx_t *emu_ctx);
void cpu_print_memory(struct emu_ctx_t *emu_ctx, uint32_t address, unsigned int length);
void cpu_dump_file(struct emu_ctx_t *emu_ctx, enum emu_dump_t dump_type, const char *filename);
void cpu_single_step(struct emu_ctx_t *emu_ctx);
void cpu_run(struct emu_ctx_t *emu_ctx);
void cpu_reset(struct emu_ctx_t *emu_ctx);

#endif
