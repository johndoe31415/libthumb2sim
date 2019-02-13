#include <stdio.h>
#include <stdint.h>

#include "decoder.h"
#include "impl_implementation.h"

%for opcode in sorted(i.getopcodes()):
static void implementation_i${len(opcode)}_${opcode.name}(${opcode.getfullprototype()}) {
	struct implementationContext *ctx = (struct implementationContext*)aCtx;
}

%endfor

const struct decodingHandler implementationCallbacks = {
%for opcode in sorted(i.getopcodes()):
	.i${len(opcode)}_${opcode.name} = implementation_i${len(opcode)}_${opcode.name},
%endfor
};

// vim: set filetype=c:
