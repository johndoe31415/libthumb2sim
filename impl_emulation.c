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
#include <stdint.h>
#include <string.h>

#include <thumb2simguest.h>
#include "decoder.h"
#include "impl_emulation.h"
#include "cpu_cm3.h"
#include "address_space.h"

#define OPCODE_WIDE						(1 << 0)
#define OPCODE_NARROW					(1 << 1)
#define OPCODE_SETFLAGS					(1 << 2)
#define OPCODE_SETFLAGS_IF_UNCOND		(1 << 3)
#define OPCODE_FORCE_CONDITION			(1 << 4)
#define OPCODE_ISCONDITIONAL(cond)		(OPCODE_FORCE_CONDITION | (((cond) & 0xf) << 8))

static uint32_t relBranchTarget(uint32_t aPC, uint32_t aImmediate, uint8_t aImmBits) {
	if (aImmediate >= (1 << (aImmBits - 1))) {
		// Negative offset
		return aPC - (4 + 2 * ((1 << aImmBits) - aImmediate)) + 8;
	} else {
		return aPC + (2 * aImmediate) + 4;
	}
}

struct barrel_shifterOutput {
	uint32_t value;
	bool carry;
};

static struct barrel_shifterOutput barrel_shift(uint32_t aInValue, uint8_t aType, uint8_t aImm) {
	struct barrel_shifterOutput outValue;
	outValue.carry = false;
	if (aType == TYPE_LSL) {
		outValue.carry = aInValue & 0x80000000;
		outValue.value = aInValue << aImm;
	} else if (aType == TYPE_LSR) {
		outValue.carry = aInValue & (1 << (aImm - 1));
		outValue.value = aInValue >> aImm;
	} else if (aType == TYPE_ROR) {
		/* ROR */
		uint32_t loMask = (1 << aImm) - 1;
		uint32_t hiMask = ~loMask;
		outValue.value = ((aInValue & loMask) << (32 - aImm)) | ((aInValue & hiMask) >> aImm);
	} else {
		outValue.value = 0xabcdef;
	}
	return outValue;
}

static void storeRegisters(struct insn_emu_ctx_t *insn_emu_ctx, uint8_t aBaseRegister, uint16_t aRegisters, bool aIncrementAfter, bool aWriteback) {
	uint32_t addr = insn_emu_ctx->emu_ctx->cpu.reg[aBaseRegister];
	int regNo = aIncrementAfter ? 0 : 15;
	for (int i = 0; i < 16; i++) {
		if (aRegisters & (1 << regNo)) {
			if (!aIncrementAfter) {
				addr -= 4;
			}
			addrspace_write32(&insn_emu_ctx->emu_ctx->addr_space, addr, insn_emu_ctx->emu_ctx->cpu.reg[regNo]);
			if (aIncrementAfter) {
				addr += 4;
			}
		}
		regNo += aIncrementAfter ? 1 : -1;
	}
	if (aWriteback) {
		insn_emu_ctx->emu_ctx->cpu.reg[aBaseRegister] = addr;
	}
}

static void loadRegisters(struct insn_emu_ctx_t *insn_emu_ctx, uint8_t aBaseRegister, uint16_t aRegisters, bool aWriteback) {
	uint32_t addr = insn_emu_ctx->emu_ctx->cpu.reg[aBaseRegister];
	for (int i = 0; i < 16; i++) {
		if (aRegisters & (1 << i)) {
			if (i != REG_PC) {
				insn_emu_ctx->emu_ctx->cpu.reg[i] = addrspace_read32(&insn_emu_ctx->emu_ctx->addr_space, addr);
			} else {
				insn_emu_ctx->emu_ctx->cpu.reg[i] = addrspace_read32(&insn_emu_ctx->emu_ctx->addr_space, addr) & ~1;
			}
			addr += 4;
		}
	}
	if (aWriteback) {
		insn_emu_ctx->emu_ctx->cpu.reg[aBaseRegister] = addr;
	}
}

static void setCondCode(const struct insn_emu_ctx_t *insn_emu_ctx, uint32_t value) {
	insn_emu_ctx->emu_ctx->cpu.psr &= ~FLAG_MASK;
	insn_emu_ctx->emu_ctx->cpu.psr |= value;
}

static bool condSatisfied(const struct insn_emu_ctx_t *insn_emu_ctx, uint8_t aCond) {
	switch (aCond) {
		case CONDITION_EQ:	return (insn_emu_ctx->emu_ctx->cpu.psr & FLAG_ZERO) == FLAG_ZERO;
		case CONDITION_NE:	return (insn_emu_ctx->emu_ctx->cpu.psr & FLAG_ZERO) == 0;
		case CONDITION_CS:	return (insn_emu_ctx->emu_ctx->cpu.psr & FLAG_CARRY) == FLAG_CARRY;
		case CONDITION_CC:	return (insn_emu_ctx->emu_ctx->cpu.psr & FLAG_CARRY) == 0;
		case CONDITION_MI:	return (insn_emu_ctx->emu_ctx->cpu.psr & FLAG_NEGATIVE) == FLAG_NEGATIVE;
		case CONDITION_PL:	return (insn_emu_ctx->emu_ctx->cpu.psr & FLAG_NEGATIVE) == 0;
		case CONDITION_VS:	return (insn_emu_ctx->emu_ctx->cpu.psr & FLAG_OVERFLOW) == FLAG_OVERFLOW;
		case CONDITION_VC:	return (insn_emu_ctx->emu_ctx->cpu.psr & FLAG_OVERFLOW) == 0;
		case CONDITION_HI:	return (insn_emu_ctx->emu_ctx->cpu.psr & (FLAG_CARRY | FLAG_ZERO)) == FLAG_CARRY;
		case CONDITION_LS:	return (insn_emu_ctx->emu_ctx->cpu.psr & (FLAG_CARRY | FLAG_ZERO)) != FLAG_CARRY;
		case CONDITION_GE:	return ((insn_emu_ctx->emu_ctx->cpu.psr & FLAG_NEGATIVE) == 0) == ((insn_emu_ctx->emu_ctx->cpu.psr & FLAG_OVERFLOW) == 0);
		case CONDITION_LT:	return ((insn_emu_ctx->emu_ctx->cpu.psr & FLAG_NEGATIVE) == 0) != ((insn_emu_ctx->emu_ctx->cpu.psr & FLAG_OVERFLOW) == 0);
		case CONDITION_GT:	return ((insn_emu_ctx->emu_ctx->cpu.psr & FLAG_ZERO) == 0) && ((insn_emu_ctx->emu_ctx->cpu.psr & FLAG_NEGATIVE) == 0) == ((insn_emu_ctx->emu_ctx->cpu.psr & FLAG_OVERFLOW) == 0);
		case CONDITION_LE:	return ((insn_emu_ctx->emu_ctx->cpu.psr & FLAG_ZERO) == FLAG_ZERO) || ((insn_emu_ctx->emu_ctx->cpu.psr & FLAG_NEGATIVE) == 0) != ((insn_emu_ctx->emu_ctx->cpu.psr & FLAG_OVERFLOW) == 0);
		case CONDITION_AL:	return true;
	}
	return true;
}

bool conditionallyExecuteInstruction(const struct insn_emu_ctx_t *insn_emu_ctx) {
	uint8_t currentItState = (insn_emu_ctx->emu_ctx->cpu.it_state & 0x03);
	bool doExecute = (currentItState == IT_NONE)
					|| ((currentItState == IT_THEN) && (condSatisfied(insn_emu_ctx, insn_emu_ctx->emu_ctx->cpu.it_cond)))
					|| ((currentItState == IT_ELSE) && (condSatisfied(insn_emu_ctx, insn_emu_ctx->emu_ctx->cpu.it_cond ^ 1)));
//	fprintf(stderr, "doExecute %d currentItState %x  ", doExecute, currentItState);
	return doExecute;
}

static bool executionIsUnconditional(const struct insn_emu_ctx_t *insn_emu_ctx) {
	return (insn_emu_ctx->emu_ctx->cpu.it_state & 0x03) == IT_NONE;
}

static uint32_t addCondCode(uint32_t aX, uint32_t aY) {
	uint32_t condCode = 0;
	uint32_t result = aX + aY;
	condCode |= (result == 0) ? FLAG_ZERO : 0;
	condCode |= (aY > aY) ? FLAG_NEGATIVE : 0;			/* TODO: THIS SEEMS LIKE A BUG, one of these aYs should probably be aX */
	condCode |= ((result < aX) || (result < aY)) ? FLAG_CARRY: 0;
	condCode |= ((aX ^ result) & (aY ^ result) & 0x80000000) ? FLAG_OVERFLOW : 0;
	return condCode;
}

static uint32_t subCondCode(uint32_t aX, uint32_t aY) {
	uint32_t condCode = 0;
	int32_t sX = (int32_t)aX;
	int32_t sY = (int32_t)aY;
	condCode |= ((sX - sY) < 0) ? FLAG_NEGATIVE : 0;
	condCode |= (aX == aY) ? FLAG_ZERO : 0;
	condCode |= (aX >= aY) ? FLAG_CARRY : 0;
	condCode |= (((sX >= 0) != (sY >= 0)) && ((sY - sX - 1) >= 0) == (aY > aX)) ? FLAG_OVERFLOW : 0;
	return condCode;
}

static uint32_t movCondCode(uint32_t aX) {
	uint32_t condCode = 0;
	condCode |= (aX & 0x80000000) ? FLAG_NEGATIVE : 0;
	condCode |= (aX == 0) ? FLAG_ZERO : 0;
	return condCode;
}

static void setMovCondCode(const struct insn_emu_ctx_t *insn_emu_ctx, bool always_set, uint32_t value) {
	if (always_set || executionIsUnconditional(insn_emu_ctx)) {
		insn_emu_ctx->emu_ctx->cpu.psr &= ~(FLAG_NEGATIVE | FLAG_ZERO);
		insn_emu_ctx->emu_ctx->cpu.psr |= movCondCode(value);
	}
}

static void setLslCondCode(const struct insn_emu_ctx_t *insn_emu_ctx, bool always_set, uint32_t value, bool aCarry) {
	if (always_set || executionIsUnconditional(insn_emu_ctx)) {
		insn_emu_ctx->emu_ctx->cpu.psr &= ~(FLAG_NEGATIVE | FLAG_ZERO | FLAG_CARRY);
		insn_emu_ctx->emu_ctx->cpu.psr |= movCondCode(value) | (aCarry ? FLAG_CARRY : 0);
	}
}

static void setSubCondCode(const struct insn_emu_ctx_t *insn_emu_ctx, bool always_set, uint32_t valueA, uint32_t valueB) {
	if (always_set || executionIsUnconditional(insn_emu_ctx)) {
		setCondCode(insn_emu_ctx, subCondCode(valueA, valueB));
	}
}

static void setAddCondCode(const struct insn_emu_ctx_t *insn_emu_ctx, bool always_set, uint32_t valueA, uint32_t valueB) {
	if (always_set || executionIsUnconditional(insn_emu_ctx)) {
		setCondCode(insn_emu_ctx, addCondCode(valueA, valueB));
	}
}

static bool conditionForExecutionSatisfied(const struct insn_emu_ctx_t *insn_emu_ctx) {
	return true;
}


static void emulation_i32_adc_imm_T1(void *vctx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	imm += (ctx->emu_ctx->cpu.psr & FLAG_CARRY) ? 1 : 0;
	if (S) {
		setAddCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rn], imm + 1);
	}
	ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rn] + imm;
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_adc_reg_T1(void *vctx, uint8_t Rdn, uint8_t Rm) {
	fprintf(stderr, "Instruction not implemented: i16_adc_reg_T1\n");
}

static void emulation_i32_adc_reg_T2(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S) {
	fprintf(stderr, "Instruction not implemented: i32_adc_reg_T2\n");
}

static void emulation_i16_add_SPi_T1(void *vctx, uint8_t Rd, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[REG_SP] + (imm * 4);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i16_add_SPi_T2(void *vctx, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[REG_SP] += imm * 4;
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_add_SPi_T3(void *vctx, uint8_t Rd, int32_t imm, bool S) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	if (S) {
		setAddCondCode(ctx, true, ctx->emu_ctx->cpu.reg[REG_SP], imm);
	}
	ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[REG_SP] + imm;
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_add_SPi_T4(void *vctx, uint8_t Rd, uint16_t imm) {
	fprintf(stderr, "Instruction not implemented: i32_add_SPi_T4\n");
}

static void emulation_i16_add_SPr_T1(void *vctx, uint8_t Rdm) {
	fprintf(stderr, "Instruction not implemented: i16_add_SPr_T1\n");
}

static void emulation_i16_add_SPr_T2(void *vctx, uint8_t Rm) {
	fprintf(stderr, "Instruction not implemented: i16_add_SPr_T2\n");
}

static void emulation_i32_add_SPr_T3(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t imm, uint8_t type, bool S) {
	fprintf(stderr, "Instruction not implemented: i32_add_SPr_T3\n");
}

static void emulation_i16_add_imm_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	setAddCondCode(ctx, false, ctx->emu_ctx->cpu.reg[Rn], imm);
	ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rn] + imm;
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i16_add_imm_T2(void *vctx, uint8_t Rdn, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	setAddCondCode(ctx, false, ctx->emu_ctx->cpu.reg[Rdn], imm);
	ctx->emu_ctx->cpu.reg[Rdn] = ctx->emu_ctx->cpu.reg[Rdn] + imm;
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_add_imm_T3(void *vctx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	if (S) {
		setAddCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rn], imm);
	}
	ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rn] + imm;
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_add_imm_T4(void *vctx, uint8_t Rd, uint8_t Rn, uint16_t imm) {
	fprintf(stderr, "Instruction not implemented: i32_add_imm_T4\n");
}

static void emulation_i16_add_reg_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rn] + ctx->emu_ctx->cpu.reg[Rm];
	setAddCondCode(ctx, false, ctx->emu_ctx->cpu.reg[Rn], ctx->emu_ctx->cpu.reg[Rm]);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i16_add_reg_T2(void *vctx, uint8_t Rdn, uint8_t Rm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[Rdn] += ctx->emu_ctx->cpu.reg[Rm];
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_add_reg_T3(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	struct barrel_shifterOutput bsOut = barrel_shift(ctx->emu_ctx->cpu.reg[Rm], type, imm);
	if (S) {
		setAddCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rn], bsOut.value);
	}
	ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rn] + bsOut.value;
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_adr_T1(void *vctx, uint8_t Rd, uint8_t imm) {
	fprintf(stderr, "Instruction not implemented: i16_adr_T1\n");
}

static void emulation_i32_adr_T2(void *vctx, uint8_t Rd, uint16_t imm) {
	fprintf(stderr, "Instruction not implemented: i32_adr_T2\n");
}

static void emulation_i32_adr_T3(void *vctx, uint8_t Rd, uint16_t imm) {
	fprintf(stderr, "Instruction not implemented: i32_adr_T3\n");
}

static void emulation_i32_and_imm_T1(void *vctx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	if ((Rd == REG_PC) && S) {
		setMovCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rn] & imm);
	} else {
		ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rn] & imm;
		if (S) {
			setMovCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rd]);
		}
	}
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_and_reg_T1(void *vctx, uint8_t Rdn, uint8_t Rm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[Rdn] &= ctx->emu_ctx->cpu.reg[Rm];
	setMovCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rdn]);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_and_reg_T2(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	if (Rd != REG_PC) {
		struct barrel_shifterOutput bsOut = barrel_shift(ctx->emu_ctx->cpu.reg[Rm], type, imm);
		ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rn] & bsOut.value;
		if (S) {
			setMovCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rd]);
		}
		ctx->emu_ctx->cpu.reg[REG_PC] += 4;
	}
}

static void emulation_i16_asr_imm_T1(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	bool carry = ctx->emu_ctx->cpu.reg[Rm] & (1 << (imm - 1));
	ctx->emu_ctx->cpu.reg[Rd] = (int32_t)ctx->emu_ctx->cpu.reg[Rm] >> imm;
	setLslCondCode(ctx, false, ctx->emu_ctx->cpu.reg[Rd], carry);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_asr_imm_T2(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t imm, bool S) {
	fprintf(stderr, "Instruction not implemented: i32_asr_imm_T2\n");
}

static void emulation_i16_asr_reg_T1(void *vctx, uint8_t Rdn, uint8_t Rm) {
	fprintf(stderr, "Instruction not implemented: i16_asr_reg_T1\n");
}

static void emulation_i32_asr_reg_T2(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, bool S) {
	fprintf(stderr, "Instruction not implemented: i32_asr_reg_T2\n");
}

static void emulation_i16_b_T1(void *vctx, uint8_t imm, uint8_t cond) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;

	if (condSatisfied(ctx, cond)) {
		ctx->emu_ctx->cpu.reg[REG_PC] = relBranchTarget(ctx->emu_ctx->cpu.reg[REG_PC], imm, 8);
	} else {
		ctx->emu_ctx->cpu.reg[REG_PC] += 2;
	}
}

static void emulation_i16_b_T2(void *vctx, uint16_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[REG_PC] = relBranchTarget(ctx->emu_ctx->cpu.reg[REG_PC], imm, 11);
}

static void emulation_i32_b_T3(void *vctx, int32_t imm, uint8_t cond) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;

	if (condSatisfied(ctx, cond)) {
		ctx->emu_ctx->cpu.reg[REG_PC] = ctx->emu_ctx->cpu.reg[REG_PC] + 2 * imm + 4;
	} else {
		ctx->emu_ctx->cpu.reg[REG_PC] += 4;
	}
}

static void emulation_i32_b_T4(void *vctx, int32_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[REG_PC] = ctx->emu_ctx->cpu.reg[REG_PC] + 2 * imm + 4;
}

static void emulation_i32_bfc_T1(void *vctx, uint8_t Rd, uint8_t imm, uint8_t msb) {
	fprintf(stderr, "Instruction not implemented: i32_bfc_T1\n");
}

static void emulation_i32_bfi_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t imm, uint8_t msb) {
	fprintf(stderr, "Instruction not implemented: i32_bfi_T1\n");
}

static void emulation_i32_bic_imm_T1(void *vctx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rn] & ~imm;
	if (S) {
		setMovCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rd]);
	}
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_bic_reg_T1(void *vctx, uint8_t Rdn, uint8_t Rm) {
	fprintf(stderr, "Instruction not implemented: i16_bic_reg_T1\n");
}

static void emulation_i32_bic_reg_T2(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	if (Rd != REG_PC) {
		struct barrel_shifterOutput bsOut = barrel_shift(ctx->emu_ctx->cpu.reg[Rm], type, imm);
		ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rn] & ~bsOut.value;
		if (S) {
			setMovCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rd]);
		}
		ctx->emu_ctx->cpu.reg[REG_PC] += 4;
	}
}

static void emulation_i16_bkpt_T1(void *vctx, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	if (ctx->emu_ctx->bkpt_callback) {
		ctx->emu_ctx->bkpt_callback(ctx->emu_ctx, imm);
	}
	if (imm == 255) {
		/* Possible syscall callback */
		/* API:
		 *     r0: syscall_no
		 *     r1: data_ptr
		 *     r2: length
		 */
		uint32_t syscall_no = ctx->emu_ctx->cpu.reg[REG_R0];
		uint32_t data_ptr = ctx->emu_ctx->cpu.reg[REG_R1];
		uint32_t length = ctx->emu_ctx->cpu.reg[REG_R2];
		if ((syscall_no == SYSCALL_GUEST_READ) && ctx->emu_ctx->emulator_syscall_read) {
			/* Guest wants to read data */
			void *data = addrspace_memptr(&ctx->emu_ctx->addr_space, data_ptr);
			length = ctx->emu_ctx->emulator_syscall_read(data, length);

			/* Return actual read length in r0, as of EABI calling convention */
			ctx->emu_ctx->cpu.reg[REG_R0] = length;
		} else if ((syscall_no == SYSCALL_GUEST_WRITE) && ctx->emu_ctx->emulator_syscall_write) {
			/* Guest wants to write data */
			const void *data = addrspace_memptr(&ctx->emu_ctx->addr_space, data_ptr);
			ctx->emu_ctx->emulator_syscall_write(data, length);
		} else if ((syscall_no == SYSCALL_GUEST_PUTS) && ctx->emu_ctx->emulator_syscall_puts) {
			/* Similar to write, but for convenience zero-terminated */
			const void *data = addrspace_memptr(&ctx->emu_ctx->addr_space, data_ptr);
			ctx->emu_ctx->emulator_syscall_puts(data);
		}
	}
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_bl_T1(void *vctx, int32_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[REG_LR] = (ctx->emu_ctx->cpu.reg[REG_PC] + 4) | 1;
	ctx->emu_ctx->cpu.reg[REG_PC] = ctx->emu_ctx->cpu.reg[REG_PC] + (2 * imm) + 4;
}

static void emulation_i16_blx_reg_T1(void *vctx, uint8_t Rm) {
	fprintf(stderr, "Instruction not implemented: i16_blx_reg_T1\n");
}

static void emulation_i16_bx_T1(void *vctx, uint8_t Rm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[REG_PC] = ctx->emu_ctx->cpu.reg[Rm] & ~1;
}

static void emulation_i16_cbnz_T1(void *vctx, uint8_t Rn, uint8_t imm, bool op) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;

	if ((ctx->emu_ctx->cpu.reg[Rn] == 0) ^ op) {
		/* Branch */
		ctx->emu_ctx->cpu.reg[REG_PC] = relBranchTarget(ctx->emu_ctx->cpu.reg[REG_PC], imm, 7);
	} else {
		/* Do not branch */
		ctx->emu_ctx->cpu.reg[REG_PC] += 2;
	}
}

static void emulation_i32_cdp2_T2(void *vctx, uint8_t coproc, uint8_t CRd, uint8_t CRn, uint8_t CRm, uint8_t opcA, uint8_t opcB) {
	fprintf(stderr, "Instruction not implemented: i32_cdp2_T2\n");
}

static void emulation_i32_cdp_T1(void *vctx, uint8_t coproc, uint8_t CRd, uint8_t CRn, uint8_t CRm, uint8_t opcA, uint8_t opcB) {
	fprintf(stderr, "Instruction not implemented: i32_cdp_T1\n");
}

static void emulation_i32_clrex_T1(void *vctx) {
	fprintf(stderr, "Instruction not implemented: i32_clrex_T1\n");
}

static void emulation_i32_clz_T1(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t Rmx) {
	// TODO: Rmx?
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t mask = (1 << 31);
	uint32_t leadingZeros = 32;
	for (int i = 0; i < 32; i++) {
		if (mask & ctx->emu_ctx->cpu.reg[Rm]) {
			leadingZeros = i;
			break;
		}
		mask >>= 1;
	}
	ctx->emu_ctx->cpu.reg[Rd] = leadingZeros;
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_cmn_imm_T1(void *vctx, uint8_t Rn, int32_t imm) {
	fprintf(stderr, "Instruction not implemented: i32_cmn_imm_T1\n");
}

static void emulation_i16_cmn_reg_T1(void *vctx, uint8_t Rn, uint8_t Rm) {
	fprintf(stderr, "Instruction not implemented: i16_cmn_reg_T1\n");
}

static void emulation_i32_cmn_reg_T2(void *vctx, uint8_t Rn, uint8_t Rm, int32_t imm, uint8_t type) {
	fprintf(stderr, "Instruction not implemented: i32_cmn_reg_T2\n");
}

static void emulation_i16_cmp_imm_T1(void *vctx, uint8_t Rn, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	setSubCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rn], imm);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_cmp_imm_T2(void *vctx, uint8_t Rn, int32_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	setSubCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rn], imm);
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_cmp_reg_T1(void *vctx, uint8_t Rn, uint8_t Rm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	setSubCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rn], ctx->emu_ctx->cpu.reg[Rm]);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i16_cmp_reg_T2(void *vctx, uint8_t Rn, uint8_t Rm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	setSubCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rn], ctx->emu_ctx->cpu.reg[Rm]);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_cmp_reg_T3(void *vctx, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type) {
	fprintf(stderr, "Instruction not implemented: i32_cmp_reg_T3\n");
}

static void emulation_i16_cps_T1(void *vctx, bool imm, bool F, bool I) {
	fprintf(stderr, "Instruction not implemented: i16_cps_T1\n");
}

static void emulation_i32_dbg_T1(void *vctx, uint8_t option) {
	fprintf(stderr, "Instruction not implemented: i32_dbg_T1\n");
}

static void emulation_i32_dmb_T1(void *vctx, uint8_t option) {
	fprintf(stderr, "Instruction not implemented: i32_dmb_T1\n");
}

static void emulation_i32_dsb_T1(void *vctx, uint8_t option) {
	fprintf(stderr, "Instruction not implemented: i32_dsb_T1\n");
}

static void emulation_i32_eor_imm_T1(void *vctx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rn] ^ imm;
	if (S) {
		setMovCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rd]);
	}
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_eor_reg_T1(void *vctx, uint8_t Rdn, uint8_t Rm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[Rdn] ^= ctx->emu_ctx->cpu.reg[Rm];
	setMovCondCode(ctx, false, ctx->emu_ctx->cpu.reg[Rdn]);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_eor_reg_T2(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;

	if (Rd != REG_PC) {
		struct barrel_shifterOutput bsOut = barrel_shift(ctx->emu_ctx->cpu.reg[Rm], type, imm);
		ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rn] ^ bsOut.value;
		if (S) {
			setMovCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rd]);
		}
		ctx->emu_ctx->cpu.reg[REG_PC] += 4;
	}

}

static void emulation_i32_isb_T1(void *vctx, uint8_t option) {
	fprintf(stderr, "Instruction not implemented: i32_isb_T1\n");
}

static void emulation_i16_it_T1(void *vctx, uint8_t firstcond, uint8_t mask) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;

	int len = 3;
	for (int i = 0; i < 3; i++) {
		if (mask & (1 << i)) {
			break;
		} else {
			len--;
		}
	}
	ctx->emu_ctx->cpu.it_cond = firstcond;
	uint8_t match = firstcond & 1;
	for (int i = len - 1; i >= 0; i--) {
		bool then = ((mask >> (3 - i)) & 1) == match;
		ctx->emu_ctx->cpu.it_state <<= 2;
		ctx->emu_ctx->cpu.it_state |= then ? IT_THEN : IT_ELSE;
	}
	ctx->emu_ctx->cpu.it_state <<= 2;
	ctx->emu_ctx->cpu.it_state |= IT_THEN;

	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
	ctx->shift_insn_it_state = false;
}

static void emulation_i32_ldc2_imm_T2(void *vctx, uint8_t Rn, uint8_t imm, uint8_t coproc, uint8_t CRd, bool D, bool P, bool U, bool W) {
	fprintf(stderr, "Instruction not implemented: i32_ldc2_imm_T2\n");
}

static void emulation_i32_ldc2_lit_T2(void *vctx, uint8_t imm, uint8_t coproc, uint8_t CRd, bool D, bool P, bool U, bool W) {
	fprintf(stderr, "Instruction not implemented: i32_ldc2_lit_T2\n");
}

static void emulation_i32_ldc_imm_T1(void *vctx, uint8_t Rn, uint8_t imm, uint8_t coproc, uint8_t CRd, bool D, bool P, bool U, bool W) {
	fprintf(stderr, "Instruction not implemented: i32_ldc_imm_T1\n");
}

static void emulation_i32_ldc_lit_T1(void *vctx, uint8_t imm, uint8_t coproc, uint8_t CRd, bool D, bool P, bool U, bool W) {
	fprintf(stderr, "Instruction not implemented: i32_ldc_lit_T1\n");
}

static void emulation_i16_ldm_T1(void *vctx, uint8_t Rn, uint8_t register_list) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	loadRegisters(ctx, Rn, register_list, (register_list & (1 << Rn)) == 0);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_ldm_T2(void *vctx, uint8_t Rn, uint16_t register_list, bool P, bool M, bool W) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	if (P) {
		register_list |= (1 << REG_PC);
	}
	if (M) {
		register_list |= (1 << REG_LR);
	}
	loadRegisters(ctx, Rn, register_list, W);
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_ldmdb_T1(void *vctx, uint8_t Rn, uint16_t register_list, bool P, bool M, bool W) {
	fprintf(stderr, "Instruction not implemented: i32_ldmdb_T1\n");
}

static void emulation_i16_ldr_imm_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + (4 * imm);
	ctx->emu_ctx->cpu.reg[Rt] = addrspace_read32(&ctx->emu_ctx->addr_space, target);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i16_ldr_imm_T2(void *vctx, uint8_t Rt, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[REG_SP] + (4 * imm) + 0;
	ctx->emu_ctx->cpu.reg[Rt] = addrspace_read32(&ctx->emu_ctx->addr_space, target);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_ldr_imm_T3(void *vctx, uint8_t Rt, uint8_t Rn, uint16_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + imm;
	ctx->emu_ctx->cpu.reg[Rt] = addrspace_read32(&ctx->emu_ctx->addr_space, target);
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_ldr_imm_T4(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm, bool P, bool U, bool W) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + (P ? 1 : 0) * (U ? 1 : -1) * imm;
	ctx->emu_ctx->cpu.reg[Rt] = addrspace_read32(&ctx->emu_ctx->addr_space, target);
	if (W) {
		ctx->emu_ctx->cpu.reg[Rn] += (U ? 1 : -1) * imm;
	}
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_ldr_lit_T1(void *vctx, uint8_t Rt, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = (ctx->emu_ctx->cpu.reg[REG_PC] & ~3) + (imm * 4) + 4;
	ctx->emu_ctx->cpu.reg[Rt] = addrspace_read32(&ctx->emu_ctx->addr_space, target);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_ldr_lit_T2(void *vctx, uint8_t Rt, uint16_t imm, bool U) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = (ctx->emu_ctx->cpu.reg[REG_PC] & ~3) + (U ? 1 : -1) * imm + 4;
	ctx->emu_ctx->cpu.reg[Rt] = addrspace_read32(&ctx->emu_ctx->addr_space, target);
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_ldr_reg_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + ctx->emu_ctx->cpu.reg[Rm];
	ctx->emu_ctx->cpu.reg[Rt] = addrspace_read32(&ctx->emu_ctx->addr_space, target);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_ldr_reg_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + (ctx->emu_ctx->cpu.reg[Rm] << imm);
	ctx->emu_ctx->cpu.reg[Rt] = addrspace_read32(&ctx->emu_ctx->addr_space, target);
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_ldrb_imm_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + imm;
	ctx->emu_ctx->cpu.reg[Rt] = addrspace_read8(&ctx->emu_ctx->addr_space, target);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_ldrb_imm_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint16_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + imm;
	ctx->emu_ctx->cpu.reg[Rt] = addrspace_read8(&ctx->emu_ctx->addr_space, target);
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_ldrb_imm_T3(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm, bool P, bool U, bool W) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + (P ? 1 : 0) * (U ? 1 : -1) * imm;
	ctx->emu_ctx->cpu.reg[Rt] = addrspace_read8(&ctx->emu_ctx->addr_space, target);
	if (W) {
		ctx->emu_ctx->cpu.reg[Rn] += (U ? 1 : -1) * imm;
	}
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_ldrb_lit_T1(void *vctx, uint8_t Rt, uint16_t imm, bool U) {
	fprintf(stderr, "Instruction not implemented: i32_ldrb_lit_T1\n");
}

static void emulation_i16_ldrb_reg_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + ctx->emu_ctx->cpu.reg[Rm];
	ctx->emu_ctx->cpu.reg[Rt] = addrspace_read8(&ctx->emu_ctx->addr_space, target);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_ldrb_reg_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + ctx->emu_ctx->cpu.reg[Rm];
	uint32_t value = addrspace_read8(&ctx->emu_ctx->addr_space, target);
	if (imm) {
		struct barrel_shifterOutput bsOut = barrel_shift(value, TYPE_LSL, imm);
		value = bsOut.value;
	}
	ctx->emu_ctx->cpu.reg[Rt] = value;
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_ldrbt_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
	fprintf(stderr, "Instruction not implemented: i32_ldrbt_T1\n");
}

static void emulation_i32_ldrd_imm_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rtx, uint8_t imm, bool P, bool U, bool W) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + (P ? 1 : 0) * (U ? 1 : -1) * (4 * imm);
	ctx->emu_ctx->cpu.reg[Rt] = addrspace_read32(&ctx->emu_ctx->addr_space, target);
	ctx->emu_ctx->cpu.reg[Rtx] = addrspace_read32(&ctx->emu_ctx->addr_space, target + 4);
	if (W) {
		ctx->emu_ctx->cpu.reg[Rn] = ctx->emu_ctx->cpu.reg[Rn] + (U ? 1 : -1) * (4 * imm);
	}
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_ldrd_lit_T1(void *vctx, uint8_t Rt, uint8_t Rtx, uint8_t imm, bool P, bool U, bool W) {
	fprintf(stderr, "Instruction not implemented: i32_ldrd_lit_T1\n");
}

static void emulation_i32_ldrex_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
	fprintf(stderr, "Instruction not implemented: i32_ldrex_T1\n");
}

static void emulation_i32_ldrexb_T1(void *vctx, uint8_t Rt, uint8_t Rn) {
	fprintf(stderr, "Instruction not implemented: i32_ldrexb_T1\n");
}

static void emulation_i32_ldrexh_T1(void *vctx, uint8_t Rt, uint8_t Rn) {
	fprintf(stderr, "Instruction not implemented: i32_ldrexh_T1\n");
}

static void emulation_i16_ldrh_imm_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + 2 * imm;
	ctx->emu_ctx->cpu.reg[Rt] = addrspace_read16(&ctx->emu_ctx->addr_space, target);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_ldrh_imm_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint16_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + imm;
	ctx->emu_ctx->cpu.reg[Rt] = addrspace_read16(&ctx->emu_ctx->addr_space, target);
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_ldrh_imm_T3(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm, bool P, bool U, bool W) {
	fprintf(stderr, "Instruction not implemented: i32_ldrh_imm_T3\n");
}

static void emulation_i32_ldrh_lit_T1(void *vctx, uint8_t Rt, uint16_t imm, bool U) {
	fprintf(stderr, "Instruction not implemented: i32_ldrh_lit_T1\n");
}

static void emulation_i16_ldrh_reg_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm) {
	fprintf(stderr, "Instruction not implemented: i16_ldrh_reg_T1\n");
}

static void emulation_i32_ldrh_reg_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm, uint8_t imm) {
	fprintf(stderr, "Instruction not implemented: i32_ldrh_reg_T2\n");
}

static void emulation_i32_ldrht_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
	fprintf(stderr, "Instruction not implemented: i32_ldrht_T1\n");
}

static void emulation_i32_ldrsb_imm_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint16_t imm) {
	fprintf(stderr, "Instruction not implemented: i32_ldrsb_imm_T1\n");
}

static void emulation_i32_ldrsb_imm_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm, bool P, bool U, bool W) {
	fprintf(stderr, "Instruction not implemented: i32_ldrsb_imm_T2\n");
}

static void emulation_i32_ldrsb_lit_T1(void *vctx, uint8_t Rt, uint16_t imm, bool U) {
	fprintf(stderr, "Instruction not implemented: i32_ldrsb_lit_T1\n");
}

static void emulation_i16_ldrsb_reg_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm) {
	fprintf(stderr, "Instruction not implemented: i16_ldrsb_reg_T1\n");
}

static void emulation_i32_ldrsb_reg_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm, uint8_t imm) {
	fprintf(stderr, "Instruction not implemented: i32_ldrsb_reg_T2\n");
}

static void emulation_i32_ldrsbt_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
	fprintf(stderr, "Instruction not implemented: i32_ldrsbt_T1\n");
}

static void emulation_i32_ldrsh_imm_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint16_t imm) {
	fprintf(stderr, "Instruction not implemented: i32_ldrsh_imm_T1\n");
}

static void emulation_i32_ldrsh_imm_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm, bool P, bool U, bool W) {
	fprintf(stderr, "Instruction not implemented: i32_ldrsh_imm_T2\n");
}

static void emulation_i32_ldrsh_lit_T1(void *vctx, uint8_t Rt, uint16_t imm, bool U) {
	fprintf(stderr, "Instruction not implemented: i32_ldrsh_lit_T1\n");
}

static void emulation_i16_ldrsh_reg_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm) {
	fprintf(stderr, "Instruction not implemented: i16_ldrsh_reg_T1\n");
}

static void emulation_i32_ldrsh_reg_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm, uint8_t imm) {
	fprintf(stderr, "Instruction not implemented: i32_ldrsh_reg_T2\n");
}

static void emulation_i32_ldrsh_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
	fprintf(stderr, "Instruction not implemented: i32_ldrsh_T1\n");
}

static void emulation_i32_ldrt_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
	fprintf(stderr, "Instruction not implemented: i32_ldrt_T1\n");
}

static void emulation_i16_lsl_imm_T1(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	// TODO: effizienter
	bool carry = ((uint64_t)ctx->emu_ctx->cpu.reg[Rm] << imm) & (uint64_t)0x100000000;
	ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rm] << imm;
	setLslCondCode(ctx, false, ctx->emu_ctx->cpu.reg[Rd], carry);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_lsl_imm_T2(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t imm, bool S) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	struct barrel_shifterOutput bsOut = barrel_shift(ctx->emu_ctx->cpu.reg[Rm], TYPE_LSL, imm);
	ctx->emu_ctx->cpu.reg[Rd] = bsOut.value;
	if (S) {
		setLslCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rd], bsOut.carry);
	}
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_lsl_reg_T1(void *vctx, uint8_t Rdn, uint8_t Rm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	struct barrel_shifterOutput bsOut = barrel_shift(ctx->emu_ctx->cpu.reg[Rdn], TYPE_LSL, ctx->emu_ctx->cpu.reg[Rm]);
	ctx->emu_ctx->cpu.reg[Rdn] = bsOut.value;
	setLslCondCode(ctx, false, ctx->emu_ctx->cpu.reg[Rdn], bsOut.carry);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_lsl_reg_T2(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, bool S) {
	// TODO: Rn/Rm vertauscht?
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	struct barrel_shifterOutput bsOut = barrel_shift(ctx->emu_ctx->cpu.reg[Rm], TYPE_LSL, ctx->emu_ctx->cpu.reg[Rn]);
	ctx->emu_ctx->cpu.reg[Rd] = bsOut.value;
	if (S) {
		setLslCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rd], bsOut.carry);
	}
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_lsr_imm_T1(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	bool carry = ctx->emu_ctx->cpu.reg[Rm] & (1 << (imm - 1));
	ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rm] >> imm;
	setLslCondCode(ctx, false, ctx->emu_ctx->cpu.reg[Rd], carry);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_lsr_imm_T2(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t imm, bool S) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	bool carry = ctx->emu_ctx->cpu.reg[Rm] & (1 << (imm - 1));
	ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rm] >> imm;
	if (S) {
		setLslCondCode(ctx, false, ctx->emu_ctx->cpu.reg[Rd], carry);
	}
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_lsr_reg_T1(void *vctx, uint8_t Rdn, uint8_t Rm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	bool carry = ctx->emu_ctx->cpu.reg[Rm] ? ctx->emu_ctx->cpu.reg[Rdn] & (1 << (ctx->emu_ctx->cpu.reg[Rm] - 1)) : false;
	ctx->emu_ctx->cpu.reg[Rdn] >>= ctx->emu_ctx->cpu.reg[Rm];
	setLslCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rdn], carry);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_lsr_reg_T2(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, bool S) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	bool carry = ctx->emu_ctx->cpu.reg[Rn] & (1 << (ctx->emu_ctx->cpu.reg[Rm] - 1));
	ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rn] >> ctx->emu_ctx->cpu.reg[Rm];
	if (S) {
		setLslCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rd], carry);
	}
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_mcr2_T2(void *vctx, uint8_t Rt, uint8_t coproc, uint8_t CRn, uint8_t CRm, uint8_t opcA, uint8_t opcB) {
	fprintf(stderr, "Instruction not implemented: i32_mcr2_T2\n");
}

static void emulation_i32_mcr_T1(void *vctx, uint8_t Rt, uint8_t coproc, uint8_t CRn, uint8_t CRm, uint8_t opcA, uint8_t opcB) {
	fprintf(stderr, "Instruction not implemented: i32_mcr_T1\n");
}

static void emulation_i32_mcrr2_T2(void *vctx, uint8_t Rt, uint8_t coproc, uint8_t CRm, uint8_t opcA, uint8_t RtB) {
	fprintf(stderr, "Instruction not implemented: i32_mcrr2_T2\n");
}

static void emulation_i32_mcrr_T1(void *vctx, uint8_t Rt, uint8_t coproc, uint8_t CRm, uint8_t opcA, uint8_t RtB) {
	fprintf(stderr, "Instruction not implemented: i32_mcrr_T1\n");
}

static void emulation_i32_mla_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t Ra) {
	fprintf(stderr, "Instruction not implemented: i32_mla_T1\n");
}

static void emulation_i32_mls_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t Ra) {
	fprintf(stderr, "Instruction not implemented: i32_mls_T1\n");
}

static void emulation_i16_mov_imm_T1(void *vctx, uint8_t Rd, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	if (conditionForExecutionSatisfied(ctx)) {
		ctx->emu_ctx->cpu.reg[Rd] = imm;
		setMovCondCode(ctx, false, imm);
	}
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_mov_imm_T2(void *vctx, uint8_t Rd, int32_t imm, bool S) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[Rd] = imm;
	if (S) {
		setMovCondCode(ctx, true, imm);
	}
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_mov_imm_T3(void *vctx, uint8_t Rd, uint16_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[Rd] = imm;
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_mov_reg_T1(void *vctx, uint8_t Rd, uint8_t Rm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rm];
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i16_mov_reg_T2(void *vctx, uint8_t Rd, uint8_t Rm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rm];
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_mov_reg_T3(void *vctx, uint8_t Rd, uint8_t Rm, bool S) {
	fprintf(stderr, "Instruction not implemented: i32_mov_reg_T3\n");
}

static void emulation_i32_movt_T1(void *vctx, uint8_t Rd, uint16_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[Rd] &= 0xffff;
	ctx->emu_ctx->cpu.reg[Rd] |= (imm << 16);
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_mrc2_T2(void *vctx, uint8_t Rt, uint8_t coproc, uint8_t Crn, uint8_t CRm, uint8_t opcA, uint8_t opcB) {
	fprintf(stderr, "Instruction not implemented: i32_mrc2_T2\n");
}

static void emulation_i32_mrc_T1(void *vctx, uint8_t Rt, uint8_t coproc, uint8_t Crn, uint8_t CRm, uint8_t opcA, uint8_t opcB) {
	fprintf(stderr, "Instruction not implemented: i32_mrc_T1\n");
}

static void emulation_i32_mrrc2_T2(void *vctx, uint8_t Rt, uint8_t coproc, uint8_t CRm, uint8_t opcA, uint8_t RtB) {
	fprintf(stderr, "Instruction not implemented: i32_mrrc2_T2\n");
}

static void emulation_i32_mrrc_T1(void *vctx, uint8_t Rt, uint8_t coproc, uint8_t CRm, uint8_t opcA, uint8_t RtB) {
	fprintf(stderr, "Instruction not implemented: i32_mrrc_T1\n");
}

static void emulation_i32_mrs_T1(void *vctx, uint8_t Rd, uint8_t SYSm) {
	fprintf(stderr, "Instruction not implemented: i32_mrs_T1\n");
}

static void emulation_i32_msr_T1(void *vctx, uint8_t Rn, uint8_t mask, uint8_t SYSm) {
	fprintf(stderr, "Instruction not implemented: i32_msr_T1\n");
}

static void emulation_i16_mul_T1(void *vctx, uint8_t Rdm, uint8_t Rn) {
	fprintf(stderr, "Instruction not implemented: i16_mul_T1\n");
}

static void emulation_i32_mul_T2(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm) {
	fprintf(stderr, "Instruction not implemented: i32_mul_T2\n");
}

static void emulation_i32_mvn_imm_T1(void *vctx, uint8_t Rd, int32_t imm, bool S) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[Rd] = ~imm;
	if (S) {
		setMovCondCode(ctx, true, imm);
	}
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_mvn_reg_T1(void *vctx, uint8_t Rd, uint8_t Rm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[Rd] = ~ctx->emu_ctx->cpu.reg[Rm];
	setMovCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rd]);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_mvn_reg_T2(void *vctx, uint8_t Rd, uint8_t Rm, int32_t imm, uint8_t type, bool S) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	struct barrel_shifterOutput bsOut = barrel_shift(ctx->emu_ctx->cpu.reg[Rm], type, imm);
	ctx->emu_ctx->cpu.reg[Rd] = ~bsOut.value;
	if (S) {
		setMovCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rd]);
	}
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_nop_T1(void *vctx) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_nop_T2(void *vctx) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_orn_imm_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint16_t imm, bool S) {
	fprintf(stderr, "Instruction not implemented: i32_orn_imm_T1\n");
}

static void emulation_i32_orn_reg_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S) {
	fprintf(stderr, "Instruction not implemented: i32_orn_reg_T1\n");
}

static void emulation_i32_orr_imm_T1(void *vctx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rn] | imm;
	if (S) {
		setMovCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rd]);
	}
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_orr_reg_T1(void *vctx, uint8_t Rdn, uint8_t Rm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[Rdn] |= ctx->emu_ctx->cpu.reg[Rm];
	setMovCondCode(ctx, false, ctx->emu_ctx->cpu.reg[Rdn]);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_orr_reg_T2(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	struct barrel_shifterOutput bsOut = barrel_shift(ctx->emu_ctx->cpu.reg[Rm], type, imm);
	if (Rn != 0x0f) {
		ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rn] | bsOut.value;
	} else {
		ctx->emu_ctx->cpu.reg[Rd] = bsOut.value;
	}

	if (S) {
		setLslCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rd], bsOut.carry);
	}
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_pkhbt_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, bool tb, bool S, bool T) {
	fprintf(stderr, "Instruction not implemented: i32_pkhbt_T1\n");
}

static void emulation_i16_pop_T1(void *vctx, uint8_t register_list, bool P) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint16_t regList = register_list;
	if (P) {
		regList |= (1 << REG_PC);
	}
	loadRegisters(ctx, REG_SP, regList, true);
	if (!P) {
		ctx->emu_ctx->cpu.reg[REG_PC] += 2;
	}
}

static void emulation_i32_pop_T2(void *vctx, uint16_t register_list, bool P, bool M) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	if (P) {
		register_list |= (1 << REG_PC);
	}
	if (M) {
		register_list |= (1 << REG_LR);
	}
	loadRegisters(ctx, REG_SP, register_list, true);
	if (!P) {
		ctx->emu_ctx->cpu.reg[REG_PC] += 4;
	}
}

static void emulation_i32_pop_T3(void *vctx, uint8_t Rt) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	loadRegisters(ctx, REG_SP, (1 << Rt), true);
	if (Rt != REG_PC) {
		ctx->emu_ctx->cpu.reg[REG_PC] += 4;
	}
}

static void emulation_i16_push_T1(void *vctx, uint8_t register_list, bool M) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint16_t regList = register_list;
	if (M) {
		regList |= (1 << REG_LR);
	}
	storeRegisters(ctx, REG_SP, regList, false, true);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_push_T2(void *vctx, uint16_t register_list, bool M) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	if (M) {
		register_list |= (1 << REG_LR);
	}
	storeRegisters(ctx, REG_SP, register_list, false, true);
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_push_T3(void *vctx, uint8_t Rt) {
	fprintf(stderr, "Instruction not implemented: i32_push_T3\n");
}

static void emulation_i32_rbit_T1(void *vctx, uint8_t Rd, uint8_t Rm) {
	fprintf(stderr, "Instruction not implemented: i32_rbit_T1\n");
}

static void emulation_i16_rev16_T1(void *vctx, uint8_t Rd, uint8_t Rm) {
	fprintf(stderr, "Instruction not implemented: i16_rev16_T1\n");
}

static void emulation_i32_rev16_T2(void *vctx, uint8_t Rd, uint8_t Rm) {
	fprintf(stderr, "Instruction not implemented: i32_rev16_T2\n");
}

static void emulation_i16_rev_T1(void *vctx, uint8_t Rd, uint8_t Rm) {
	fprintf(stderr, "Instruction not implemented: i16_rev_T1\n");
}

static void emulation_i32_rev_T2(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t Rmx) {
	fprintf(stderr, "Instruction not implemented: i32_rev_T2\n");
}

static void emulation_i16_revsh_T1(void *vctx, uint8_t Rd, uint8_t Rm) {
	fprintf(stderr, "Instruction not implemented: i16_revsh_T1\n");
}

static void emulation_i32_revsh_T2(void *vctx, uint8_t Rd, uint8_t Rm) {
	fprintf(stderr, "Instruction not implemented: i32_revsh_T2\n");
}

static void emulation_i32_ror_imm_T1(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t imm, bool S) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	struct barrel_shifterOutput bsOut = barrel_shift(ctx->emu_ctx->cpu.reg[Rm], TYPE_ROR, imm);
	ctx->emu_ctx->cpu.reg[Rd] = bsOut.value;
	if (S) {
		setLslCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rd], bsOut.carry);
	}
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_ror_reg_T1(void *vctx, uint8_t Rdn, uint8_t Rm) {
	fprintf(stderr, "Instruction not implemented: i16_ror_reg_T1\n");
}

static void emulation_i32_ror_reg_T2(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, bool S) {
	fprintf(stderr, "Instruction not implemented: i32_ror_reg_T2\n");
}

static void emulation_i32_rrx_T1(void *vctx, uint8_t Rd, uint8_t Rm, bool S) {
	fprintf(stderr, "Instruction not implemented: i32_rrx_T1\n");
}

static void emulation_i16_rsb_imm_T1(void *vctx, uint8_t Rd, uint8_t Rn) {
	fprintf(stderr, "Instruction not implemented: i16_rsb_imm_T1\n");
}

static void emulation_i32_rsb_imm_T2(void *vctx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	if (S) {
		setSubCondCode(ctx, true, imm, ctx->emu_ctx->cpu.reg[Rn]);
	}
	ctx->emu_ctx->cpu.reg[Rd] = imm - ctx->emu_ctx->cpu.reg[Rn];
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_rsb_reg_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	struct barrel_shifterOutput bsOut = barrel_shift(ctx->emu_ctx->cpu.reg[Rm], type, imm);
	if (S) {
		setSubCondCode(ctx, true, bsOut.value, ctx->emu_ctx->cpu.reg[Rn]);
	}
	ctx->emu_ctx->cpu.reg[Rd] = bsOut.value - ctx->emu_ctx->cpu.reg[Rn];
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_sbc_imm_T1(void *vctx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S) {
	fprintf(stderr, "Instruction not implemented: i32_sbc_imm_T1\n");
}

static void emulation_i16_sbc_reg_T1(void *vctx, uint8_t Rdn, uint8_t Rm) {
	fprintf(stderr, "Instruction not implemented: i16_sbc_reg_T1\n");
}

static void emulation_i32_sbc_reg_T2(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S) {
	fprintf(stderr, "Instruction not implemented: i32_sbc_reg_T2\n");
}

static void emulation_i32_sdiv_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	// TODO: Condcodes?
	ctx->emu_ctx->cpu.reg[Rd] = (int32_t)ctx->emu_ctx->cpu.reg[Rn] / (int32_t)ctx->emu_ctx->cpu.reg[Rm];
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_sev_T1(void *vctx) {
	fprintf(stderr, "Instruction not implemented: i16_sev_T1\n");
}

static void emulation_i32_sev_T2(void *vctx) {
	fprintf(stderr, "Instruction not implemented: i32_sev_T2\n");
}

static void emulation_i32_smull_T1(void *vctx, uint8_t Rn, uint8_t Rm, uint8_t RdLo, uint8_t RdHi) {
	fprintf(stderr, "Instruction not implemented: i32_smull_T1\n");
}

static void emulation_i32_stc2_T2(void *vctx, uint8_t Rn, uint8_t imm, uint8_t coproc, uint8_t CRd, bool P, bool N, bool U, bool W) {
	fprintf(stderr, "Instruction not implemented: i32_stc2_T2\n");
}

static void emulation_i32_stc_T1(void *vctx, uint8_t Rn, uint8_t imm, uint8_t coproc, uint8_t CRd, bool P, bool N, bool U, bool W) {
	fprintf(stderr, "Instruction not implemented: i32_stc_T1\n");
}

static void emulation_i16_stm_T1(void *vctx, uint8_t Rn, uint8_t register_list) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	storeRegisters(ctx, Rn, register_list, true, true);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_stm_T2(void *vctx, uint8_t Rn, uint16_t register_list, bool M, bool W) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	if (M) {
		register_list |= 1 << REG_LR;
	}
	storeRegisters(ctx, Rn, register_list, true, W);
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_stmdb_T1(void *vctx, uint8_t Rn, uint16_t register_list, bool M, bool W) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	if (M) {
		register_list |= (1 << REG_LR);
	}
	storeRegisters(ctx, Rn, register_list, false, W);
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_str_imm_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + (4 * imm);
	addrspace_write32(&ctx->emu_ctx->addr_space, target, ctx->emu_ctx->cpu.reg[Rt]);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i16_str_imm_T2(void *vctx, uint8_t Rt, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[REG_SP] + (4 * imm);
	addrspace_write32(&ctx->emu_ctx->addr_space, target, ctx->emu_ctx->cpu.reg[Rt]);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_str_imm_T3(void *vctx, uint8_t Rt, uint8_t Rn, uint16_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + imm;
	addrspace_write32(&ctx->emu_ctx->addr_space, target, ctx->emu_ctx->cpu.reg[Rt]);
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_str_imm_T4(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm, bool P, bool U, bool W) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + (P ? 1 : 0) * (U ? 1 : -1) * imm;
	addrspace_write32(&ctx->emu_ctx->addr_space, target, ctx->emu_ctx->cpu.reg[Rt]);
	if (W) {
		ctx->emu_ctx->cpu.reg[Rn] = ctx->emu_ctx->cpu.reg[Rn] + (U ? 1 : -1) * imm;
	}
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_str_reg_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + ctx->emu_ctx->cpu.reg[Rm];
	addrspace_write32(&ctx->emu_ctx->addr_space, target, ctx->emu_ctx->cpu.reg[Rt]);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_str_reg_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + (ctx->emu_ctx->cpu.reg[Rm] << imm);
	addrspace_write32(&ctx->emu_ctx->addr_space, target, ctx->emu_ctx->cpu.reg[Rt]);
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_strb_imm_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + imm;
	addrspace_write8(&ctx->emu_ctx->addr_space, target, ctx->emu_ctx->cpu.reg[Rt] & 0xff);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_strb_imm_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint16_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + imm;
	addrspace_write8(&ctx->emu_ctx->addr_space, target, ctx->emu_ctx->cpu.reg[Rt] & 0xff);
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_strb_imm_T3(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm, bool P, bool U, bool W) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + (P ? 1 : 0) * (U ? 1 : -1) * imm;
	addrspace_write8(&ctx->emu_ctx->addr_space, target, ctx->emu_ctx->cpu.reg[Rt] & 0xff);
	if (W) {
		ctx->emu_ctx->cpu.reg[Rn] = ctx->emu_ctx->cpu.reg[Rn] + (U ? 1 : -1) * imm;
	}
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_strb_reg_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + ctx->emu_ctx->cpu.reg[Rm];
	addrspace_write8(&ctx->emu_ctx->addr_space, target, ctx->emu_ctx->cpu.reg[Rt] & 0xff);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_strb_reg_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm, uint8_t imm) {
	// TODO: ??? imm unbenutzt?
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + ctx->emu_ctx->cpu.reg[Rm] + imm;
	addrspace_write8(&ctx->emu_ctx->addr_space, target, ctx->emu_ctx->cpu.reg[Rt] & 0xff);
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_strd_imm_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rtx, uint8_t imm, bool P, bool U, bool W) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + (P ? 1 : 0) * (U ? 1 : -1) * (4 * imm);
	addrspace_write32(&ctx->emu_ctx->addr_space, target, ctx->emu_ctx->cpu.reg[Rt]);
	addrspace_write32(&ctx->emu_ctx->addr_space, target + 4, ctx->emu_ctx->cpu.reg[Rtx]);
	if (W) {
		ctx->emu_ctx->cpu.reg[Rn] = ctx->emu_ctx->cpu.reg[Rn] + (U ? 1 : -1) * (4 * imm);
	}
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_strh_imm_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + 2 * imm;
	addrspace_write16(&ctx->emu_ctx->addr_space, target, ctx->emu_ctx->cpu.reg[Rt] & 0xffff);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_strh_imm_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint16_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint32_t target = ctx->emu_ctx->cpu.reg[Rn] + imm;
	addrspace_write16(&ctx->emu_ctx->addr_space, target, ctx->emu_ctx->cpu.reg[Rt] & 0xffff);
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_strh_imm_T3(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm, bool P, bool U, bool W) {
	fprintf(stderr, "Instruction not implemented: i32_strh_imm_T3\n");
}

static void emulation_i16_strh_reg_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm) {
	fprintf(stderr, "Instruction not implemented: i16_strh_reg_T1\n");
}

static void emulation_i32_strh_reg_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm, uint8_t imm) {
	fprintf(stderr, "Instruction not implemented: i32_strh_reg_T2\n");
}

static void emulation_i32_strht_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
	fprintf(stderr, "Instruction not implemented: i32_strht_T1\n");
}

static void emulation_i32_strt_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
	fprintf(stderr, "Instruction not implemented: i32_strt_T1\n");
}

static void emulation_i16_sub_SPimm_T1(void *vctx, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[REG_SP] -= imm * 4;
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_sub_SPimm_T2(void *vctx, uint8_t Rd, uint16_t imm, bool S) {
	fprintf(stderr, "Instruction not implemented: i32_sub_SPimm_T2\n");
}

static void emulation_i32_sub_SPimm_T3(void *vctx, uint8_t Rd, uint16_t imm) {
	fprintf(stderr, "Instruction not implemented: i32_sub_SPimm_T3\n");
}

static void emulation_i32_sub_SPreg_T1(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t imm, uint8_t type, bool S) {
	fprintf(stderr, "Instruction not implemented: i32_sub_SPreg_T1\n");
}

static void emulation_i16_sub_imm_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	setSubCondCode(ctx, false, ctx->emu_ctx->cpu.reg[Rn], imm);
	ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rn] - imm;
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i16_sub_imm_T2(void *vctx, uint8_t Rdn, uint8_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	setSubCondCode(ctx, false, ctx->emu_ctx->cpu.reg[Rdn], imm);
	ctx->emu_ctx->cpu.reg[Rdn] -= imm;
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_sub_imm_T3(void *vctx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	if (S) {
		setSubCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rn], imm);
	}
	ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rn] - imm;
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_sub_imm_T4(void *vctx, uint8_t Rd, uint8_t Rn, uint16_t imm) {
	fprintf(stderr, "Instruction not implemented: i32_sub_imm_T4\n");
}

static void emulation_i16_sub_reg_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	setSubCondCode(ctx, false, ctx->emu_ctx->cpu.reg[Rn], ctx->emu_ctx->cpu.reg[Rm]);
	ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rn] - ctx->emu_ctx->cpu.reg[Rm];
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_sub_reg_T2(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	struct barrel_shifterOutput bsOut = barrel_shift(ctx->emu_ctx->cpu.reg[Rm], type, imm);
	if (S) {
		setSubCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rn], bsOut.value);
	}
	ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rn] - bsOut.value;
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_svc_T1(void *vctx, uint8_t imm) {
	fprintf(stderr, "Instruction not implemented: i16_svc_T1\n");
}

static void emulation_i32_sxtb16_T1(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t rotate) {
	fprintf(stderr, "Instruction not implemented: i32_sxtb16_T1\n");
}

static void emulation_i16_sxtb_T1(void *vctx, uint8_t Rd, uint8_t Rm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[Rd] = (int8_t)(ctx->emu_ctx->cpu.reg[Rm] & 0xff);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_sxtb_T2(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t rotate) {
	fprintf(stderr, "Instruction not implemented: i32_sxtb_T2\n");
}

static void emulation_i16_sxth_T1(void *vctx, uint8_t Rd, uint8_t Rm) {
	fprintf(stderr, "Instruction not implemented: i16_sxth_T1\n");
}

static void emulation_i32_sxth_T2(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t rotate) {
	fprintf(stderr, "Instruction not implemented: i32_sxth_T2\n");
}

static void emulation_i32_tbb_T1(void *vctx, uint8_t Rn, uint8_t Rm, bool H) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
	if (H) {
		// TODO negative offsets?
		ctx->emu_ctx->cpu.reg[REG_PC] += 2 * addrspace_read16(&ctx->emu_ctx->addr_space, ctx->emu_ctx->cpu.reg[Rn] + (2 * ctx->emu_ctx->cpu.reg[Rm]));
	} else {
		// TODO implement me
	}
}

static void emulation_i32_tst_imm_T1(void *vctx, uint8_t Rn, int32_t imm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	setMovCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rn] & imm);
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_tst_reg_T1(void *vctx, uint8_t Rn, uint8_t Rm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	setMovCondCode(ctx, true, ctx->emu_ctx->cpu.reg[Rn] & ctx->emu_ctx->cpu.reg[Rm]);
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_tst_reg_T2(void *vctx, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type) {
	fprintf(stderr, "Instruction not implemented: i32_tst_reg_T2\n");
}

static void emulation_i32_ubfx_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t imm, uint8_t width) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[Rd] = (ctx->emu_ctx->cpu.reg[Rn] >> imm) & ((1 << width) - 1);
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i16_udf_T1(void *vctx, uint8_t imm) {
	fprintf(stderr, "Instruction not implemented: i16_udf_T1\n");
}

static void emulation_i32_udf_T2(void *vctx, uint16_t imm) {
	fprintf(stderr, "Instruction not implemented: i32_udf_T2\n");
}

static void emulation_i32_udiv_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm) {
	fprintf(stderr, "Instruction not implemented: i32_udiv_T1\n");
}

static void emulation_i32_umlal_T1(void *vctx, uint8_t Rn, uint8_t Rm, uint8_t RdLo, uint8_t RdHi) {
	fprintf(stderr, "Instruction not implemented: i32_umlal_T1\n");
}

static void emulation_i32_umull_T1(void *vctx, uint8_t Rn, uint8_t Rm, uint8_t RdLo, uint8_t RdHi) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	uint64_t result = (uint64_t)ctx->emu_ctx->cpu.reg[Rn] * ctx->emu_ctx->cpu.reg[Rm];
	ctx->emu_ctx->cpu.reg[RdLo] = (result >> 0) & 0xffffffff;
	ctx->emu_ctx->cpu.reg[RdHi] = (result >> 32) & 0xffffffff;
	// TODO: flags?
	ctx->emu_ctx->cpu.reg[REG_PC] += 4;
}

static void emulation_i32_uxtb16_T1(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t rotate) {
	fprintf(stderr, "Instruction not implemented: i32_uxtb16_T1\n");
}

static void emulation_i16_uxtb_T1(void *vctx, uint8_t Rd, uint8_t Rm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rm] & 0xff;
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_uxtb_T2(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t rotate) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	if (rotate == 0) {
		ctx->emu_ctx->cpu.reg[Rd] = (ctx->emu_ctx->cpu.reg[Rm] & 0xff) << (8 * rotate);
		ctx->emu_ctx->cpu.reg[REG_PC] += 4;
	} else {
	}
}

static void emulation_i16_uxth_T1(void *vctx, uint8_t Rd, uint8_t Rm) {
	struct insn_emu_ctx_t *ctx = (struct insn_emu_ctx_t*)vctx;
	ctx->emu_ctx->cpu.reg[Rd] = ctx->emu_ctx->cpu.reg[Rm] & 0xffff;
	ctx->emu_ctx->cpu.reg[REG_PC] += 2;
}

static void emulation_i32_uxth_T2(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t rotate) {
	fprintf(stderr, "Instruction not implemented: i32_uxth_T2\n");
}

static void emulation_i16_yield_T1(void *vctx) {
	fprintf(stderr, "Instruction not implemented: i16_yield_T1\n");
}

static void emulation_i32_yield_T2(void *vctx) {
	fprintf(stderr, "Instruction not implemented: i32_yield_T2\n");
}

const struct decoding_handler_t decodeOnlyCallbacks;

const struct decoding_handler_t emulationCallbacks = {
	.i32_adc_imm_T1 = emulation_i32_adc_imm_T1,
	.i16_adc_reg_T1 = emulation_i16_adc_reg_T1,
	.i32_adc_reg_T2 = emulation_i32_adc_reg_T2,
	.i16_add_SPi_T1 = emulation_i16_add_SPi_T1,
	.i16_add_SPi_T2 = emulation_i16_add_SPi_T2,
	.i32_add_SPi_T3 = emulation_i32_add_SPi_T3,
	.i32_add_SPi_T4 = emulation_i32_add_SPi_T4,
	.i16_add_SPr_T1 = emulation_i16_add_SPr_T1,
	.i16_add_SPr_T2 = emulation_i16_add_SPr_T2,
	.i32_add_SPr_T3 = emulation_i32_add_SPr_T3,
	.i16_add_imm_T1 = emulation_i16_add_imm_T1,
	.i16_add_imm_T2 = emulation_i16_add_imm_T2,
	.i32_add_imm_T3 = emulation_i32_add_imm_T3,
	.i32_add_imm_T4 = emulation_i32_add_imm_T4,
	.i16_add_reg_T1 = emulation_i16_add_reg_T1,
	.i16_add_reg_T2 = emulation_i16_add_reg_T2,
	.i32_add_reg_T3 = emulation_i32_add_reg_T3,
	.i16_adr_T1 = emulation_i16_adr_T1,
	.i32_adr_T2 = emulation_i32_adr_T2,
	.i32_adr_T3 = emulation_i32_adr_T3,
	.i32_and_imm_T1 = emulation_i32_and_imm_T1,
	.i16_and_reg_T1 = emulation_i16_and_reg_T1,
	.i32_and_reg_T2 = emulation_i32_and_reg_T2,
	.i16_asr_imm_T1 = emulation_i16_asr_imm_T1,
	.i32_asr_imm_T2 = emulation_i32_asr_imm_T2,
	.i16_asr_reg_T1 = emulation_i16_asr_reg_T1,
	.i32_asr_reg_T2 = emulation_i32_asr_reg_T2,
	.i16_b_T1 = emulation_i16_b_T1,
	.i16_b_T2 = emulation_i16_b_T2,
	.i32_b_T3 = emulation_i32_b_T3,
	.i32_b_T4 = emulation_i32_b_T4,
	.i32_bfc_T1 = emulation_i32_bfc_T1,
	.i32_bfi_T1 = emulation_i32_bfi_T1,
	.i32_bic_imm_T1 = emulation_i32_bic_imm_T1,
	.i16_bic_reg_T1 = emulation_i16_bic_reg_T1,
	.i32_bic_reg_T2 = emulation_i32_bic_reg_T2,
	.i16_bkpt_T1 = emulation_i16_bkpt_T1,
	.i32_bl_T1 = emulation_i32_bl_T1,
	.i16_blx_reg_T1 = emulation_i16_blx_reg_T1,
	.i16_bx_T1 = emulation_i16_bx_T1,
	.i16_cbnz_T1 = emulation_i16_cbnz_T1,
	.i32_cdp2_T2 = emulation_i32_cdp2_T2,
	.i32_cdp_T1 = emulation_i32_cdp_T1,
	.i32_clrex_T1 = emulation_i32_clrex_T1,
	.i32_clz_T1 = emulation_i32_clz_T1,
	.i32_cmn_imm_T1 = emulation_i32_cmn_imm_T1,
	.i16_cmn_reg_T1 = emulation_i16_cmn_reg_T1,
	.i32_cmn_reg_T2 = emulation_i32_cmn_reg_T2,
	.i16_cmp_imm_T1 = emulation_i16_cmp_imm_T1,
	.i32_cmp_imm_T2 = emulation_i32_cmp_imm_T2,
	.i16_cmp_reg_T1 = emulation_i16_cmp_reg_T1,
	.i16_cmp_reg_T2 = emulation_i16_cmp_reg_T2,
	.i32_cmp_reg_T3 = emulation_i32_cmp_reg_T3,
	.i16_cps_T1 = emulation_i16_cps_T1,
	.i32_dbg_T1 = emulation_i32_dbg_T1,
	.i32_dmb_T1 = emulation_i32_dmb_T1,
	.i32_dsb_T1 = emulation_i32_dsb_T1,
	.i32_eor_imm_T1 = emulation_i32_eor_imm_T1,
	.i16_eor_reg_T1 = emulation_i16_eor_reg_T1,
	.i32_eor_reg_T2 = emulation_i32_eor_reg_T2,
	.i32_isb_T1 = emulation_i32_isb_T1,
	.i16_it_T1 = emulation_i16_it_T1,
	.i32_ldc2_imm_T2 = emulation_i32_ldc2_imm_T2,
	.i32_ldc2_lit_T2 = emulation_i32_ldc2_lit_T2,
	.i32_ldc_imm_T1 = emulation_i32_ldc_imm_T1,
	.i32_ldc_lit_T1 = emulation_i32_ldc_lit_T1,
	.i16_ldm_T1 = emulation_i16_ldm_T1,
	.i32_ldm_T2 = emulation_i32_ldm_T2,
	.i32_ldmdb_T1 = emulation_i32_ldmdb_T1,
	.i16_ldr_imm_T1 = emulation_i16_ldr_imm_T1,
	.i16_ldr_imm_T2 = emulation_i16_ldr_imm_T2,
	.i32_ldr_imm_T3 = emulation_i32_ldr_imm_T3,
	.i32_ldr_imm_T4 = emulation_i32_ldr_imm_T4,
	.i16_ldr_lit_T1 = emulation_i16_ldr_lit_T1,
	.i32_ldr_lit_T2 = emulation_i32_ldr_lit_T2,
	.i16_ldr_reg_T1 = emulation_i16_ldr_reg_T1,
	.i32_ldr_reg_T2 = emulation_i32_ldr_reg_T2,
	.i16_ldrb_imm_T1 = emulation_i16_ldrb_imm_T1,
	.i32_ldrb_imm_T2 = emulation_i32_ldrb_imm_T2,
	.i32_ldrb_imm_T3 = emulation_i32_ldrb_imm_T3,
	.i32_ldrb_lit_T1 = emulation_i32_ldrb_lit_T1,
	.i16_ldrb_reg_T1 = emulation_i16_ldrb_reg_T1,
	.i32_ldrb_reg_T2 = emulation_i32_ldrb_reg_T2,
	.i32_ldrbt_T1 = emulation_i32_ldrbt_T1,
	.i32_ldrd_imm_T1 = emulation_i32_ldrd_imm_T1,
	.i32_ldrd_lit_T1 = emulation_i32_ldrd_lit_T1,
	.i32_ldrex_T1 = emulation_i32_ldrex_T1,
	.i32_ldrexb_T1 = emulation_i32_ldrexb_T1,
	.i32_ldrexh_T1 = emulation_i32_ldrexh_T1,
	.i16_ldrh_imm_T1 = emulation_i16_ldrh_imm_T1,
	.i32_ldrh_imm_T2 = emulation_i32_ldrh_imm_T2,
	.i32_ldrh_imm_T3 = emulation_i32_ldrh_imm_T3,
	.i32_ldrh_lit_T1 = emulation_i32_ldrh_lit_T1,
	.i16_ldrh_reg_T1 = emulation_i16_ldrh_reg_T1,
	.i32_ldrh_reg_T2 = emulation_i32_ldrh_reg_T2,
	.i32_ldrht_T1 = emulation_i32_ldrht_T1,
	.i32_ldrsb_imm_T1 = emulation_i32_ldrsb_imm_T1,
	.i32_ldrsb_imm_T2 = emulation_i32_ldrsb_imm_T2,
	.i32_ldrsb_lit_T1 = emulation_i32_ldrsb_lit_T1,
	.i16_ldrsb_reg_T1 = emulation_i16_ldrsb_reg_T1,
	.i32_ldrsb_reg_T2 = emulation_i32_ldrsb_reg_T2,
	.i32_ldrsbt_T1 = emulation_i32_ldrsbt_T1,
	.i32_ldrsh_imm_T1 = emulation_i32_ldrsh_imm_T1,
	.i32_ldrsh_imm_T2 = emulation_i32_ldrsh_imm_T2,
	.i32_ldrsh_lit_T1 = emulation_i32_ldrsh_lit_T1,
	.i16_ldrsh_reg_T1 = emulation_i16_ldrsh_reg_T1,
	.i32_ldrsh_reg_T2 = emulation_i32_ldrsh_reg_T2,
	.i32_ldrsh_T1 = emulation_i32_ldrsh_T1,
	.i32_ldrt_T1 = emulation_i32_ldrt_T1,
	.i16_lsl_imm_T1 = emulation_i16_lsl_imm_T1,
	.i32_lsl_imm_T2 = emulation_i32_lsl_imm_T2,
	.i16_lsl_reg_T1 = emulation_i16_lsl_reg_T1,
	.i32_lsl_reg_T2 = emulation_i32_lsl_reg_T2,
	.i16_lsr_imm_T1 = emulation_i16_lsr_imm_T1,
	.i32_lsr_imm_T2 = emulation_i32_lsr_imm_T2,
	.i16_lsr_reg_T1 = emulation_i16_lsr_reg_T1,
	.i32_lsr_reg_T2 = emulation_i32_lsr_reg_T2,
	.i32_mcr2_T2 = emulation_i32_mcr2_T2,
	.i32_mcr_T1 = emulation_i32_mcr_T1,
	.i32_mcrr2_T2 = emulation_i32_mcrr2_T2,
	.i32_mcrr_T1 = emulation_i32_mcrr_T1,
	.i32_mla_T1 = emulation_i32_mla_T1,
	.i32_mls_T1 = emulation_i32_mls_T1,
	.i16_mov_imm_T1 = emulation_i16_mov_imm_T1,
	.i32_mov_imm_T2 = emulation_i32_mov_imm_T2,
	.i32_mov_imm_T3 = emulation_i32_mov_imm_T3,
	.i16_mov_reg_T1 = emulation_i16_mov_reg_T1,
	.i16_mov_reg_T2 = emulation_i16_mov_reg_T2,
	.i32_mov_reg_T3 = emulation_i32_mov_reg_T3,
	.i32_movt_T1 = emulation_i32_movt_T1,
	.i32_mrc2_T2 = emulation_i32_mrc2_T2,
	.i32_mrc_T1 = emulation_i32_mrc_T1,
	.i32_mrrc2_T2 = emulation_i32_mrrc2_T2,
	.i32_mrrc_T1 = emulation_i32_mrrc_T1,
	.i32_mrs_T1 = emulation_i32_mrs_T1,
	.i32_msr_T1 = emulation_i32_msr_T1,
	.i16_mul_T1 = emulation_i16_mul_T1,
	.i32_mul_T2 = emulation_i32_mul_T2,
	.i32_mvn_imm_T1 = emulation_i32_mvn_imm_T1,
	.i16_mvn_reg_T1 = emulation_i16_mvn_reg_T1,
	.i32_mvn_reg_T2 = emulation_i32_mvn_reg_T2,
	.i16_nop_T1 = emulation_i16_nop_T1,
	.i32_nop_T2 = emulation_i32_nop_T2,
	.i32_orn_imm_T1 = emulation_i32_orn_imm_T1,
	.i32_orn_reg_T1 = emulation_i32_orn_reg_T1,
	.i32_orr_imm_T1 = emulation_i32_orr_imm_T1,
	.i16_orr_reg_T1 = emulation_i16_orr_reg_T1,
	.i32_orr_reg_T2 = emulation_i32_orr_reg_T2,
	.i32_pkhbt_T1 = emulation_i32_pkhbt_T1,
	.i16_pop_T1 = emulation_i16_pop_T1,
	.i32_pop_T2 = emulation_i32_pop_T2,
	.i32_pop_T3 = emulation_i32_pop_T3,
	.i16_push_T1 = emulation_i16_push_T1,
	.i32_push_T2 = emulation_i32_push_T2,
	.i32_push_T3 = emulation_i32_push_T3,
	.i32_rbit_T1 = emulation_i32_rbit_T1,
	.i16_rev16_T1 = emulation_i16_rev16_T1,
	.i32_rev16_T2 = emulation_i32_rev16_T2,
	.i16_rev_T1 = emulation_i16_rev_T1,
	.i32_rev_T2 = emulation_i32_rev_T2,
	.i16_revsh_T1 = emulation_i16_revsh_T1,
	.i32_revsh_T2 = emulation_i32_revsh_T2,
	.i32_ror_imm_T1 = emulation_i32_ror_imm_T1,
	.i16_ror_reg_T1 = emulation_i16_ror_reg_T1,
	.i32_ror_reg_T2 = emulation_i32_ror_reg_T2,
	.i32_rrx_T1 = emulation_i32_rrx_T1,
	.i16_rsb_imm_T1 = emulation_i16_rsb_imm_T1,
	.i32_rsb_imm_T2 = emulation_i32_rsb_imm_T2,
	.i32_rsb_reg_T1 = emulation_i32_rsb_reg_T1,
	.i32_sbc_imm_T1 = emulation_i32_sbc_imm_T1,
	.i16_sbc_reg_T1 = emulation_i16_sbc_reg_T1,
	.i32_sbc_reg_T2 = emulation_i32_sbc_reg_T2,
	.i32_sdiv_T1 = emulation_i32_sdiv_T1,
	.i16_sev_T1 = emulation_i16_sev_T1,
	.i32_sev_T2 = emulation_i32_sev_T2,
	.i32_smull_T1 = emulation_i32_smull_T1,
	.i32_stc2_T2 = emulation_i32_stc2_T2,
	.i32_stc_T1 = emulation_i32_stc_T1,
	.i16_stm_T1 = emulation_i16_stm_T1,
	.i32_stm_T2 = emulation_i32_stm_T2,
	.i32_stmdb_T1 = emulation_i32_stmdb_T1,
	.i16_str_imm_T1 = emulation_i16_str_imm_T1,
	.i16_str_imm_T2 = emulation_i16_str_imm_T2,
	.i32_str_imm_T3 = emulation_i32_str_imm_T3,
	.i32_str_imm_T4 = emulation_i32_str_imm_T4,
	.i16_str_reg_T1 = emulation_i16_str_reg_T1,
	.i32_str_reg_T2 = emulation_i32_str_reg_T2,
	.i16_strb_imm_T1 = emulation_i16_strb_imm_T1,
	.i32_strb_imm_T2 = emulation_i32_strb_imm_T2,
	.i32_strb_imm_T3 = emulation_i32_strb_imm_T3,
	.i16_strb_reg_T1 = emulation_i16_strb_reg_T1,
	.i32_strb_reg_T2 = emulation_i32_strb_reg_T2,
	.i32_strd_imm_T1 = emulation_i32_strd_imm_T1,
	.i16_strh_imm_T1 = emulation_i16_strh_imm_T1,
	.i32_strh_imm_T2 = emulation_i32_strh_imm_T2,
	.i32_strh_imm_T3 = emulation_i32_strh_imm_T3,
	.i16_strh_reg_T1 = emulation_i16_strh_reg_T1,
	.i32_strh_reg_T2 = emulation_i32_strh_reg_T2,
	.i32_strht_T1 = emulation_i32_strht_T1,
	.i32_strt_T1 = emulation_i32_strt_T1,
	.i16_sub_SPimm_T1 = emulation_i16_sub_SPimm_T1,
	.i32_sub_SPimm_T2 = emulation_i32_sub_SPimm_T2,
	.i32_sub_SPimm_T3 = emulation_i32_sub_SPimm_T3,
	.i32_sub_SPreg_T1 = emulation_i32_sub_SPreg_T1,
	.i16_sub_imm_T1 = emulation_i16_sub_imm_T1,
	.i16_sub_imm_T2 = emulation_i16_sub_imm_T2,
	.i32_sub_imm_T3 = emulation_i32_sub_imm_T3,
	.i32_sub_imm_T4 = emulation_i32_sub_imm_T4,
	.i16_sub_reg_T1 = emulation_i16_sub_reg_T1,
	.i32_sub_reg_T2 = emulation_i32_sub_reg_T2,
	.i16_svc_T1 = emulation_i16_svc_T1,
	.i32_sxtb16_T1 = emulation_i32_sxtb16_T1,
	.i16_sxtb_T1 = emulation_i16_sxtb_T1,
	.i32_sxtb_T2 = emulation_i32_sxtb_T2,
	.i16_sxth_T1 = emulation_i16_sxth_T1,
	.i32_sxth_T2 = emulation_i32_sxth_T2,
	.i32_tbb_T1 = emulation_i32_tbb_T1,
	.i32_tst_imm_T1 = emulation_i32_tst_imm_T1,
	.i16_tst_reg_T1 = emulation_i16_tst_reg_T1,
	.i32_tst_reg_T2 = emulation_i32_tst_reg_T2,
	.i32_ubfx_T1 = emulation_i32_ubfx_T1,
	.i16_udf_T1 = emulation_i16_udf_T1,
	.i32_udf_T2 = emulation_i32_udf_T2,
	.i32_udiv_T1 = emulation_i32_udiv_T1,
	.i32_umlal_T1 = emulation_i32_umlal_T1,
	.i32_umull_T1 = emulation_i32_umull_T1,
	.i32_uxtb16_T1 = emulation_i32_uxtb16_T1,
	.i16_uxtb_T1 = emulation_i16_uxtb_T1,
	.i32_uxtb_T2 = emulation_i32_uxtb_T2,
	.i16_uxth_T1 = emulation_i16_uxth_T1,
	.i32_uxth_T2 = emulation_i32_uxth_T2,
	.i16_yield_T1 = emulation_i16_yield_T1,
	.i32_yield_T2 = emulation_i32_yield_T2,
};

// vim: set filetype=c:
