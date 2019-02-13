#ifndef __IMPL_EMULATION_H__
#define __IMPL_EMULATION_H__

struct emulationContext;
typedef void (*breakptFnc_t)(struct emulationContext *aCtx, uint8_t aBreakpoint);

struct emulationContext {
	struct CM3CPUState *cpu;
	void *localContext;
	breakptFnc_t breakpointCallback;
	bool countNextInstruction;
	bool shiftInstructionITState;
};

bool conditionallyExecuteInstruction(const struct emulationContext *aCtx);
extern const struct decodingHandler decodeOnlyCallbacks;
extern const struct decodingHandler emulationCallbacks;

#endif
