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

#ifndef __CPU_CM3_H__
#define __CPU_CM3_H__

#include <thumb2sim.h>

/*************** AUTO GENERATED SECTION FOLLOWS ***************/
void cpu_print_state(const struct emu_ctx_t *emu_ctx);
void cpu_print_memory(struct emu_ctx_t *emu_ctx, uint32_t address, unsigned int length);
void cpu_dump_file(struct emu_ctx_t *emu_ctx, enum emu_dump_t dump_type, const char *filename);
void cpu_single_step(struct emu_ctx_t *emu_ctx);
void cpu_run(struct emu_ctx_t *emu_ctx);
void cpu_reset(struct emu_ctx_t *emu_ctx);
/***************  AUTO GENERATED SECTION ENDS   ***************/

#endif
