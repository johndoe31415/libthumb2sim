#ifndef __IMPL_IMPLEMENTATION__
#define __IMPL_IMPLEMENTATION__

struct disassemblyContext {
	void (*printDisassembly)(struct disassemblyContext *aCtx, const char *aMsg, ...);
	uint32_t pc;
	uint8_t itState;
	uint8_t itCond;
	char disasBuffer[128];
};

extern const struct decodingHandler disassemblyCallbacks;

#endif
