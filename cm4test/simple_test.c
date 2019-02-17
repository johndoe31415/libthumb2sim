/*
	libthumb2sim - Emulator for the Thumb-2 ISA (Cortex-M)
	Copyright (C) 2019-2019 Johannes Bauer

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

#include <stdbool.h>
#include <thumb2simguest.h>

int main(void) {
	__asm__ __volatile__("bkpt #1");

	/* Example of how to print strings in the emulator, for easy debugging */
	thumb2sim_puts("Hello from the Cortex-M");

	/* Example of how to write data to the emulator */
	for (uint32_t i = 0; i < 12345; i++) {
		if ((i & 255) == 0) {
			thumb2sim_write(&i, 4);
		}
	}

	/* Now we read integer values from the emulator, add them together with
	 * 12345 and send the 32-bit result back */
	struct {
		uint32_t value_a;
		uint32_t value_b;
	} integer_read;
	thumb2sim_read(&integer_read, sizeof(integer_read));
	uint32_t result = integer_read.value_a + integer_read.value_b + 12345;
	thumb2sim_write(&result, 4);

	thumb2sim_puts("Goodbyte from the Cortex-M");
	__asm__ __volatile__("bkpt #2");
	return 0;
}
