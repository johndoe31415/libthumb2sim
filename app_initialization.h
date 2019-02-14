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

#ifndef __CPU_INITIALIZATION_H__
#define __CPU_INITIALIZATION_H__

#include "cpu_cm3.h"

typedef void yieldFunction_t(void *aYieldData, const char *aFilename);

/*************** AUTO GENERATED SECTION FOLLOWS ***************/
void* appInitContext(void);
const char *appInitialization(void *aAppCtx, struct CM3CPUState *aCPUState);
void appPostRun(void *aAppCtx, struct CM3CPUState *aCPUState);
void appPreSentinelDetection(void *aAppCtx, struct CM3CPUState *aCPUState);
void appPostSentinelDetection(void *aAppCtx, struct CM3CPUState *aCPUState);
const char *hexifyBuffer(char *cBuf, const uint8_t *aData, int aLength);
void appRunData(void *aAppCtx, struct CM3CPUState *aCPUState, yieldFunction_t aYield, void *aYieldData);
/***************  AUTO GENERATED SECTION ENDS   ***************/

#endif
