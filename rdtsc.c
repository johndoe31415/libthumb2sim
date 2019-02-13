#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

uint64_t rdtsc(void) {
	uint64_t value;
	__asm__ __volatile__ (
		"xor %%rax, %%rax"			"\n\t"
		"cpuid"						"\n\t"
		"rdtsc"						"\n\t"
	: "=A" (value)
	:
	: "ebx", "ecx", "edx"
	);
	return value;
}

