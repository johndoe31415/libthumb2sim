#include <stdio.h>
#include <stdint.h>

#include "cpu_stub.h"

%for opcode in sorted(i.getopcodes()):
static void thumbOpcode${len(opcode)}_${opcode.name}(${opcode.getfullprototype()}) {
}

%endfor


void handleInstruction(struct DecodeContext *aCtx, uint32_t aOpcode) {
	if (1 == 0) {
		// Just for easier code generation
%for opcode in i.getopcodes():
	} else if ((aOpcode & ${"%#x" % (opcode.bitfield.constantmask)}) == ${"%#x" % (opcode.bitfield.constantcmp)}) {
		// ${opcode.name}
		%for (varname, variable) in opcode.itervars():
		${variable.ctype()} ${varname} = ${variable.cexpression("aOpcode")};
		%endfor
#if PRINT_INSN_DECODING == 1
		fprintf(stderr, "Insn 0x%04x: ${len(opcode)} ${opcode.name} ", aOpcode);
		%for (varname, variable) in opcode.itervars():
		%if not variable.hasextension:
		fprintf(stderr, "${varname} = 0x%x ", ${varname});
		%else:
		fprintf(stderr, "${varname} = 0x%x [orig 0x%x] ", ${varname}, ${variable.origcexpression("aOpcode")});
		%endif
		%endfor
		fprintf(stderr, "\t");
#endif
		thumbOpcode${len(opcode)}_${opcode.name}(${", ".join([ "aCtx" ] + opcode.variablenames)});
%endfor
	} else {
		fprintf(stderr, "No decoding possible for 0x%x\n", aOpcode);
	}
}

// vim: set filetype=c:
