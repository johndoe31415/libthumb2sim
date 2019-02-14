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
%for opcode in i.getopcodes():
	} else if ((opcode & ${"%#x" % (opcode.bitfield.constantmask)}) == ${"%#x" % (opcode.bitfield.constantcmp)}) {
		// ${opcode.name}
		decoded_insn_length = ${len(opcode) // 8};
		if (handler->i${len(opcode)}_${opcode.name}) {
			%for (varname, variable) in opcode.itervars():
			${variable.ctype()} ${varname} = ${variable.cexpression("opcode")};
			%endfor
			if (insn_debugging_info) {
				fprintf(insn_debugging_info, "Insn 0x%04x: ${len(opcode)} ${opcode.name} ", opcode);
				%for (varname, variable) in opcode.itervars():
				%if not variable.hasextension:
				fprintf(insn_debugging_info, "${varname} = 0x%x ", ${varname});
				%else:
				fprintf(insn_debugging_info, "${varname} = 0x%x [orig 0x%x] ", ${varname}, ${variable.origcexpression("opcode")});
				%endif
				%endfor
				fprintf(insn_debugging_info, "\t");
			}
			handler->i${len(opcode)}_${opcode.name}(${", ".join([ "vctx" ] + opcode.variablenames)});
		}
%endfor
	} else {
		if (insn_debugging_info) {
			fprintf(insn_debugging_info, "No decoding possible for opcode 0x%x\n", opcode);
		}
	}
	return decoded_insn_length;
}

// vim: set filetype=c:
