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
void cpu_dump_state(const struct cm3_cpu_state_t *cpu_state);
void cpu_dump_memory(struct cm3_cpu_state_t *cpu_state, uint32_t address, uint16_t length);
void cpu_dump_memory_file(struct cm3_cpu_state_t *cpu_state, const char *filename);
void cpu_run(struct cm3_cpu_state_t *cpu_state, const char *aTraceOutputFile, bool runUntilSentinel);
void cpu_reset(struct cm3_cpu_state_t *cpu_state);
/***************  AUTO GENERATED SECTION ENDS   ***************/

#endif
