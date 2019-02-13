#ifndef __CPU_INITIALIZATION_H__
#define __CPU_INITIALIZATION_H__

#include "cpu_cm3.h"

typedef void yieldFunction_t(void *aYieldData, const char *aFilename);

/*************** AUTO GENERATED SECTION FOLLOWS ***************/
struct appContext;
struct supposedRun;
void* appInitContext(void);
const char *appInitialization(void *aAppCtx, struct CM3CPUState *aCPUState);
void appPostRun(void *aAppCtx, struct CM3CPUState *aCPUState);
void appPreSentinelDetection(void *aAppCtx, struct CM3CPUState *aCPUState);
void appPostSentinelDetection(void *aAppCtx, struct CM3CPUState *aCPUState);
void appRunData(void *aAppCtx, struct CM3CPUState *aCPUState, yieldFunction_t aYield, void *aYieldData);
/***************  AUTO GENERATED SECTION ENDS   ***************/

#endif
