#ifndef __ADDRESS_SPACE_H__
#define __ADDRESS_SPACE_H__

#include <stdint.h>
#include <stdbool.h>

#define MAX_ADDRESS_SLICES		4

struct addressSlice {
	uint32_t begin;
	uint32_t end;
	uint8_t *data;
	bool readOnly;
};

struct addressSpace {
	int sliceCnt;
	struct addressSlice slices[MAX_ADDRESS_SLICES];
};

/*************** AUTO GENERATED SECTION FOLLOWS ***************/
const uint8_t *getMemoryAt(const struct addressSpace *aAddressSpace, uint32_t aAddress);
uint8_t memRead8(struct addressSpace *aAddressSpace, uint32_t aAddress);
void memWrite8(struct addressSpace *aAddressSpace, uint32_t aAddress, uint8_t aValue);
void memWrite16(struct addressSpace *aAddressSpace, uint32_t aAddress, uint16_t aValue);
uint16_t memRead16(struct addressSpace *aAddressSpace, uint32_t aAddress);
uint32_t memRead32(struct addressSpace *aAddressSpace, uint32_t aAddress);
void memWrite32(struct addressSpace *aAddressSpace, uint32_t aAddress, uint32_t aValue);
void addMemory(struct addressSpace *aAddressSpace, uint32_t aStartAddress, uint32_t aLength, uint8_t *aData, bool aReadOnly);
void initAddressSpace(struct addressSpace *aAddressSpace);
/***************  AUTO GENERATED SECTION ENDS   ***************/

#endif
