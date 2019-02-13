#include <stdio.h>
#include <stdint.h>

#include "decoder.h"
#include "config.h"

static uint32_t ror32(uint32_t aIn, uint8_t aRor) {
	uint32_t aLoMask = (1 << aRor) - 1;
	uint32_t aHiMask = ((1 << (32 - aRor)) - 1) << aRor;
	return ((aIn & aLoMask) << (32 - aRor)) | ((aIn & aHiMask) >> aRor);
}

static int32_t ThumbExpandImm(uint16_t aImm) {
	if ((aImm & 0xc00) == 0x0) {
		if ((aImm & 0x300) == 0x0) {
			return aImm & 0xff;
		} else if ((aImm & 0x300) == 0x100) {
			return ((aImm & 0xff) << 16) | (aImm & 0xff);
		} else if ((aImm & 0x300) == 0x200) {
			return ((aImm & 0xff) << 24) | ((aImm & 0xff) << 8);
		} else {
			return ((aImm & 0xff) << 24) | ((aImm & 0xff) << 16) | ((aImm & 0xff) << 8) | ((aImm & 0xff) << 0);
		}
	} else {
		/* Rotate thingy */
		return ror32(0x80 | (aImm & 0x7f), (aImm & 0xf80) >> 7);
	}
}

static int32_t SignExtend(uint32_t aImm, uint8_t aBitLen) {
	if (aImm & (1 << (aBitLen - 1))) {
		return aImm | ~((1 << aBitLen) - 1);
	} else {
		return aImm;
	}
}

static int32_t SignExtend20(uint32_t aImm) {
	return SignExtend(aImm, 20);
}

static int32_t SignExtend24_EOR(uint32_t aImm) {
	aImm ^= ((1 << 21) ^ ((aImm & (1 << 23)) >> 2));
	aImm ^= ((1 << 22) ^ ((aImm & (1 << 23)) >> 1));
	return SignExtend(aImm, 24);
}

int decodeInstruction(void *aCtx, uint32_t aOpcode, const struct decodingHandler *aHandler, FILE *instructionDebuggingInfo) {
	int decodedInstructionLength = 0;
	if (1 == 0) {
		// Just for easier code generation
%for opcode in i.getopcodes():
	} else if ((aOpcode & ${"%#x" % (opcode.bitfield.constantmask)}) == ${"%#x" % (opcode.bitfield.constantcmp)}) {
		// ${opcode.name}
		decodedInstructionLength = ${len(opcode) // 8};
		if (aHandler->i${len(opcode)}_${opcode.name}) {
			%for (varname, variable) in opcode.itervars():
			${variable.ctype()} ${varname} = ${variable.cexpression("aOpcode")};
			%endfor
			if (instructionDebuggingInfo) {
				fprintf(instructionDebuggingInfo, "Insn 0x%04x: ${len(opcode)} ${opcode.name} ", aOpcode);
				%for (varname, variable) in opcode.itervars():
				%if not variable.hasextension:
				fprintf(instructionDebuggingInfo, "${varname} = 0x%x ", ${varname});
				%else:
				fprintf(instructionDebuggingInfo, "${varname} = 0x%x [orig 0x%x] ", ${varname}, ${variable.origcexpression("aOpcode")});
				%endif
				%endfor
				fprintf(instructionDebuggingInfo, "\t");
			}
			aHandler->i${len(opcode)}_${opcode.name}(${", ".join([ "aCtx" ] + opcode.variablenames)});
		}
%endfor
	} else {
		if (instructionDebuggingInfo) {
			fprintf(instructionDebuggingInfo, "No decoding possible for opcode 0x%x\n", aOpcode);
		}
	}
	return decodedInstructionLength;
}

// vim: set filetype=c:
