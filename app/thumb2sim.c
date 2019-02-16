#include <stdio.h>
#include <stdlib.h>
#include <thumb2sim.h>

struct user_ctx_t {
	bool end_emulation;
};

static void bkpt_callback(struct emu_ctx_t *emu_ctx, uint8_t bkpt_number) {
	struct user_ctx_t *usr = (struct user_ctx_t*)emu_ctx->user;
	fprintf(stderr, "Hit breakpoint %d at instruction %u.\n", bkpt_number, emu_ctx->cpu.clockcycle);
	if (bkpt_number == 2) {
		usr->end_emulation = true;
	}
}

static bool end_emulation_callback(struct emu_ctx_t *emu_ctx) {
	struct user_ctx_t *usr = (struct user_ctx_t*)emu_ctx->user;
	return usr->end_emulation;
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
	emu_ctx->user = &user;

	cpu_print_state(emu_ctx);
	cpu_run(emu_ctx);
	cpu_print_state(emu_ctx);
	free_cortexm(emu_ctx);
	return 0;
}
