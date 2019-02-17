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
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>

#include "cpu_cm3.h"
#include "address_space.h"
#include "hexdump.h"
#include "config.h"
#include "decoder.h"
#include "impl_emulation.h"
#include "impl_disassembly.h"

static const char *register_name[] = {
	[0] = "r0",
	[1] = "r1",
	[2] = "r2",
	[3] = "r3",
	[4] = "r4",
	[5] = "r5",
	[6] = "r6",
	[7] = "r7",
	[8] = "r8",
	[9] = "r9",
	[10] = "r10",
	[11] = "r11",
	[12] = "r12",
	[REG_SP] = "sp",
	[REG_LR] = "lr",
	[REG_PC] = "pc",
};

static void print_disas_callback(struct disas_ctx_t *ctx, const char *msg, ...) {
	va_list ap;
	va_start(ap, msg);
	int l = strlen(ctx->disasBuffer);
	vsnprintf(ctx->disasBuffer + l, 128 - l, msg, ap);
	va_end(ap);
}

void cpu_print_state(const struct emu_ctx_t *emu_ctx) {
	for (int i = 0; i < 16; i++) {
		fprintf(stderr, "%-3s = %8x    ", register_name[i], emu_ctx->cpu.reg[i]);
		if ((i % 4) == 3) {
			fprintf(stderr, "\n");
		}
	}
	fprintf(stderr, "PSR = %8x    >", emu_ctx->cpu.psr);
	fprintf(stderr, "%c", (emu_ctx->cpu.psr & FLAG_NEGATIVE) ? 'N' : ' ');
	fprintf(stderr, "%c", (emu_ctx->cpu.psr & FLAG_ZERO) ? 'Z' : ' ');
	fprintf(stderr, "%c", (emu_ctx->cpu.psr & FLAG_CARRY) ? 'C' : ' ');
	fprintf(stderr, "%c", (emu_ctx->cpu.psr & FLAG_OVERFLOW) ? 'V' : ' ');
	fprintf(stderr, "%c", (emu_ctx->cpu.psr & FLAG_SATURATION) ? 'Q' : ' ');
	fprintf(stderr, "<\n");
	fprintf(stderr, "\n");
}

void cpu_print_memory(struct emu_ctx_t *emu_ctx, uint32_t address, unsigned int length) {
	uint8_t *data = addrspace_memptr(&emu_ctx->addr_space, address, length);
	hexdump_data(data, length);
}

void cpu_dump_file(struct emu_ctx_t *emu_ctx, enum emu_dump_t dump_type, const char *filename) {
	for (unsigned int i = 0; i < emu_ctx->addr_space.slice_cnt; i++) {
		const struct address_slice_t *slice = &emu_ctx->addr_space.slices[i];
		if (!slice->read_only && !slice->shadow_mapping) {
			char full_filename[256];
			snprintf(full_filename, sizeof(full_filename), "%s/slice_%s.bin", filename, slice->name);

			FILE *f = fopen(full_filename, "wb");
			if (f) {
				if (fwrite(slice->data, slice->end - slice->begin, 1, f) != 1) {
					fprintf(stderr, "%s: short write (%s)\n", full_filename, strerror(errno));
				}
				fclose(f);
			} else {
				perror(full_filename);
			}
		}
	}

	char full_filename[256];
	snprintf(full_filename, sizeof(full_filename), "%s/cpu.json", filename);
	FILE *f = fopen(full_filename, "wb");
	if (f) {
		fprintf(f, "{\n");
		fprintf(f, "	\"regs\": {\n");
		for (int i = 0; i < 16; i++) {
			fprintf(f, "		\"r%d\": %u%s\n", i, emu_ctx->cpu.reg[i], i == 15 ? "" : ",");
		}
		fprintf(f, "	},\n");
		fprintf(f, "	\"psr\": {\n");
		fprintf(f, "		\"value\": %u,\n", emu_ctx->cpu.psr);
		fprintf(f, "		\"flags\": \"");
		fprintf(f, "%s", (emu_ctx->cpu.psr & FLAG_NEGATIVE) ? "N" : "n");
		fprintf(f, "%s", (emu_ctx->cpu.psr & FLAG_ZERO) ? "Z" : "z");
		fprintf(f, "%s", (emu_ctx->cpu.psr & FLAG_CARRY) ? "C" : "c");
		fprintf(f, "%s", (emu_ctx->cpu.psr & FLAG_OVERFLOW) ? "V" : "v");
		fprintf(f, "%s", (emu_ctx->cpu.psr & FLAG_SATURATION) ? "Q" : "q");
		fprintf(f, "\"\n");
		fprintf(f, "	}\n");
		fprintf(f, "}\n");
		fclose(f);
	} else {
		perror(full_filename);
	}
}

static uint32_t addrspace_read_insn_word(struct addrspace_t *addr_space, uint32_t pc) {
	return (addrspace_read16(addr_space, pc) << 16) | addrspace_read16(addr_space, pc + 2);
}

static void cpu_debug_insn(struct emu_ctx_t *emu_ctx, uint32_t insn_word) {
	struct disas_ctx_t disasCtx = {
		.pc = emu_ctx->cpu.reg[REG_PC],
		.it_state = emu_ctx->cpu.it_state,
		.it_cond = emu_ctx->cpu.it_cond,
		.printDisassembly = print_disas_callback,
		.disasBuffer = { 0 }
	};

	decode_insn(&disasCtx, insn_word, &disassemblyCallbacks, emu_ctx->debug_instruction_decoding ? stderr : NULL);
	if (disasCtx.disasBuffer[0] == 0) {
		fprintf(stderr, "Warning: Cannot disassemble instruction 0x%08x at 0x%x\n", insn_word, emu_ctx->cpu.reg[REG_PC]);
		if (!emu_ctx->debug_instruction_decoding) {
			/* Even if we don't decode by default, when we cannot disassemble,
			 * decode again to print information about decoding */
			decode_insn(&disasCtx, insn_word, &disassemblyCallbacks, stderr);
		}
	}
	fprintf(stderr, "\n");
	fprintf(stderr, "< %5d %8x: %-30s [sp=%x] ", 1 + emu_ctx->cpu.insn_ctr, emu_ctx->cpu.reg[REG_PC], disasCtx.disasBuffer, emu_ctx->cpu.reg[REG_SP]);
	fprintf(stderr, "%c", (emu_ctx->cpu.psr & FLAG_NEGATIVE) ? 'N' : ' ');
	fprintf(stderr, "%c", (emu_ctx->cpu.psr & FLAG_ZERO) ? 'Z' : ' ');
	fprintf(stderr, "%c", (emu_ctx->cpu.psr & FLAG_CARRY) ? 'C' : ' ');
	fprintf(stderr, "%c", (emu_ctx->cpu.psr & FLAG_OVERFLOW) ? 'V' : ' ');
	fprintf(stderr, "%c", (emu_ctx->cpu.psr & FLAG_SATURATION) ? 'Q' : ' ');
	fprintf(stderr, "\n");
}

void cpu_single_step(struct emu_ctx_t *emu_ctx) {
	struct insn_emu_ctx_t insn_ctx = {
		.emu_ctx = emu_ctx,
		.count_next_insn = true,
		.shift_insn_it_state = true,
		.advance_pc = true,
	};
	uint32_t insn_word = addrspace_read_insn_word(&emu_ctx->addr_space, emu_ctx->cpu.reg[REG_PC]);

	if (emu_ctx->print_disassembly) {
		cpu_debug_insn(emu_ctx, insn_word);
	}

	/* Determine if execution of next instruction is unconditional or
	 * conditions are satisfied to execute */
	bool should_execute = emulator_should_exec_next_insn(&insn_ctx);

	int insn_length = decode_insn(&insn_ctx, insn_word, should_execute ? &emulation_callbacks : NULL, NULL);
	if (insn_ctx.advance_pc) {
		/* Unless we branch, we just regularly advance the PC */
		emu_ctx->cpu.reg[REG_PC] += insn_length;
	}

	if (insn_ctx.count_next_insn) {
		emu_ctx->cpu.insn_ctr++;
	}
	if (insn_ctx.shift_insn_it_state) {
		emu_ctx->cpu.it_state >>= 2;
	}

	emu_ctx->cpu.psr &= ~0xff00;
	if (emu_ctx->cpu.it_state == 0) {
		emu_ctx->cpu.psr |= 0x0100;
	} else {
		emu_ctx->cpu.psr |= 0x0900 | (emu_ctx->cpu.it_cond << 12);
		if (emu_ctx->cpu.it_state == 5) {
			emu_ctx->cpu.psr |= 0x400;
		}
	}
}

void cpu_run(struct emu_ctx_t *emu_ctx) {
	while (true) {
		bool end_emulation = emu_ctx->end_emulation_callback && (emu_ctx->end_emulation_callback(emu_ctx));
		if (end_emulation) {
			break;
		}
//		uint32_t prevLoc = emu_ctx->cpu.reg[REG_PC];

	//	memcpy(localContext.registerCopy, cpu_state->reg, sizeof(uint32_t) * 16);
	//	localContext.disassemblyBuffer[0] = 0;
		cpu_single_step(emu_ctx);
#if 0
#if DO_TRACE == 1
		if (ctx.count_next_insn) {
			traceCPUStateFull(&ctx, prevLoc);
		}
#endif
#endif
	}
}

void cpu_reset(struct emu_ctx_t *emu_ctx) {
	memset(emu_ctx->cpu.reg, 0, 16 * sizeof(uint32_t));
	emu_ctx->cpu.psr = 0x173;
	emu_ctx->cpu.insn_ctr = 0;

	/* Load stack pointer and program counter */
	emu_ctx->cpu.reg[REG_SP] = addrspace_read32(&emu_ctx->addr_space, emu_ctx->ivt_base_address + 0);
	emu_ctx->cpu.reg[REG_PC] = addrspace_read32(&emu_ctx->addr_space, emu_ctx->ivt_base_address + 4) & ~1;
	//fprintf(stderr, "CPU reset, IVT base 0x%x: SP 0x%x, PC 0x%x\n", emu_ctx->ivt_base_address, emu_ctx->cpu.reg[REG_SP], emu_ctx->cpu.reg[REG_PC]);
}
