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

#include "decoder.h"
#include "impl_disassembly.h"
#include "cpu_cm3.h"

#define OPCODE_WIDE						(1 << 0)
#define OPCODE_NARROW					(1 << 1)
#define OPCODE_SETFLAGS					(1 << 2)
#define OPCODE_SETFLAGS_IF_UNCOND		(1 << 3)
#define OPCODE_FORCE_CONDITION			(1 << 4)
#define OPCODE_ISCONDITIONAL(cond)		(OPCODE_FORCE_CONDITION | (((cond) & 0xf) << 8))

static const char *ccString(const enum it_cond_t aCondition) {
	switch (aCondition) {
		case CONDITION_EQ:		return "eq";
		case CONDITION_NE:		return "ne";
		case CONDITION_CS:		return "cs";
		case CONDITION_CC:		return "cc";
		case CONDITION_MI:		return "mi";
		case CONDITION_PL:		return "pl";
		case CONDITION_VS:		return "vs";
		case CONDITION_VC:		return "vc";
		case CONDITION_HI:		return "hi";
		case CONDITION_LS:		return "ls";
		case CONDITION_GE:		return "ge";
		case CONDITION_LT:		return "lt";
		case CONDITION_GT:		return "gt";
		case CONDITION_LE:		return "le";
		case CONDITION_AL:		return "al";
		default:				return "?";
	}
}

static const char *regString(uint8_t aReg) {
	switch (aReg) {
		case 0:		return "r0";
		case 1:		return "r1";
		case 2:		return "r2";
		case 3:		return "r3";
		case 4:		return "r4";
		case 5:		return "r5";
		case 6:		return "r6";
		case 7:		return "r7";
		case 8:		return "r8";
		case 9:		return "r9";
		case 10:	return "sl";
		case 11:	return "fp";
		case 12:	return "ip";
		case 13:	return "sp";
		case 14:	return "lr";
		case 15:	return "pc";
		default:	return "r?";
	}
}

static uint32_t relative_branch_target(uint32_t aPC, uint32_t aImmediate, uint8_t aImmBits) {
	if (aImmediate >= (1 << (aImmBits - 1))) {
		// Negative offset
		return aPC - (4 + 2 * ((1 << aImmBits) - aImmediate)) + 8;
	} else {
		return aPC + (2 * aImmediate) + 4;
	}
}

static void printOpcode(struct disas_ctx_t *disas_ctx, const char *aOpcode, uint16_t aFlags) {
	disas_ctx->printDisassembly(disas_ctx, "%s", aOpcode);
	if ((aFlags & OPCODE_SETFLAGS) || (((disas_ctx->it_state & 0x03) == IT_NONE) && (aFlags & OPCODE_SETFLAGS_IF_UNCOND))) {
		disas_ctx->printDisassembly(disas_ctx, "s");
	}
	if ((aFlags & OPCODE_FORCE_CONDITION) == 0) {
		/* Honor implicit condition flags */
		if ((disas_ctx->it_state & 0x03) == IT_THEN) {
			disas_ctx->printDisassembly(disas_ctx, "%s", ccString(disas_ctx->it_cond));
		} else if ((disas_ctx->it_state & 0x03) == IT_ELSE) {
			disas_ctx->printDisassembly(disas_ctx, "%s", ccString(disas_ctx->it_cond ^ 1));
		}
	} else {
		/* Override with given condition flag */
		disas_ctx->printDisassembly(disas_ctx, "%s", ccString((aFlags & 0xf00) >> 8));
	}
	if (aFlags & OPCODE_WIDE) {
		disas_ctx->printDisassembly(disas_ctx, ".w");
	} else if (aFlags & OPCODE_NARROW) {
		disas_ctx->printDisassembly(disas_ctx, ".n");
	}
}

static void print_reglist(struct disas_ctx_t *disas_ctx, uint16_t aRegList) {
	bool first = true;
	disas_ctx->printDisassembly(disas_ctx, " {");
	for (int i = 0; i < 16; i++) {
		if (aRegList & (1 << i)) {
			if (!first) {
				disas_ctx->printDisassembly(disas_ctx, ", ");
			} else {
				first = false;
			}
			disas_ctx->printDisassembly(disas_ctx, "%s", regString(i));
		}
	}
	disas_ctx->printDisassembly(disas_ctx, "}");
}

static void print_i(struct disas_ctx_t *disas_ctx, uint8_t aImmediate) {
	disas_ctx->printDisassembly(disas_ctx, " #%d", aImmediate);
}

static void print_r(struct disas_ctx_t *disas_ctx, uint8_t aR1) {
	disas_ctx->printDisassembly(disas_ctx, " %s", regString(aR1));
}

static void print_rr(struct disas_ctx_t *disas_ctx, uint8_t aR1, uint8_t aR2) {
	disas_ctx->printDisassembly(disas_ctx, " %s, %s", regString(aR1), regString(aR2));
}

static void print_rrr(struct disas_ctx_t *disas_ctx, uint8_t aR1, uint8_t aR2, uint8_t aR3) {
	disas_ctx->printDisassembly(disas_ctx, " %s, %s, %s", regString(aR1), regString(aR2), regString(aR3));
}

static void print_rrrr(struct disas_ctx_t *disas_ctx, uint8_t aR1, uint8_t aR2, uint8_t aR3, uint8_t aR4) {
	disas_ctx->printDisassembly(disas_ctx, " %s, %s, %s, %s", regString(aR1), regString(aR2), regString(aR3), regString(aR4));
}

static void print_s(struct disas_ctx_t *disas_ctx, uint8_t aType, uint8_t aImm) {
	if (aType == TYPE_LSL) {
		disas_ctx->printDisassembly(disas_ctx, ", lsl #%d", aImm);
	} else if (aType == TYPE_LSR) {
		if (aImm == 0) {
			aImm = 32;
		}
		disas_ctx->printDisassembly(disas_ctx, ", lsr #%d", aImm);
	} else if (aType == TYPE_ASR) {
		if (aImm == 0) {
			aImm = 32;
		}
		disas_ctx->printDisassembly(disas_ctx, ", asr #%d", aImm);
	} else {
		if (aImm == 0) {
			disas_ctx->printDisassembly(disas_ctx, ", rrx");
		} else {
			disas_ctx->printDisassembly(disas_ctx, ", ror #%d", aImm);
		}
	}
}

static void print_Irr(struct disas_ctx_t *disas_ctx, uint8_t aR1, uint8_t aR2) {
	disas_ctx->printDisassembly(disas_ctx, " [%s, %s]", regString(aR1), regString(aR2));
}

static void print_Irrs(struct disas_ctx_t *disas_ctx, uint8_t aR1, uint8_t aR2, uint8_t aType, uint8_t aImm) {
	disas_ctx->printDisassembly(disas_ctx, " [%s, %s", regString(aR1), regString(aR2));
	print_s(disas_ctx, aType, aImm);
	disas_ctx->printDisassembly(disas_ctx, "]");
}

static void print_rrs(struct disas_ctx_t *disas_ctx, uint8_t aR1, uint8_t aR2, uint8_t aType, uint8_t aImm) {
	disas_ctx->printDisassembly(disas_ctx, " %s, %s", regString(aR1), regString(aR2));
	if (aType || aImm) {
		print_s(disas_ctx, aType, aImm);
	}
}

static void print_rIrrs(struct disas_ctx_t *disas_ctx, uint8_t aR1, uint8_t aR2, uint8_t aR3, uint8_t aType, uint8_t aImm) {
	disas_ctx->printDisassembly(disas_ctx, " %s, [%s, %s", regString(aR1), regString(aR2), regString(aR3));
	if (aType || aImm) {
		print_s(disas_ctx, aType, aImm);
	}
	disas_ctx->printDisassembly(disas_ctx, "]");
}

static void print_rrrs(struct disas_ctx_t *disas_ctx, uint8_t aR1, uint8_t aR2, uint8_t aR3, uint8_t aType, uint8_t aImm) {
	disas_ctx->printDisassembly(disas_ctx, " %s, %s, %s", regString(aR1), regString(aR2), regString(aR3));
	if (aType || aImm) {
		print_s(disas_ctx, aType, aImm);
	}
}

static void print_rri(struct disas_ctx_t *disas_ctx, uint8_t aR1, uint8_t aR2, int aImmediate) {
	disas_ctx->printDisassembly(disas_ctx, " %s, %s, #%u", regString(aR1), regString(aR2), aImmediate);
}

static void print_rrii(struct disas_ctx_t *disas_ctx, uint8_t aR1, uint8_t aR2, int aImmediate1, int aImmediate2) {
	disas_ctx->printDisassembly(disas_ctx, " %s, %s, #%u, #%u", regString(aR1), regString(aR2), aImmediate1, aImmediate2);
}

static void print_rIro(struct disas_ctx_t *disas_ctx, uint8_t aR1, uint8_t aR2, int aImmediate) {
	disas_ctx->printDisassembly(disas_ctx, " %s, [%s, #%d]", regString(aR1), regString(aR2), aImmediate);
}

static void print_rrIrO(struct disas_ctx_t *disas_ctx, uint8_t aR1, uint8_t aR2, uint8_t aR3, int aImmediate) {
	if (aImmediate) {
		disas_ctx->printDisassembly(disas_ctx, " %s, %s, [%s, #%d]", regString(aR1), regString(aR2), regString(aR3), aImmediate);
	} else {
		disas_ctx->printDisassembly(disas_ctx, " %s, %s, [%s]", regString(aR1), regString(aR2), regString(aR3));
	}
}

static void print_rIrO(struct disas_ctx_t *disas_ctx, uint8_t aR1, uint8_t aR2, int aImmediate) {
	if (aImmediate) {
		disas_ctx->printDisassembly(disas_ctx, " %s, [%s, #%d]", regString(aR1), regString(aR2), aImmediate);
	} else {
		disas_ctx->printDisassembly(disas_ctx, " %s, [%s]", regString(aR1), regString(aR2));
	}
}

static void print_rIri(struct disas_ctx_t *disas_ctx, uint8_t aR1, uint8_t aR2, int aImmediate) {
	disas_ctx->printDisassembly(disas_ctx, " %s, [%s], #%d", regString(aR1), regString(aR2), aImmediate);
}

static void print_rrIri(struct disas_ctx_t *disas_ctx, uint8_t aR1, uint8_t aR2, uint8_t aR3, int aImmediate) {
	disas_ctx->printDisassembly(disas_ctx, " %s, %s, [%s], #%d", regString(aR1), regString(aR2), regString(aR3), aImmediate);
}

static void print_rIrr(struct disas_ctx_t *disas_ctx, uint8_t aR1, uint8_t aR2, uint8_t aR3) {
	disas_ctx->printDisassembly(disas_ctx, " %s, [%s, %s]", regString(aR1), regString(aR2), regString(aR3));
}

static void print_ri(struct disas_ctx_t *disas_ctx, uint8_t aR1, int aImmediate) {
	disas_ctx->printDisassembly(disas_ctx, " %s, #%u", regString(aR1), aImmediate);
}

static void disassembly_i32_adc_imm_T1(void *vctx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "adc", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
	print_rri(vctx, Rd, Rn, imm);
}

static void disassembly_i16_adc_reg_T1(void *vctx, uint8_t Rdn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "adc", OPCODE_SETFLAGS_IF_UNCOND);
	print_rr(ctx, Rdn, Rm);
}

static void disassembly_i32_adc_reg_T2(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "adc", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
	print_rrrs(vctx, Rd, Rn, Rm, type, imm);

}

static void disassembly_i16_add_SPi_T1(void *vctx, uint8_t Rd, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "add", 0);
	print_rri(vctx, Rd, REG_SP, imm * 4);
}

static void disassembly_i16_add_SPi_T2(void *vctx, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "add", 0);
	print_ri(vctx, REG_SP, imm * 4);
}

static void disassembly_i32_add_SPi_T3(void *vctx, uint8_t Rd, int32_t imm, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "add", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
	print_rri(vctx, Rd, REG_SP, imm);
}

static void disassembly_i32_add_SPi_T4(void *vctx, uint8_t Rd, uint16_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "add", OPCODE_WIDE);
	print_rri(vctx, REG_SP, Rd, imm);
}

static void disassembly_i16_add_SPr_T1(void *vctx, uint8_t Rdm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "add", 0);
	print_rr(vctx, Rdm, REG_SP);
}

static void disassembly_i16_add_SPr_T2(void *vctx, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "add", 0);
	print_rr(vctx, REG_SP, Rm);
}

static void disassembly_i32_add_SPr_T3(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t imm, uint8_t type, bool S) {
}

static void disassembly_i16_add_imm_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "add", OPCODE_SETFLAGS_IF_UNCOND);
	print_rri(vctx, Rd, Rn, imm);
}

static void disassembly_i16_add_imm_T2(void *vctx, uint8_t Rdn, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "add", OPCODE_SETFLAGS_IF_UNCOND);
	print_ri(vctx, Rdn, imm);
}

static void disassembly_i32_add_imm_T3(void *vctx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "add", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
	print_rri(vctx, Rd, Rn, imm);
}

static void disassembly_i32_add_imm_T4(void *vctx, uint8_t Rd, uint8_t Rn, uint16_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "addw", 0);
	print_rri(vctx, Rd, Rn, imm);
}

static void disassembly_i16_add_reg_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "add", OPCODE_SETFLAGS_IF_UNCOND);
	print_rrr(vctx, Rd, Rn, Rm);
}

static void disassembly_i16_add_reg_T2(void *vctx, uint8_t Rdn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "add", 0);
	print_rr(ctx, Rdn, Rm);
}

static void disassembly_i32_add_reg_T3(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "add", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
	print_rrrs(vctx, Rd, Rn, Rm, type, imm);
}

static void disassembly_i16_adr_T1(void *vctx, uint8_t Rd, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "add", 0);
	print_rri(vctx, Rd, REG_PC, imm * 4);
}

static void disassembly_i32_adr_T2(void *vctx, uint8_t Rd, uint16_t imm) {
}

static void disassembly_i32_adr_T3(void *vctx, uint8_t Rd, uint16_t imm) {
}

static void disassembly_i32_and_imm_T1(void *vctx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	if ((Rd == REG_PC) && S) {
		printOpcode(ctx, "tst", OPCODE_WIDE);
		print_ri(vctx, Rn, imm);
	} else {
		printOpcode(ctx, "and", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
		print_rri(vctx, Rd, Rn, imm);
	}
}

static void disassembly_i16_and_reg_T1(void *vctx, uint8_t Rdn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "and", OPCODE_SETFLAGS_IF_UNCOND);
	print_rr(ctx, Rdn, Rm);
}

static void disassembly_i32_and_reg_T2(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "and", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
	print_rrrs(vctx, Rd, Rn, Rm, type, imm);
}

static void disassembly_i16_asr_imm_T1(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	if (imm == 0) {
		imm = 32;
	}
	printOpcode(ctx, "asr", OPCODE_SETFLAGS_IF_UNCOND);
	print_rri(vctx, Rd, Rm, imm);
}

static void disassembly_i32_asr_imm_T2(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t imm, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "mov", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
	print_rrs(vctx, Rd, Rm, TYPE_ASR, imm);
}

static void disassembly_i16_asr_reg_T1(void *vctx, uint8_t Rdn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "asr", OPCODE_SETFLAGS_IF_UNCOND);
	print_rr(ctx, Rdn, Rm);
}

static void disassembly_i32_asr_reg_T2(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "asr", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
	print_rrr(vctx, Rd, Rn, Rm);
}

static void disassembly_i16_b_T1(void *vctx, uint8_t imm, uint8_t cond) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "b", OPCODE_NARROW | OPCODE_ISCONDITIONAL(cond));
	ctx->printDisassembly(vctx, " 0x%x", relative_branch_target(ctx->pc, imm, 8));
}

static void disassembly_i16_b_T2(void *vctx, uint16_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "b", OPCODE_NARROW);
	ctx->printDisassembly(vctx, " 0x%x", relative_branch_target(ctx->pc, imm, 11));
}

static void disassembly_i32_b_T3(void *vctx, int32_t imm, uint8_t cond) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "b", OPCODE_WIDE | OPCODE_ISCONDITIONAL(cond));
	ctx->printDisassembly(vctx, " 0x%x", ctx->pc + (2 * imm) + 4);
}

static void disassembly_i32_b_T4(void *vctx, int32_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "b", OPCODE_WIDE);
	ctx->printDisassembly(vctx, " 0x%x", ctx->pc + (2 * imm) + 4);
}

static void disassembly_i32_bfc_T1(void *vctx, uint8_t Rd, uint8_t imm, uint8_t msb) {
}

static void disassembly_i32_bfi_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t imm, uint8_t msb) {
}

static void disassembly_i32_bic_imm_T1(void *vctx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "bic", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
	print_rri(vctx, Rd, Rn, imm);
}

static void disassembly_i16_bic_reg_T1(void *vctx, uint8_t Rdn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "bic", OPCODE_SETFLAGS_IF_UNCOND);
	print_rr(ctx, Rdn, Rm);
}

static void disassembly_i32_bic_reg_T2(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "bic", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
	print_rrrs(vctx, Rd, Rn, Rm, type, imm);
}

static void disassembly_i16_bkpt_T1(void *vctx, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "bkpt", 0);
	ctx->printDisassembly(vctx, " 0x%04x", imm);
}

static void disassembly_i32_bl_T1(void *vctx, int32_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "bl", 0);
	ctx->printDisassembly(vctx, " 0x%x", ctx->pc + (2 * imm) + 4);
}

static void disassembly_i16_blx_reg_T1(void *vctx, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "blx", 0);
	print_r(vctx, Rm);
}

static void disassembly_i16_bx_T1(void *vctx, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "bx", 0);
	print_r(vctx, Rm);
}

static void disassembly_i16_cbnz_T1(void *vctx, uint8_t Rn, uint8_t imm, bool op) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, op ? "cbnz" : "cbz", 0);
	ctx->printDisassembly(vctx, " %s, 0x%x", regString(Rn), relative_branch_target(ctx->pc, imm, 7));
}

static void disassembly_i32_cdp2_T2(void *vctx, uint8_t coproc, uint8_t CRd, uint8_t CRn, uint8_t CRm, uint8_t opcA, uint8_t opcB) {
}

static void disassembly_i32_cdp_T1(void *vctx, uint8_t coproc, uint8_t CRd, uint8_t CRn, uint8_t CRm, uint8_t opcA, uint8_t opcB) {
}

static void disassembly_i32_clrex_T1(void *vctx) {
}

static void disassembly_i32_clz_T1(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t Rmx) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "clz", 0);
	print_rr(ctx, Rd, Rm);

}

static void disassembly_i32_cmn_imm_T1(void *vctx, uint8_t Rn, int32_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "cmn", OPCODE_WIDE);
	print_ri(vctx, Rn, imm);
}

static void disassembly_i16_cmn_reg_T1(void *vctx, uint8_t Rn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "cmn", 0);
	print_rr(ctx, Rn, Rm);
}

static void disassembly_i32_cmn_reg_T2(void *vctx, uint8_t Rn, uint8_t Rm, int32_t imm, uint8_t type) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "cmn", 0);
	print_rrs(vctx, Rn, Rm, type, imm);
}

static void disassembly_i16_cmp_imm_T1(void *vctx, uint8_t Rn, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "cmp", 0);
	print_ri(vctx, Rn, imm);
}

static void disassembly_i32_cmp_imm_T2(void *vctx, uint8_t Rn, int32_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "cmp", OPCODE_WIDE);
	print_ri(vctx, Rn, imm);
}

static void disassembly_i16_cmp_reg_T1(void *vctx, uint8_t Rn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "cmp", 0);
	print_rr(ctx, Rn, Rm);
}

static void disassembly_i16_cmp_reg_T2(void *vctx, uint8_t Rn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "cmp", 0);
	print_rr(ctx, Rn, Rm);
}

static void disassembly_i32_cmp_reg_T3(void *vctx, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "cmp", OPCODE_WIDE);
	print_rrs(vctx, Rn, Rm, type, imm);
}

static void disassembly_i16_cps_T1(void *vctx, bool imm, bool F, bool I) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;

	printOpcode(ctx, (imm == 0) ? "cpsie" : "cpsid", 0);
	if (I || F) {
		ctx->printDisassembly(vctx, " ");
	}
	if (I) {
		ctx->printDisassembly(vctx, "i");
	}
	if (F) {
		ctx->printDisassembly(vctx, "f");
	}
}

static void disassembly_i32_dbg_T1(void *vctx, uint8_t option) {
}

static void disassembly_i32_dmb_T1(void *vctx, uint8_t option) {
}

static void disassembly_i32_dsb_T1(void *vctx, uint8_t option) {
}

static void disassembly_i32_eor_imm_T1(void *vctx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	if ((Rd == REG_PC) && S) {
		printOpcode(ctx, "teq", 0);
		print_ri(vctx, Rn, imm);
	} else {
		printOpcode(ctx, "eor", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
		print_rri(vctx, Rd, Rn, imm);
	}
}

static void disassembly_i16_eor_reg_T1(void *vctx, uint8_t Rdn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "eor", OPCODE_SETFLAGS_IF_UNCOND);
	print_rr(ctx, Rdn, Rm);
}

static void disassembly_i32_eor_reg_T2(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	if ((Rd == REG_PC) && S) {
		printOpcode(ctx, "teq", 0);
		print_rr(ctx, Rn, Rm);
	} else {
		printOpcode(ctx, "eor", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
		print_rrrs(vctx, Rd, Rn, Rm, type, imm);
	}
}

static void disassembly_i32_isb_T1(void *vctx, uint8_t option) {
}

static void disassembly_i16_it_T1(void *vctx, uint8_t firstcond, uint8_t mask) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;

	int len = 3;
	for (int i = 0; i < 3; i++) {
		if (mask & (1 << i)) {
			break;
		} else {
			len--;
		}
	}

	char opcode[8];
	strcpy(opcode, "it");
	uint8_t match = firstcond & 1;
	for (int i = 0; i < len; i++) {
		bool then = ((mask >> (3 - i)) & 1) == match;
		strcat(opcode, then ? "t" : "e");
	}
	printOpcode(ctx, opcode, 0);
	ctx->printDisassembly(vctx, " %s", ccString(firstcond));

}

static void disassembly_i32_ldc2_imm_T2(void *vctx, uint8_t Rn, uint8_t imm, uint8_t coproc, uint8_t CRd, bool D, bool P, bool U, bool W) {
}

static void disassembly_i32_ldc2_lit_T2(void *vctx, uint8_t imm, uint8_t coproc, uint8_t CRd, bool D, bool P, bool U, bool W) {
}

static void disassembly_i32_ldc_imm_T1(void *vctx, uint8_t Rn, uint8_t imm, uint8_t coproc, uint8_t CRd, bool D, bool P, bool U, bool W) {
}

static void disassembly_i32_ldc_lit_T1(void *vctx, uint8_t imm, uint8_t coproc, uint8_t CRd, bool D, bool P, bool U, bool W) {
}

static void disassembly_i16_ldm_T1(void *vctx, uint8_t Rn, uint8_t register_list) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldmia", 0);
	ctx->printDisassembly(vctx, " %s%s,", regString(Rn), register_list & (1 << Rn) ? "" : "!");
	print_reglist(vctx, register_list);
}

static void disassembly_i32_ldm_T2(void *vctx, uint8_t Rn, uint16_t register_list, bool P, bool M, bool W) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldmia", OPCODE_WIDE);
	ctx->printDisassembly(vctx, " %s%s,", regString(Rn), W ? "!" : "");
	if (P) {
		register_list |= (1 << REG_PC);
	}
	if (M) {
		register_list |= (1 << REG_LR);
	}
	print_reglist(vctx, register_list);
}

static void disassembly_i32_ldmdb_T1(void *vctx, uint8_t Rn, uint16_t register_list, bool P, bool M, bool W) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldmdb", 0);
	ctx->printDisassembly(vctx, " %s,", regString(Rn));
	if (P) {
		register_list |= (1 << REG_PC);
	}
	if (M) {
		register_list |= (1 << REG_LR);
	}
	print_reglist(vctx, register_list);
}

static void disassembly_i16_ldr_imm_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldr", 0);
	print_rIro(vctx, Rt, Rn, imm * 4);
}

static void disassembly_i16_ldr_imm_T2(void *vctx, uint8_t Rt, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldr", 0);
	print_rIro(vctx, Rt, REG_SP, imm * 4);
}

static void disassembly_i32_ldr_imm_T3(void *vctx, uint8_t Rt, uint8_t Rn, uint16_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldr", OPCODE_WIDE);
	print_rIrO(vctx, Rt, Rn, imm);
}

static void disassembly_i32_ldr_imm_T4(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm, bool P, bool U, bool W) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldr", OPCODE_WIDE);
	if (P) {
		print_rIrO(vctx, Rt, Rn, imm * (U ? 1 : -1));
		if (W) {
			ctx->printDisassembly(vctx, "!");
		}
	} else {
		print_rIri(vctx, Rt, Rn, imm * (U ? 1 : -1));
	}
}

static void disassembly_i16_ldr_lit_T1(void *vctx, uint8_t Rt, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldr", 0);
	print_rIro(vctx, Rt, REG_PC, imm * 4);
}

static void disassembly_i32_ldr_lit_T2(void *vctx, uint8_t Rt, uint16_t imm, bool U) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldr", OPCODE_WIDE);
	print_rIro(vctx, Rt, REG_PC, imm * (U ? 1 : -1));
}

static void disassembly_i16_ldr_reg_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldr", 0);
	print_rIrr(vctx, Rt, Rn, Rm);
}

static void disassembly_i32_ldr_reg_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldr", OPCODE_WIDE);
	print_rIrrs(vctx, Rt, Rn, Rm, TYPE_LSL, imm);
}

static void disassembly_i16_ldrb_imm_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldrb", 0);
	print_rIro(vctx, Rt, Rn, imm);
}

static void disassembly_i32_ldrb_imm_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint16_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldrb", OPCODE_WIDE);
	print_rIrO(vctx, Rt, Rn, imm);
}

static void disassembly_i32_ldrb_imm_T3(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm, bool P, bool U, bool W) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldrb", OPCODE_WIDE);
	if (P) {
		print_rIrO(vctx, Rt, Rn, imm * (U ? 1 : -1));
		if (W) {
			ctx->printDisassembly(vctx, "!");
		}
	} else {
		print_rIri(vctx, Rt, Rn, imm * (U ? 1 : -1));
	}
}

static void disassembly_i32_ldrb_lit_T1(void *vctx, uint8_t Rt, uint16_t imm, bool U) {
}

static void disassembly_i16_ldrb_reg_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldrb", 0);
	print_rIrr(vctx, Rt, Rn, Rm);
}

static void disassembly_i32_ldrb_reg_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldrb", OPCODE_WIDE);
	if (imm == 0) {
		print_rIrr(vctx, Rt, Rn, Rm);
	} else {
		print_rIrrs(vctx, Rt, Rn, Rm, TYPE_LSL, imm);
	}
}

static void disassembly_i32_ldrbt_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
}

static void disassembly_i32_ldrd_imm_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rtx, uint8_t imm, bool P, bool U, bool W) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldrd", 0);
	if (P) {
		print_rrIrO(vctx, Rt, Rtx, Rn, imm * (U ? 4 : -4));
	} else {
		print_rrIri(vctx, Rt, Rtx, Rn, imm * (U ? 4 : -4));
	}
}

static void disassembly_i32_ldrd_lit_T1(void *vctx, uint8_t Rt, uint8_t Rtx, uint8_t imm, bool P, bool U, bool W) {
}

static void disassembly_i32_ldrex_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
}

static void disassembly_i32_ldrexb_T1(void *vctx, uint8_t Rt, uint8_t Rn) {
}

static void disassembly_i32_ldrexh_T1(void *vctx, uint8_t Rt, uint8_t Rn) {
}

static void disassembly_i16_ldrh_imm_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldrh", 0);
	print_rIro(vctx, Rt, Rn, imm * 2);
}

static void disassembly_i32_ldrh_imm_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint16_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldrh", OPCODE_WIDE);
	print_rIrO(vctx, Rt, Rn, imm);
}

static void disassembly_i32_ldrh_imm_T3(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm, bool P, bool U, bool W) {
}

static void disassembly_i32_ldrh_lit_T1(void *vctx, uint8_t Rt, uint16_t imm, bool U) {
}

static void disassembly_i16_ldrh_reg_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldrh", 0);
	print_rIrr(vctx, Rt, Rn, Rm);
}

static void disassembly_i32_ldrh_reg_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm, uint8_t imm) {
}

static void disassembly_i32_ldrht_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
}

static void disassembly_i32_ldrsb_imm_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint16_t imm) {
}

static void disassembly_i32_ldrsb_imm_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm, bool P, bool U, bool W) {
}

static void disassembly_i32_ldrsb_lit_T1(void *vctx, uint8_t Rt, uint16_t imm, bool U) {
}

static void disassembly_i16_ldrsb_reg_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldrsb", 0);
	print_rIrr(vctx, Rt, Rn, Rm);
}

static void disassembly_i32_ldrsb_reg_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm, uint8_t imm) {
}

static void disassembly_i32_ldrsbt_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
}

static void disassembly_i32_ldrsh_imm_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint16_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldrsh", OPCODE_WIDE);
	print_rIrO(vctx, Rt, Rn, imm);
}

static void disassembly_i32_ldrsh_imm_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm, bool P, bool U, bool W) {
}

static void disassembly_i32_ldrsh_lit_T1(void *vctx, uint8_t Rt, uint16_t imm, bool U) {
}

static void disassembly_i16_ldrsh_reg_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldrsh", 0);
	print_rIrr(vctx, Rt, Rn, Rm);
}

static void disassembly_i32_ldrsh_reg_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm, uint8_t imm) {
}

static void disassembly_i32_ldrsh_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
}

static void disassembly_i32_ldrt_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
}

static void disassembly_i16_lsl_imm_T1(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	if (imm != 0) {
		printOpcode(ctx, "lsl", OPCODE_SETFLAGS_IF_UNCOND);
		print_rri(vctx, Rd, Rm, imm);
	} else {
		printOpcode(ctx, "mov", OPCODE_SETFLAGS_IF_UNCOND);
		print_rr(ctx, Rd, Rm);
	}
}

static void disassembly_i32_lsl_imm_T2(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t imm, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "mov", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
	print_rrs(vctx, Rd, Rm, TYPE_LSL, imm);
}

static void disassembly_i16_lsl_reg_T1(void *vctx, uint8_t Rdn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "lsl", OPCODE_SETFLAGS_IF_UNCOND);
	print_rr(ctx, Rdn, Rm);
}

static void disassembly_i32_lsl_reg_T2(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "lsl", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
	print_rrr(vctx, Rd, Rn, Rm);
}

static void disassembly_i16_lsr_imm_T1(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "lsr", OPCODE_SETFLAGS_IF_UNCOND);
	if (imm == 0) {
		imm = 32;
	}
	print_rri(vctx, Rd, Rm, imm);
}

static void disassembly_i32_lsr_imm_T2(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t imm, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "mov", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
	print_rrs(vctx, Rd, Rm, TYPE_LSR, imm);
}

static void disassembly_i16_lsr_reg_T1(void *vctx, uint8_t Rdn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "lsr", OPCODE_SETFLAGS_IF_UNCOND);
	print_rr(ctx, Rdn, Rm);
}

static void disassembly_i32_lsr_reg_T2(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "lsr", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
	print_rrr(vctx, Rd, Rn, Rm);
}

static void disassembly_i32_mcr2_T2(void *vctx, uint8_t Rt, uint8_t coproc, uint8_t CRn, uint8_t CRm, uint8_t opcA, uint8_t opcB) {
}

static void disassembly_i32_mcr_T1(void *vctx, uint8_t Rt, uint8_t coproc, uint8_t CRn, uint8_t CRm, uint8_t opcA, uint8_t opcB) {
}

static void disassembly_i32_mcrr2_T2(void *vctx, uint8_t Rt, uint8_t coproc, uint8_t CRm, uint8_t opcA, uint8_t RtB) {
}

static void disassembly_i32_mcrr_T1(void *vctx, uint8_t Rt, uint8_t coproc, uint8_t CRm, uint8_t opcA, uint8_t RtB) {
}

static void disassembly_i32_mla_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t Ra) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "mla", 0);
	print_rrrr(vctx, Rd, Rn, Rm, Ra);
}

static void disassembly_i32_mls_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t Ra) {
}

static void disassembly_i16_mov_imm_T1(void *vctx, uint8_t Rd, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "mov", OPCODE_SETFLAGS_IF_UNCOND);
	print_ri(vctx, Rd, imm);
}

static void disassembly_i32_mov_imm_T2(void *vctx, uint8_t Rd, int32_t imm, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "mov", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
	print_ri(vctx, Rd, imm);
}

static void disassembly_i32_mov_imm_T3(void *vctx, uint8_t Rd, uint16_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "movw", 0);
	print_ri(vctx, Rd, imm);
}

static void disassembly_i16_mov_reg_T1(void *vctx, uint8_t Rd, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "mov", 0);
	print_rr(ctx, Rd, Rm);
}

static void disassembly_i16_mov_reg_T2(void *vctx, uint8_t Rd, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "mov", OPCODE_SETFLAGS_IF_UNCOND);
	print_rr(ctx, Rd, Rm);
}

static void disassembly_i32_mov_reg_T3(void *vctx, uint8_t Rd, uint8_t Rm, bool S) {
}

static void disassembly_i32_movt_T1(void *vctx, uint8_t Rd, uint16_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "movt", 0);
	print_ri(vctx, Rd, imm);
}

static void disassembly_i32_mrc2_T2(void *vctx, uint8_t Rt, uint8_t coproc, uint8_t Crn, uint8_t CRm, uint8_t opcA, uint8_t opcB) {
}

static void disassembly_i32_mrc_T1(void *vctx, uint8_t Rt, uint8_t coproc, uint8_t Crn, uint8_t CRm, uint8_t opcA, uint8_t opcB) {
}

static void disassembly_i32_mrrc2_T2(void *vctx, uint8_t Rt, uint8_t coproc, uint8_t CRm, uint8_t opcA, uint8_t RtB) {
}

static void disassembly_i32_mrrc_T1(void *vctx, uint8_t Rt, uint8_t coproc, uint8_t CRm, uint8_t opcA, uint8_t RtB) {
}

static void disassembly_i32_mrs_T1(void *vctx, uint8_t Rd, uint8_t SYSm) {
}

static void disassembly_i32_msr_T1(void *vctx, uint8_t Rn, uint8_t mask, uint8_t SYSm) {
}

static void disassembly_i16_mul_T1(void *vctx, uint8_t Rdm, uint8_t Rn) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "mul", OPCODE_SETFLAGS_IF_UNCOND);
	print_rr(ctx, Rdm, Rn);
}

static void disassembly_i32_mul_T2(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "mul", OPCODE_WIDE);
	print_rrr(vctx, Rd, Rn, Rm);
}

static void disassembly_i32_mvn_imm_T1(void *vctx, uint8_t Rd, int32_t imm, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "mvn", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
	print_ri(vctx, Rd, imm);
}

static void disassembly_i16_mvn_reg_T1(void *vctx, uint8_t Rd, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "mvn", OPCODE_SETFLAGS_IF_UNCOND);
	print_rr(ctx, Rd, Rm);
}

static void disassembly_i32_mvn_reg_T2(void *vctx, uint8_t Rd, uint8_t Rm, int32_t imm, uint8_t type, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "mvn", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
	print_rrs(vctx, Rd, Rm, type, imm);
}

static void disassembly_i16_nop_T1(void *vctx) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "nop", 0);
}

static void disassembly_i32_nop_T2(void *vctx) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "nop", OPCODE_WIDE);
}

static void disassembly_i32_orn_imm_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint16_t imm, bool S) {
}

static void disassembly_i32_orn_reg_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S) {
}

static void disassembly_i32_orr_imm_T1(void *vctx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "orr", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
	print_rri(vctx, Rd, Rn, imm);
}

static void disassembly_i16_orr_reg_T1(void *vctx, uint8_t Rdn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "orr", OPCODE_SETFLAGS_IF_UNCOND);
	print_rr(ctx, Rdn, Rm);
}

static void disassembly_i32_orr_reg_T2(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	if (Rn != 0x0f) {
		printOpcode(ctx, "orr", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
		print_rrrs(vctx, Rd, Rn, Rm, type, imm);
	} else {
		printOpcode(ctx, "mov", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
		print_rrs(vctx, Rd, Rm, type, imm);
	}
}

static void disassembly_i32_pkhbt_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, bool tb, bool S, bool T) {
}

static void disassembly_i16_pop_T1(void *vctx, uint8_t register_list, bool P) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "pop", 0);
	uint16_t regList = register_list;
	if (P) {
		regList |= (1 << REG_PC);
	}
	print_reglist(vctx, regList);
}

static void disassembly_i32_pop_T2(void *vctx, uint16_t register_list, bool P, bool M) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldmia", OPCODE_WIDE);
	if (P) {
		register_list |= (1 << REG_PC);
	}
	if (M) {
		register_list |= (1 << REG_LR);
	}
	print_r(ctx, REG_SP);
	ctx->printDisassembly(ctx, "!, ");
	print_reglist(ctx, register_list);
}

static void disassembly_i32_pop_T3(void *vctx, uint8_t Rt) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ldr", OPCODE_WIDE);
	print_rIri(ctx, Rt, REG_SP, 4);
}

static void disassembly_i16_push_T1(void *vctx, uint8_t register_list, bool M) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "push", 0);
	uint16_t regList = register_list;
	if (M) {
		regList |= (1 << REG_LR);
	}
	print_reglist(vctx, regList);
}

static void disassembly_i32_push_T2(void *vctx, uint16_t register_list, bool M) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "push", 0);
	uint16_t regList = register_list;
	if (M) {
		regList |= (1 << REG_LR);
	}
	print_reglist(vctx, regList);
}

static void disassembly_i32_push_T3(void *vctx, uint8_t Rt) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "str", OPCODE_WIDE);
	print_rIro(vctx, Rt, REG_SP, -4);
	ctx->printDisassembly(vctx, "!");
}

static void disassembly_i32_rbit_T1(void *vctx, uint8_t Rd, uint8_t Rm) {
}

static void disassembly_i16_rev16_T1(void *vctx, uint8_t Rd, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "rev16", 0);
	print_rr(ctx, Rd, Rm);
}

static void disassembly_i32_rev16_T2(void *vctx, uint8_t Rd, uint8_t Rm) {
}

static void disassembly_i16_rev_T1(void *vctx, uint8_t Rd, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "rev", 0);
	print_rr(ctx, Rd, Rm);
}

static void disassembly_i32_rev_T2(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t Rmx) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "rev", OPCODE_WIDE);
	print_rr(ctx, Rd, Rm);
}

static void disassembly_i16_revsh_T1(void *vctx, uint8_t Rd, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "revsh", 0);
	print_rr(ctx, Rd, Rm);
}

static void disassembly_i32_revsh_T2(void *vctx, uint8_t Rd, uint8_t Rm) {
}

static void disassembly_i32_ror_imm_T1(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t imm, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "mov", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
	print_rr(ctx, Rd, Rm);
	print_s(ctx, TYPE_ROR, imm);
}

static void disassembly_i16_ror_reg_T1(void *vctx, uint8_t Rdn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ror", OPCODE_SETFLAGS_IF_UNCOND);
	print_rr(ctx, Rdn, Rm);
}

static void disassembly_i32_ror_reg_T2(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, bool S) {
}

static void disassembly_i32_rrx_T1(void *vctx, uint8_t Rd, uint8_t Rm, bool S) {
}

static void disassembly_i16_rsb_imm_T1(void *vctx, uint8_t Rd, uint8_t Rn) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "neg", OPCODE_SETFLAGS_IF_UNCOND);
	print_rr(ctx, Rd, Rn);
}

static void disassembly_i32_rsb_imm_T2(void *vctx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "rsb", S ? OPCODE_SETFLAGS : 0);
	print_rri(vctx, Rd, Rn, imm);
}

static void disassembly_i32_rsb_reg_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "rsb", S ? OPCODE_SETFLAGS : 0);
	print_rrrs(vctx, Rd, Rn, Rm, type, imm);
}

static void disassembly_i32_sbc_imm_T1(void *vctx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "sbc", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
	print_rri(vctx, Rd, Rn, imm);
}

static void disassembly_i16_sbc_reg_T1(void *vctx, uint8_t Rdn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "sbc", OPCODE_SETFLAGS_IF_UNCOND);
	print_rr(ctx, Rdn, Rm);
}

static void disassembly_i32_sbc_reg_T2(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "sbc", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
	print_rrrs(vctx, Rd, Rn, Rm, type, imm);
}

static void disassembly_i32_sdiv_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "sdiv", 0);
	print_rrr(vctx, Rd, Rn, Rm);
}

static void disassembly_i16_sev_T1(void *vctx) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "sev", 0);
}

static void disassembly_i32_sev_T2(void *vctx) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "sev", 0);
}

static void disassembly_i32_smull_T1(void *vctx, uint8_t Rn, uint8_t Rm, uint8_t RdLo, uint8_t RdHi) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "smull", 0);
	print_rrrr(vctx, RdLo, RdHi, Rn, Rm);
}

static void disassembly_i32_stc2_T2(void *vctx, uint8_t Rn, uint8_t imm, uint8_t coproc, uint8_t CRd, bool P, bool N, bool U, bool W) {
}

static void disassembly_i32_stc_T1(void *vctx, uint8_t Rn, uint8_t imm, uint8_t coproc, uint8_t CRd, bool P, bool N, bool U, bool W) {
}

static void disassembly_i16_stm_T1(void *vctx, uint8_t Rn, uint8_t register_list) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "stmia", 0);
	ctx->printDisassembly(vctx, " %s!,", regString(Rn));
	print_reglist(vctx, register_list);
}

static void disassembly_i32_stm_T2(void *vctx, uint8_t Rn, uint16_t register_list, bool M, bool W) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	if (M) {
		register_list |= 1 << REG_LR;
	}

	printOpcode(ctx, "stmia", OPCODE_WIDE);
	ctx->printDisassembly(vctx, " %s%s,", regString(Rn), W ? "!" : "");
	print_reglist(vctx, register_list);
}

static void disassembly_i32_stmdb_T1(void *vctx, uint8_t Rn, uint16_t register_list, bool M, bool W) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "stmdb", 0);
	ctx->printDisassembly(vctx, " %s!,", regString(Rn));
	if (M) {
		register_list |= (1 << REG_LR);
	}
	print_reglist(vctx, register_list);
}

static void disassembly_i16_str_imm_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "str", 0);
	print_rIro(vctx, Rt, Rn, imm * 4);
}

static void disassembly_i16_str_imm_T2(void *vctx, uint8_t Rt, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "str", 0);
	print_rIro(vctx, Rt, REG_SP, imm * 4);
}

static void disassembly_i32_str_imm_T3(void *vctx, uint8_t Rt, uint8_t Rn, uint16_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "str", OPCODE_WIDE);
	print_rIrO(vctx, Rt, Rn, imm);
}

static void disassembly_i32_str_imm_T4(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm, bool P, bool U, bool W) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "str", OPCODE_WIDE);
	if (P) {
		/* Index */
		ctx->printDisassembly(vctx, " %s, [%s, #%s%d]%s", regString(Rt), regString(Rn), U ? "" : "-", imm, W ? "!" : "");
	} else {
		ctx->printDisassembly(vctx, " %s, [%s], #%s%d", regString(Rt), regString(Rn), U ? "" : "-", imm);
	}
}

static void disassembly_i16_str_reg_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "str", 0);
	print_rIrr(vctx, Rt, Rn, Rm);
}

static void disassembly_i32_str_reg_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "str", OPCODE_WIDE);
	ctx->printDisassembly(vctx, " %s, [%s, %s, lsl #%d]", regString(Rt), regString(Rn), regString(Rm), imm);
}

static void disassembly_i16_strb_imm_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "strb", 0);
	print_rIro(vctx, Rt, Rn, imm);
}

static void disassembly_i32_strb_imm_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint16_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "strb", OPCODE_WIDE);
	print_rIrO(vctx, Rt, Rn, imm);
}

static void disassembly_i32_strb_imm_T3(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm, bool P, bool U, bool W) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "strb", OPCODE_WIDE);
	if (P) {
		/* Index */
		ctx->printDisassembly(vctx, " %s, [%s, #%s%d]%s", regString(Rt), regString(Rn), U ? "" : "-", imm, W ? "!" : "");
	} else {
		ctx->printDisassembly(vctx, " %s, [%s], #%s%d", regString(Rt), regString(Rn), U ? "" : "-", imm);
	}
}

static void disassembly_i16_strb_reg_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "strb", 0);
	print_rIrr(vctx, Rt, Rn, Rm);
}

static void disassembly_i32_strb_reg_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "strb", OPCODE_WIDE);
	if (imm == 0) {
		print_rIrr(vctx, Rt, Rn, Rm);
	} else {
		print_rIrrs(vctx, Rt, Rn, Rm, TYPE_LSL, imm);
	}
}

static void disassembly_i32_strd_imm_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rtx, uint8_t imm, bool P, bool U, bool W) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "strd", 0);
	print_rrIrO(vctx, Rt, Rtx, Rn, imm * 4);
}

static void disassembly_i16_strh_imm_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "strh", 0);
	print_rIro(vctx, Rt, Rn, imm * 2);
}

static void disassembly_i32_strh_imm_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint16_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "strh", OPCODE_WIDE);
	print_rIro(vctx, Rt, Rn, imm);
}

static void disassembly_i32_strh_imm_T3(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm, bool P, bool U, bool W) {
}

static void disassembly_i16_strh_reg_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "strh", 0);
	print_rIrr(vctx, Rt, Rn, Rm);
}

static void disassembly_i32_strh_reg_T2(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t Rm, uint8_t imm) {
}

static void disassembly_i32_strht_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
}

static void disassembly_i32_strt_T1(void *vctx, uint8_t Rt, uint8_t Rn, uint8_t imm) {
}

static void disassembly_i16_sub_SPimm_T1(void *vctx, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "sub", 0);
	print_ri(vctx, REG_SP, imm * 4);
}

static void disassembly_i32_sub_SPimm_T2(void *vctx, uint8_t Rd, uint16_t imm, bool S) {
}

static void disassembly_i32_sub_SPimm_T3(void *vctx, uint8_t Rd, uint16_t imm) {
}

static void disassembly_i32_sub_SPreg_T1(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t imm, uint8_t type, bool S) {
}

static void disassembly_i16_sub_imm_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "sub", OPCODE_SETFLAGS_IF_UNCOND);
	print_rri(vctx, Rd, Rn, imm);
}

static void disassembly_i16_sub_imm_T2(void *vctx, uint8_t Rdn, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "sub", OPCODE_SETFLAGS_IF_UNCOND);
	print_ri(vctx, Rdn, imm);
}

static void disassembly_i32_sub_imm_T3(void *vctx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "sub", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
	print_rri(vctx, Rd, Rn, imm);
}

static void disassembly_i32_sub_imm_T4(void *vctx, uint8_t Rd, uint8_t Rn, uint16_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "subw", 0);
	print_rri(vctx, Rd, Rn, imm);
}

static void disassembly_i16_sub_reg_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "sub", OPCODE_SETFLAGS_IF_UNCOND);
	print_rrr(vctx, Rd, Rn, Rm);
}

static void disassembly_i32_sub_reg_T2(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "sub", OPCODE_WIDE | (S ? OPCODE_SETFLAGS : 0));
	print_rrrs(vctx, Rd, Rn, Rm, type, imm);
}

static void disassembly_i16_svc_T1(void *vctx, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "svc", 0);
	ctx->printDisassembly(vctx, " %d", imm);
}

static void disassembly_i32_sxtb16_T1(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t rotate) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "sxtb", 0);
	print_rr(ctx, Rd, Rm);
}

static void disassembly_i16_sxtb_T1(void *vctx, uint8_t Rd, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "sxtb", 0);
	print_rr(ctx, Rd, Rm);
}

static void disassembly_i32_sxtb_T2(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t rotate) {
}

static void disassembly_i16_sxth_T1(void *vctx, uint8_t Rd, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "sxth", 0);
	print_rr(ctx, Rd, Rm);
}

static void disassembly_i32_sxth_T2(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t rotate) {
}

static void disassembly_i32_tbb_T1(void *vctx, uint8_t Rn, uint8_t Rm, bool H) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	if (H) {
		printOpcode(ctx, "tbh", 0);
		print_Irrs(vctx, Rn, Rm, TYPE_LSL, 1);
	} else {
		printOpcode(ctx, "tbb", 0);
		print_Irr(vctx, Rn, Rm);
	}
}

static void disassembly_i32_tst_imm_T1(void *vctx, uint8_t Rn, int32_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "tst", OPCODE_WIDE);
	print_ri(ctx, Rn, imm);
}

static void disassembly_i16_tst_reg_T1(void *vctx, uint8_t Rn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "tst", 0);
	print_rr(ctx, Rn, Rm);
}

static void disassembly_i32_tst_reg_T2(void *vctx, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type) {
}

static void disassembly_i32_ubfx_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t imm, uint8_t width) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "ubfx", 0);
	print_rrii(vctx, Rd, Rn, imm, width + 1);
}

static void disassembly_i16_udf_T1(void *vctx, uint8_t imm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "udf", 0);
	print_i(vctx, imm);
}

static void disassembly_i32_udf_T2(void *vctx, uint16_t imm) {
}

static void disassembly_i32_udiv_T1(void *vctx, uint8_t Rd, uint8_t Rn, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "udiv", 0);
	print_rrr(vctx, Rd, Rn, Rm);
}

static void disassembly_i32_umlal_T1(void *vctx, uint8_t Rn, uint8_t Rm, uint8_t RdLo, uint8_t RdHi) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "umlal", 0);
	print_rrrr(vctx, RdLo, RdHi, Rn, Rm);
}

static void disassembly_i32_umull_T1(void *vctx, uint8_t Rn, uint8_t Rm, uint8_t RdLo, uint8_t RdHi) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "umull", 0);
	print_rrrr(vctx, RdLo, RdHi, Rn, Rm);
}

static void disassembly_i32_uxtb16_T1(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t rotate) {
}

static void disassembly_i16_uxtb_T1(void *vctx, uint8_t Rd, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "uxtb", 0);
	print_rr(ctx, Rd, Rm);
}

static void disassembly_i32_uxtb_T2(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t rotate) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	if (rotate == 0) {
		printOpcode(ctx, "uxtb", OPCODE_WIDE);
		print_rr(ctx, Rd, Rm);
	} else {
		printOpcode(ctx, "uxtb", OPCODE_WIDE);
		print_rrs(vctx, Rd, Rm, TYPE_ROR, rotate * 8);
	}
}

static void disassembly_i16_uxth_T1(void *vctx, uint8_t Rd, uint8_t Rm) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "uxth", 0);
	print_rr(ctx, Rd, Rm);
}

static void disassembly_i32_uxth_T2(void *vctx, uint8_t Rd, uint8_t Rm, uint8_t rotate) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	if (rotate == 0) {
		printOpcode(ctx, "uxth", OPCODE_WIDE);
		print_rr(ctx, Rd, Rm);
	} else {
		printOpcode(ctx, "uxth", OPCODE_WIDE);
		print_rrs(vctx, Rd, Rm, TYPE_ROR, rotate * 8);
	}
}

static void disassembly_i16_yield_T1(void *vctx) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "yield", 0);
}

static void disassembly_i32_yield_T2(void *vctx) {
	struct disas_ctx_t *ctx = (struct disas_ctx_t*)vctx;
	printOpcode(ctx, "yield", OPCODE_WIDE);
}

const struct decoding_handler_t disassemblyCallbacks = {
	.i32_adc_imm_T1 = disassembly_i32_adc_imm_T1,
	.i16_adc_reg_T1 = disassembly_i16_adc_reg_T1,
	.i32_adc_reg_T2 = disassembly_i32_adc_reg_T2,
	.i16_add_SPi_T1 = disassembly_i16_add_SPi_T1,
	.i16_add_SPi_T2 = disassembly_i16_add_SPi_T2,
	.i32_add_SPi_T3 = disassembly_i32_add_SPi_T3,
	.i32_add_SPi_T4 = disassembly_i32_add_SPi_T4,
	.i16_add_SPr_T1 = disassembly_i16_add_SPr_T1,
	.i16_add_SPr_T2 = disassembly_i16_add_SPr_T2,
	.i32_add_SPr_T3 = disassembly_i32_add_SPr_T3,
	.i16_add_imm_T1 = disassembly_i16_add_imm_T1,
	.i16_add_imm_T2 = disassembly_i16_add_imm_T2,
	.i32_add_imm_T3 = disassembly_i32_add_imm_T3,
	.i32_add_imm_T4 = disassembly_i32_add_imm_T4,
	.i16_add_reg_T1 = disassembly_i16_add_reg_T1,
	.i16_add_reg_T2 = disassembly_i16_add_reg_T2,
	.i32_add_reg_T3 = disassembly_i32_add_reg_T3,
	.i16_adr_T1 = disassembly_i16_adr_T1,
	.i32_adr_T2 = disassembly_i32_adr_T2,
	.i32_adr_T3 = disassembly_i32_adr_T3,
	.i32_and_imm_T1 = disassembly_i32_and_imm_T1,
	.i16_and_reg_T1 = disassembly_i16_and_reg_T1,
	.i32_and_reg_T2 = disassembly_i32_and_reg_T2,
	.i16_asr_imm_T1 = disassembly_i16_asr_imm_T1,
	.i32_asr_imm_T2 = disassembly_i32_asr_imm_T2,
	.i16_asr_reg_T1 = disassembly_i16_asr_reg_T1,
	.i32_asr_reg_T2 = disassembly_i32_asr_reg_T2,
	.i16_b_T1 = disassembly_i16_b_T1,
	.i16_b_T2 = disassembly_i16_b_T2,
	.i32_b_T3 = disassembly_i32_b_T3,
	.i32_b_T4 = disassembly_i32_b_T4,
	.i32_bfc_T1 = disassembly_i32_bfc_T1,
	.i32_bfi_T1 = disassembly_i32_bfi_T1,
	.i32_bic_imm_T1 = disassembly_i32_bic_imm_T1,
	.i16_bic_reg_T1 = disassembly_i16_bic_reg_T1,
	.i32_bic_reg_T2 = disassembly_i32_bic_reg_T2,
	.i16_bkpt_T1 = disassembly_i16_bkpt_T1,
	.i32_bl_T1 = disassembly_i32_bl_T1,
	.i16_blx_reg_T1 = disassembly_i16_blx_reg_T1,
	.i16_bx_T1 = disassembly_i16_bx_T1,
	.i16_cbnz_T1 = disassembly_i16_cbnz_T1,
	.i32_cdp2_T2 = disassembly_i32_cdp2_T2,
	.i32_cdp_T1 = disassembly_i32_cdp_T1,
	.i32_clrex_T1 = disassembly_i32_clrex_T1,
	.i32_clz_T1 = disassembly_i32_clz_T1,
	.i32_cmn_imm_T1 = disassembly_i32_cmn_imm_T1,
	.i16_cmn_reg_T1 = disassembly_i16_cmn_reg_T1,
	.i32_cmn_reg_T2 = disassembly_i32_cmn_reg_T2,
	.i16_cmp_imm_T1 = disassembly_i16_cmp_imm_T1,
	.i32_cmp_imm_T2 = disassembly_i32_cmp_imm_T2,
	.i16_cmp_reg_T1 = disassembly_i16_cmp_reg_T1,
	.i16_cmp_reg_T2 = disassembly_i16_cmp_reg_T2,
	.i32_cmp_reg_T3 = disassembly_i32_cmp_reg_T3,
	.i16_cps_T1 = disassembly_i16_cps_T1,
	.i32_dbg_T1 = disassembly_i32_dbg_T1,
	.i32_dmb_T1 = disassembly_i32_dmb_T1,
	.i32_dsb_T1 = disassembly_i32_dsb_T1,
	.i32_eor_imm_T1 = disassembly_i32_eor_imm_T1,
	.i16_eor_reg_T1 = disassembly_i16_eor_reg_T1,
	.i32_eor_reg_T2 = disassembly_i32_eor_reg_T2,
	.i32_isb_T1 = disassembly_i32_isb_T1,
	.i16_it_T1 = disassembly_i16_it_T1,
	.i32_ldc2_imm_T2 = disassembly_i32_ldc2_imm_T2,
	.i32_ldc2_lit_T2 = disassembly_i32_ldc2_lit_T2,
	.i32_ldc_imm_T1 = disassembly_i32_ldc_imm_T1,
	.i32_ldc_lit_T1 = disassembly_i32_ldc_lit_T1,
	.i16_ldm_T1 = disassembly_i16_ldm_T1,
	.i32_ldm_T2 = disassembly_i32_ldm_T2,
	.i32_ldmdb_T1 = disassembly_i32_ldmdb_T1,
	.i16_ldr_imm_T1 = disassembly_i16_ldr_imm_T1,
	.i16_ldr_imm_T2 = disassembly_i16_ldr_imm_T2,
	.i32_ldr_imm_T3 = disassembly_i32_ldr_imm_T3,
	.i32_ldr_imm_T4 = disassembly_i32_ldr_imm_T4,
	.i16_ldr_lit_T1 = disassembly_i16_ldr_lit_T1,
	.i32_ldr_lit_T2 = disassembly_i32_ldr_lit_T2,
	.i16_ldr_reg_T1 = disassembly_i16_ldr_reg_T1,
	.i32_ldr_reg_T2 = disassembly_i32_ldr_reg_T2,
	.i16_ldrb_imm_T1 = disassembly_i16_ldrb_imm_T1,
	.i32_ldrb_imm_T2 = disassembly_i32_ldrb_imm_T2,
	.i32_ldrb_imm_T3 = disassembly_i32_ldrb_imm_T3,
	.i32_ldrb_lit_T1 = disassembly_i32_ldrb_lit_T1,
	.i16_ldrb_reg_T1 = disassembly_i16_ldrb_reg_T1,
	.i32_ldrb_reg_T2 = disassembly_i32_ldrb_reg_T2,
	.i32_ldrbt_T1 = disassembly_i32_ldrbt_T1,
	.i32_ldrd_imm_T1 = disassembly_i32_ldrd_imm_T1,
	.i32_ldrd_lit_T1 = disassembly_i32_ldrd_lit_T1,
	.i32_ldrex_T1 = disassembly_i32_ldrex_T1,
	.i32_ldrexb_T1 = disassembly_i32_ldrexb_T1,
	.i32_ldrexh_T1 = disassembly_i32_ldrexh_T1,
	.i16_ldrh_imm_T1 = disassembly_i16_ldrh_imm_T1,
	.i32_ldrh_imm_T2 = disassembly_i32_ldrh_imm_T2,
	.i32_ldrh_imm_T3 = disassembly_i32_ldrh_imm_T3,
	.i32_ldrh_lit_T1 = disassembly_i32_ldrh_lit_T1,
	.i16_ldrh_reg_T1 = disassembly_i16_ldrh_reg_T1,
	.i32_ldrh_reg_T2 = disassembly_i32_ldrh_reg_T2,
	.i32_ldrht_T1 = disassembly_i32_ldrht_T1,
	.i32_ldrsb_imm_T1 = disassembly_i32_ldrsb_imm_T1,
	.i32_ldrsb_imm_T2 = disassembly_i32_ldrsb_imm_T2,
	.i32_ldrsb_lit_T1 = disassembly_i32_ldrsb_lit_T1,
	.i16_ldrsb_reg_T1 = disassembly_i16_ldrsb_reg_T1,
	.i32_ldrsb_reg_T2 = disassembly_i32_ldrsb_reg_T2,
	.i32_ldrsbt_T1 = disassembly_i32_ldrsbt_T1,
	.i32_ldrsh_imm_T1 = disassembly_i32_ldrsh_imm_T1,
	.i32_ldrsh_imm_T2 = disassembly_i32_ldrsh_imm_T2,
	.i32_ldrsh_lit_T1 = disassembly_i32_ldrsh_lit_T1,
	.i16_ldrsh_reg_T1 = disassembly_i16_ldrsh_reg_T1,
	.i32_ldrsh_reg_T2 = disassembly_i32_ldrsh_reg_T2,
	.i32_ldrsh_T1 = disassembly_i32_ldrsh_T1,
	.i32_ldrt_T1 = disassembly_i32_ldrt_T1,
	.i16_lsl_imm_T1 = disassembly_i16_lsl_imm_T1,
	.i32_lsl_imm_T2 = disassembly_i32_lsl_imm_T2,
	.i16_lsl_reg_T1 = disassembly_i16_lsl_reg_T1,
	.i32_lsl_reg_T2 = disassembly_i32_lsl_reg_T2,
	.i16_lsr_imm_T1 = disassembly_i16_lsr_imm_T1,
	.i32_lsr_imm_T2 = disassembly_i32_lsr_imm_T2,
	.i16_lsr_reg_T1 = disassembly_i16_lsr_reg_T1,
	.i32_lsr_reg_T2 = disassembly_i32_lsr_reg_T2,
	.i32_mcr2_T2 = disassembly_i32_mcr2_T2,
	.i32_mcr_T1 = disassembly_i32_mcr_T1,
	.i32_mcrr2_T2 = disassembly_i32_mcrr2_T2,
	.i32_mcrr_T1 = disassembly_i32_mcrr_T1,
	.i32_mla_T1 = disassembly_i32_mla_T1,
	.i32_mls_T1 = disassembly_i32_mls_T1,
	.i16_mov_imm_T1 = disassembly_i16_mov_imm_T1,
	.i32_mov_imm_T2 = disassembly_i32_mov_imm_T2,
	.i32_mov_imm_T3 = disassembly_i32_mov_imm_T3,
	.i16_mov_reg_T1 = disassembly_i16_mov_reg_T1,
	.i16_mov_reg_T2 = disassembly_i16_mov_reg_T2,
	.i32_mov_reg_T3 = disassembly_i32_mov_reg_T3,
	.i32_movt_T1 = disassembly_i32_movt_T1,
	.i32_mrc2_T2 = disassembly_i32_mrc2_T2,
	.i32_mrc_T1 = disassembly_i32_mrc_T1,
	.i32_mrrc2_T2 = disassembly_i32_mrrc2_T2,
	.i32_mrrc_T1 = disassembly_i32_mrrc_T1,
	.i32_mrs_T1 = disassembly_i32_mrs_T1,
	.i32_msr_T1 = disassembly_i32_msr_T1,
	.i16_mul_T1 = disassembly_i16_mul_T1,
	.i32_mul_T2 = disassembly_i32_mul_T2,
	.i32_mvn_imm_T1 = disassembly_i32_mvn_imm_T1,
	.i16_mvn_reg_T1 = disassembly_i16_mvn_reg_T1,
	.i32_mvn_reg_T2 = disassembly_i32_mvn_reg_T2,
	.i16_nop_T1 = disassembly_i16_nop_T1,
	.i32_nop_T2 = disassembly_i32_nop_T2,
	.i32_orn_imm_T1 = disassembly_i32_orn_imm_T1,
	.i32_orn_reg_T1 = disassembly_i32_orn_reg_T1,
	.i32_orr_imm_T1 = disassembly_i32_orr_imm_T1,
	.i16_orr_reg_T1 = disassembly_i16_orr_reg_T1,
	.i32_orr_reg_T2 = disassembly_i32_orr_reg_T2,
	.i32_pkhbt_T1 = disassembly_i32_pkhbt_T1,
	.i16_pop_T1 = disassembly_i16_pop_T1,
	.i32_pop_T2 = disassembly_i32_pop_T2,
	.i32_pop_T3 = disassembly_i32_pop_T3,
	.i16_push_T1 = disassembly_i16_push_T1,
	.i32_push_T2 = disassembly_i32_push_T2,
	.i32_push_T3 = disassembly_i32_push_T3,
	.i32_rbit_T1 = disassembly_i32_rbit_T1,
	.i16_rev16_T1 = disassembly_i16_rev16_T1,
	.i32_rev16_T2 = disassembly_i32_rev16_T2,
	.i16_rev_T1 = disassembly_i16_rev_T1,
	.i32_rev_T2 = disassembly_i32_rev_T2,
	.i16_revsh_T1 = disassembly_i16_revsh_T1,
	.i32_revsh_T2 = disassembly_i32_revsh_T2,
	.i32_ror_imm_T1 = disassembly_i32_ror_imm_T1,
	.i16_ror_reg_T1 = disassembly_i16_ror_reg_T1,
	.i32_ror_reg_T2 = disassembly_i32_ror_reg_T2,
	.i32_rrx_T1 = disassembly_i32_rrx_T1,
	.i16_rsb_imm_T1 = disassembly_i16_rsb_imm_T1,
	.i32_rsb_imm_T2 = disassembly_i32_rsb_imm_T2,
	.i32_rsb_reg_T1 = disassembly_i32_rsb_reg_T1,
	.i32_sbc_imm_T1 = disassembly_i32_sbc_imm_T1,
	.i16_sbc_reg_T1 = disassembly_i16_sbc_reg_T1,
	.i32_sbc_reg_T2 = disassembly_i32_sbc_reg_T2,
	.i32_sdiv_T1 = disassembly_i32_sdiv_T1,
	.i16_sev_T1 = disassembly_i16_sev_T1,
	.i32_sev_T2 = disassembly_i32_sev_T2,
	.i32_smull_T1 = disassembly_i32_smull_T1,
	.i32_stc2_T2 = disassembly_i32_stc2_T2,
	.i32_stc_T1 = disassembly_i32_stc_T1,
	.i16_stm_T1 = disassembly_i16_stm_T1,
	.i32_stm_T2 = disassembly_i32_stm_T2,
	.i32_stmdb_T1 = disassembly_i32_stmdb_T1,
	.i16_str_imm_T1 = disassembly_i16_str_imm_T1,
	.i16_str_imm_T2 = disassembly_i16_str_imm_T2,
	.i32_str_imm_T3 = disassembly_i32_str_imm_T3,
	.i32_str_imm_T4 = disassembly_i32_str_imm_T4,
	.i16_str_reg_T1 = disassembly_i16_str_reg_T1,
	.i32_str_reg_T2 = disassembly_i32_str_reg_T2,
	.i16_strb_imm_T1 = disassembly_i16_strb_imm_T1,
	.i32_strb_imm_T2 = disassembly_i32_strb_imm_T2,
	.i32_strb_imm_T3 = disassembly_i32_strb_imm_T3,
	.i16_strb_reg_T1 = disassembly_i16_strb_reg_T1,
	.i32_strb_reg_T2 = disassembly_i32_strb_reg_T2,
	.i32_strd_imm_T1 = disassembly_i32_strd_imm_T1,
	.i16_strh_imm_T1 = disassembly_i16_strh_imm_T1,
	.i32_strh_imm_T2 = disassembly_i32_strh_imm_T2,
	.i32_strh_imm_T3 = disassembly_i32_strh_imm_T3,
	.i16_strh_reg_T1 = disassembly_i16_strh_reg_T1,
	.i32_strh_reg_T2 = disassembly_i32_strh_reg_T2,
	.i32_strht_T1 = disassembly_i32_strht_T1,
	.i32_strt_T1 = disassembly_i32_strt_T1,
	.i16_sub_SPimm_T1 = disassembly_i16_sub_SPimm_T1,
	.i32_sub_SPimm_T2 = disassembly_i32_sub_SPimm_T2,
	.i32_sub_SPimm_T3 = disassembly_i32_sub_SPimm_T3,
	.i32_sub_SPreg_T1 = disassembly_i32_sub_SPreg_T1,
	.i16_sub_imm_T1 = disassembly_i16_sub_imm_T1,
	.i16_sub_imm_T2 = disassembly_i16_sub_imm_T2,
	.i32_sub_imm_T3 = disassembly_i32_sub_imm_T3,
	.i32_sub_imm_T4 = disassembly_i32_sub_imm_T4,
	.i16_sub_reg_T1 = disassembly_i16_sub_reg_T1,
	.i32_sub_reg_T2 = disassembly_i32_sub_reg_T2,
	.i16_svc_T1 = disassembly_i16_svc_T1,
	.i32_sxtb16_T1 = disassembly_i32_sxtb16_T1,
	.i16_sxtb_T1 = disassembly_i16_sxtb_T1,
	.i32_sxtb_T2 = disassembly_i32_sxtb_T2,
	.i16_sxth_T1 = disassembly_i16_sxth_T1,
	.i32_sxth_T2 = disassembly_i32_sxth_T2,
	.i32_tbb_T1 = disassembly_i32_tbb_T1,
	.i32_tst_imm_T1 = disassembly_i32_tst_imm_T1,
	.i16_tst_reg_T1 = disassembly_i16_tst_reg_T1,
	.i32_tst_reg_T2 = disassembly_i32_tst_reg_T2,
	.i32_ubfx_T1 = disassembly_i32_ubfx_T1,
	.i16_udf_T1 = disassembly_i16_udf_T1,
	.i32_udf_T2 = disassembly_i32_udf_T2,
	.i32_udiv_T1 = disassembly_i32_udiv_T1,
	.i32_umlal_T1 = disassembly_i32_umlal_T1,
	.i32_umull_T1 = disassembly_i32_umull_T1,
	.i32_uxtb16_T1 = disassembly_i32_uxtb16_T1,
	.i16_uxtb_T1 = disassembly_i16_uxtb_T1,
	.i32_uxtb_T2 = disassembly_i32_uxtb_T2,
	.i16_uxth_T1 = disassembly_i16_uxth_T1,
	.i32_uxth_T2 = disassembly_i32_uxth_T2,
	.i16_yield_T1 = disassembly_i16_yield_T1,
	.i32_yield_T2 = disassembly_i32_yield_T2,
};

// vim: set filetype=c:
