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

static void addTracePoint(struct LocalContext *aLCtx, uint16_t aPoint) {
	if (aLCtx->tracePointCnt < aLCtx->maxTracePointCnt) {
		aLCtx->tracePoints[aLCtx->tracePointCnt] = aPoint;
		aLCtx->tracePointCnt++;
	}
}

static void printDisassemblyCallback(struct disassemblyContext *aCtx, const char *aMsg, ...) {
	va_list ap;
	va_start(ap, aMsg);
	int l = strlen(aCtx->disasBuffer);
	vsnprintf(aCtx->disasBuffer + l, 128 - l, aMsg, ap);
	va_end(ap);
}


#define INTERCEPT_BKPT 1
static void breakpointCallback(struct emulationContext *aCtx, uint8_t aBreakpoint) {
	struct LocalContext *lctx = (struct LocalContext*)aCtx->localContext;
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
	aCtx->countNextInstruction = false;
}

void dumpCPUState(const struct CM3CPUState *aCPUState) {
	for (int i = 0; i < 16; i++) {
		fprintf(stderr, "r%-2d = %8x    ", i, aCPUState->reg[i]);
		if ((i % 4) == 3) {
			fprintf(stderr, "\n");
		}
	}
	fprintf(stderr, "PSR = %8x    >", aCPUState->psr);
	fprintf(stderr, "%c", (aCPUState->psr & FLAG_NEGATIVE) ? 'N' : ' ');
	fprintf(stderr, "%c", (aCPUState->psr & FLAG_ZERO) ? 'Z' : ' ');
	fprintf(stderr, "%c", (aCPUState->psr & FLAG_CARRY) ? 'C' : ' ');
	fprintf(stderr, "%c", (aCPUState->psr & FLAG_OVERFLOW) ? 'V' : ' ');
	fprintf(stderr, "%c", (aCPUState->psr & FLAG_SATURATION) ? 'Q' : ' ');
	fprintf(stderr, "<\n");
	fprintf(stderr, "\n");
}

void traceCPUStateFull(const struct emulationContext *aCtx, uint32_t aPreviousPC) {
	struct LocalContext *lctx = (struct LocalContext*)aCtx->localContext;
	if (lctx->traceFile) {
		fprintf(lctx->traceFile, "%d ", aCtx->cpu->clockcycle);
		fprintf(lctx->traceFile, "%x ", aPreviousPC);
		for (int i = 0; i < 16; i++) {
			fprintf(lctx->traceFile, "r%d=%-8x ", i, aCtx->cpu->reg[i]);
		}
		fprintf(lctx->traceFile, "PSR=%-8x\n", aCtx->cpu->psr);
	}
}

void dumpMemoryAt(struct CM3CPUState *aCPUState, uint32_t aAddress, uint16_t aLength) {
	const uint8_t *data = getMemoryAt(&aCPUState->addrSpace, aAddress);
	hexdumpData(data, aLength);
}

void dumpMemoryToFile(struct CM3CPUState *aCPUState, const char *aFilename) {
	FILE *f = fopen(aFilename, "w");
	if (!f) {
		perror(aFilename);
		return;
	}
	const uint8_t *data = getMemoryAt(&aCPUState->addrSpace, RAM_BASE);
	if (fwrite(data, RAM_SIZE, 1, f) != 1) {
		perror("fwrite");
	}
	fclose(f);
}

static void executeNextInstruction(struct emulationContext *aCtx) {
//	bool instructionDebug = (aCtx->cpu->clockcycle + 1 == 2363);
	bool instructionDebug = true;
	uint32_t insnWord = (memRead16(&aCtx->cpu->addrSpace, aCtx->cpu->reg[REG_PC]) << 16) | memRead16(&aCtx->cpu->addrSpace, aCtx->cpu->reg[REG_PC] + 2);
	aCtx->countNextInstruction = true;
	aCtx->shiftInstructionITState = true;

	if (instructionDebug) {
		struct disassemblyContext disasCtx = {
			.pc = aCtx->cpu->reg[REG_PC],
			.itState = aCtx->cpu->itState,
			.itCond = aCtx->cpu->itCond,
			.printDisassembly = printDisassemblyCallback,
			.disasBuffer = { 0 }
		};
		FILE *decodeInfos = instructionDebug ? stderr : NULL;
		decodeInstruction(&disasCtx, insnWord, &disassemblyCallbacks, decodeInfos);
		if (disasCtx.disasBuffer[0] == 0) {
			fprintf(stderr, "Warning: Cannot disassemble instruction at 0x%x\n", aCtx->cpu->reg[REG_PC]);
			if (!instructionDebug) {
				decodeInstruction(&disasCtx, insnWord, &disassemblyCallbacks, stderr);
			}
		}
		if (instructionDebug) {
			fprintf(stderr, "\n");
		}
		fprintf(stderr, "< %5d %8x: %-30s [sp=%x] ", 1 + aCtx->cpu->clockcycle, aCtx->cpu->reg[REG_PC], disasCtx.disasBuffer, aCtx->cpu->reg[REG_SP]);
		fprintf(stderr, "%c", (aCtx->cpu->psr & FLAG_NEGATIVE) ? 'N' : ' ');
		fprintf(stderr, "%c", (aCtx->cpu->psr & FLAG_ZERO) ? 'Z' : ' ');
		fprintf(stderr, "%c", (aCtx->cpu->psr & FLAG_CARRY) ? 'C' : ' ');
		fprintf(stderr, "%c", (aCtx->cpu->psr & FLAG_OVERFLOW) ? 'V' : ' ');
		fprintf(stderr, "%c", (aCtx->cpu->psr & FLAG_SATURATION) ? 'Q' : ' ');
		fprintf(stderr, "\n");
	
		if (instructionDebug) {
			dumpCPUState(aCtx->cpu);
		}
	}

	if (conditionallyExecuteInstruction(aCtx)) {
		decodeInstruction(aCtx, insnWord, &emulationCallbacks, NULL);
	} else {
		/* Skip instruction, decode to find out how long it is */
		int length = decodeInstruction(aCtx, insnWord, &decodeOnlyCallbacks, NULL);
		aCtx->cpu->reg[REG_PC] += length;
	}

	if (aCtx->countNextInstruction) {
		aCtx->cpu->clockcycle++;
	}
	if (aCtx->shiftInstructionITState) {
		aCtx->cpu->itState >>= 2;
	}

	aCtx->cpu->psr &= ~0xff00;
	if (aCtx->cpu->itState == 0) {
		aCtx->cpu->psr |= 0x0100;
	} else {
		aCtx->cpu->psr |= 0x0900 | (aCtx->cpu->itCond << 12);
		if (aCtx->cpu->itState == 5) {
			aCtx->cpu->psr |= 0x400;
		}
	}

	if (instructionDebug) {
//		dumpCPUState(aCtx->cpu);
	}
}

static uint8_t hammingWeight(uint32_t aValue) {
	uint8_t weight = 0;
	while (aValue) {
		if (aValue & 1) {
			weight += 1;
		}
		aValue >>= 1;
	}
	return weight;
}

void cpuRun(struct CM3CPUState *aCPUState, const char *aTraceOutputFile, bool runUntilSentinel) {
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

	struct emulationContext ctx = {
		.localContext = &localContext,
		.breakpointCallback = breakpointCallback,
		.cpu = aCPUState,
	};
	while (localContext.simulationState != STATE_FINISHED) {
		uint32_t prevLoc = aCPUState->reg[REG_PC];
		memcpy(localContext.registerCopy, aCPUState->reg, sizeof(uint32_t) * 16);
		localContext.disassemblyBuffer[0] = 0;
		executeNextInstruction(&ctx);
#if DO_TRACE == 1
		if (ctx.countNextInstruction) {
			traceCPUStateFull(&ctx, prevLoc);
		}
#endif

		if ((ctx.countNextInstruction) && (localContext.simulationState == STATE_SIMULATING)) {
			/* Emit tracepoint */
			int tracePointWeight = 0;
			for (int i = 0; i < TRACE_CONSIDER_REGISTER_CNT; i++) {
				tracePointWeight += hammingWeight(localContext.registerCopy[i] ^ aCPUState->reg[i]);
			}
			addTracePoint(&localContext, tracePointWeight);
		}

		if (localContext.simulationState == STATE_FINISHED) {
//			fprintf(stderr, "Simulation in %s finished successfuly after %d instructions and %d tracepoints.\n", aTraceOutputFile, aCPUState->clockcycle, localContext.tracePointCnt);
			printf("INSNS %d\n", localContext.tracePointCnt);
			break;
		}
		if (runUntilSentinel && localContext.simulationState == STATE_SIMULATING) {
			fprintf(stderr, "Simulation hit sentinel point.\n");
			aCPUState->reg[REG_PC] = prevLoc;
			break;
		}
		
		if (aCPUState->reg[REG_PC] == prevLoc) {
			fprintf(stderr, "Simulation terminated with an infinite loop after %d instructions at 0x%x.\n", aCPUState->clockcycle, prevLoc);
			dumpCPUState(aCPUState);
			dumpMemoryToFile(aCPUState, "output.bin");
			exit(EXIT_SUCCESS);
		}


	}

	if (localContext.traceFile) {
		int ptIndex = 0;
		for (int i = 0; i < localContext.tracePointCnt; i++) {	
			if (localContext.tracePoints[i] != 0xffff) {
				fprintf(localContext.traceFile, "%d %d\n", ptIndex, localContext.tracePoints[i]);
				ptIndex++;
			} else {
				fprintf(localContext.traceFile, "# Sync hint\n");
			}
		}
		fclose(localContext.traceFile);		
	}
	free(localContext.tracePoints);
}

void cpuReset(struct CM3CPUState *aCPUState) {
	memset(aCPUState->reg, 0, 64);
	aCPUState->psr = 0x173;
	aCPUState->clockcycle = 0;

	/* Load stack pointer and program counter */
	aCPUState->reg[REG_SP] = memRead32(&aCPUState->addrSpace, ROM_BASE + 0);
	aCPUState->reg[REG_PC] = memRead32(&aCPUState->addrSpace, ROM_BASE + 4) & ~1;
}

