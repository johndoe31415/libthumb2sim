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
#include "hexdump.h"

static char hexdumpChar(char aInChar) {
	if ((aInChar <= 32) || (aInChar >= 127)) {
		return '.';
	} else {
		return aInChar;
	}
}

void hexdumpData(const unsigned char *aData, uint16_t aLength) {
	uint16_t i;
	fprintf(stderr, "%d bytes (0x%x) follow:\r\n", aLength, aLength);
	for (i = 0; i < aLength; i++) {
		if ((i % 16) == 0) {
			fprintf(stderr, "%4x   ", i);
		}
		fprintf(stderr, "%02x ", aData[i]);
		if (((i + 1) % 16) == 0) {
			uint16_t j;
			fprintf(stderr, "   ");
			for (j = i - 15; j <= i; j++) {
				fprintf(stderr, "%c", hexdumpChar(aData[j]));
			}
			fprintf(stderr, "\r\n");
		} else if (((i + 1) % 8) == 0) {
			fprintf(stderr, "   ");
		} else if (((i + 1) % 4) == 0) {
			fprintf(stderr, " ");
		}
	}
	if (aLength % 16) {
		uint16_t j;
		for (j = 0; j < 16 - (aLength % 16); j++) {
			fprintf(stderr, "   ");
		}
		if ((aLength % 16) < 4) {
			fprintf(stderr, "     ");
		} else if ((aLength % 16) < 8) {
			fprintf(stderr, "    ");
		} else if ((aLength % 16) < 12) {
			fprintf(stderr, " ");
		}
		fprintf(stderr, "   ");
		for (j = aLength - (aLength % 16); j < aLength; j++) {
			fprintf(stderr, "%c", hexdumpChar(aData[j]));
		}
		fprintf(stderr, "\r\n");
	}
}

void hexdumpDataLead(const char *aLead, const unsigned char *aData, uint16_t aLength) {
	fprintf(stderr, "%s ", aLead);
	hexdumpData(aData, aLength);
	fprintf(stderr, "\n");
}

#ifdef TEST_HEXDUMP
int main(int argc, char **argv) {
	int i;
	for (i = 0; i < 34; i++) {
		hexdumpData((unsigned char*)"Hallo du da. Wie geht es dir", i);
	}
	return 0;
}
#endif
