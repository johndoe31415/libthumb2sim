#ifndef __DECODER_H__
#define __DECODER_H__

#include <stdint.h>
#include <stdbool.h>

struct decodingHandler {
%for opcode in sorted(i.getopcodes()):
	void (*i${len(opcode)}_${opcode.name})(${opcode.getfullprototype()});
%endfor
};

int decodeInstruction(void *aCtx, uint32_t aOpcode, const struct decodingHandler *aHandler);

#endif
// vim: set filetype=c:
