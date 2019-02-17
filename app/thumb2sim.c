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

#include <stdio.h>
#include <stdlib.h>
#include <thumb2sim.h>

/* Using this allows to to end emulation prematurely */
struct user_ctx_t {
	bool end_emulation;
};

static void bkpt_callback(struct emu_ctx_t *emu_ctx, uint8_t bkpt_number) {
	struct user_ctx_t *usr = (struct user_ctx_t*)emu_ctx->user;
	if (bkpt_number != 255) {
		fprintf(stderr, "Hit breakpoint %d at instruction %u.\n", bkpt_number, emu_ctx->cpu.insn_ctr);
	}
	if  (emu_ctx->cpu.insn_ctr >= 1000000) {
		/* Abort after a million instructions */
		fprintf(stderr, "We're tired. Quitting emulation.\n");
		usr->end_emulation = true;
	}
}

static bool end_emulation_callback(struct emu_ctx_t *emu_ctx) {
	struct user_ctx_t *usr = (struct user_ctx_t*)emu_ctx->user;
	return usr->end_emulation;
}

static uint32_t syscall_read(struct emu_ctx_t *emu_ctx, void *data, uint32_t max_length) {
	fprintf(stderr, "Guest read: max of %d bytes, write to %p.\n", max_length, data);
	if (max_length == 8) {
		/* 12345 + 75844 + 12257489 = 12345678 */
		uint32_t *int_data = (uint32_t*)data;
		int_data[0] = 75844;
		int_data[1] = 12257489;
	}
	return 0;
}

static void syscall_write(struct emu_ctx_t *emu_ctx, const void *data, uint32_t length) {
	if (length == 4) {
		fprintf(stderr, "Guest write: %d bytes, dereferenced uint32_t value: %u\n", length, *((uint32_t*)data));
	} else {
		fprintf(stderr, "Guest write: %d bytes: ", length);
		for (uint32_t i = 0; i < length; i++) {
			fprintf(stderr, "%02x ", ((const uint8_t*)data)[i]);
		}
		fprintf(stderr, "\n");
	}
}

static void syscall_puts(struct emu_ctx_t *emu_ctx, const char *msg) {
	fprintf(stderr, "Guest puts: \"%s\"\n", msg);
}

static void syscall_exit(struct emu_ctx_t *emu_ctx, uint32_t status) {
	struct user_ctx_t *usr = (struct user_ctx_t*)emu_ctx->user;
	fprintf(stderr, "Emulation exit with status: %d\n", status);
	usr->end_emulation = true;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "%s [rom image]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	const char *rom_image_filename = argv[1];
	const struct hardware_params_t cpu_parameters = {
		.rom_size_bytes = 1024 * 1024,
		.ram_size_bytes = 128 * 1024,
		.ivt_base_address = 0x08000000,
		.rom_base_address = 0x08000000,
		.ram_base_address = 0x20000000,
		.rom_image_filename = rom_image_filename,
		.ram_image_filename = NULL,
	};

	struct user_ctx_t user = {
		.end_emulation = false,
	};
	struct emu_ctx_t *emu_ctx = init_cortexm(&cpu_parameters);
	emu_ctx->bkpt_callback = bkpt_callback;
	emu_ctx->end_emulation_callback = end_emulation_callback;
	emu_ctx->emulator_syscall_read = syscall_read;
	emu_ctx->emulator_syscall_write = syscall_write;
	emu_ctx->emulator_syscall_puts = syscall_puts;
	emu_ctx->emulator_syscall_exit = syscall_exit;
	emu_ctx->user = &user;

	cpu_print_state(emu_ctx);

	cpu_run(emu_ctx);
	cpu_print_state(emu_ctx);
//	cpu_dump_file(emu_ctx, MULTI_FILE_RW_MEMORY, "/tmp");

	free_cortexm(emu_ctx);
	return 0;
}
