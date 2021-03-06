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

#include "decoder.h"
#include "config.h"

static uint32_t ror32(uint32_t in_value, uint8_t rotate) {
	uint32_t lo_mask = (1 << rotate) - 1;
	uint32_t hi_mask = ((1 << (32 - rotate)) - 1) << rotate;
	return ((in_value & lo_mask) << (32 - rotate)) | ((in_value & hi_mask) >> rotate);
}

static int32_t thumb_expand_imm(uint16_t imm) {
	if ((imm & 0xc00) == 0x0) {
		if ((imm & 0x300) == 0x0) {
			return imm & 0xff;
		} else if ((imm & 0x300) == 0x100) {
			return ((imm & 0xff) << 16) | (imm & 0xff);
		} else if ((imm & 0x300) == 0x200) {
			return ((imm & 0xff) << 24) | ((imm & 0xff) << 8);
		} else {
			return ((imm & 0xff) << 24) | ((imm & 0xff) << 16) | ((imm & 0xff) << 8) | ((imm & 0xff) << 0);
		}
	} else {
		/* Rotate thingy */
		return ror32(0x80 | (imm & 0x7f), (imm & 0xf80) >> 7);
	}
}

static int32_t thumb_sign_extend(uint32_t imm, uint8_t bit_length) {
	if (imm & (1 << (bit_length - 1))) {
		return imm | ~((1 << bit_length) - 1);
	} else {
		return imm;
	}
}

static int32_t thumb_sign_extend20(uint32_t imm) {
	return thumb_sign_extend(imm, 20);
}

static int32_t thumb_sign_extend24_EOR(uint32_t imm) {
	imm ^= ((1 << 21) ^ ((imm & (1 << 23)) >> 2));
	imm ^= ((1 << 22) ^ ((imm & (1 << 23)) >> 1));
	return thumb_sign_extend(imm, 24);
}

int decode_insn(void *vctx, uint32_t opcode, const struct decoding_handler_t *handler, FILE *insn_debugging_info) {
	int decoded_insn_length = 0;
	if (1 == 0) {
		// Just for easier code generation
	} else if ((opcode & 0xfbe08000) == 0xf1400000) {
		// adc_imm_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_adc_imm_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			int32_t imm = thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 adc_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x [orig 0x%x] ", imm, ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_adc_imm_T1(vctx, Rd, Rn, imm, S);
		}
	} else if ((opcode & 0xffc00000) == 0x41400000) {
		// adc_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_adc_reg_T1) {
			uint8_t Rdn = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 adc_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rdn = 0x%x ", Rdn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_adc_reg_T1(vctx, Rdn, Rm);
		}
	} else if ((opcode & 0xffe08000) == 0xeb400000) {
		// adc_reg_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_adc_reg_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			uint8_t type = ((opcode & 0x30) >> 4);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 adc_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "type = 0x%x ", type);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_adc_reg_T2(vctx, Rd, Rn, Rm, imm, type, S);
		}
	} else if ((opcode & 0xf8000000) == 0xa8000000) {
		// add_SPi_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_add_SPi_T1) {
			uint8_t Rd = ((opcode & 0x7000000) >> 24);
			uint8_t imm = ((opcode & 0xff0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 add_SPi_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_add_SPi_T1(vctx, Rd, imm);
		}
	} else if ((opcode & 0xff800000) == 0xb0000000) {
		// add_SPi_T2
		decoded_insn_length = 2;
		if (handler && handler->i16_add_SPi_T2) {
			uint8_t imm = ((opcode & 0x7f0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 add_SPi_T2 ", opcode);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_add_SPi_T2(vctx, imm);
		}
	} else if ((opcode & 0xfe000000) == 0x1c000000) {
		// add_imm_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_add_imm_T1) {
			uint8_t Rd = ((opcode & 0x70000) >> 16);
			uint8_t Rn = ((opcode & 0x380000) >> 19);
			uint8_t imm = ((opcode & 0x1c00000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 add_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_add_imm_T1(vctx, Rd, Rn, imm);
		}
	} else if ((opcode & 0xf8000000) == 0x30000000) {
		// add_imm_T2
		decoded_insn_length = 2;
		if (handler && handler->i16_add_imm_T2) {
			uint8_t Rdn = ((opcode & 0x7000000) >> 24);
			uint8_t imm = ((opcode & 0xff0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 add_imm_T2 ", opcode);
				fprintf(insn_debugging_info, "Rdn = 0x%x ", Rdn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_add_imm_T2(vctx, Rdn, imm);
		}
	} else if ((opcode & 0xfe000000) == 0x18000000) {
		// add_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_add_reg_T1) {
			uint8_t Rd = ((opcode & 0x70000) >> 16);
			uint8_t Rn = ((opcode & 0x380000) >> 19);
			uint8_t Rm = ((opcode & 0x1c00000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 add_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_add_reg_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xf8000000) == 0xa0000000) {
		// adr_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_adr_T1) {
			uint8_t Rd = ((opcode & 0x7000000) >> 24);
			uint8_t imm = ((opcode & 0xff0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 adr_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_adr_T1(vctx, Rd, imm);
		}
	} else if ((opcode & 0xffc00000) == 0x40000000) {
		// and_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_and_reg_T1) {
			uint8_t Rdn = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 and_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rdn = 0x%x ", Rdn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_and_reg_T1(vctx, Rdn, Rm);
		}
	} else if ((opcode & 0xf8000000) == 0x10000000) {
		// asr_imm_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_asr_imm_T1) {
			uint8_t Rd = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			uint8_t imm = ((opcode & 0x7c00000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 asr_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_asr_imm_T1(vctx, Rd, Rm, imm);
		}
	} else if ((opcode & 0xffc00000) == 0x41000000) {
		// asr_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_asr_reg_T1) {
			uint8_t Rdn = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 asr_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rdn = 0x%x ", Rdn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_asr_reg_T1(vctx, Rdn, Rm);
		}
	} else if ((opcode & 0xffe0f0f0) == 0xfa40f000) {
		// asr_reg_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_asr_reg_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 asr_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_asr_reg_T2(vctx, Rd, Rn, Rm, S);
		}
	} else if ((opcode & 0xf8000000) == 0xe0000000) {
		// b_T2
		decoded_insn_length = 2;
		if (handler && handler->i16_b_T2) {
			uint16_t imm = ((opcode & 0x7ff0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 b_T2 ", opcode);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_b_T2(vctx, imm);
		}
	} else if ((opcode & 0xfbe08000) == 0xf0200000) {
		// bic_imm_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_bic_imm_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			int32_t imm = thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 bic_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x [orig 0x%x] ", imm, ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_bic_imm_T1(vctx, Rd, Rn, imm, S);
		}
	} else if ((opcode & 0xffc00000) == 0x43800000) {
		// bic_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_bic_reg_T1) {
			uint8_t Rdn = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 bic_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rdn = 0x%x ", Rdn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_bic_reg_T1(vctx, Rdn, Rm);
		}
	} else if ((opcode & 0xffe00000) == 0xea200000) {
		// bic_reg_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_bic_reg_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			uint8_t type = ((opcode & 0x30) >> 4);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 bic_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "type = 0x%x ", type);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_bic_reg_T2(vctx, Rd, Rn, Rm, imm, type, S);
		}
	} else if ((opcode & 0xff000000) == 0xbe000000) {
		// bkpt_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_bkpt_T1) {
			uint8_t imm = ((opcode & 0xff0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 bkpt_T1 ", opcode);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_bkpt_T1(vctx, imm);
		}
	} else if ((opcode & 0xff800000) == 0x47800000) {
		// blx_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_blx_reg_T1) {
			uint8_t Rm = ((opcode & 0x780000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 blx_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_blx_reg_T1(vctx, Rm);
		}
	} else if ((opcode & 0xff800000) == 0x47000000) {
		// bx_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_bx_T1) {
			uint8_t Rm = ((opcode & 0x780000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 bx_T1 ", opcode);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_bx_T1(vctx, Rm);
		}
	} else if ((opcode & 0xf5000000) == 0xb1000000) {
		// cbnz_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_cbnz_T1) {
			uint8_t Rn = ((opcode & 0x70000) >> 16);
			uint8_t imm = ((opcode & 0x2000000) >> 20) | ((opcode & 0xf80000) >> 19);
			bool op = ((opcode & 0x8000000) >> 27);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 cbnz_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "op = 0x%x ", op);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_cbnz_T1(vctx, Rn, imm, op);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfab0f080) {
		// clz_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_clz_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rm = (opcode & 0xf);
			uint8_t Rmx = ((opcode & 0xf0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 clz_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "Rmx = 0x%x ", Rmx);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_clz_T1(vctx, Rd, Rm, Rmx);
		}
	} else if ((opcode & 0xffc00000) == 0x42c00000) {
		// cmn_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_cmn_reg_T1) {
			uint8_t Rn = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 cmn_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_cmn_reg_T1(vctx, Rn, Rm);
		}
	} else if ((opcode & 0xf8000000) == 0x28000000) {
		// cmp_imm_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_cmp_imm_T1) {
			uint8_t Rn = ((opcode & 0x7000000) >> 24);
			uint8_t imm = ((opcode & 0xff0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 cmp_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_cmp_imm_T1(vctx, Rn, imm);
		}
	} else if ((opcode & 0xffc00000) == 0x42800000) {
		// cmp_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_cmp_reg_T1) {
			uint8_t Rn = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 cmp_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_cmp_reg_T1(vctx, Rn, Rm);
		}
	} else if ((opcode & 0xff000000) == 0x45000000) {
		// cmp_reg_T2
		decoded_insn_length = 2;
		if (handler && handler->i16_cmp_reg_T2) {
			uint8_t Rn = ((opcode & 0x800000) >> 20) | ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x780000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 cmp_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_cmp_reg_T2(vctx, Rn, Rm);
		}
	} else if ((opcode & 0xffe00000) == 0xb6600000) {
		// cps_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_cps_T1) {
			bool imm = ((opcode & 0x100000) >> 20);
			bool F = ((opcode & 0x10000) >> 16);
			bool I = ((opcode & 0x20000) >> 17);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 cps_T1 ", opcode);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "F = 0x%x ", F);
				fprintf(insn_debugging_info, "I = 0x%x ", I);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_cps_T1(vctx, imm, F, I);
		}
	} else if ((opcode & 0xffc00000) == 0x40400000) {
		// eor_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_eor_reg_T1) {
			uint8_t Rdn = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 eor_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rdn = 0x%x ", Rdn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_eor_reg_T1(vctx, Rdn, Rm);
		}
	} else if ((opcode & 0xf8000000) == 0xc8000000) {
		// ldm_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_ldm_T1) {
			uint8_t Rn = ((opcode & 0x7000000) >> 24);
			uint8_t register_list = ((opcode & 0xff0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 ldm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "register_list = 0x%x ", register_list);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_ldm_T1(vctx, Rn, register_list);
		}
	} else if ((opcode & 0xffd02000) == 0xe9100000) {
		// ldmdb_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ldmdb_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint16_t register_list = (opcode & 0x1fff);
			bool P = ((opcode & 0x8000) >> 15);
			bool M = ((opcode & 0x4000) >> 14);
			bool W = ((opcode & 0x200000) >> 21);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldmdb_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "register_list = 0x%x ", register_list);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldmdb_T1(vctx, Rn, register_list, P, M, W);
		}
	} else if ((opcode & 0xf8000000) == 0x68000000) {
		// ldr_imm_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_ldr_imm_T1) {
			uint8_t Rt = ((opcode & 0x70000) >> 16);
			uint8_t Rn = ((opcode & 0x380000) >> 19);
			uint8_t imm = ((opcode & 0x7c00000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 ldr_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_ldr_imm_T1(vctx, Rt, Rn, imm);
		}
	} else if ((opcode & 0xf8000000) == 0x98000000) {
		// ldr_imm_T2
		decoded_insn_length = 2;
		if (handler && handler->i16_ldr_imm_T2) {
			uint8_t Rt = ((opcode & 0x7000000) >> 24);
			uint8_t imm = ((opcode & 0xff0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 ldr_imm_T2 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_ldr_imm_T2(vctx, Rt, imm);
		}
	} else if ((opcode & 0xf8000000) == 0x48000000) {
		// ldr_lit_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_ldr_lit_T1) {
			uint8_t Rt = ((opcode & 0x7000000) >> 24);
			uint8_t imm = ((opcode & 0xff0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 ldr_lit_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_ldr_lit_T1(vctx, Rt, imm);
		}
	} else if ((opcode & 0xfe000000) == 0x58000000) {
		// ldr_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_ldr_reg_T1) {
			uint8_t Rt = ((opcode & 0x70000) >> 16);
			uint8_t Rn = ((opcode & 0x380000) >> 19);
			uint8_t Rm = ((opcode & 0x1c00000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 ldr_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_ldr_reg_T1(vctx, Rt, Rn, Rm);
		}
	} else if ((opcode & 0xf8000000) == 0x78000000) {
		// ldrb_imm_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_ldrb_imm_T1) {
			uint8_t Rt = ((opcode & 0x70000) >> 16);
			uint8_t Rn = ((opcode & 0x380000) >> 19);
			uint8_t imm = ((opcode & 0x7c00000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 ldrb_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_ldrb_imm_T1(vctx, Rt, Rn, imm);
		}
	} else if ((opcode & 0xfe000000) == 0x5c000000) {
		// ldrb_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_ldrb_reg_T1) {
			uint8_t Rt = ((opcode & 0x70000) >> 16);
			uint8_t Rn = ((opcode & 0x380000) >> 19);
			uint8_t Rm = ((opcode & 0x1c00000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 ldrb_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_ldrb_reg_T1(vctx, Rt, Rn, Rm);
		}
	} else if ((opcode & 0xf8000000) == 0x88000000) {
		// ldrh_imm_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_ldrh_imm_T1) {
			uint8_t Rt = ((opcode & 0x70000) >> 16);
			uint8_t Rn = ((opcode & 0x380000) >> 19);
			uint8_t imm = ((opcode & 0x7c00000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 ldrh_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_ldrh_imm_T1(vctx, Rt, Rn, imm);
		}
	} else if ((opcode & 0xfe000000) == 0x5a000000) {
		// ldrh_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_ldrh_reg_T1) {
			uint8_t Rt = ((opcode & 0x70000) >> 16);
			uint8_t Rn = ((opcode & 0x380000) >> 19);
			uint8_t Rm = ((opcode & 0x1c00000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 ldrh_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_ldrh_reg_T1(vctx, Rt, Rn, Rm);
		}
	} else if ((opcode & 0xfe000000) == 0x56000000) {
		// ldrsb_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_ldrsb_reg_T1) {
			uint8_t Rt = ((opcode & 0x70000) >> 16);
			uint8_t Rn = ((opcode & 0x380000) >> 19);
			uint8_t Rm = ((opcode & 0x1c00000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 ldrsb_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_ldrsb_reg_T1(vctx, Rt, Rn, Rm);
		}
	} else if ((opcode & 0xfe000000) == 0x5e000000) {
		// ldrsh_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_ldrsh_reg_T1) {
			uint8_t Rt = ((opcode & 0x70000) >> 16);
			uint8_t Rn = ((opcode & 0x380000) >> 19);
			uint8_t Rm = ((opcode & 0x1c00000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 ldrsh_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_ldrsh_reg_T1(vctx, Rt, Rn, Rm);
		}
	} else if ((opcode & 0xffc00000) == 0x40800000) {
		// lsl_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_lsl_reg_T1) {
			uint8_t Rdn = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 lsl_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rdn = 0x%x ", Rdn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_lsl_reg_T1(vctx, Rdn, Rm);
		}
	} else if ((opcode & 0xffe0f0f0) == 0xfa00f000) {
		// lsl_reg_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_lsl_reg_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 lsl_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_lsl_reg_T2(vctx, Rd, Rn, Rm, S);
		}
	} else if ((opcode & 0xf8000000) == 0x8000000) {
		// lsr_imm_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_lsr_imm_T1) {
			uint8_t Rd = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			uint8_t imm = ((opcode & 0x7c00000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 lsr_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_lsr_imm_T1(vctx, Rd, Rm, imm);
		}
	} else if ((opcode & 0xffc00000) == 0x40c00000) {
		// lsr_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_lsr_reg_T1) {
			uint8_t Rdn = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 lsr_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rdn = 0x%x ", Rdn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_lsr_reg_T1(vctx, Rdn, Rm);
		}
	} else if ((opcode & 0xffe0f0f0) == 0xfa20f000) {
		// lsr_reg_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_lsr_reg_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 lsr_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_lsr_reg_T2(vctx, Rd, Rn, Rm, S);
		}
	} else if ((opcode & 0xff100010) == 0xfe000010) {
		// mcr2_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_mcr2_T2) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t coproc = ((opcode & 0xf00) >> 8);
			uint8_t CRn = ((opcode & 0xf0000) >> 16);
			uint8_t CRm = (opcode & 0xf);
			uint8_t opcA = ((opcode & 0xe00000) >> 21);
			uint8_t opcB = ((opcode & 0xe0) >> 5);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 mcr2_T2 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "coproc = 0x%x ", coproc);
				fprintf(insn_debugging_info, "CRn = 0x%x ", CRn);
				fprintf(insn_debugging_info, "CRm = 0x%x ", CRm);
				fprintf(insn_debugging_info, "opcA = 0x%x ", opcA);
				fprintf(insn_debugging_info, "opcB = 0x%x ", opcB);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_mcr2_T2(vctx, Rt, coproc, CRn, CRm, opcA, opcB);
		}
	} else if ((opcode & 0xfff000f0) == 0xfb000010) {
		// mls_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_mls_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t Ra = ((opcode & 0xf000) >> 12);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 mls_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "Ra = 0x%x ", Ra);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_mls_T1(vctx, Rd, Rn, Rm, Ra);
		}
	} else if ((opcode & 0xf8000000) == 0x20000000) {
		// mov_imm_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_mov_imm_T1) {
			uint8_t Rd = ((opcode & 0x7000000) >> 24);
			uint8_t imm = ((opcode & 0xff0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 mov_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_mov_imm_T1(vctx, Rd, imm);
		}
	} else if ((opcode & 0xff000000) == 0x46000000) {
		// mov_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_mov_reg_T1) {
			uint8_t Rd = ((opcode & 0x800000) >> 20) | ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x780000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 mov_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_mov_reg_T1(vctx, Rd, Rm);
		}
	} else if ((opcode & 0xfbf08000) == 0xf2c00000) {
		// movt_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_movt_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint16_t imm = ((opcode & 0x4000000) >> 15) | ((opcode & 0xf7000) >> 4) | (opcode & 0xff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 movt_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_movt_T1(vctx, Rd, imm);
		}
	} else if ((opcode & 0xff100010) == 0xfe100010) {
		// mrc2_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_mrc2_T2) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t coproc = ((opcode & 0xf00) >> 8);
			uint8_t Crn = ((opcode & 0xf0000) >> 16);
			uint8_t CRm = (opcode & 0xf);
			uint8_t opcA = ((opcode & 0xe00000) >> 21);
			uint8_t opcB = ((opcode & 0xe0) >> 5);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 mrc2_T2 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "coproc = 0x%x ", coproc);
				fprintf(insn_debugging_info, "Crn = 0x%x ", Crn);
				fprintf(insn_debugging_info, "CRm = 0x%x ", CRm);
				fprintf(insn_debugging_info, "opcA = 0x%x ", opcA);
				fprintf(insn_debugging_info, "opcB = 0x%x ", opcB);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_mrc2_T2(vctx, Rt, coproc, Crn, CRm, opcA, opcB);
		}
	} else if ((opcode & 0xffc00000) == 0x43400000) {
		// mul_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_mul_T1) {
			uint8_t Rdm = ((opcode & 0x70000) >> 16);
			uint8_t Rn = ((opcode & 0x380000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 mul_T1 ", opcode);
				fprintf(insn_debugging_info, "Rdm = 0x%x ", Rdm);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_mul_T1(vctx, Rdm, Rn);
		}
	} else if ((opcode & 0xffc00000) == 0x43c00000) {
		// mvn_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_mvn_reg_T1) {
			uint8_t Rd = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 mvn_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_mvn_reg_T1(vctx, Rd, Rm);
		}
	} else if ((opcode & 0xffc00000) == 0x43000000) {
		// orr_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_orr_reg_T1) {
			uint8_t Rdn = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 orr_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rdn = 0x%x ", Rdn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_orr_reg_T1(vctx, Rdn, Rm);
		}
	} else if ((opcode & 0xffe00000) == 0xeac00000) {
		// pkhbt_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_pkhbt_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			bool tb = ((opcode & 0x20) >> 5);
			bool S = ((opcode & 0x100000) >> 20);
			bool T = ((opcode & 0x10) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 pkhbt_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "tb = 0x%x ", tb);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "T = 0x%x ", T);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_pkhbt_T1(vctx, Rd, Rn, Rm, imm, tb, S, T);
		}
	} else if ((opcode & 0xfe000000) == 0xbc000000) {
		// pop_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_pop_T1) {
			uint8_t register_list = ((opcode & 0xff0000) >> 16);
			bool P = ((opcode & 0x1000000) >> 24);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 pop_T1 ", opcode);
				fprintf(insn_debugging_info, "register_list = 0x%x ", register_list);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_pop_T1(vctx, register_list, P);
		}
	} else if ((opcode & 0xfe000000) == 0xb4000000) {
		// push_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_push_T1) {
			uint8_t register_list = ((opcode & 0xff0000) >> 16);
			bool M = ((opcode & 0x1000000) >> 24);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 push_T1 ", opcode);
				fprintf(insn_debugging_info, "register_list = 0x%x ", register_list);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_push_T1(vctx, register_list, M);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfa90f010) {
		// qadd16_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_qadd16_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 qadd16_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_qadd16_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfa80f010) {
		// qadd8_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_qadd8_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 qadd8_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_qadd8_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfa80f080) {
		// qadd_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_qadd_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 qadd_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_qadd_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfa80f090) {
		// qdadd_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_qdadd_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 qdadd_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_qdadd_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfa80f0b0) {
		// qdsub_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_qdsub_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 qdsub_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_qdsub_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfad0f010) {
		// qsub16_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_qsub16_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 qsub16_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_qsub16_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfac0f010) {
		// qsub8_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_qsub8_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 qsub8_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_qsub8_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfa80f0a0) {
		// qsub_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_qsub_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 qsub_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_qsub_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfa90f0a0) {
		// rbit_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_rbit_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rm = ((opcode & 0xf0000) >> 12) | (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 rbit_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_rbit_T1(vctx, Rd, Rm);
		}
	} else if ((opcode & 0xffc00000) == 0xba400000) {
		// rev16_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_rev16_T1) {
			uint8_t Rd = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 rev16_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_rev16_T1(vctx, Rd, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfa90f090) {
		// rev16_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_rev16_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rm = ((opcode & 0xf0000) >> 12) | (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 rev16_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_rev16_T2(vctx, Rd, Rm);
		}
	} else if ((opcode & 0xffc00000) == 0xba000000) {
		// rev_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_rev_T1) {
			uint8_t Rd = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 rev_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_rev_T1(vctx, Rd, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfa90f080) {
		// rev_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_rev_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rm = (opcode & 0xf);
			uint8_t Rmx = ((opcode & 0xf0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 rev_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "Rmx = 0x%x ", Rmx);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_rev_T2(vctx, Rd, Rm, Rmx);
		}
	} else if ((opcode & 0xffc00000) == 0xbac00000) {
		// revsh_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_revsh_T1) {
			uint8_t Rd = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 revsh_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_revsh_T1(vctx, Rd, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfa90f0b0) {
		// revsh_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_revsh_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rm = ((opcode & 0xf0000) >> 12) | (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 revsh_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_revsh_T2(vctx, Rd, Rm);
		}
	} else if ((opcode & 0xffc00000) == 0x41c00000) {
		// ror_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_ror_reg_T1) {
			uint8_t Rdn = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 ror_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rdn = 0x%x ", Rdn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_ror_reg_T1(vctx, Rdn, Rm);
		}
	} else if ((opcode & 0xffe0f0f0) == 0xfa60f000) {
		// ror_reg_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_ror_reg_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ror_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ror_reg_T2(vctx, Rd, Rn, Rm, S);
		}
	} else if ((opcode & 0xffc00000) == 0x42400000) {
		// rsb_imm_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_rsb_imm_T1) {
			uint8_t Rd = ((opcode & 0x70000) >> 16);
			uint8_t Rn = ((opcode & 0x380000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 rsb_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_rsb_imm_T1(vctx, Rd, Rn);
		}
	} else if ((opcode & 0xfbe08000) == 0xf1c00000) {
		// rsb_imm_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_rsb_imm_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			int32_t imm = thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 rsb_imm_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x [orig 0x%x] ", imm, ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_rsb_imm_T2(vctx, Rd, Rn, imm, S);
		}
	} else if ((opcode & 0xffe00000) == 0xebc00000) {
		// rsb_reg_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_rsb_reg_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			uint8_t type = ((opcode & 0x30) >> 4);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 rsb_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "type = 0x%x ", type);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_rsb_reg_T1(vctx, Rd, Rn, Rm, imm, type, S);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfa90f000) {
		// sadd16_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_sadd16_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 sadd16_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_sadd16_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfa80f000) {
		// sadd8_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_sadd8_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 sadd8_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_sadd8_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfaa0f000) {
		// sasx_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_sasx_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 sasx_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_sasx_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfbe08000) == 0xf1600000) {
		// sbc_imm_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_sbc_imm_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			int32_t imm = thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 sbc_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x [orig 0x%x] ", imm, ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_sbc_imm_T1(vctx, Rd, Rn, imm, S);
		}
	} else if ((opcode & 0xffc00000) == 0x41800000) {
		// sbc_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_sbc_reg_T1) {
			uint8_t Rdn = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 sbc_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rdn = 0x%x ", Rdn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_sbc_reg_T1(vctx, Rdn, Rm);
		}
	} else if ((opcode & 0xffe00000) == 0xeb600000) {
		// sbc_reg_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_sbc_reg_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			uint8_t type = ((opcode & 0x30) >> 4);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 sbc_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "type = 0x%x ", type);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_sbc_reg_T2(vctx, Rd, Rn, Rm, imm, type, S);
		}
	} else if ((opcode & 0xfbf08000) == 0xf3400000) {
		// sbfx_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_sbfx_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			uint8_t width = (opcode & 0x1f);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 sbfx_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "width = 0x%x ", width);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_sbfx_T1(vctx, Rd, Rn, imm, width);
		}
	} else if ((opcode & 0xfff000f0) == 0xfb9000f0) {
		// sdiv_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_sdiv_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 sdiv_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_sdiv_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfaa0f080) {
		// sel_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_sel_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 sel_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_sel_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfa90f020) {
		// shadd16_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_shadd16_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 shadd16_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_shadd16_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfa80f020) {
		// shadd8_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_shadd8_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 shadd8_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_shadd8_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfaa0f020) {
		// shasx_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_shasx_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 shasx_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_shasx_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfae0f020) {
		// shsax_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_shsax_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 shsax_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_shsax_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfad0f020) {
		// shsub16_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_shsub16_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 shsub16_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_shsub16_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfac0f020) {
		// shsub8_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_shsub8_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 shsub8_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_shsub8_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff000f0) == 0xfbc00000) {
		// smlal_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_smlal_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t RdLo = ((opcode & 0xf000) >> 12);
			uint8_t RdHi = ((opcode & 0xf00) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 smlal_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "RdLo = 0x%x ", RdLo);
				fprintf(insn_debugging_info, "RdHi = 0x%x ", RdHi);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_smlal_T1(vctx, Rn, Rm, RdLo, RdHi);
		}
	} else if ((opcode & 0xfff000c0) == 0xfbc00080) {
		// smlalbb_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_smlalbb_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t RdLo = ((opcode & 0xf000) >> 12);
			uint8_t RdHi = ((opcode & 0xf00) >> 8);
			bool N = ((opcode & 0x20) >> 5);
			bool M = ((opcode & 0x10) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 smlalbb_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "RdLo = 0x%x ", RdLo);
				fprintf(insn_debugging_info, "RdHi = 0x%x ", RdHi);
				fprintf(insn_debugging_info, "N = 0x%x ", N);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_smlalbb_T1(vctx, Rn, Rm, RdLo, RdHi, N, M);
		}
	} else if ((opcode & 0xfff000e0) == 0xfbc000c0) {
		// smlald_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_smlald_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t RdLo = ((opcode & 0xf000) >> 12);
			uint8_t RdHi = ((opcode & 0xf00) >> 8);
			bool M = ((opcode & 0x10) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 smlald_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "RdLo = 0x%x ", RdLo);
				fprintf(insn_debugging_info, "RdHi = 0x%x ", RdHi);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_smlald_T1(vctx, Rn, Rm, RdLo, RdHi, M);
		}
	} else if ((opcode & 0xfff000e0) == 0xfbd000c0) {
		// smlsld_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_smlsld_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t RdLo = ((opcode & 0xf000) >> 12);
			uint8_t RdHi = ((opcode & 0xf00) >> 8);
			bool M = ((opcode & 0x10) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 smlsld_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "RdLo = 0x%x ", RdLo);
				fprintf(insn_debugging_info, "RdHi = 0x%x ", RdHi);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_smlsld_T1(vctx, Rn, Rm, RdLo, RdHi, M);
		}
	} else if ((opcode & 0xfff000e0) == 0xfb600000) {
		// smmls_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_smmls_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t Ra = ((opcode & 0xf000) >> 12);
			bool R = ((opcode & 0x10) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 smmls_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "Ra = 0x%x ", Ra);
				fprintf(insn_debugging_info, "R = 0x%x ", R);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_smmls_T1(vctx, Rd, Rn, Rm, Ra, R);
		}
	} else if ((opcode & 0xfff000f0) == 0xfb800000) {
		// smull_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_smull_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t RdLo = ((opcode & 0xf000) >> 12);
			uint8_t RdHi = ((opcode & 0xf00) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 smull_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "RdLo = 0x%x ", RdLo);
				fprintf(insn_debugging_info, "RdHi = 0x%x ", RdHi);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_smull_T1(vctx, Rn, Rm, RdLo, RdHi);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfae0f000) {
		// ssax_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ssax_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ssax_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ssax_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfad0f000) {
		// ssub16_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ssub16_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ssub16_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ssub16_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfac0f000) {
		// ssub8_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ssub8_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ssub8_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ssub8_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xf8000000) == 0xc0000000) {
		// stm_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_stm_T1) {
			uint8_t Rn = ((opcode & 0x7000000) >> 24);
			uint8_t register_list = ((opcode & 0xff0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 stm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "register_list = 0x%x ", register_list);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_stm_T1(vctx, Rn, register_list);
		}
	} else if ((opcode & 0xffd0a000) == 0xe8800000) {
		// stm_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_stm_T2) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint16_t register_list = (opcode & 0x1fff);
			bool M = ((opcode & 0x4000) >> 14);
			bool W = ((opcode & 0x200000) >> 21);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 stm_T2 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "register_list = 0x%x ", register_list);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_stm_T2(vctx, Rn, register_list, M, W);
		}
	} else if ((opcode & 0xf8000000) == 0x60000000) {
		// str_imm_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_str_imm_T1) {
			uint8_t Rt = ((opcode & 0x70000) >> 16);
			uint8_t Rn = ((opcode & 0x380000) >> 19);
			uint8_t imm = ((opcode & 0x7c00000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 str_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_str_imm_T1(vctx, Rt, Rn, imm);
		}
	} else if ((opcode & 0xf8000000) == 0x90000000) {
		// str_imm_T2
		decoded_insn_length = 2;
		if (handler && handler->i16_str_imm_T2) {
			uint8_t Rt = ((opcode & 0x7000000) >> 24);
			uint8_t imm = ((opcode & 0xff0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 str_imm_T2 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_str_imm_T2(vctx, Rt, imm);
		}
	} else if ((opcode & 0xfff00000) == 0xf8c00000) {
		// str_imm_T3
		decoded_insn_length = 4;
		if (handler && handler->i32_str_imm_T3) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint16_t imm = (opcode & 0xfff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 str_imm_T3 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_str_imm_T3(vctx, Rt, Rn, imm);
		}
	} else if ((opcode & 0xfe000000) == 0x50000000) {
		// str_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_str_reg_T1) {
			uint8_t Rt = ((opcode & 0x70000) >> 16);
			uint8_t Rn = ((opcode & 0x380000) >> 19);
			uint8_t Rm = ((opcode & 0x1c00000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 str_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_str_reg_T1(vctx, Rt, Rn, Rm);
		}
	} else if ((opcode & 0xfff00fc0) == 0xf8400000) {
		// str_reg_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_str_reg_T2) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 str_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_str_reg_T2(vctx, Rt, Rn, Rm, imm);
		}
	} else if ((opcode & 0xf8000000) == 0x70000000) {
		// strb_imm_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_strb_imm_T1) {
			uint8_t Rt = ((opcode & 0x70000) >> 16);
			uint8_t Rn = ((opcode & 0x380000) >> 19);
			uint8_t imm = ((opcode & 0x7c00000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 strb_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_strb_imm_T1(vctx, Rt, Rn, imm);
		}
	} else if ((opcode & 0xfff00000) == 0xf8800000) {
		// strb_imm_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_strb_imm_T2) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint16_t imm = (opcode & 0xfff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 strb_imm_T2 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_strb_imm_T2(vctx, Rt, Rn, imm);
		}
	} else if ((opcode & 0xfe000000) == 0x54000000) {
		// strb_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_strb_reg_T1) {
			uint8_t Rt = ((opcode & 0x70000) >> 16);
			uint8_t Rn = ((opcode & 0x380000) >> 19);
			uint8_t Rm = ((opcode & 0x1c00000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 strb_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_strb_reg_T1(vctx, Rt, Rn, Rm);
		}
	} else if ((opcode & 0xfff00fc0) == 0xf8000000) {
		// strb_reg_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_strb_reg_T2) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 strb_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_strb_reg_T2(vctx, Rt, Rn, Rm, imm);
		}
	} else if ((opcode & 0xf8000000) == 0x80000000) {
		// strh_imm_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_strh_imm_T1) {
			uint8_t Rt = ((opcode & 0x70000) >> 16);
			uint8_t Rn = ((opcode & 0x380000) >> 19);
			uint8_t imm = ((opcode & 0x7c00000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 strh_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_strh_imm_T1(vctx, Rt, Rn, imm);
		}
	} else if ((opcode & 0xfff00000) == 0xf8a00000) {
		// strh_imm_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_strh_imm_T2) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint16_t imm = (opcode & 0xfff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 strh_imm_T2 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_strh_imm_T2(vctx, Rt, Rn, imm);
		}
	} else if ((opcode & 0xfe000000) == 0x52000000) {
		// strh_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_strh_reg_T1) {
			uint8_t Rt = ((opcode & 0x70000) >> 16);
			uint8_t Rn = ((opcode & 0x380000) >> 19);
			uint8_t Rm = ((opcode & 0x1c00000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 strh_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_strh_reg_T1(vctx, Rt, Rn, Rm);
		}
	} else if ((opcode & 0xfff00fc0) == 0xf8200000) {
		// strh_reg_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_strh_reg_T2) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 strh_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_strh_reg_T2(vctx, Rt, Rn, Rm, imm);
		}
	} else if ((opcode & 0xff800000) == 0xb0800000) {
		// sub_SPimm_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_sub_SPimm_T1) {
			uint8_t imm = ((opcode & 0x7f0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 sub_SPimm_T1 ", opcode);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_sub_SPimm_T1(vctx, imm);
		}
	} else if ((opcode & 0xfe000000) == 0x1e000000) {
		// sub_imm_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_sub_imm_T1) {
			uint8_t Rd = ((opcode & 0x70000) >> 16);
			uint8_t Rn = ((opcode & 0x380000) >> 19);
			uint8_t imm = ((opcode & 0x1c00000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 sub_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_sub_imm_T1(vctx, Rd, Rn, imm);
		}
	} else if ((opcode & 0xf8000000) == 0x38000000) {
		// sub_imm_T2
		decoded_insn_length = 2;
		if (handler && handler->i16_sub_imm_T2) {
			uint8_t Rdn = ((opcode & 0x7000000) >> 24);
			uint8_t imm = ((opcode & 0xff0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 sub_imm_T2 ", opcode);
				fprintf(insn_debugging_info, "Rdn = 0x%x ", Rdn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_sub_imm_T2(vctx, Rdn, imm);
		}
	} else if ((opcode & 0xfe000000) == 0x1a000000) {
		// sub_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_sub_reg_T1) {
			uint8_t Rd = ((opcode & 0x70000) >> 16);
			uint8_t Rn = ((opcode & 0x380000) >> 19);
			uint8_t Rm = ((opcode & 0x1c00000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 sub_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_sub_reg_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xffc00000) == 0xb2400000) {
		// sxtb_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_sxtb_T1) {
			uint8_t Rd = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 sxtb_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_sxtb_T1(vctx, Rd, Rm);
		}
	} else if ((opcode & 0xffc00000) == 0xb2000000) {
		// sxth_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_sxth_T1) {
			uint8_t Rd = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 sxth_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_sxth_T1(vctx, Rd, Rm);
		}
	} else if ((opcode & 0xffc00000) == 0x42000000) {
		// tst_reg_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_tst_reg_T1) {
			uint8_t Rn = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 tst_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_tst_reg_T1(vctx, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfa90f040) {
		// uadd16_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_uadd16_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 uadd16_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_uadd16_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfa80f040) {
		// uadd8_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_uadd8_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 uadd8_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_uadd8_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfaa0f040) {
		// uasx_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_uasx_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 uasx_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_uasx_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfbf08000) == 0xf3c00000) {
		// ubfx_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ubfx_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			uint8_t width = (opcode & 0x1f);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ubfx_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "width = 0x%x ", width);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ubfx_T1(vctx, Rd, Rn, imm, width);
		}
	} else if ((opcode & 0xfff000f0) == 0xfbb000f0) {
		// udiv_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_udiv_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 udiv_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_udiv_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfa90f060) {
		// uhadd16_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_uhadd16_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 uhadd16_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_uhadd16_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfa80f060) {
		// uhadd8_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_uhadd8_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 uhadd8_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_uhadd8_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfaa0f060) {
		// uhasx_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_uhasx_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 uhasx_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_uhasx_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfae0f060) {
		// uhsax_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_uhsax_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 uhsax_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_uhsax_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfad0f060) {
		// uhsub16_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_uhsub16_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 uhsub16_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_uhsub16_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfac0f060) {
		// uhsub8_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_uhsub8_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 uhsub8_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_uhsub8_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff000f0) == 0xfbe00060) {
		// umaal_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_umaal_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t RdLo = ((opcode & 0xf000) >> 12);
			uint8_t RdHi = ((opcode & 0xf00) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 umaal_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "RdLo = 0x%x ", RdLo);
				fprintf(insn_debugging_info, "RdHi = 0x%x ", RdHi);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_umaal_T1(vctx, Rn, Rm, RdLo, RdHi);
		}
	} else if ((opcode & 0xfff000f0) == 0xfbe00000) {
		// umlal_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_umlal_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t RdLo = ((opcode & 0xf000) >> 12);
			uint8_t RdHi = ((opcode & 0xf00) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 umlal_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "RdLo = 0x%x ", RdLo);
				fprintf(insn_debugging_info, "RdHi = 0x%x ", RdHi);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_umlal_T1(vctx, Rn, Rm, RdLo, RdHi);
		}
	} else if ((opcode & 0xfff000f0) == 0xfba00000) {
		// umull_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_umull_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t RdLo = ((opcode & 0xf000) >> 12);
			uint8_t RdHi = ((opcode & 0xf00) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 umull_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "RdLo = 0x%x ", RdLo);
				fprintf(insn_debugging_info, "RdHi = 0x%x ", RdHi);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_umull_T1(vctx, Rn, Rm, RdLo, RdHi);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfa90f050) {
		// uqadd16_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_uqadd16_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 uqadd16_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_uqadd16_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfa80f050) {
		// uqadd8_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_uqadd8_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 uqadd8_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_uqadd8_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfaa0f050) {
		// uqasx_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_uqasx_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 uqasx_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_uqasx_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfae0f050) {
		// uqsax_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_uqsax_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 uqsax_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_uqsax_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfad0f050) {
		// uqsub16_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_uqsub16_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 uqsub16_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_uqsub16_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfac0f050) {
		// uqsub8_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_uqsub8_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 uqsub8_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_uqsub8_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfae0f040) {
		// usax_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_usax_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 usax_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_usax_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfad0f040) {
		// usub16_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_usub16_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 usub16_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_usub16_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfac0f040) {
		// usub8_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_usub8_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 usub8_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_usub8_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xffc00000) == 0xb2c00000) {
		// uxtb_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_uxtb_T1) {
			uint8_t Rd = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 uxtb_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_uxtb_T1(vctx, Rd, Rm);
		}
	} else if ((opcode & 0xffc00000) == 0xb2800000) {
		// uxth_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_uxth_T1) {
			uint8_t Rd = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 uxth_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_uxth_T1(vctx, Rd, Rm);
		}
	} else if ((opcode & 0xfbff8000) == 0xf20d0000) {
		// add_SPi_T4
		decoded_insn_length = 4;
		if (handler && handler->i32_add_SPi_T4) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint16_t imm = ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 add_SPi_T4 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_add_SPi_T4(vctx, Rd, imm);
		}
	} else if ((opcode & 0xff870000) == 0x44850000) {
		// add_SPr_T2
		decoded_insn_length = 2;
		if (handler && handler->i16_add_SPr_T2) {
			uint8_t Rm = ((opcode & 0x780000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 add_SPr_T2 ", opcode);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_add_SPr_T2(vctx, Rm);
		}
	} else if ((opcode & 0xfbff8000) == 0xf2af0000) {
		// adr_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_adr_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint16_t imm = ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 adr_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_adr_T2(vctx, Rd, imm);
		}
	} else if ((opcode & 0xfbff8000) == 0xf20f0000) {
		// adr_T3
		decoded_insn_length = 4;
		if (handler && handler->i32_adr_T3) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint16_t imm = ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 adr_T3 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_adr_T3(vctx, Rd, imm);
		}
	} else if ((opcode & 0xfbff8000) == 0xf36f0000) {
		// bfc_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_bfc_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			uint8_t msb = (opcode & 0x1f);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 bfc_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "msb = 0x%x ", msb);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_bfc_T1(vctx, Rd, imm, msb);
		}
	} else if ((opcode & 0xfbf08f00) == 0xf1100f00) {
		// cmn_imm_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_cmn_imm_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			int32_t imm = thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 cmn_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x [orig 0x%x] ", imm, ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_cmn_imm_T1(vctx, Rn, imm);
		}
	} else if ((opcode & 0xfff00f00) == 0xeb100f00) {
		// cmn_reg_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_cmn_reg_T2) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			int32_t imm = thumb_expand_imm(((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6));
			uint8_t type = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 cmn_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x [orig 0x%x] ", imm, ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6));
				fprintf(insn_debugging_info, "type = 0x%x ", type);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_cmn_reg_T2(vctx, Rn, Rm, imm, type);
		}
	} else if ((opcode & 0xfff08f00) == 0xebb00f00) {
		// cmp_reg_T3
		decoded_insn_length = 4;
		if (handler && handler->i32_cmp_reg_T3) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			uint8_t type = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 cmp_reg_T3 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "type = 0x%x ", type);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_cmp_reg_T3(vctx, Rn, Rm, imm, type);
		}
	} else if ((opcode & 0xfff00fff) == 0xe8d00f4f) {
		// ldrexb_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrexb_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrexb_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrexb_T1(vctx, Rt, Rn);
		}
	} else if ((opcode & 0xfff00fff) == 0xe8d00f5f) {
		// ldrexh_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrexh_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrexh_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrexh_T1(vctx, Rt, Rn);
		}
	} else if ((opcode & 0xfff00fc0) == 0xf8300000) {
		// ldrh_reg_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrh_reg_T2) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrh_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrh_reg_T2(vctx, Rt, Rn, Rm, imm);
		}
	} else if ((opcode & 0xfff00f00) == 0xf9300e00) {
		// ldrsh_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrsh_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = (opcode & 0xff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrsh_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrsh_T1(vctx, Rt, Rn, imm);
		}
	} else if ((opcode & 0xfff00fc0) == 0xf9300000) {
		// ldrsh_reg_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrsh_reg_T2) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrsh_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrsh_reg_T2(vctx, Rt, Rn, Rm, imm);
		}
	} else if ((opcode & 0xfff00000) == 0xfc400000) {
		// mcrr2_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_mcrr2_T2) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t coproc = ((opcode & 0xf00) >> 8);
			uint8_t CRm = (opcode & 0xf);
			uint8_t opcA = ((opcode & 0xf0) >> 4);
			uint8_t RtB = ((opcode & 0xf0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 mcrr2_T2 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "coproc = 0x%x ", coproc);
				fprintf(insn_debugging_info, "CRm = 0x%x ", CRm);
				fprintf(insn_debugging_info, "opcA = 0x%x ", opcA);
				fprintf(insn_debugging_info, "RtB = 0x%x ", RtB);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_mcrr2_T2(vctx, Rt, coproc, CRm, opcA, RtB);
		}
	} else if ((opcode & 0xfbef8000) == 0xf04f0000) {
		// mov_imm_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_mov_imm_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			int32_t imm = thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 mov_imm_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "imm = 0x%x [orig 0x%x] ", imm, ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_mov_imm_T2(vctx, Rd, imm, S);
		}
	} else if ((opcode & 0xffc00000) == 0x0) {
		// mov_reg_T2
		decoded_insn_length = 2;
		if (handler && handler->i16_mov_reg_T2) {
			uint8_t Rd = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 mov_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_mov_reg_T2(vctx, Rd, Rm);
		}
	} else if ((opcode & 0xffeff0f0) == 0xf24f0000) {
		// mov_reg_T3
		decoded_insn_length = 4;
		if (handler && handler->i32_mov_reg_T3) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rm = (opcode & 0xf);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 mov_reg_T3 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_mov_reg_T3(vctx, Rd, Rm, S);
		}
	} else if ((opcode & 0xfff00000) == 0xfc500000) {
		// mrrc2_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_mrrc2_T2) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t coproc = ((opcode & 0xf00) >> 8);
			uint8_t CRm = (opcode & 0xf);
			uint8_t opcA = ((opcode & 0xf0) >> 4);
			uint8_t RtB = ((opcode & 0xf0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 mrrc2_T2 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "coproc = 0x%x ", coproc);
				fprintf(insn_debugging_info, "CRm = 0x%x ", CRm);
				fprintf(insn_debugging_info, "opcA = 0x%x ", opcA);
				fprintf(insn_debugging_info, "RtB = 0x%x ", RtB);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_mrrc2_T2(vctx, Rt, coproc, CRm, opcA, RtB);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfb00f000) {
		// mul_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_mul_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 mul_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_mul_T2(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfbef8000) == 0xf06f0000) {
		// mvn_imm_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_mvn_imm_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			int32_t imm = thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 mvn_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "imm = 0x%x [orig 0x%x] ", imm, ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_mvn_imm_T1(vctx, Rd, imm, S);
		}
	} else if ((opcode & 0xffef8000) == 0xea6f0000) {
		// mvn_reg_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_mvn_reg_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rm = (opcode & 0xf);
			int32_t imm = thumb_expand_imm(((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6));
			uint8_t type = ((opcode & 0x30) >> 4);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 mvn_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x [orig 0x%x] ", imm, ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6));
				fprintf(insn_debugging_info, "type = 0x%x ", type);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_mvn_reg_T2(vctx, Rd, Rm, imm, type, S);
		}
	} else if ((opcode & 0xffff0000) == 0xbf000000) {
		// nop_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_nop_T1) {
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 nop_T1 ", opcode);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_nop_T1(vctx);
		}
	} else if ((opcode & 0xffffffff) == 0xf3af8000) {
		// nop_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_nop_T2) {
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 nop_T2 ", opcode);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_nop_T2(vctx);
		}
	} else if ((opcode & 0xff7ff000) == 0xf91ff000) {
		// pld_immlit_T3
		decoded_insn_length = 4;
		if (handler && handler->i32_pld_immlit_T3) {
			uint16_t imm = (opcode & 0xfff);
			bool U = ((opcode & 0x800000) >> 23);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 pld_immlit_T3 ", opcode);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_pld_immlit_T3(vctx, imm, U);
		}
	} else if ((opcode & 0xff7ff000) == 0xf81ff000) {
		// pld_lit_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_pld_lit_T1) {
			uint16_t imm = (opcode & 0xfff);
			bool U = ((opcode & 0x800000) >> 23);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 pld_lit_T1 ", opcode);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_pld_lit_T1(vctx, imm, U);
		}
	} else if ((opcode & 0xffff2000) == 0xe8bd0000) {
		// pop_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_pop_T2) {
			uint16_t register_list = (opcode & 0x1fff);
			bool P = ((opcode & 0x8000) >> 15);
			bool M = ((opcode & 0x4000) >> 14);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 pop_T2 ", opcode);
				fprintf(insn_debugging_info, "register_list = 0x%x ", register_list);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_pop_T2(vctx, register_list, P, M);
		}
	} else if ((opcode & 0xffff0fff) == 0xf85d0b04) {
		// pop_T3
		decoded_insn_length = 4;
		if (handler && handler->i32_pop_T3) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 pop_T3 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_pop_T3(vctx, Rt);
		}
	} else if ((opcode & 0xffffa000) == 0xe92d0000) {
		// push_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_push_T2) {
			uint16_t register_list = (opcode & 0x1fff);
			bool M = ((opcode & 0x4000) >> 14);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 push_T2 ", opcode);
				fprintf(insn_debugging_info, "register_list = 0x%x ", register_list);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_push_T2(vctx, register_list, M);
		}
	} else if ((opcode & 0xffff0fff) == 0xf84d0d04) {
		// push_T3
		decoded_insn_length = 4;
		if (handler && handler->i32_push_T3) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 push_T3 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_push_T3(vctx, Rt);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfae0f010) {
		// qasx_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_qasx_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 qasx_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_qasx_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfae0f010) {
		// qsax_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_qsax_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 qsax_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_qsax_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xffef70f0) == 0xea4f0030) {
		// rrx_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_rrx_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rm = (opcode & 0xf);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 rrx_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_rrx_T1(vctx, Rd, Rm, S);
		}
	} else if ((opcode & 0xffff0000) == 0xbf400000) {
		// sev_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_sev_T1) {
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 sev_T1 ", opcode);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_sev_T1(vctx);
		}
	} else if ((opcode & 0xfff0f0e0) == 0xfb50f000) {
		// smmul_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_smmul_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			bool R = ((opcode & 0x10) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 smmul_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "R = 0x%x ", R);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_smmul_T1(vctx, Rd, Rn, Rm, R);
		}
	} else if ((opcode & 0xfff0f0e0) == 0xfb20f000) {
		// smuad_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_smuad_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			bool M = ((opcode & 0x10) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 smuad_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_smuad_T1(vctx, Rd, Rn, Rm, M);
		}
	} else if ((opcode & 0xfff0f0c0) == 0xfb10f000) {
		// smulbb_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_smulbb_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			bool N = ((opcode & 0x20) >> 5);
			bool M = ((opcode & 0x10) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 smulbb_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "N = 0x%x ", N);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_smulbb_T1(vctx, Rd, Rn, Rm, N, M);
		}
	} else if ((opcode & 0xfff0f0e0) == 0xfb30f000) {
		// smulwb_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_smulwb_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			bool M = ((opcode & 0x10) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 smulwb_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_smulwb_T1(vctx, Rd, Rn, Rm, M);
		}
	} else if ((opcode & 0xfff0f0e0) == 0xfb40f000) {
		// smusd_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_smusd_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			bool M = ((opcode & 0x10) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 smusd_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_smusd_T1(vctx, Rd, Rn, Rm, M);
		}
	} else if ((opcode & 0xfbf0f0c0) == 0xf3200000) {
		// ssat16_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ssat16_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t satimm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ssat16_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "satimm = 0x%x ", satimm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ssat16_T1(vctx, Rd, Rn, satimm);
		}
	} else if ((opcode & 0xfff00f00) == 0xf8000e00) {
		// strb_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_strb_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = (opcode & 0xff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 strb_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_strb_T1(vctx, Rt, Rn, imm);
		}
	} else if ((opcode & 0xfff000f0) == 0xe8c00040) {
		// strexb_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_strexb_T1) {
			uint8_t Rd = (opcode & 0xf);
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 strexb_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_strexb_T1(vctx, Rd, Rt, Rn);
		}
	} else if ((opcode & 0xfff000f0) == 0xe8c00050) {
		// strexh_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_strexh_T1) {
			uint8_t Rd = (opcode & 0xf);
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 strexh_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_strexh_T1(vctx, Rd, Rt, Rn);
		}
	} else if ((opcode & 0xfff00f00) == 0xf8200e00) {
		// strht_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_strht_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = (opcode & 0xff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 strht_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_strht_T1(vctx, Rt, Rn, imm);
		}
	} else if ((opcode & 0xfbff8000) == 0xf2ad0000) {
		// sub_SPimm_T3
		decoded_insn_length = 4;
		if (handler && handler->i32_sub_SPimm_T3) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint16_t imm = ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 sub_SPimm_T3 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_sub_SPimm_T3(vctx, Rd, imm);
		}
	} else if ((opcode & 0xff000000) == 0xdf000000) {
		// svc_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_svc_T1) {
			uint8_t imm = ((opcode & 0xff0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 svc_T1 ", opcode);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_svc_T1(vctx, imm);
		}
	} else if ((opcode & 0xfffff0c0) == 0xfa2ff080) {
		// sxtb16_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_sxtb16_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rm = (opcode & 0xf);
			uint8_t rotate = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 sxtb16_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "rotate = 0x%x ", rotate);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_sxtb16_T1(vctx, Rd, Rm, rotate);
		}
	} else if ((opcode & 0xfffff0c0) == 0xfa4ff080) {
		// sxtb_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_sxtb_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rm = (opcode & 0xf);
			uint8_t rotate = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 sxtb_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "rotate = 0x%x ", rotate);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_sxtb_T2(vctx, Rd, Rm, rotate);
		}
	} else if ((opcode & 0xfffff0c0) == 0xfa0ff080) {
		// sxth_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_sxth_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rm = (opcode & 0xf);
			uint8_t rotate = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 sxth_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "rotate = 0x%x ", rotate);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_sxth_T2(vctx, Rd, Rm, rotate);
		}
	} else if ((opcode & 0xfbf08f00) == 0xf0900f00) {
		// teq_imm_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_teq_imm_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint16_t imm = ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 teq_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_teq_imm_T1(vctx, Rn, imm);
		}
	} else if ((opcode & 0xfff00f00) == 0xea900f00) {
		// teq_reg_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_teq_reg_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			uint8_t type = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 teq_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "type = 0x%x ", type);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_teq_reg_T1(vctx, Rn, Rm, imm, type);
		}
	} else if ((opcode & 0xfbf08f00) == 0xf0100f00) {
		// tst_imm_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_tst_imm_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			int32_t imm = thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 tst_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x [orig 0x%x] ", imm, ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_tst_imm_T1(vctx, Rn, imm);
		}
	} else if ((opcode & 0xfff08f00) == 0xea100f00) {
		// tst_reg_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_tst_reg_T2) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			uint8_t type = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 tst_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "type = 0x%x ", type);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_tst_reg_T2(vctx, Rn, Rm, imm, type);
		}
	} else if ((opcode & 0xff000000) == 0xde000000) {
		// udf_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_udf_T1) {
			uint8_t imm = ((opcode & 0xff0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 udf_T1 ", opcode);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_udf_T1(vctx, imm);
		}
	} else if ((opcode & 0xfff0f0f0) == 0xfb70f000) {
		// usad8_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_usad8_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 usad8_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_usad8_T1(vctx, Rd, Rn, Rm);
		}
	} else if ((opcode & 0xfbf0f0c0) == 0xf3a00000) {
		// usat16_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_usat16_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t satimm = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 usat16_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "satimm = 0x%x ", satimm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_usat16_T1(vctx, Rd, Rn, satimm);
		}
	} else if ((opcode & 0xfffff0c0) == 0xfa3ff080) {
		// uxtb16_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_uxtb16_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rm = (opcode & 0xf);
			uint8_t rotate = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 uxtb16_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "rotate = 0x%x ", rotate);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_uxtb16_T1(vctx, Rd, Rm, rotate);
		}
	} else if ((opcode & 0xfffff0c0) == 0xfa5ff080) {
		// uxtb_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_uxtb_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rm = (opcode & 0xf);
			uint8_t rotate = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 uxtb_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "rotate = 0x%x ", rotate);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_uxtb_T2(vctx, Rd, Rm, rotate);
		}
	} else if ((opcode & 0xfffff0c0) == 0xfa1ff080) {
		// uxth_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_uxth_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rm = (opcode & 0xf);
			uint8_t rotate = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 uxth_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "rotate = 0x%x ", rotate);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_uxth_T2(vctx, Rd, Rm, rotate);
		}
	} else if ((opcode & 0xffbf0ed0) == 0xeeb00ac0) {
		// vabs_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vabs_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t Vm = (opcode & 0xf);
			bool D = ((opcode & 0x400000) >> 22);
			bool M = ((opcode & 0x20) >> 5);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vabs_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vabs_T1(vctx, Vd, Vm, D, M, sz);
		}
	} else if ((opcode & 0xffbf0ed0) == 0xeeb70ac0) {
		// vcvt_dpsp_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vcvt_dpsp_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t Vm = (opcode & 0xf);
			bool D = ((opcode & 0x400000) >> 22);
			bool M = ((opcode & 0x20) >> 5);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vcvt_dpsp_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vcvt_dpsp_T1(vctx, Vd, Vm, D, M, sz);
		}
	} else if ((opcode & 0xffe00fd0) == 0xec400a10) {
		// vmov_2arm2sp_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vmov_2arm2sp_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rtx = ((opcode & 0xf0000) >> 16);
			uint8_t Vm = (opcode & 0xf);
			bool op = ((opcode & 0x100000) >> 20);
			bool M = ((opcode & 0x20) >> 5);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vmov_2arm2sp_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rtx = 0x%x ", Rtx);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "op = 0x%x ", op);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vmov_2arm2sp_T1(vctx, Rt, Rtx, Vm, op, M);
		}
	} else if ((opcode & 0xffe00fd0) == 0xec400b10) {
		// vmov_2armdw_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vmov_2armdw_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rtx = ((opcode & 0xf0000) >> 16);
			uint8_t Vm = (opcode & 0xf);
			bool op = ((opcode & 0x100000) >> 20);
			bool M = ((opcode & 0x20) >> 5);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vmov_2armdw_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rtx = 0x%x ", Rtx);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "op = 0x%x ", op);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vmov_2armdw_T1(vctx, Rt, Rtx, Vm, op, M);
		}
	} else if ((opcode & 0xffd00f70) == 0xee000b10) {
		// vmov_armscl_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vmov_armscl_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Vd = ((opcode & 0xf0000) >> 16);
			bool H = ((opcode & 0x200000) >> 21);
			bool D = ((opcode & 0x80) >> 7);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vmov_armscl_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "H = 0x%x ", H);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vmov_armscl_T1(vctx, Rt, Vd, H, D);
		}
	} else if ((opcode & 0xffbf0ed0) == 0xeeb00a40) {
		// vmov_reg_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vmov_reg_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t Vm = (opcode & 0xf);
			bool D = ((opcode & 0x400000) >> 22);
			bool M = ((opcode & 0x20) >> 5);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vmov_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vmov_reg_T1(vctx, Vd, Vm, D, M, sz);
		}
	} else if ((opcode & 0xffd00f70) == 0xee100b10) {
		// vmov_sclarm_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vmov_sclarm_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Vd = ((opcode & 0xf0000) >> 16);
			bool H = ((opcode & 0x200000) >> 21);
			bool D = ((opcode & 0x80) >> 7);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vmov_sclarm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "H = 0x%x ", H);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vmov_sclarm_T1(vctx, Rt, Vd, H, D);
		}
	} else if ((opcode & 0xffbf0ed0) == 0xeeb10a40) {
		// vneg_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vneg_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t Vm = (opcode & 0xf);
			bool D = ((opcode & 0x400000) >> 22);
			bool M = ((opcode & 0x20) >> 5);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vneg_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vneg_T1(vctx, Vd, Vm, D, M, sz);
		}
	} else if ((opcode & 0xffbc0ed0) == 0xfeb80a40) {
		// vrinta_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vrinta_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t Vm = (opcode & 0xf);
			uint8_t RM = ((opcode & 0x30000) >> 16);
			bool D = ((opcode & 0x400000) >> 22);
			bool M = ((opcode & 0x20) >> 5);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vrinta_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "RM = 0x%x ", RM);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vrinta_T1(vctx, Vd, Vm, RM, D, M, sz);
		}
	} else if ((opcode & 0xffbf0ed0) == 0xeeb70a40) {
		// vrintx_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vrintx_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t Vm = (opcode & 0xf);
			bool D = ((opcode & 0x400000) >> 22);
			bool M = ((opcode & 0x20) >> 5);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vrintx_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vrintx_T1(vctx, Vd, Vm, D, M, sz);
		}
	} else if ((opcode & 0xffbf0ed0) == 0xeeb10ac0) {
		// vsqrt_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vsqrt_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t Vm = (opcode & 0xf);
			bool D = ((opcode & 0x400000) >> 22);
			bool M = ((opcode & 0x20) >> 5);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vsqrt_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vsqrt_T1(vctx, Vd, Vm, D, M, sz);
		}
	} else if ((opcode & 0xffff0000) == 0xbf200000) {
		// wfe_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_wfe_T1) {
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 wfe_T1 ", opcode);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_wfe_T1(vctx);
		}
	} else if ((opcode & 0xffff0000) == 0xbf300000) {
		// wfi_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_wfi_T1) {
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 wfi_T1 ", opcode);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_wfi_T1(vctx);
		}
	} else if ((opcode & 0xffff0000) == 0xbf100000) {
		// yield_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_yield_T1) {
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 yield_T1 ", opcode);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_yield_T1(vctx);
		}
	} else if ((opcode & 0xfbef8000) == 0xf10d0000) {
		// add_SPi_T3
		decoded_insn_length = 4;
		if (handler && handler->i32_add_SPi_T3) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			int32_t imm = thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 add_SPi_T3 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "imm = 0x%x [orig 0x%x] ", imm, ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_add_SPi_T3(vctx, Rd, imm, S);
		}
	} else if ((opcode & 0xff780000) == 0x44680000) {
		// add_SPr_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_add_SPr_T1) {
			uint8_t Rdm = ((opcode & 0x800000) >> 20) | ((opcode & 0x70000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 add_SPr_T1 ", opcode);
				fprintf(insn_debugging_info, "Rdm = 0x%x ", Rdm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_add_SPr_T1(vctx, Rdm);
		}
	} else if ((opcode & 0xffef8000) == 0xeb0d0000) {
		// add_SPr_T3
		decoded_insn_length = 4;
		if (handler && handler->i32_add_SPr_T3) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			uint8_t type = ((opcode & 0x30) >> 4);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 add_SPr_T3 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "type = 0x%x ", type);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_add_SPr_T3(vctx, Rd, Rm, imm, type, S);
		}
	} else if ((opcode & 0xfbf08000) == 0xf2000000) {
		// add_imm_T4
		decoded_insn_length = 4;
		if (handler && handler->i32_add_imm_T4) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint16_t imm = ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 add_imm_T4 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_add_imm_T4(vctx, Rd, Rn, imm);
		}
	} else if ((opcode & 0xfbe08000) == 0xf0000000) {
		// and_imm_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_and_imm_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			int32_t imm = thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 and_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x [orig 0x%x] ", imm, ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_and_imm_T1(vctx, Rd, Rn, imm, S);
		}
	} else if ((opcode & 0xffe08000) == 0xea000000) {
		// and_reg_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_and_reg_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			uint8_t type = ((opcode & 0x30) >> 4);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 and_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "type = 0x%x ", type);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_and_reg_T2(vctx, Rd, Rn, Rm, imm, type, S);
		}
	} else if ((opcode & 0xffef8030) == 0xea4f0020) {
		// asr_imm_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_asr_imm_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 asr_imm_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_asr_imm_T2(vctx, Rd, Rm, imm, S);
		}
	} else if ((opcode & 0xf0000000) == 0xd0000000) {
		// b_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_b_T1) {
			uint8_t imm = ((opcode & 0xff0000) >> 16);
			uint8_t cond = ((opcode & 0xf000000) >> 24);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 b_T1 ", opcode);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "cond = 0x%x ", cond);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_b_T1(vctx, imm, cond);
		}
	} else if ((opcode & 0xfbf08000) == 0xf3600000) {
		// bfi_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_bfi_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			uint8_t msb = (opcode & 0x1f);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 bfi_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "msb = 0x%x ", msb);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_bfi_T1(vctx, Rd, Rn, imm, msb);
		}
	} else if ((opcode & 0xfbe08000) == 0xf0800000) {
		// eor_imm_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_eor_imm_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			int32_t imm = thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 eor_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x [orig 0x%x] ", imm, ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_eor_imm_T1(vctx, Rd, Rn, imm, S);
		}
	} else if ((opcode & 0xffe08000) == 0xea800000) {
		// eor_reg_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_eor_reg_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			uint8_t type = ((opcode & 0x30) >> 4);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 eor_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "type = 0x%x ", type);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_eor_reg_T2(vctx, Rd, Rn, Rm, imm, type, S);
		}
	} else if ((opcode & 0xff000000) == 0xbf000000) {
		// it_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_it_T1) {
			uint8_t firstcond = ((opcode & 0xf00000) >> 20);
			uint8_t mask = ((opcode & 0xf0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 it_T1 ", opcode);
				fprintf(insn_debugging_info, "firstcond = 0x%x ", firstcond);
				fprintf(insn_debugging_info, "mask = 0x%x ", mask);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_it_T1(vctx, firstcond, mask);
		}
	} else if ((opcode & 0xfe1f0000) == 0xfc1f0000) {
		// ldc2_lit_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_ldc2_lit_T2) {
			uint8_t imm = (opcode & 0xff);
			uint8_t coproc = ((opcode & 0xf00) >> 8);
			uint8_t CRd = ((opcode & 0xf000) >> 12);
			bool D = ((opcode & 0x400000) >> 22);
			bool P = ((opcode & 0x1000000) >> 24);
			bool U = ((opcode & 0x800000) >> 23);
			bool W = ((opcode & 0x200000) >> 21);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldc2_lit_T2 ", opcode);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "coproc = 0x%x ", coproc);
				fprintf(insn_debugging_info, "CRd = 0x%x ", CRd);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldc2_lit_T2(vctx, imm, coproc, CRd, D, P, U, W);
		}
	} else if ((opcode & 0xffd02000) == 0xe8900000) {
		// ldm_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_ldm_T2) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint16_t register_list = (opcode & 0x1fff);
			bool P = ((opcode & 0x8000) >> 15);
			bool M = ((opcode & 0x4000) >> 14);
			bool W = ((opcode & 0x200000) >> 21);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldm_T2 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "register_list = 0x%x ", register_list);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldm_T2(vctx, Rn, register_list, P, M, W);
		}
	} else if ((opcode & 0xff7f0000) == 0xf85f0000) {
		// ldr_lit_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_ldr_lit_T2) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint16_t imm = (opcode & 0xfff);
			bool U = ((opcode & 0x800000) >> 23);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldr_lit_T2 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldr_lit_T2(vctx, Rt, imm, U);
		}
	} else if ((opcode & 0xfff00f00) == 0xe8500f00) {
		// ldrex_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrex_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = (opcode & 0xff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrex_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrex_T1(vctx, Rt, Rn, imm);
		}
	} else if ((opcode & 0xfff00f00) == 0xf8300e00) {
		// ldrht_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrht_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = (opcode & 0xff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrht_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrht_T1(vctx, Rt, Rn, imm);
		}
	} else if ((opcode & 0xff7f0000) == 0xf91f0000) {
		// ldrsb_lit_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrsb_lit_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint16_t imm = (opcode & 0xfff);
			bool U = ((opcode & 0x800000) >> 23);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrsb_lit_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrsb_lit_T1(vctx, Rt, imm, U);
		}
	} else if ((opcode & 0xfff00800) == 0xf9300800) {
		// ldrsh_imm_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrsh_imm_T2) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = (opcode & 0xff);
			bool P = ((opcode & 0x400) >> 10);
			bool U = ((opcode & 0x200) >> 9);
			bool W = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrsh_imm_T2 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrsh_imm_T2(vctx, Rt, Rn, imm, P, U, W);
		}
	} else if ((opcode & 0xf8000000) == 0x0) {
		// lsl_imm_T1
		decoded_insn_length = 2;
		if (handler && handler->i16_lsl_imm_T1) {
			uint8_t Rd = ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x380000) >> 19);
			uint8_t imm = ((opcode & 0x7c00000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 lsl_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_lsl_imm_T1(vctx, Rd, Rm, imm);
		}
	} else if ((opcode & 0xffef8030) == 0xea4f0000) {
		// lsl_imm_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_lsl_imm_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 lsl_imm_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_lsl_imm_T2(vctx, Rd, Rm, imm, S);
		}
	} else if ((opcode & 0xffef8030) == 0xea4f0010) {
		// lsr_imm_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_lsr_imm_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 lsr_imm_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_lsr_imm_T2(vctx, Rd, Rm, imm, S);
		}
	} else if ((opcode & 0xfff000f0) == 0xfb000000) {
		// mla_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_mla_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t Ra = ((opcode & 0xf000) >> 12);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 mla_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "Ra = 0x%x ", Ra);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_mla_T1(vctx, Rd, Rn, Rm, Ra);
		}
	} else if ((opcode & 0xfbf08000) == 0xf2400000) {
		// mov_imm_T3
		decoded_insn_length = 4;
		if (handler && handler->i32_mov_imm_T3) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint16_t imm = ((opcode & 0x4000000) >> 15) | ((opcode & 0xf7000) >> 4) | (opcode & 0xff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 mov_imm_T3 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_mov_imm_T3(vctx, Rd, imm);
		}
	} else if ((opcode & 0xfbe08000) == 0xf0600000) {
		// orn_imm_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_orn_imm_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint16_t imm = ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 orn_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_orn_imm_T1(vctx, Rd, Rn, imm, S);
		}
	} else if ((opcode & 0xffe08000) == 0xea600000) {
		// orn_reg_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_orn_reg_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			uint8_t type = ((opcode & 0x30) >> 4);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 orn_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "type = 0x%x ", type);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_orn_reg_T1(vctx, Rd, Rn, Rm, imm, type, S);
		}
	} else if ((opcode & 0xfbe08000) == 0xf0400000) {
		// orr_imm_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_orr_imm_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			int32_t imm = thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 orr_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x [orig 0x%x] ", imm, ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_orr_imm_T1(vctx, Rd, Rn, imm, S);
		}
	} else if ((opcode & 0xfff0ffc0) == 0xf810f000) {
		// pld_reg_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_pld_reg_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 pld_reg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_pld_reg_T1(vctx, Rn, Rm, imm);
		}
	} else if ((opcode & 0xffef0030) == 0xea4f0030) {
		// ror_imm_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ror_imm_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ror_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ror_imm_T1(vctx, Rd, Rm, imm, S);
		}
	} else if ((opcode & 0xfff000c0) == 0xfb100000) {
		// smlabb_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_smlabb_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t Ra = ((opcode & 0xf000) >> 12);
			bool N = ((opcode & 0x20) >> 5);
			bool M = ((opcode & 0x10) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 smlabb_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "Ra = 0x%x ", Ra);
				fprintf(insn_debugging_info, "N = 0x%x ", N);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_smlabb_T1(vctx, Rd, Rn, Rm, Ra, N, M);
		}
	} else if ((opcode & 0xfff000e0) == 0xfb200000) {
		// smlad_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_smlad_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t Ra = ((opcode & 0xf000) >> 12);
			bool M = ((opcode & 0x10) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 smlad_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "Ra = 0x%x ", Ra);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_smlad_T1(vctx, Rd, Rn, Rm, Ra, M);
		}
	} else if ((opcode & 0xfff000e0) == 0xfb300000) {
		// smlawb_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_smlawb_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t Ra = ((opcode & 0xf000) >> 12);
			bool M = ((opcode & 0x10) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 smlawb_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "Ra = 0x%x ", Ra);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_smlawb_T1(vctx, Rd, Rn, Rm, Ra, M);
		}
	} else if ((opcode & 0xfff000e0) == 0xfb400000) {
		// smlsd_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_smlsd_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t Ra = ((opcode & 0xf000) >> 12);
			bool M = ((opcode & 0x10) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 smlsd_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "Ra = 0x%x ", Ra);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_smlsd_T1(vctx, Rd, Rn, Rm, Ra, M);
		}
	} else if ((opcode & 0xfff000e0) == 0xfb500000) {
		// smmla_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_smmla_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t Ra = ((opcode & 0xf000) >> 12);
			bool R = ((opcode & 0x10) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 smmla_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "Ra = 0x%x ", Ra);
				fprintf(insn_debugging_info, "R = 0x%x ", R);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_smmla_T1(vctx, Rd, Rn, Rm, Ra, R);
		}
	} else if ((opcode & 0xfbd08000) == 0xf3000000) {
		// ssat_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ssat_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			bool shift = ((opcode & 0x200000) >> 21);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			uint8_t satimm = (opcode & 0x1f);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ssat_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "shift = 0x%x ", shift);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "satimm = 0x%x ", satimm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ssat_T1(vctx, Rd, Rn, shift, imm, satimm);
		}
	} else if ((opcode & 0xfe100000) == 0xfc000000) {
		// stc2_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_stc2_T2) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = (opcode & 0xff);
			uint8_t coproc = ((opcode & 0xf00) >> 8);
			uint8_t CRd = ((opcode & 0xf000) >> 12);
			bool P = ((opcode & 0x1000000) >> 24);
			bool N = ((opcode & 0x400000) >> 22);
			bool U = ((opcode & 0x800000) >> 23);
			bool W = ((opcode & 0x200000) >> 21);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 stc2_T2 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "coproc = 0x%x ", coproc);
				fprintf(insn_debugging_info, "CRd = 0x%x ", CRd);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "N = 0x%x ", N);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_stc2_T2(vctx, Rn, imm, coproc, CRd, P, N, U, W);
		}
	} else if ((opcode & 0xffd0a000) == 0xe9000000) {
		// stmdb_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_stmdb_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint16_t register_list = (opcode & 0x1fff);
			bool M = ((opcode & 0x4000) >> 14);
			bool W = ((opcode & 0x200000) >> 21);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 stmdb_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "register_list = 0x%x ", register_list);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_stmdb_T1(vctx, Rn, register_list, M, W);
		}
	} else if ((opcode & 0xfff00800) == 0xf8000800) {
		// strb_imm_T3
		decoded_insn_length = 4;
		if (handler && handler->i32_strb_imm_T3) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = (opcode & 0xff);
			bool P = ((opcode & 0x400) >> 10);
			bool U = ((opcode & 0x200) >> 9);
			bool W = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 strb_imm_T3 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_strb_imm_T3(vctx, Rt, Rn, imm, P, U, W);
		}
	} else if ((opcode & 0xfff00000) == 0xe8400000) {
		// strex_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_strex_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = (opcode & 0xff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 strex_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_strex_T1(vctx, Rd, Rt, Rn, imm);
		}
	} else if ((opcode & 0xfff00800) == 0xf8200800) {
		// strh_imm_T3
		decoded_insn_length = 4;
		if (handler && handler->i32_strh_imm_T3) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = (opcode & 0xff);
			bool P = ((opcode & 0x400) >> 10);
			bool U = ((opcode & 0x200) >> 9);
			bool W = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 strh_imm_T3 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_strh_imm_T3(vctx, Rt, Rn, imm, P, U, W);
		}
	} else if ((opcode & 0xfff00f00) == 0xf8400e00) {
		// strt_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_strt_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = (opcode & 0xff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 strt_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_strt_T1(vctx, Rt, Rn, imm);
		}
	} else if ((opcode & 0xffef8000) == 0xebad0000) {
		// sub_SPreg_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_sub_SPreg_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			uint8_t type = ((opcode & 0x30) >> 4);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 sub_SPreg_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "type = 0x%x ", type);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_sub_SPreg_T1(vctx, Rd, Rm, imm, type, S);
		}
	} else if ((opcode & 0xfbf08000) == 0xf2a00000) {
		// sub_imm_T4
		decoded_insn_length = 4;
		if (handler && handler->i32_sub_imm_T4) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint16_t imm = ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 sub_imm_T4 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_sub_imm_T4(vctx, Rd, Rn, imm);
		}
	} else if ((opcode & 0xfff0f080) == 0xfa20f080) {
		// sxtab16_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_sxtab16_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t rotate = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 sxtab16_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "rotate = 0x%x ", rotate);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_sxtab16_T1(vctx, Rd, Rn, Rm, rotate);
		}
	} else if ((opcode & 0xfff0f080) == 0xfa40f080) {
		// sxtab_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_sxtab_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t rotate = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 sxtab_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "rotate = 0x%x ", rotate);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_sxtab_T1(vctx, Rd, Rn, Rm, rotate);
		}
	} else if ((opcode & 0xfff0f080) == 0xfa00f080) {
		// sxtah_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_sxtah_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t rotate = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 sxtah_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "rotate = 0x%x ", rotate);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_sxtah_T1(vctx, Rd, Rn, Rm, rotate);
		}
	} else if ((opcode & 0xfff0f000) == 0xf7f0a000) {
		// udf_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_udf_T2) {
			uint16_t imm = ((opcode & 0xf0000) >> 4) | (opcode & 0xfff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 udf_T2 ", opcode);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_udf_T2(vctx, imm);
		}
	} else if ((opcode & 0xfff000f0) == 0xfb700000) {
		// usada8_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_usada8_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t Ra = ((opcode & 0xf000) >> 12);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 usada8_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "Ra = 0x%x ", Ra);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_usada8_T1(vctx, Rd, Rn, Rm, Ra);
		}
	} else if ((opcode & 0xfbd08000) == 0xf3800000) {
		// usat_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_usat_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			bool shift = ((opcode & 0x200000) >> 21);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			uint8_t satimm = (opcode & 0x1f);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 usat_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "shift = 0x%x ", shift);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "satimm = 0x%x ", satimm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_usat_T1(vctx, Rd, Rn, shift, imm, satimm);
		}
	} else if ((opcode & 0xfff0f080) == 0xfa30f080) {
		// uxtab16_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_uxtab16_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t rotate = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 uxtab16_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "rotate = 0x%x ", rotate);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_uxtab16_T1(vctx, Rd, Rn, Rm, rotate);
		}
	} else if ((opcode & 0xfff0f080) == 0xfa50f080) {
		// uxtab_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_uxtab_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t rotate = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 uxtab_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "rotate = 0x%x ", rotate);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_uxtab_T1(vctx, Rd, Rn, Rm, rotate);
		}
	} else if ((opcode & 0xfff0f080) == 0xfa10f080) {
		// uxtah_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_uxtah_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t rotate = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 uxtah_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "rotate = 0x%x ", rotate);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_uxtah_T1(vctx, Rd, Rn, Rm, rotate);
		}
	} else if ((opcode & 0xffbf0e50) == 0xeeb40a40) {
		// vcmp_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vcmp_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t Vm = (opcode & 0xf);
			bool D = ((opcode & 0x400000) >> 22);
			bool E = ((opcode & 0x80) >> 7);
			bool M = ((opcode & 0x20) >> 5);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vcmp_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "E = 0x%x ", E);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vcmp_T1(vctx, Vd, Vm, D, E, M, sz);
		}
	} else if ((opcode & 0xffbf0e50) == 0xeeb50a40) {
		// vcmp_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_vcmp_T2) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			bool D = ((opcode & 0x400000) >> 22);
			bool E = ((opcode & 0x80) >> 7);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vcmp_T2 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "E = 0x%x ", E);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vcmp_T2(vctx, Vd, D, E, sz);
		}
	} else if ((opcode & 0xffbc0e50) == 0xfebc0a40) {
		// vcvt_fltint_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vcvt_fltint_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t Vm = (opcode & 0xf);
			bool op = ((opcode & 0x80) >> 7);
			uint8_t RM = ((opcode & 0x30000) >> 16);
			bool D = ((opcode & 0x400000) >> 22);
			bool M = ((opcode & 0x20) >> 5);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vcvt_fltint_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "op = 0x%x ", op);
				fprintf(insn_debugging_info, "RM = 0x%x ", RM);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vcvt_fltint_T1(vctx, Vd, Vm, op, RM, D, M, sz);
		}
	} else if ((opcode & 0xffbc0e50) == 0xfebc0a40) {
		// vcvta_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vcvta_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t Vm = (opcode & 0xf);
			bool op = ((opcode & 0x80) >> 7);
			uint8_t RM = ((opcode & 0x30000) >> 16);
			bool D = ((opcode & 0x400000) >> 22);
			bool M = ((opcode & 0x20) >> 5);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vcvta_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "op = 0x%x ", op);
				fprintf(insn_debugging_info, "RM = 0x%x ", RM);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vcvta_T1(vctx, Vd, Vm, op, RM, D, M, sz);
		}
	} else if ((opcode & 0xfff00f10) == 0xeef00a10) {
		// vmrs_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vmrs_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vmrs_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vmrs_T1(vctx, Rt);
		}
	} else if ((opcode & 0xfff00f10) == 0xeee00a10) {
		// vmsr_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vmsr_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vmsr_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vmsr_T1(vctx, Rt);
		}
	} else if ((opcode & 0xffbf0f00) == 0xecbd0b00) {
		// vpop_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vpop_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t imm = (opcode & 0xff);
			bool D = ((opcode & 0x400000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vpop_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vpop_T1(vctx, Vd, imm, D);
		}
	} else if ((opcode & 0xffbf0f00) == 0xecbd0a00) {
		// vpop_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_vpop_T2) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t imm = (opcode & 0xff);
			bool D = ((opcode & 0x400000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vpop_T2 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vpop_T2(vctx, Vd, imm, D);
		}
	} else if ((opcode & 0xffbf0f00) == 0xed2d0b00) {
		// vpush_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vpush_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t imm = (opcode & 0xff);
			bool D = ((opcode & 0x400000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vpush_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vpush_T1(vctx, Vd, imm, D);
		}
	} else if ((opcode & 0xffbf0f00) == 0xed2d0a00) {
		// vpush_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_vpush_T2) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t imm = (opcode & 0xff);
			bool D = ((opcode & 0x400000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vpush_T2 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vpush_T2(vctx, Vd, imm, D);
		}
	} else if ((opcode & 0xffbf0e50) == 0xeeb60a40) {
		// vrintz_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vrintz_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t Vm = (opcode & 0xf);
			bool op = ((opcode & 0x80) >> 7);
			bool D = ((opcode & 0x400000) >> 22);
			bool M = ((opcode & 0x20) >> 5);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vrintz_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "op = 0x%x ", op);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vrintz_T1(vctx, Vd, Vm, op, D, M, sz);
		}
	} else if ((opcode & 0xfbe08000) == 0xf1000000) {
		// add_imm_T3
		decoded_insn_length = 4;
		if (handler && handler->i32_add_imm_T3) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			int32_t imm = thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 add_imm_T3 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x [orig 0x%x] ", imm, ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_add_imm_T3(vctx, Rd, Rn, imm, S);
		}
	} else if ((opcode & 0xff000000) == 0x44000000) {
		// add_reg_T2
		decoded_insn_length = 2;
		if (handler && handler->i16_add_reg_T2) {
			uint8_t Rdn = ((opcode & 0x800000) >> 20) | ((opcode & 0x70000) >> 16);
			uint8_t Rm = ((opcode & 0x780000) >> 19);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 16 add_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rdn = 0x%x ", Rdn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i16_add_reg_T2(vctx, Rdn, Rm);
		}
	} else if ((opcode & 0xffe08000) == 0xeb000000) {
		// add_reg_T3
		decoded_insn_length = 4;
		if (handler && handler->i32_add_reg_T3) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			uint8_t type = ((opcode & 0x30) >> 4);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 add_reg_T3 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "type = 0x%x ", type);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_add_reg_T3(vctx, Rd, Rn, Rm, imm, type, S);
		}
	} else if ((opcode & 0xfe100000) == 0xfc100000) {
		// ldc2_imm_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_ldc2_imm_T2) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = (opcode & 0xff);
			uint8_t coproc = ((opcode & 0xf00) >> 8);
			uint8_t CRd = ((opcode & 0xf000) >> 12);
			bool D = ((opcode & 0x400000) >> 22);
			bool P = ((opcode & 0x1000000) >> 24);
			bool U = ((opcode & 0x800000) >> 23);
			bool W = ((opcode & 0x200000) >> 21);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldc2_imm_T2 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "coproc = 0x%x ", coproc);
				fprintf(insn_debugging_info, "CRd = 0x%x ", CRd);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldc2_imm_T2(vctx, Rn, imm, coproc, CRd, D, P, U, W);
		}
	} else if ((opcode & 0xfff00fc0) == 0xf8500000) {
		// ldr_reg_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_ldr_reg_T2) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldr_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldr_reg_T2(vctx, Rt, Rn, Rm, imm);
		}
	} else if ((opcode & 0xff7f0000) == 0xf83f0000) {
		// ldrh_lit_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrh_lit_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint16_t imm = (opcode & 0xfff);
			bool U = ((opcode & 0x800000) >> 23);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrh_lit_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrh_lit_T1(vctx, Rt, imm, U);
		}
	} else if ((opcode & 0xff7f0000) == 0xf93f0000) {
		// ldrsh_lit_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrsh_lit_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint16_t imm = (opcode & 0xfff);
			bool U = ((opcode & 0x800000) >> 23);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrsh_lit_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrsh_lit_T1(vctx, Rt, imm, U);
		}
	} else if ((opcode & 0xffe08000) == 0xea400000) {
		// orr_reg_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_orr_reg_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			uint8_t type = ((opcode & 0x30) >> 4);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 orr_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "type = 0x%x ", type);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_orr_reg_T2(vctx, Rd, Rn, Rm, imm, type, S);
		}
	} else if ((opcode & 0xfff0ff00) == 0xf810fc00) {
		// pld_imm_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_pld_imm_T2) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = (opcode & 0xff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 pld_imm_T2 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_pld_imm_T2(vctx, Rn, imm);
		}
	} else if ((opcode & 0xfff0ff00) == 0xf910fc00) {
		// pld_immlit_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_pld_immlit_T2) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = (opcode & 0xff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 pld_immlit_T2 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_pld_immlit_T2(vctx, Rn, imm);
		}
	} else if ((opcode & 0xfff0d7ff) == 0xf3a08004) {
		// sev_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_sev_T2) {
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 sev_T2 ", opcode);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_sev_T2(vctx);
		}
	} else if ((opcode & 0xfff00800) == 0xf8400800) {
		// str_imm_T4
		decoded_insn_length = 4;
		if (handler && handler->i32_str_imm_T4) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = (opcode & 0xff);
			bool P = ((opcode & 0x400) >> 10);
			bool U = ((opcode & 0x200) >> 9);
			bool W = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 str_imm_T4 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_str_imm_T4(vctx, Rt, Rn, imm, P, U, W);
		}
	} else if ((opcode & 0xfe500000) == 0xe8400000) {
		// strd_imm_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_strd_imm_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rtx = ((opcode & 0xf00) >> 8);
			uint8_t imm = (opcode & 0xff);
			bool P = ((opcode & 0x1000000) >> 24);
			bool U = ((opcode & 0x800000) >> 23);
			bool W = ((opcode & 0x200000) >> 21);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 strd_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rtx = 0x%x ", Rtx);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_strd_imm_T1(vctx, Rt, Rn, Rtx, imm, P, U, W);
		}
	} else if ((opcode & 0xffe08000) == 0xeba00000) {
		// sub_reg_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_sub_reg_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6);
			uint8_t type = ((opcode & 0x30) >> 4);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 sub_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "type = 0x%x ", type);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_sub_reg_T2(vctx, Rd, Rn, Rm, imm, type, S);
		}
	} else if ((opcode & 0xfff000e0) == 0xe8d00000) {
		// tbb_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_tbb_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			bool H = ((opcode & 0x10) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 tbb_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "H = 0x%x ", H);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_tbb_T1(vctx, Rn, Rm, H);
		}
	} else if ((opcode & 0xffbe0e50) == 0xeeb20a40) {
		// vcvtb_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vcvtb_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t Vm = (opcode & 0xf);
			bool op = ((opcode & 0x10000) >> 16);
			bool D = ((opcode & 0x400000) >> 22);
			bool M = ((opcode & 0x20) >> 5);
			bool T = ((opcode & 0x80) >> 7);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vcvtb_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "op = 0x%x ", op);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "T = 0x%x ", T);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vcvtb_T1(vctx, Vd, Vm, op, D, M, T, sz);
		}
	} else if ((opcode & 0xff300f00) == 0xed100b00) {
		// vldr_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vldr_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t imm = (opcode & 0xff);
			bool D = ((opcode & 0x400000) >> 22);
			bool U = ((opcode & 0x800000) >> 23);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vldr_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vldr_T1(vctx, Rn, Vd, imm, D, U);
		}
	} else if ((opcode & 0xff300f00) == 0xed100a00) {
		// vldr_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_vldr_T2) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t imm = (opcode & 0xff);
			bool D = ((opcode & 0x400000) >> 22);
			bool U = ((opcode & 0x800000) >> 23);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vldr_T2 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vldr_T2(vctx, Rn, Vd, imm, D, U);
		}
	} else if ((opcode & 0xffb00e10) == 0xfe800a00) {
		// vmaxnm_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vmaxnm_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t Vn = ((opcode & 0xf0000) >> 16);
			uint8_t Vm = (opcode & 0xf);
			bool op = ((opcode & 0x40) >> 6);
			bool D = ((opcode & 0x400000) >> 22);
			bool N = ((opcode & 0x80) >> 7);
			bool M = ((opcode & 0x20) >> 5);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vmaxnm_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "Vn = 0x%x ", Vn);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "op = 0x%x ", op);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "N = 0x%x ", N);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vmaxnm_T1(vctx, Vd, Vn, Vm, op, D, N, M, sz);
		}
	} else if ((opcode & 0xffe00f10) == 0xee000a10) {
		// vmov_armsp_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vmov_armsp_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Vn = ((opcode & 0xf0000) >> 16);
			bool op = ((opcode & 0x100000) >> 20);
			bool N = ((opcode & 0x80) >> 7);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vmov_armsp_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Vn = 0x%x ", Vn);
				fprintf(insn_debugging_info, "op = 0x%x ", op);
				fprintf(insn_debugging_info, "N = 0x%x ", N);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vmov_armsp_T1(vctx, Rt, Vn, op, N);
		}
	} else if ((opcode & 0xff300f00) == 0xed000b00) {
		// vstr_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vstr_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t imm = (opcode & 0xff);
			bool D = ((opcode & 0x400000) >> 22);
			bool U = ((opcode & 0x800000) >> 23);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vstr_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vstr_T1(vctx, Rn, Vd, imm, D, U);
		}
	} else if ((opcode & 0xff300f00) == 0xed000a00) {
		// vstr_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_vstr_T2) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t imm = (opcode & 0xff);
			bool D = ((opcode & 0x400000) >> 22);
			bool U = ((opcode & 0x800000) >> 23);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vstr_T2 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vstr_T2(vctx, Rn, Vd, imm, D, U);
		}
	} else if ((opcode & 0xfff0d7ff) == 0xf3a08002) {
		// wfe_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_wfe_T2) {
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 wfe_T2 ", opcode);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_wfe_T2(vctx);
		}
	} else if ((opcode & 0xfff0d7ff) == 0xf3a08003) {
		// wfi_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_wfi_T2) {
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 wfi_T2 ", opcode);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_wfi_T2(vctx);
		}
	} else if ((opcode & 0xfff0d7ff) == 0xf3a08001) {
		// yield_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_yield_T2) {
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 yield_T2 ", opcode);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_yield_T2(vctx);
		}
	} else if ((opcode & 0xfff0d7f0) == 0xf3a080f0) {
		// dbg_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_dbg_T1) {
			uint8_t option = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 dbg_T1 ", opcode);
				fprintf(insn_debugging_info, "option = 0x%x ", option);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_dbg_T1(vctx, option);
		}
	} else if ((opcode & 0xfe5f0000) == 0xe85f0000) {
		// ldrd_lit_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrd_lit_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rtx = ((opcode & 0xf00) >> 8);
			uint8_t imm = (opcode & 0xff);
			bool P = ((opcode & 0x1000000) >> 24);
			bool U = ((opcode & 0x800000) >> 23);
			bool W = ((opcode & 0x200000) >> 21);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrd_lit_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rtx = 0x%x ", Rtx);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrd_lit_T1(vctx, Rt, Rtx, imm, P, U, W);
		}
	} else if ((opcode & 0xfff00800) == 0xf8300800) {
		// ldrh_imm_T3
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrh_imm_T3) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = (opcode & 0xff);
			bool P = ((opcode & 0x400) >> 10);
			bool U = ((opcode & 0x200) >> 9);
			bool W = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrh_imm_T3 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrh_imm_T3(vctx, Rt, Rn, imm, P, U, W);
		}
	} else if ((opcode & 0xfff00000) == 0xf9b00000) {
		// ldrsh_imm_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrsh_imm_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint16_t imm = (opcode & 0xfff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrsh_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrsh_imm_T1(vctx, Rt, Rn, imm);
		}
	} else if ((opcode & 0xfff00f00) == 0xf8500e00) {
		// ldrt_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrt_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = (opcode & 0xff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrt_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrt_T1(vctx, Rt, Rn, imm);
		}
	} else if ((opcode & 0xff100010) == 0xee000010) {
		// mcr_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_mcr_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t coproc = ((opcode & 0xf00) >> 8);
			uint8_t CRn = ((opcode & 0xf0000) >> 16);
			uint8_t CRm = (opcode & 0xf);
			uint8_t opcA = ((opcode & 0xe00000) >> 21);
			uint8_t opcB = ((opcode & 0xe0) >> 5);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 mcr_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "coproc = 0x%x ", coproc);
				fprintf(insn_debugging_info, "CRn = 0x%x ", CRn);
				fprintf(insn_debugging_info, "CRm = 0x%x ", CRm);
				fprintf(insn_debugging_info, "opcA = 0x%x ", opcA);
				fprintf(insn_debugging_info, "opcB = 0x%x ", opcB);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_mcr_T1(vctx, Rt, coproc, CRn, CRm, opcA, opcB);
		}
	} else if ((opcode & 0xff100010) == 0xee100010) {
		// mrc_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_mrc_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t coproc = ((opcode & 0xf00) >> 8);
			uint8_t Crn = ((opcode & 0xf0000) >> 16);
			uint8_t CRm = (opcode & 0xf);
			uint8_t opcA = ((opcode & 0xe00000) >> 21);
			uint8_t opcB = ((opcode & 0xe0) >> 5);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 mrc_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "coproc = 0x%x ", coproc);
				fprintf(insn_debugging_info, "Crn = 0x%x ", Crn);
				fprintf(insn_debugging_info, "CRm = 0x%x ", CRm);
				fprintf(insn_debugging_info, "opcA = 0x%x ", opcA);
				fprintf(insn_debugging_info, "opcB = 0x%x ", opcB);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_mrc_T1(vctx, Rt, coproc, Crn, CRm, opcA, opcB);
		}
	} else if ((opcode & 0xfff0f000) == 0xf890f000) {
		// pld_imm_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_pld_imm_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint16_t imm = (opcode & 0xfff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 pld_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_pld_imm_T1(vctx, Rn, imm);
		}
	} else if ((opcode & 0xfff0f000) == 0xf990f000) {
		// pld_immlit_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_pld_immlit_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint16_t imm = (opcode & 0xfff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 pld_immlit_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_pld_immlit_T1(vctx, Rn, imm);
		}
	} else if ((opcode & 0xffba0e50) == 0xeeba0a40) {
		// vcvt_fltfpt_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vcvt_fltfpt_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t imm = ((opcode & 0x20) >> 5) | ((opcode & 0xf) << 1);
			bool op = ((opcode & 0x40000) >> 18);
			bool D = ((opcode & 0x400000) >> 22);
			bool U = ((opcode & 0x10000) >> 16);
			bool sf = ((opcode & 0x100) >> 8);
			bool sx = ((opcode & 0x80) >> 7);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vcvt_fltfpt_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "op = 0x%x ", op);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "sf = 0x%x ", sf);
				fprintf(insn_debugging_info, "sx = 0x%x ", sx);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vcvt_fltfpt_T1(vctx, Vd, imm, op, D, U, sf, sx);
		}
	} else if ((opcode & 0xfe100f00) == 0xec100b00) {
		// vldm_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vldm_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t imm = (opcode & 0xff);
			bool D = ((opcode & 0x400000) >> 22);
			bool P = ((opcode & 0x1000000) >> 24);
			bool U = ((opcode & 0x800000) >> 23);
			bool W = ((opcode & 0x200000) >> 21);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vldm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vldm_T1(vctx, Rn, Vd, imm, D, P, U, W);
		}
	} else if ((opcode & 0xfe100f00) == 0xec100a00) {
		// vldm_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_vldm_T2) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t imm = (opcode & 0xff);
			bool D = ((opcode & 0x400000) >> 22);
			bool P = ((opcode & 0x1000000) >> 24);
			bool U = ((opcode & 0x800000) >> 23);
			bool W = ((opcode & 0x200000) >> 21);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vldm_T2 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vldm_T2(vctx, Rn, Vd, imm, D, P, U, W);
		}
	} else if ((opcode & 0xff800e50) == 0xfe000a00) {
		// vsel_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vsel_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t Vn = ((opcode & 0xf0000) >> 16);
			uint8_t Vm = (opcode & 0xf);
			uint8_t cond = ((opcode & 0x300000) >> 20);
			bool D = ((opcode & 0x400000) >> 22);
			bool N = ((opcode & 0x80) >> 7);
			bool M = ((opcode & 0x20) >> 5);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vsel_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "Vn = 0x%x ", Vn);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "cond = 0x%x ", cond);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "N = 0x%x ", N);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vsel_T1(vctx, Vd, Vn, Vm, cond, D, N, M, sz);
		}
	} else if ((opcode & 0xfe100f00) == 0xec000b00) {
		// vstm_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vstm_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t imm = (opcode & 0xff);
			bool D = ((opcode & 0x400000) >> 22);
			bool P = ((opcode & 0x1000000) >> 24);
			bool U = ((opcode & 0x800000) >> 23);
			bool W = ((opcode & 0x200000) >> 21);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vstm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vstm_T1(vctx, Rn, Vd, imm, D, P, U, W);
		}
	} else if ((opcode & 0xfe100f00) == 0xec000a00) {
		// vstm_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_vstm_T2) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t imm = (opcode & 0xff);
			bool D = ((opcode & 0x400000) >> 22);
			bool P = ((opcode & 0x1000000) >> 24);
			bool U = ((opcode & 0x800000) >> 23);
			bool W = ((opcode & 0x200000) >> 21);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vstm_T2 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vstm_T2(vctx, Rn, Vd, imm, D, P, U, W);
		}
	} else if ((opcode & 0xff000010) == 0xfe000000) {
		// cdp2_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_cdp2_T2) {
			uint8_t coproc = ((opcode & 0xf00) >> 8);
			uint8_t CRd = ((opcode & 0xf000) >> 12);
			uint8_t CRn = ((opcode & 0xf0000) >> 16);
			uint8_t CRm = (opcode & 0xf);
			uint8_t opcA = ((opcode & 0xf00000) >> 20);
			uint8_t opcB = ((opcode & 0xe0) >> 5);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 cdp2_T2 ", opcode);
				fprintf(insn_debugging_info, "coproc = 0x%x ", coproc);
				fprintf(insn_debugging_info, "CRd = 0x%x ", CRd);
				fprintf(insn_debugging_info, "CRn = 0x%x ", CRn);
				fprintf(insn_debugging_info, "CRm = 0x%x ", CRm);
				fprintf(insn_debugging_info, "opcA = 0x%x ", opcA);
				fprintf(insn_debugging_info, "opcB = 0x%x ", opcB);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_cdp2_T2(vctx, coproc, CRd, CRn, CRm, opcA, opcB);
		}
	} else if ((opcode & 0xfff0d0f0) == 0xf3b08020) {
		// clrex_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_clrex_T1) {
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 clrex_T1 ", opcode);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_clrex_T1(vctx);
		}
	} else if ((opcode & 0xfff0d0f0) == 0xf3b08050) {
		// dmb_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_dmb_T1) {
			uint8_t option = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 dmb_T1 ", opcode);
				fprintf(insn_debugging_info, "option = 0x%x ", option);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_dmb_T1(vctx, option);
		}
	} else if ((opcode & 0xfff0d0f0) == 0xf3b08040) {
		// dsb_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_dsb_T1) {
			uint8_t option = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 dsb_T1 ", opcode);
				fprintf(insn_debugging_info, "option = 0x%x ", option);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_dsb_T1(vctx, option);
		}
	} else if ((opcode & 0xfff0d0f0) == 0xf3b08060) {
		// isb_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_isb_T1) {
			uint8_t option = (opcode & 0xf);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 isb_T1 ", opcode);
				fprintf(insn_debugging_info, "option = 0x%x ", option);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_isb_T1(vctx, option);
		}
	} else if ((opcode & 0xfe1f0000) == 0xec1f0000) {
		// ldc_lit_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ldc_lit_T1) {
			uint8_t imm = (opcode & 0xff);
			uint8_t coproc = ((opcode & 0xf00) >> 8);
			uint8_t CRd = ((opcode & 0xf000) >> 12);
			bool D = ((opcode & 0x400000) >> 22);
			bool P = ((opcode & 0x1000000) >> 24);
			bool U = ((opcode & 0x800000) >> 23);
			bool W = ((opcode & 0x200000) >> 21);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldc_lit_T1 ", opcode);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "coproc = 0x%x ", coproc);
				fprintf(insn_debugging_info, "CRd = 0x%x ", CRd);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldc_lit_T1(vctx, imm, coproc, CRd, D, P, U, W);
		}
	} else if ((opcode & 0xfff00800) == 0xf8500800) {
		// ldr_imm_T4
		decoded_insn_length = 4;
		if (handler && handler->i32_ldr_imm_T4) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = (opcode & 0xff);
			bool P = ((opcode & 0x400) >> 10);
			bool U = ((opcode & 0x200) >> 9);
			bool W = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldr_imm_T4 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldr_imm_T4(vctx, Rt, Rn, imm, P, U, W);
		}
	} else if ((opcode & 0xfff00fc0) == 0xf8100000) {
		// ldrb_reg_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrb_reg_T2) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrb_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrb_reg_T2(vctx, Rt, Rn, Rm, imm);
		}
	} else if ((opcode & 0xfe500000) == 0xe8500000) {
		// ldrd_imm_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrd_imm_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rtx = ((opcode & 0xf00) >> 8);
			uint8_t imm = (opcode & 0xff);
			bool P = ((opcode & 0x1000000) >> 24);
			bool U = ((opcode & 0x800000) >> 23);
			bool W = ((opcode & 0x200000) >> 21);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrd_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rtx = 0x%x ", Rtx);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrd_imm_T1(vctx, Rt, Rn, Rtx, imm, P, U, W);
		}
	} else if ((opcode & 0xfff00000) == 0xf8b00000) {
		// ldrh_imm_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrh_imm_T2) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint16_t imm = (opcode & 0xfff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrh_imm_T2 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrh_imm_T2(vctx, Rt, Rn, imm);
		}
	} else if ((opcode & 0xfff00fc0) == 0xf9100000) {
		// ldrsb_reg_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrsb_reg_T2) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t Rm = (opcode & 0xf);
			uint8_t imm = ((opcode & 0x30) >> 4);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrsb_reg_T2 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "Rm = 0x%x ", Rm);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrsb_reg_T2(vctx, Rt, Rn, Rm, imm);
		}
	} else if ((opcode & 0xffb00e50) == 0xee300a00) {
		// vadd_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vadd_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t Vn = ((opcode & 0xf0000) >> 16);
			uint8_t Vm = (opcode & 0xf);
			bool D = ((opcode & 0x400000) >> 22);
			bool N = ((opcode & 0x80) >> 7);
			bool M = ((opcode & 0x20) >> 5);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vadd_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "Vn = 0x%x ", Vn);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "N = 0x%x ", N);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vadd_T1(vctx, Vd, Vn, Vm, D, N, M, sz);
		}
	} else if ((opcode & 0xffb00e50) == 0xee800a00) {
		// vdiv_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vdiv_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t Vn = ((opcode & 0xf0000) >> 16);
			uint8_t Vm = (opcode & 0xf);
			bool D = ((opcode & 0x400000) >> 22);
			bool N = ((opcode & 0x80) >> 7);
			bool M = ((opcode & 0x20) >> 5);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vdiv_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "Vn = 0x%x ", Vn);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "N = 0x%x ", N);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vdiv_T1(vctx, Vd, Vn, Vm, D, N, M, sz);
		}
	} else if ((opcode & 0xffb00e50) == 0xeeb00a00) {
		// vmov_imm_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vmov_imm_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t imm = ((opcode & 0xf0000) >> 12) | (opcode & 0xf);
			bool D = ((opcode & 0x400000) >> 22);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vmov_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vmov_imm_T1(vctx, Vd, imm, D, sz);
		}
	} else if ((opcode & 0xffb00e50) == 0xee200a00) {
		// vmul_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vmul_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t Vn = ((opcode & 0xf0000) >> 16);
			uint8_t Vm = (opcode & 0xf);
			bool D = ((opcode & 0x400000) >> 22);
			bool N = ((opcode & 0x80) >> 7);
			bool M = ((opcode & 0x20) >> 5);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vmul_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "Vn = 0x%x ", Vn);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "N = 0x%x ", N);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vmul_T1(vctx, Vd, Vn, Vm, D, N, M, sz);
		}
	} else if ((opcode & 0xffb00e50) == 0xee200a40) {
		// vnmla_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_vnmla_T2) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t Vn = ((opcode & 0xf0000) >> 16);
			uint8_t Vm = (opcode & 0xf);
			bool D = ((opcode & 0x400000) >> 22);
			bool N = ((opcode & 0x80) >> 7);
			bool M = ((opcode & 0x20) >> 5);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vnmla_T2 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "Vn = 0x%x ", Vn);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "N = 0x%x ", N);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vnmla_T2(vctx, Vd, Vn, Vm, D, N, M, sz);
		}
	} else if ((opcode & 0xffb00e50) == 0xee300a40) {
		// vsub_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vsub_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t Vn = ((opcode & 0xf0000) >> 16);
			uint8_t Vm = (opcode & 0xf);
			bool D = ((opcode & 0x400000) >> 22);
			bool N = ((opcode & 0x80) >> 7);
			bool M = ((opcode & 0x20) >> 5);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vsub_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "Vn = 0x%x ", Vn);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "N = 0x%x ", N);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vsub_T1(vctx, Vd, Vn, Vm, D, N, M, sz);
		}
	} else if ((opcode & 0xfff00000) == 0xf8d00000) {
		// ldr_imm_T3
		decoded_insn_length = 4;
		if (handler && handler->i32_ldr_imm_T3) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint16_t imm = (opcode & 0xfff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldr_imm_T3 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldr_imm_T3(vctx, Rt, Rn, imm);
		}
	} else if ((opcode & 0xfff00f00) == 0xf8100e00) {
		// ldrbt_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrbt_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = (opcode & 0xff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrbt_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrbt_T1(vctx, Rt, Rn, imm);
		}
	} else if ((opcode & 0xfff00f00) == 0xf9100e00) {
		// ldrsbt_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrsbt_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = (opcode & 0xff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrsbt_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrsbt_T1(vctx, Rt, Rn, imm);
		}
	} else if ((opcode & 0xfff00000) == 0xec400000) {
		// mcrr_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_mcrr_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t coproc = ((opcode & 0xf00) >> 8);
			uint8_t CRm = (opcode & 0xf);
			uint8_t opcA = ((opcode & 0xf0) >> 4);
			uint8_t RtB = ((opcode & 0xf0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 mcrr_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "coproc = 0x%x ", coproc);
				fprintf(insn_debugging_info, "CRm = 0x%x ", CRm);
				fprintf(insn_debugging_info, "opcA = 0x%x ", opcA);
				fprintf(insn_debugging_info, "RtB = 0x%x ", RtB);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_mcrr_T1(vctx, Rt, coproc, CRm, opcA, RtB);
		}
	} else if ((opcode & 0xfff00000) == 0xec500000) {
		// mrrc_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_mrrc_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t coproc = ((opcode & 0xf00) >> 8);
			uint8_t CRm = (opcode & 0xf);
			uint8_t opcA = ((opcode & 0xf0) >> 4);
			uint8_t RtB = ((opcode & 0xf0000) >> 16);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 mrrc_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "coproc = 0x%x ", coproc);
				fprintf(insn_debugging_info, "CRm = 0x%x ", CRm);
				fprintf(insn_debugging_info, "opcA = 0x%x ", opcA);
				fprintf(insn_debugging_info, "RtB = 0x%x ", RtB);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_mrrc_T1(vctx, Rt, coproc, CRm, opcA, RtB);
		}
	} else if ((opcode & 0xffe0d000) == 0xf3e08000) {
		// mrs_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_mrs_T1) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t SYSm = (opcode & 0xff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 mrs_T1 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "SYSm = 0x%x ", SYSm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_mrs_T1(vctx, Rd, SYSm);
		}
	} else if ((opcode & 0xffe0d000) == 0xf3808000) {
		// msr_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_msr_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t mask = ((opcode & 0xc00) >> 10);
			uint8_t SYSm = (opcode & 0xff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 msr_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "mask = 0x%x ", mask);
				fprintf(insn_debugging_info, "SYSm = 0x%x ", SYSm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_msr_T1(vctx, Rn, mask, SYSm);
		}
	} else if ((opcode & 0xffb00e10) == 0xeea00a00) {
		// vfma_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vfma_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t Vn = ((opcode & 0xf0000) >> 16);
			uint8_t Vm = (opcode & 0xf);
			bool op = ((opcode & 0x40) >> 6);
			bool D = ((opcode & 0x400000) >> 22);
			bool N = ((opcode & 0x80) >> 7);
			bool M = ((opcode & 0x20) >> 5);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vfma_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "Vn = 0x%x ", Vn);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "op = 0x%x ", op);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "N = 0x%x ", N);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vfma_T1(vctx, Vd, Vn, Vm, op, D, N, M, sz);
		}
	} else if ((opcode & 0xffb00e10) == 0xee900a00) {
		// vfnma_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vfnma_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t Vn = ((opcode & 0xf0000) >> 16);
			uint8_t Vm = (opcode & 0xf);
			bool op = ((opcode & 0x40) >> 6);
			bool D = ((opcode & 0x400000) >> 22);
			bool N = ((opcode & 0x80) >> 7);
			bool M = ((opcode & 0x20) >> 5);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vfnma_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "Vn = 0x%x ", Vn);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "op = 0x%x ", op);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "N = 0x%x ", N);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vfnma_T1(vctx, Vd, Vn, Vm, op, D, N, M, sz);
		}
	} else if ((opcode & 0xffb00e10) == 0xee000a00) {
		// vmla_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vmla_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t Vn = ((opcode & 0xf0000) >> 16);
			uint8_t Vm = (opcode & 0xf);
			bool op = ((opcode & 0x40) >> 6);
			bool D = ((opcode & 0x400000) >> 22);
			bool N = ((opcode & 0x80) >> 7);
			bool M = ((opcode & 0x20) >> 5);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vmla_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "Vn = 0x%x ", Vn);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "op = 0x%x ", op);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "N = 0x%x ", N);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vmla_T1(vctx, Vd, Vn, Vm, op, D, N, M, sz);
		}
	} else if ((opcode & 0xffb00e10) == 0xee100a00) {
		// vnmla_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_vnmla_T1) {
			uint8_t Vd = ((opcode & 0xf000) >> 12);
			uint8_t Vn = ((opcode & 0xf0000) >> 16);
			uint8_t Vm = (opcode & 0xf);
			bool op = ((opcode & 0x40) >> 6);
			bool D = ((opcode & 0x400000) >> 22);
			bool N = ((opcode & 0x80) >> 7);
			bool M = ((opcode & 0x20) >> 5);
			bool sz = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 vnmla_T1 ", opcode);
				fprintf(insn_debugging_info, "Vd = 0x%x ", Vd);
				fprintf(insn_debugging_info, "Vn = 0x%x ", Vn);
				fprintf(insn_debugging_info, "Vm = 0x%x ", Vm);
				fprintf(insn_debugging_info, "op = 0x%x ", op);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "N = 0x%x ", N);
				fprintf(insn_debugging_info, "M = 0x%x ", M);
				fprintf(insn_debugging_info, "sz = 0x%x ", sz);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_vnmla_T1(vctx, Vd, Vn, Vm, op, D, N, M, sz);
		}
	} else if ((opcode & 0xff000010) == 0xee000000) {
		// cdp_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_cdp_T1) {
			uint8_t coproc = ((opcode & 0xf00) >> 8);
			uint8_t CRd = ((opcode & 0xf000) >> 12);
			uint8_t CRn = ((opcode & 0xf0000) >> 16);
			uint8_t CRm = (opcode & 0xf);
			uint8_t opcA = ((opcode & 0xf00000) >> 20);
			uint8_t opcB = ((opcode & 0xe0) >> 5);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 cdp_T1 ", opcode);
				fprintf(insn_debugging_info, "coproc = 0x%x ", coproc);
				fprintf(insn_debugging_info, "CRd = 0x%x ", CRd);
				fprintf(insn_debugging_info, "CRn = 0x%x ", CRn);
				fprintf(insn_debugging_info, "CRm = 0x%x ", CRm);
				fprintf(insn_debugging_info, "opcA = 0x%x ", opcA);
				fprintf(insn_debugging_info, "opcB = 0x%x ", opcB);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_cdp_T1(vctx, coproc, CRd, CRn, CRm, opcA, opcB);
		}
	} else if ((opcode & 0xfe100000) == 0xec100000) {
		// ldc_imm_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ldc_imm_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = (opcode & 0xff);
			uint8_t coproc = ((opcode & 0xf00) >> 8);
			uint8_t CRd = ((opcode & 0xf000) >> 12);
			bool D = ((opcode & 0x400000) >> 22);
			bool P = ((opcode & 0x1000000) >> 24);
			bool U = ((opcode & 0x800000) >> 23);
			bool W = ((opcode & 0x200000) >> 21);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldc_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "coproc = 0x%x ", coproc);
				fprintf(insn_debugging_info, "CRd = 0x%x ", CRd);
				fprintf(insn_debugging_info, "D = 0x%x ", D);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldc_imm_T1(vctx, Rn, imm, coproc, CRd, D, P, U, W);
		}
	} else if ((opcode & 0xfff00800) == 0xf8100800) {
		// ldrb_imm_T3
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrb_imm_T3) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = (opcode & 0xff);
			bool P = ((opcode & 0x400) >> 10);
			bool U = ((opcode & 0x200) >> 9);
			bool W = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrb_imm_T3 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrb_imm_T3(vctx, Rt, Rn, imm, P, U, W);
		}
	} else if ((opcode & 0xfff00800) == 0xf9100800) {
		// ldrsb_imm_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrsb_imm_T2) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = (opcode & 0xff);
			bool P = ((opcode & 0x400) >> 10);
			bool U = ((opcode & 0x200) >> 9);
			bool W = ((opcode & 0x100) >> 8);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrsb_imm_T2 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrsb_imm_T2(vctx, Rt, Rn, imm, P, U, W);
		}
	} else if ((opcode & 0xfe100000) == 0xec000000) {
		// stc_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_stc_T1) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint8_t imm = (opcode & 0xff);
			uint8_t coproc = ((opcode & 0xf00) >> 8);
			uint8_t CRd = ((opcode & 0xf000) >> 12);
			bool P = ((opcode & 0x1000000) >> 24);
			bool N = ((opcode & 0x400000) >> 22);
			bool U = ((opcode & 0x800000) >> 23);
			bool W = ((opcode & 0x200000) >> 21);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 stc_T1 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "coproc = 0x%x ", coproc);
				fprintf(insn_debugging_info, "CRd = 0x%x ", CRd);
				fprintf(insn_debugging_info, "P = 0x%x ", P);
				fprintf(insn_debugging_info, "N = 0x%x ", N);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "W = 0x%x ", W);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_stc_T1(vctx, Rn, imm, coproc, CRd, P, N, U, W);
		}
	} else if ((opcode & 0xfbef8000) == 0xf1ad0000) {
		// sub_SPimm_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_sub_SPimm_T2) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint16_t imm = ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff);
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 sub_SPimm_T2 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_sub_SPimm_T2(vctx, Rd, imm, S);
		}
	} else if ((opcode & 0xfbf00f00) == 0xf1b00f00) {
		// cmp_imm_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_cmp_imm_T2) {
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			int32_t imm = thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 cmp_imm_T2 ", opcode);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x [orig 0x%x] ", imm, ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_cmp_imm_T2(vctx, Rn, imm);
		}
	} else if ((opcode & 0xff7f0000) == 0xf81f0000) {
		// ldrb_lit_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrb_lit_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint16_t imm = (opcode & 0xfff);
			bool U = ((opcode & 0x800000) >> 23);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrb_lit_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "U = 0x%x ", U);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrb_lit_T1(vctx, Rt, imm, U);
		}
	} else if ((opcode & 0xfff00000) == 0xf9900000) {
		// ldrsb_imm_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrsb_imm_T1) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint16_t imm = (opcode & 0xfff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrsb_imm_T1 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrsb_imm_T1(vctx, Rt, Rn, imm);
		}
	} else if ((opcode & 0xfff00000) == 0xf8900000) {
		// ldrb_imm_T2
		decoded_insn_length = 4;
		if (handler && handler->i32_ldrb_imm_T2) {
			uint8_t Rt = ((opcode & 0xf000) >> 12);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			uint16_t imm = (opcode & 0xfff);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 ldrb_imm_T2 ", opcode);
				fprintf(insn_debugging_info, "Rt = 0x%x ", Rt);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x ", imm);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_ldrb_imm_T2(vctx, Rt, Rn, imm);
		}
	} else if ((opcode & 0xfbe08000) == 0xf1a00000) {
		// sub_imm_T3
		decoded_insn_length = 4;
		if (handler && handler->i32_sub_imm_T3) {
			uint8_t Rd = ((opcode & 0xf00) >> 8);
			uint8_t Rn = ((opcode & 0xf0000) >> 16);
			int32_t imm = thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
			bool S = ((opcode & 0x100000) >> 20);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 sub_imm_T3 ", opcode);
				fprintf(insn_debugging_info, "Rd = 0x%x ", Rd);
				fprintf(insn_debugging_info, "Rn = 0x%x ", Rn);
				fprintf(insn_debugging_info, "imm = 0x%x [orig 0x%x] ", imm, ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff));
				fprintf(insn_debugging_info, "S = 0x%x ", S);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_sub_imm_T3(vctx, Rd, Rn, imm, S);
		}
	} else if ((opcode & 0xf800d000) == 0xf0008000) {
		// b_T3
		decoded_insn_length = 4;
		if (handler && handler->i32_b_T3) {
			int32_t imm = thumb_sign_extend20(((opcode & 0x4000000) >> 7) | ((opcode & 0x3f0000) >> 5) | (opcode & 0x7ff) | ((opcode & 0x2000) << 4) | ((opcode & 0x800) << 7));
			uint8_t cond = ((opcode & 0x3c00000) >> 22);
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 b_T3 ", opcode);
				fprintf(insn_debugging_info, "imm = 0x%x [orig 0x%x] ", imm, ((opcode & 0x4000000) >> 7) | ((opcode & 0x3f0000) >> 5) | (opcode & 0x7ff) | ((opcode & 0x2000) << 4) | ((opcode & 0x800) << 7));
				fprintf(insn_debugging_info, "cond = 0x%x ", cond);
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_b_T3(vctx, imm, cond);
		}
	} else if ((opcode & 0xf800d000) == 0xf0009000) {
		// b_T4
		decoded_insn_length = 4;
		if (handler && handler->i32_b_T4) {
			int32_t imm = thumb_sign_extend24_EOR(((opcode & 0x3ff0000) >> 5) | ((opcode & 0x4000000) >> 3) | (opcode & 0x7ff) | ((opcode & 0x2000) << 9) | ((opcode & 0x800) << 10));
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 b_T4 ", opcode);
				fprintf(insn_debugging_info, "imm = 0x%x [orig 0x%x] ", imm, ((opcode & 0x3ff0000) >> 5) | ((opcode & 0x4000000) >> 3) | (opcode & 0x7ff) | ((opcode & 0x2000) << 9) | ((opcode & 0x800) << 10));
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_b_T4(vctx, imm);
		}
	} else if ((opcode & 0xf800d000) == 0xf000d000) {
		// bl_T1
		decoded_insn_length = 4;
		if (handler && handler->i32_bl_T1) {
			int32_t imm = thumb_sign_extend24_EOR(((opcode & 0x3ff0000) >> 5) | ((opcode & 0x4000000) >> 3) | (opcode & 0x7ff) | ((opcode & 0x2000) << 9) | ((opcode & 0x800) << 10));
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: 32 bl_T1 ", opcode);
				fprintf(insn_debugging_info, "imm = 0x%x [orig 0x%x] ", imm, ((opcode & 0x3ff0000) >> 5) | ((opcode & 0x4000000) >> 3) | (opcode & 0x7ff) | ((opcode & 0x2000) << 9) | ((opcode & 0x800) << 10));
				fprintf(insn_debugging_info, "\t");
			}
			handler->i32_bl_T1(vctx, imm);
		}
	} else {
		if (insn_debugging_info) {
			fprintf(insn_debugging_info, "No decoding possible for opcode 0x%x\n", opcode);
		}
	}
	return decoded_insn_length;
}

// vim: set filetype=c:
