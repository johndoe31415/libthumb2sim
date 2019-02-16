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
#include "rdtsc.h"

#define STATE_WAITING			0
#define STATE_SIMULATING		1
#define STATE_FINISHED			2

static void printDisassemblyCallback(struct disas_ctx_t *ctx, const char *aMsg, ...) {
	va_list ap;
	va_start(ap, aMsg);
	int l = strlen(ctx->disasBuffer);
	vsnprintf(ctx->disasBuffer + l, 128 - l, aMsg, ap);
	va_end(ap);
}

void cpu_dump_state(const struct cm3_cpu_state_t *cpu_state) {
	for (int i = 0; i < 16; i++) {
		fprintf(stderr, "r%-2d = %8x    ", i, cpu_state->reg[i]);
		if ((i % 4) == 3) {
			fprintf(stderr, "\n");
		}
	}
	fprintf(stderr, "PSR = %8x    >", cpu_state->psr);
	fprintf(stderr, "%c", (cpu_state->psr & FLAG_NEGATIVE) ? 'N' : ' ');
	fprintf(stderr, "%c", (cpu_state->psr & FLAG_ZERO) ? 'Z' : ' ');
	fprintf(stderr, "%c", (cpu_state->psr & FLAG_CARRY) ? 'C' : ' ');
	fprintf(stderr, "%c", (cpu_state->psr & FLAG_OVERFLOW) ? 'V' : ' ');
	fprintf(stderr, "%c", (cpu_state->psr & FLAG_SATURATION) ? 'Q' : ' ');
	fprintf(stderr, "<\n");
	fprintf(stderr, "\n");
}

#if 0
static void traceCPUStateFull(const struct insn_emu_ctx_t *insn_emu_ctx, uint32_t previous_pc) {
	struct emu_ctx_t *lctx = (struct emu_ctx_t*)emu_ctx->localContext;
	if (lctx->traceFile) {
		fprintf(lctx->traceFile, "%d ", emu_ctx->cpu->clockcycle);
		fprintf(lctx->traceFile, "%x ", previous_pc);
		for (int i = 0; i < 16; i++) {
			fprintf(lctx->traceFile, "r%d=%-8x ", i, emu_ctx->cpu->reg[i]);
		}
		fprintf(lctx->traceFile, "PSR=%-8x\n", emu_ctx->cpu->psr);
	}
}
#endif

void cpu_dump_memory(struct emu_ctx_t *emu_ctx, uint32_t address, uint16_t length) {
	const uint8_t *data = addrspace_memptr(&emu_ctx->addr_space, address);
	hexdump_data(data, length);
}

void cpu_dump_memory_file(struct emu_ctx_t *emu_ctx, const char *filename) {
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
		.countNextInstruction = true,
		.shiftInstructionITState = true,
	};
//	bool instructionDebug = (ctx->cpu->clockcycle + 1 == 2363);
//	bool instructionDebug = true;
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
		fprintf(stderr, "< %5d %8x: %-30s [sp=%x] ", 1 + ctx->cpu->clockcycle, ctx->cpu->reg[REG_PC], disasCtx.disasBuffer, ctx->cpu->reg[REG_SP]);
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
		int length = decode_insn(&insn_ctx, insnWord, &decodeOnlyCallbacks, NULL);
		emu_ctx->cpu.reg[REG_PC] += length;
	}

	if (insn_ctx.countNextInstruction) {
		emu_ctx->cpu.clockcycle++;
	}
	if (insn_ctx.shiftInstructionITState) {
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
		if (ctx.countNextInstruction) {
			traceCPUStateFull(&ctx, prevLoc);
		}
#endif
#endif
	}
}

void cpu_reset(struct emu_ctx_t *emu_ctx) {
	memset(emu_ctx->cpu.reg, 0, 16 * sizeof(uint32_t));
	emu_ctx->cpu.psr = 0x173;
	emu_ctx->cpu.clockcycle = 0;

	/* Load stack pointer and program counter */
	emu_ctx->cpu.reg[REG_SP] = addrspace_read32(&emu_ctx->addr_space, emu_ctx->ivt_base_address + 0);
	emu_ctx->cpu.reg[REG_PC] = addrspace_read32(&emu_ctx->addr_space, emu_ctx->ivt_base_address + 4) & ~1;
}
