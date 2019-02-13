#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu_cm3.h"
#include "app_initialization.h"

struct appContext {
	uint8_t *RAM;
	struct CM3CPUState savedCPUState;
	int keyOffset;
	int plaintextOffset;
	int ciphertextOffset;
	int runCnt;
	int b0, b1;
	uint8_t plainData[16];
	char filename[64];
	FILE *rnd;
};

struct supposedRun {
	const char *key;
	const char *plain;
	const char *filename;
};

static const char *keySentinel = "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c";
static const char *plaintextSentinel = "\x32\x43\xf6\xa8\x88\x5a\x30\x8d\x31\x31\x98\xa2\xe0\x37\x07\x34";
static const char *ciphertextSentinel = "\x39\x25\x84\x1D\x02\xDC\x09\xFB\xDC\x11\x85\x97\x19\x6A\x0B\x32";

void* appInitContext(void) {
	struct appContext *ctx = malloc(sizeof(struct appContext));
	memset(ctx, 0, sizeof(struct appContext));
	ctx->RAM = malloc(RAM_SIZE);
	ctx->keyOffset = -1;
	ctx->plaintextOffset = -1;
	ctx->ciphertextOffset = -1;
	ctx->rnd = fopen("/dev/urandom", "r");
	if (!ctx->rnd) {
		perror("/dev/urandom");
	}
	return ctx;
}

static void appPreInitialization(struct appContext *aAppCtx, struct CM3CPUState *aCPUState, const uint8_t *aKey, const uint8_t *aPlaintext) {
	/* Restore CPU state */
	memcpy(aCPUState, &aAppCtx->savedCPUState, sizeof(struct CM3CPUState));
	
	/* Restore RAM */
	uint8_t* ram = (uint8_t*)getMemoryAt(&aCPUState->addrSpace, RAM_BASE);
	memcpy(ram, aAppCtx->RAM, RAM_SIZE);

	/* Set key */
	if (aAppCtx->keyOffset != -1) {
		memcpy(ram + aAppCtx->keyOffset, aKey, 16);
	}
	
	/* Set plaintext */
	if (aAppCtx->plaintextOffset != -1) {
		memcpy(ram + aAppCtx->plaintextOffset, aPlaintext, 16);
	}
}

const uint8_t keys[6][16] = {
	{ 0x27, 0x81, 0x44, 0xf6, 0x8a, 0x7e, 0xd7, 0x26, 0x89, 0xc8, 0x1d, 0xa4, 0xbb, 0x5a, 0x30, 0x3b },
	{ 0x94, 0x31, 0x1c, 0xbc, 0xc0, 0xa4, 0x8e, 0xb9, 0x44, 0x96, 0x4e, 0x99, 0xc2, 0xad, 0x87, 0xb0 },
	{ 0x4b, 0xd7, 0x9b, 0x1b, 0xfc, 0x2d, 0x4f, 0xcd, 0x8d, 0xf9, 0x88, 0x27, 0xa0, 0xdc, 0x6a, 0x54 },
	{ 0x51, 0x86, 0x24, 0x6a, 0x2a, 0x6f, 0xf7, 0x69, 0x45, 0x14, 0xc2, 0x94, 0x6c, 0xb4, 0xc3, 0x0e },
	{ 0x33, 0xa3, 0xde, 0x79, 0x90, 0x25, 0xc6, 0x57, 0x44, 0xfe, 0x36, 0x48, 0x51, 0x16, 0x11, 0x00 },
	{ 0xb4, 0x87, 0x9c, 0x64, 0xc6, 0x8e, 0x90, 0xa9, 0x02, 0x64, 0x5f, 0x5c, 0xc0, 0x4c, 0x54, 0x84 }
};

#if 0
const char *appInitialization(void *aAppCtx, struct CM3CPUState *aCPUState) {
	struct appContext *ctx = (struct appContext*)aAppCtx;
	uint8_t key[16];
	ctx->runCnt++;



	char *filename = ctx->filename;
	ctx->filename[0] = 0;
	filename += sprintf(filename, "dpa/key01/");
	if (ctx->runCnt < 8196) {
		memset(ctx->plainData, 0, 16);
	}

	appPreInitialization(ctx, aCPUState, key01, ctx->plainData);

	return ctx->filename;
}
#endif

void appPostRun(void *aAppCtx, struct CM3CPUState *aCPUState) {
#if VERIFY_CIPHERTEXT
	struct appContext *ctx = (struct appContext*)aAppCtx;
	const uint8_t* ram = getMemoryAt(&aCPUState->addrSpace, RAM_BASE);
	uint8_t cipherText[16];	
	memcpy(cipherText, ram + ctx->ciphertextOffset, 16);
	fprintf(stderr, "Verified ciphertext: ");
	for (int i = 0; i < 16; i++) {
		fprintf(stderr, "%02x", cipherText[i]);
	}
	fprintf(stderr, "\n");
#endif
}

void appPreSentinelDetection(void *aAppCtx, struct CM3CPUState *aCPUState) {
	struct appContext *ctx = (struct appContext*)aAppCtx;
	memcpy(&ctx->savedCPUState, aCPUState, sizeof(struct CM3CPUState));
	const uint8_t* memory = getMemoryAt(&aCPUState->addrSpace, RAM_BASE);
	for (int i = 0; i < RAM_SIZE - 16; i++) {
		if (!memcmp(memory + i, keySentinel, 16)) {
			fprintf(stderr, "Key sentinel found at 0x%x\n", i);
			ctx->keyOffset = i;			
		} else if (!memcmp(memory + i, plaintextSentinel, 16)) {
			fprintf(stderr, "Plain text sentinel found at 0x%x\n", i);
			ctx->plaintextOffset = i;
		}
	}
}

void appPostSentinelDetection(void *aAppCtx, struct CM3CPUState *aCPUState) {
	struct appContext *ctx = (struct appContext*)aAppCtx;
	const uint8_t* memory = getMemoryAt(&aCPUState->addrSpace, RAM_BASE);
	bool found = false;
	for (int i = 0; i < RAM_SIZE - 16; i++) {
		if (!memcmp(memory + i, ciphertextSentinel, 16)) {
			found = true;
			fprintf(stderr, "Cipher text sentinel found at 0x%x\n", i);
			ctx->ciphertextOffset = i;			
		}
	}
	if (!found) {
		fprintf(stderr, "Cipher text sentinel NOT found!\n");
		abort();
	}
}

const char *hexifyBuffer(char *cBuf, const uint8_t *aData, int aLength) {
	char *writePtr = cBuf;
	writePtr[0] = 0;
	for (int i = 0; i < aLength; i++) {
		writePtr += sprintf(writePtr, "%02x", aData[i]);
	}	
	return cBuf;
}

void appRunData(void *aAppCtx, struct CM3CPUState *aCPUState, yieldFunction_t aYield, void *aYieldData) {
	struct appContext *ctx = (struct appContext*)aAppCtx;
	char filename[128];
	char plaintextStr[40];
	uint8_t plaintext[16];
	
	for (int key = 4; key < 6; key++) {

		for (int i = 0; i < 50; i++) {
			for (int j = 0; j < 1000; j++) {
				if (fread(plaintext, 16, 1, ctx->rnd) != 1) {
					perror("fread");
				}
				sprintf(filename, "dpa/key%d/rnd_%02d/%s.txt", key, i, hexifyBuffer(plaintextStr, plaintext, 16));
				appPreInitialization(ctx, aCPUState, keys[key], plaintext);
				aYield(aYieldData, filename);
			}
		}

		for (int i = 0; i < 128; i++) {
			memset(plaintext, 0, 16);
			plaintext[i / 8] |= (1 << (i % 8));
			sprintf(filename, "dpa/key%d/bit_set_1/%s.txt", key, hexifyBuffer(plaintextStr, plaintext, 16));
			appPreInitialization(ctx, aCPUState, keys[key], plaintext);
			aYield(aYieldData, filename);
		}
		
		for (int i = 0; i < 128; i++) {
			memset(plaintext, 0xff, 16);
			plaintext[i / 8] &= ~(1 << (i % 8));
			sprintf(filename, "dpa/key%d/bit_clr_1/%s.txt", key, hexifyBuffer(plaintextStr, plaintext, 16));
			appPreInitialization(ctx, aCPUState, keys[key], plaintext);
			aYield(aYieldData, filename);
		}
		
		for (int i = 0; i < 127; i++) {
			for (int j = i + 1; j < 128; j++) {
				memset(plaintext, 0, 16);
				plaintext[i / 8] |= (1 << (i % 8));
				plaintext[j / 8] |= (1 << (j % 8));
				sprintf(filename, "dpa/key%d/bit_set_2/%s.txt", key, hexifyBuffer(plaintextStr, plaintext, 16));
				appPreInitialization(ctx, aCPUState, keys[key], plaintext);
				aYield(aYieldData, filename);
			}
		}
		
		for (int i = 0; i < 127; i++) {
			for (int j = i + 1; j < 128; j++) {
				memset(plaintext, 0xff, 16);
				plaintext[i / 8] &= ~(1 << (i % 8));
				plaintext[j / 8] &= ~(1 << (j % 8));
				sprintf(filename, "dpa/key%d/bit_clr_2/%s.txt", key, hexifyBuffer(plaintextStr, plaintext, 16));
				appPreInitialization(ctx, aCPUState, keys[key], plaintext);
				aYield(aYieldData, filename);
			}
		}
	

	}
}
