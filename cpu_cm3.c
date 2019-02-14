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

struct LocalContext {
	char disassemblyBuffer[128];
	int simulationState;
	FILE *traceFile;
	uint16_t *tracePoints;
	uint32_t tracePointCnt;
	uint32_t maxTracePointCnt;
	uint32_t registerCopy[16];
};

static void printDisassemblyCallback(struct disas_ctx_t *ctx, const char *aMsg, ...) {
	va_list ap;
	va_start(ap, aMsg);
	int l = strlen(ctx->disasBuffer);
	vsnprintf(ctx->disasBuffer + l, 128 - l, aMsg, ap);
	va_end(ap);
}


#define INTERCEPT_BKPT 1
static void bkpt_callback(struct emu_ctx_t *ctx, uint8_t aBreakpoint) {
	struct LocalContext *lctx = (struct LocalContext*)ctx->localContext;
	static uint64_t begin = 0;
	if (aBreakpoint == INTERCEPT_BKPT) {
		begin = rdtsc();
		lctx->simulationState = STATE_SIMULATING;
	} else if (aBreakpoint == INTERCEPT_BKPT + 1) {
		uint64_t now = rdtsc();
		printf("CLOCKS %lu\n", now - begin);
		lctx->simulationState = STATE_FINISHED;
	}
	/*
	if (aBreakpoint == 1) {
		lctx->simulationState = STATE_SIMULATING;
	} else if (aBreakpoint == 2) {
		addTracePoint(lctx, 0xffff);
	} else if (aBreakpoint == 3) {
	}
	*/
	ctx->countNextInstruction = false;
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

static void traceCPUStateFull(const struct emu_ctx_t *emu_ctx, uint32_t previous_pc) {
	struct LocalContext *lctx = (struct LocalContext*)emu_ctx->localContext;
	if (lctx->traceFile) {
		fprintf(lctx->traceFile, "%d ", emu_ctx->cpu->clockcycle);
		fprintf(lctx->traceFile, "%x ", previous_pc);
		for (int i = 0; i < 16; i++) {
			fprintf(lctx->traceFile, "r%d=%-8x ", i, emu_ctx->cpu->reg[i]);
		}
		fprintf(lctx->traceFile, "PSR=%-8x\n", emu_ctx->cpu->psr);
	}
}

void cpu_dump_memory(struct cm3_cpu_state_t *cpu_state, uint32_t address, uint16_t length) {
	const uint8_t *data = addrspace_memptr(&cpu_state->addr_space, address);
	hexdump_data(data, length);
}

void cpu_dump_memory_file(struct cm3_cpu_state_t *cpu_state, const char *filename) {
	FILE *f = fopen(filename, "w");
	if (!f) {
		perror(filename);
		return;
	}
	const uint8_t *data = addrspace_memptr(&cpu_state->addr_space, RAM_BASE);
	if (fwrite(data, RAM_SIZE, 1, f) != 1) {
		perror("fwrite");
	}
	fclose(f);
}

static void executeNextInstruction(struct emu_ctx_t *ctx) {
//	bool instructionDebug = (ctx->cpu->clockcycle + 1 == 2363);
	bool instructionDebug = true;
	uint32_t insnWord = (addrspace_read16(&ctx->cpu->addr_space, ctx->cpu->reg[REG_PC]) << 16) | addrspace_read16(&ctx->cpu->addr_space, ctx->cpu->reg[REG_PC] + 2);
	ctx->countNextInstruction = true;
	ctx->shiftInstructionITState = true;

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

	if (conditionallyExecuteInstruction(ctx)) {
		decode_insn(ctx, insnWord, &emulationCallbacks, NULL);
	} else {
		/* Skip instruction, decode to find out how long it is */
		int length = decode_insn(ctx, insnWord, &decodeOnlyCallbacks, NULL);
		ctx->cpu->reg[REG_PC] += length;
	}

	if (ctx->countNextInstruction) {
		ctx->cpu->clockcycle++;
	}
	if (ctx->shiftInstructionITState) {
		ctx->cpu->it_state >>= 2;
	}

	ctx->cpu->psr &= ~0xff00;
	if (ctx->cpu->it_state == 0) {
		ctx->cpu->psr |= 0x0100;
	} else {
		ctx->cpu->psr |= 0x0900 | (ctx->cpu->it_cond << 12);
		if (ctx->cpu->it_state == 5) {
			ctx->cpu->psr |= 0x400;
		}
	}

	if (instructionDebug) {
//		cpu_dump_state(ctx->cpu);
	}
}

void cpu_run(struct cm3_cpu_state_t *cpu_state, const char *aTraceOutputFile, bool runUntilSentinel) {
	struct LocalContext localContext = {
		.simulationState = STATE_WAITING,
		.tracePointCnt = 0,
		.maxTracePointCnt = 128 * 1024,
		.traceFile = NULL,
	};
	if (aTraceOutputFile) {
		localContext.traceFile = fopen(aTraceOutputFile, "w");
	}
	localContext.tracePoints = malloc(localContext.maxTracePointCnt * sizeof(uint16_t));

	struct emu_ctx_t ctx = {
		.localContext = &localContext,
		.bkpt_callback = bkpt_callback,
		.cpu = cpu_state,
	};
	while (localContext.simulationState != STATE_FINISHED) {
		uint32_t prevLoc = cpu_state->reg[REG_PC];
		memcpy(localContext.registerCopy, cpu_state->reg, sizeof(uint32_t) * 16);
		localContext.disassemblyBuffer[0] = 0;
		executeNextInstruction(&ctx);
#if DO_TRACE == 1
		if (ctx.countNextInstruction) {
			traceCPUStateFull(&ctx, prevLoc);
		}
#endif

	}
}

void cpu_reset(struct cm3_cpu_state_t *cpu_state) {
	memset(cpu_state->reg, 0, 64);
	cpu_state->psr = 0x173;
	cpu_state->clockcycle = 0;

	/* Load stack pointer and program counter */
	cpu_state->reg[REG_SP] = addrspace_read32(&cpu_state->addr_space, ROM_BASE + 0);
	cpu_state->reg[REG_PC] = addrspace_read32(&cpu_state->addr_space, ROM_BASE + 4) & ~1;
}

