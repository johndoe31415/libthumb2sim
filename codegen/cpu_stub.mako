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
