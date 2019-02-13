#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "app_initialization.h"
#include "address_space.h"
#include "cpu_cm3.h"

struct loadedFile {
	uint8_t *data;
	uint32_t length;
};

struct loadedFile loadFile(const char *aFilename, uint32_t aMaxSize, bool aAllocMax) {
	struct loadedFile lfile;
	memset(&lfile, 0, sizeof(lfile));

	struct stat sstat;
	if (stat(aFilename, &sstat) != 0) {
		fprintf(stderr, "Cannot open %s: %s\n", aFilename, strerror(errno));
		return lfile;
	}
	if (sstat.st_size > aMaxSize) {
		fprintf(stderr, "File size of %s is greater than maximum permissible (0x%x).\n", aFilename, aMaxSize);
		return lfile;
	}
	
	int allocLen = aAllocMax ? aMaxSize : sstat.st_size;
	uint8_t *data = malloc(allocLen);
	if (!data) {
		fprintf(stderr, "Memory alloc error when reading %s (tried to alloc 0x%x bytes).\n", aFilename, allocLen);
		return lfile;
	}

	memset(data, 0, allocLen);
	FILE *f = fopen(aFilename, "rb");
	if (!f) {
		fprintf(stderr, "Cannot open %s: %s\n", aFilename, strerror(errno));
		free(data);
		return lfile;
	}

	if (fread(data, sstat.st_size, 1, f) != 1) {
		fprintf(stderr, "Error reading %s: %s\n", aFilename, strerror(errno));
		free(data);
		return lfile;
	}

	lfile.data = data;
	lfile.length = allocLen;
	return lfile;
}

struct YieldData {
	void *appContext;
	struct CM3CPUState *cpuState;
};

static void mkbasedir(const char *aFilename) {
	int l = strlen(aFilename);
	char path[l + 1];
	strcpy(path, aFilename);

	for (int i = 0; i < l; i++) {
		if (path[i] == '/') {
			path[i] = 0;
			mkdir(path, 0755);
			path[i] = '/';			
		}
	}
}

static void yieldFunction(void *aYieldData, const char *aFilename) {
	struct YieldData *yd = (struct YieldData*)aYieldData;
	
	mkbasedir(aFilename);

	cpuRun(yd->cpuState, aFilename, false);
	appPostRun(yd->appContext, yd->cpuState);
}


int main(int argc, char **argv) {
	if (argc < 3) {
		fprintf(stderr, "%s [ROM-Image] [RAM-Image]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Load file images */
	struct loadedFile ram, rom;
	rom = loadFile(argv[1], ROM_SIZE, false);
	if (strcmp(argv[2], "-")) {
		ram = loadFile(argv[2], RAM_SIZE, true);	
	} else {
		ram.data = calloc(RAM_SIZE, 1);
		if (!ram.data) {
			fprintf(stderr, "Cannot alloc RAM\n");
			exit(EXIT_FAILURE);
		}
		ram.length = RAM_SIZE;
	}

	if (rom.length == 0) {
		fprintf(stderr, "Could not init ROM!\n");
		exit(EXIT_FAILURE);
	}
	if (ram.length == 0) {
		fprintf(stderr, "Could not init RAM!\n");
		exit(EXIT_FAILURE);
	}

	fprintf(stderr, "RAM at %p (0x%x bytes), ROM at %p (0x%x bytes)\n", ram.data, ram.length, rom.data, rom.length);
	
	struct CM3CPUState cpuState;
	memset(&cpuState, 0, sizeof(struct CM3CPUState));
	initAddressSpace(&cpuState.addrSpace);
	addMemory(&cpuState.addrSpace, ROM_BASE, rom.length, rom.data, true);
	addMemory(&cpuState.addrSpace, 0x08000000, rom.length, rom.data, true);
	addMemory(&cpuState.addrSpace, RAM_BASE, ram.length, ram.data, false);
	fprintf(stderr, "Address space initialized.\n");

	void *appContext = appInitContext();

	
	int runs = 50000;
	if (argc >= 4) {
		runs = atoi(argv[3]);
	}
	for (int i = 0; i < runs; i++) {
		cpuReset(&cpuState);
//		fprintf(stderr, "CPU initialized and reset: SP = 0x%x, PC = 0x%x.\n", cpuState.reg[REG_SP], cpuState.reg[REG_PC]);

//		fprintf(stderr, "Application prepared for first run: SP = 0x%x, PC = 0x%x.\n", cpuState.reg[REG_SP], cpuState.reg[REG_PC]);
	
		cpuRun(&cpuState, NULL, false);
//		cpuRun(&cpuState, "tracefile.txt", true);
	}
//	appPreSentinelDetection(appContext, &cpuState);
//	cpuRun(&cpuState, "tracefile.txt", false);
//	appPostSentinelDetection(appContext, &cpuState);

	struct YieldData yd = {
		.appContext = appContext,
		.cpuState = &cpuState,
	};
	appRunData(appContext, &cpuState, yieldFunction, &yd);
	return 0;
}

