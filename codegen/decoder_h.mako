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

#ifndef __DECODER_H__
#define __DECODER_H__

#include <stdint.h>
#include <stdbool.h>

struct decoding_handler_t {
%for opcode in sorted(i.getopcodes()):
	void (*i${len(opcode)}_${opcode.name})(${opcode.getfullprototype()});
%endfor
};

int decode_insn(void *vctx, uint32_t opcode, const struct decoding_handler_t *handler, FILE *insn_debugging_info);

#endif
// vim: set filetype=c:
