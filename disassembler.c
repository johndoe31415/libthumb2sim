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
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include "app_initialization.h"
#include "address_space.h"
#include "cpu_cm3.h"
#include "impl_disassembly.h"
#include "decoder.h"

#define MAX_ELEMENTS		32

static void outputFnc(struct disassemblyContext *aCtx, const char *aMsg, ...) {
	va_list ap;
	va_start(ap, aMsg);
	int l = strlen(aCtx->disasBuffer);
	vsnprintf(aCtx->disasBuffer + l, 128 - l, aMsg, ap);
	va_end(ap);
}

static int parseHexStr(const char *aHexStr, uint8_t *aData, int aMaxLen) {
	int l = strlen(aHexStr);
	if ((l % 2) == 1) {
		return -1;
	}
	if (l / 2 > aMaxLen) {
		return -1;
	}
	for (int i = 0; i < l; i += 2) {
		char nextChar[3];
		nextChar[0] = aHexStr[i + 0];
		nextChar[1] = aHexStr[i + 1];
		nextChar[2] = 0;
		int parsedChar;
		parsedChar = strtoul(nextChar, NULL, 16);
		aData[i / 2] = parsedChar;
	}
	return l / 2;
}

static bool executeLine(int aElemCnt, char **aElements) {
	if (aElemCnt != 6) {
		fprintf(stderr, "Warning: cannot parse line with %d elements\n", aElemCnt);
		return false;
	}
	
	uint8_t data[128];
	int parsedLen = parseHexStr(aElements[0], data, sizeof(data));
	if ((parsedLen != 2) && (parsedLen != 4)) {
		fprintf(stderr, "Warning: cannot parse '%s'\n", aElements[0]);
		return false;
	}

	int pc = atoi(aElements[1]);
	int condCode = atoi(aElements[2]);


//	const char *opcode = aElements[3];
	const char *params = aElements[4];
	const char *fullOpcode = aElements[5];


	uint32_t binOpcode;
	if (parsedLen == 4) {
		binOpcode = (data[0] << 16) | (data[1] << 24) | (data[2] << 0) | (data[3] << 8);
	} else {
		binOpcode = ((data[0] << 0) | (data[1] << 8)) << 16;
	}	


	struct disassemblyContext ctx = {
		.printDisassembly = outputFnc,
		.pc = pc,
		.itState = (condCode == -1) ? IT_NONE : IT_THEN,
		.itCond = condCode,
	};
	ctx.disasBuffer[0] = 0;
	decodeInstruction(&ctx, binOpcode, &disassemblyCallbacks);

	char expect[128];
	if (params[0] != 0) {
		snprintf(expect, 128, "%s %s", fullOpcode, params);
	} else {
		snprintf(expect, 128, "%s", fullOpcode);
	}

	bool success = strcmp(expect, ctx.disasBuffer) == 0;
	fprintf(stderr, "%6x %s%s (Expect) %s -> (Actual) %s\n\n", pc, success ? "PASS" : "FAIL", (strlen(ctx.disasBuffer) == 0) ? "-" : "x",  expect, ctx.disasBuffer);
	return success;
}

static char *strtok_empty(char *aSrc, const char *aTokens, char **aSavePtr) {
    if (aSrc) {
        *aSavePtr = aSrc;
    } else {
        aSrc = *aSavePtr;
    }
    
    if (aSrc == NULL) {
        return NULL;
    }

	int i = 0;
	while (true) {
		if ((strchr(aTokens, aSrc[i])) || (aSrc[i] == 0)) {
			/* Token found or end of string */
			if (aSrc[i] == 0) {
				*aSavePtr = NULL;
			} else {
				*aSavePtr = aSrc + i + 1;
			}
			aSrc[i] = 0;
			return aSrc;
		}
		i++;
	}
	return NULL;
}   

static bool parseLine(char *aLine) {
	/* Remove NL first */
	{
		int l = strlen(aLine);
		if ((l > 0) && (aLine[l - 1] == '\n')) {
			aLine[l - 1] = 0;
		}
	}

	char *elements[MAX_ELEMENTS];
	char *saveptr;
	char *next = strtok_empty(aLine, "|", &saveptr);
	int i = 0;
	while ((next) && (i < MAX_ELEMENTS)) {
		elements[i] = next;
		next = strtok_empty(NULL, "|", &saveptr);
		i++;
	}

	return executeLine(i, elements);
}


int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "%s [Testcase-Image]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	FILE *f = fopen(argv[1], "r");
	if (!f) {
		perror(argv[1]);
		exit(EXIT_FAILURE);
	}

	char line[1024];
	int failed = 0;
	while (fgets(line, sizeof(line) - 2, f)) {
		line[sizeof(line) - 1] = 0;
		if (line[0] == '#') {
			// Comment
			continue;
		}
		bool success = parseLine(line);
		if (!success) {
			failed++;
			fprintf(stderr, "FAiled: '%s'\n", line);
//			if (failed == 100) {
//				break;
//			}
		}
	}
	fprintf(stderr, "FAILED: %d\n", failed);

	fclose(f);
	return 0;
}
