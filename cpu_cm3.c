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

#if 0
static void printDisassemblyCallback(struct disas_ctx_t *ctx, const char *aMsg, ...) {
	va_list ap;
	va_start(ap, aMsg);
	int l = strlen(ctx->disasBuffer);
	vsnprintf(ctx->disasBuffer + l, 128 - l, aMsg, ap);
	va_end(ap);
}
#endif



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
	uint8_t *data = addrspace_memptr(&emu_ctx->addr_space, address);
	hexdump_data(data, length);
}

void cpu_dump_file(struct emu_ctx_t *emu_ctx, enum emu_dump_t dump_type, const char *filename) {
	FILE *f = fopen(filename, "w");
	if (!f) {
		perror(filename);
		return;
	}
#if 0
	const uint8_t *data = addrspace_memptr(&emu_ctx->addr_space, RAM_BASE);
	if (fwrite(data, RAM_SIZE, 1, f) != 1) {
		perror("fwrite");
	}
#endif
	fclose(f);
}

static uint32_t addrspace_read_insn_word(struct addrspace_t *addr_space, uint32_t pc) {
	return (addrspace_read16(addr_space, pc) << 16) | addrspace_read16(addr_space, pc + 2);
}

void cpu_single_step(struct emu_ctx_t *emu_ctx) {
	struct insn_emu_ctx_t insn_ctx = {
		.emu_ctx = emu_ctx,
		.count_next_insn = true,
		.shift_insn_it_state = true,
	};
	uint32_t insnWord = addrspace_read_insn_word(&emu_ctx->addr_space, emu_ctx->cpu.reg[REG_PC]);

#if 0
	if (instructionDebug) {
		struct disas_ctx_t disasCtx = {
			.pc = ctx->cpu->reg[REG_PC],
			.it_state = ctx->cpu->it_state,
			.it_cond = ctx->cpu->it_cond,
			.printDisassembly = printDisassemblyCallback,
			.disasBuffer = { 0 }
		};
		FILE *decodeInfos = instructionDebug ? stderr : NULL;
		decode_insn(&disasCtx, insnWord, &disassemblyCallbacks, decodeInfos);
		if (disasCtx.disasBuffer[0] == 0) {
			fprintf(stderr, "Warning: Cannot disassemble instruction at 0x%x\n", ctx->cpu->reg[REG_PC]);
			if (!instructionDebug) {
				decode_insn(&disasCtx, insnWord, &disassemblyCallbacks, stderr);
			}
		}
		if (instructionDebug) {
			fprintf(stderr, "\n");
		}
		fprintf(stderr, "< %5d %8x: %-30s [sp=%x] ", 1 + ctx->cpu->insn_ctr, ctx->cpu->reg[REG_PC], disasCtx.disasBuffer, ctx->cpu->reg[REG_SP]);
		fprintf(stderr, "%c", (ctx->cpu->psr & FLAG_NEGATIVE) ? 'N' : ' ');
		fprintf(stderr, "%c", (ctx->cpu->psr & FLAG_ZERO) ? 'Z' : ' ');
		fprintf(stderr, "%c", (ctx->cpu->psr & FLAG_CARRY) ? 'C' : ' ');
		fprintf(stderr, "%c", (ctx->cpu->psr & FLAG_OVERFLOW) ? 'V' : ' ');
		fprintf(stderr, "%c", (ctx->cpu->psr & FLAG_SATURATION) ? 'Q' : ' ');
		fprintf(stderr, "\n");

		if (instructionDebug) {
			cpu_dump_state(ctx->cpu);
		}
	}
#endif

	if (conditionallyExecuteInstruction(&insn_ctx)) {
		decode_insn(&insn_ctx, insnWord, &emulationCallbacks, NULL);
	} else {
		/* Skip instruction, decode to find out how long it is */
		int length = decode_insn(&insn_ctx, insnWord, NULL, NULL);
		emu_ctx->cpu.reg[REG_PC] += length;
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
