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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include <thumb2sim.h>
#include "md5.h"

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

static void tx_frame(FILE *f, const char *data) {
	uint8_t cksum = 0;
	const unsigned int len = strlen(data);
	for (unsigned int i = 0; i < len; i++) {
		cksum += data[i];
	}
	fprintf(f, "+$%s#%02x", data, cksum);
//	fprintf(stderr, "-> %s\n", data);
}

static bool gdb_dump_register_set(FILE *f, struct emu_ctx_t *emu_ctx) {
	// Dump register set
	char reply_buffer[4096];
	reply_buffer[0] = 0;
	char *reply = reply_buffer;

	for (unsigned int i = 0; i < 16; i++) {
		reply += snprintf(reply, reply_buffer + sizeof(reply_buffer) - reply, "%08x",  htonl(emu_ctx->cpu.reg[i]));
	}
	for (unsigned int i = 0; i < 25; i++) {
		reply += snprintf(reply, reply_buffer + sizeof(reply_buffer) - reply, "00000000");
	}
	reply += snprintf(reply, reply_buffer + sizeof(reply_buffer) - reply, "%08x", htonl(emu_ctx->cpu.psr));
	tx_frame(f, reply_buffer);
	return true;
}

static char nibble_to_hexchar(uint8_t nibble) {
	nibble &= 0xf;
	if (nibble < 10) {
		return '0' + nibble;
	} else {
		return 'a' + nibble - 10;
	}
}

static void memory_to_hexstr(char *dest, const uint8_t *src, unsigned int length) {
	for (unsigned int i = 0; i < length; i++) {
		dest[(2 * i) + 0] = nibble_to_hexchar(src[i] >> 4);
		dest[(2 * i) + 1] = nibble_to_hexchar(src[i] >> 0);
	}
	dest[2 * length] = 0;
}

static bool gdb_dump_memory(FILE *f, struct emu_ctx_t *emu_ctx, uint32_t start_address, uint32_t length) {
	char reply[(2 * length) + 1];
	reply[0] = 0;

	const uint8_t *memory = addrspace_memptr(&emu_ctx->addr_space, start_address, length);
	if (memory) {
		memory_to_hexstr(reply, memory, length);
		tx_frame(f, reply);
		return true;
	} else {
		fprintf(f, "-");
		return false;
	}
}

static bool gdb_hash_memory(FILE *f, struct emu_ctx_t *emu_ctx, uint32_t start_address, uint32_t length) {
	char reply[(2 * 16) + 1];
	reply[0] = 0;

	const uint8_t *memory = addrspace_memptr(&emu_ctx->addr_space, start_address, length);
	if (memory) {
		MD5_CTX md5_ctx;
		uint8_t digest[16];

		MD5_Init(&md5_ctx);
		MD5_Update(&md5_ctx, memory, length);
		MD5_Final(digest, &md5_ctx);
		memory_to_hexstr(reply, digest, 16);
		tx_frame(f, reply);
		return true;
	} else {
		fprintf(f, "-");
		return false;
	}
}

static bool gdb_handle_command(FILE *f, struct emu_ctx_t *emu_ctx, char *msg) {
	//fprintf(stderr, "RX: '%s'\n", msg);
	if (!strcmp(msg, "g")) {
		return gdb_dump_register_set(f, emu_ctx);
	} else if (msg[0] == 'm') {
		char *comma_char = strchr(msg, ',');
		if (comma_char) {
			*comma_char = 0;
			long long int start_address = strtoll(msg + 1, NULL, 16);
			long long int length = strtoll(comma_char + 1, NULL, 16);
			gdb_dump_memory(f, emu_ctx, start_address, length);
		} else {
			fprintf(f, "-");
			return false;
		}
	} else if (!strcmp(msg, "vCont;s:1;c")) {
		cpu_single_step(emu_ctx);
		tx_frame(f, "");
	} else if (msg[0] == 'k') {
		exit(EXIT_SUCCESS);
	} else if (!strncmp(msg, "qmemhash:", 9)) {
		char *comma_char = strchr(msg + 9, ',');
		if (comma_char) {
			*comma_char = 0;
			long long int start_address = strtoll(msg + 9, NULL, 16);
			long long int length = strtoll(comma_char + 1, NULL, 16);
			gdb_hash_memory(f, emu_ctx, start_address, length);
		} else {
			fprintf(f, "-");
			return false;
		}
	} else {
		fprintf(stderr, "Unknown debugger command: %s\n", msg);
		fprintf(f, "-");
		return false;
	}
	return true;
}

static bool interpret_rxdata(FILE *f, struct emu_ctx_t *emu_ctx, char *rx_data, size_t rx_length) {
	char *hash_char = strchr(rx_data, '#');
	if ((!strncmp(rx_data, "+$", 2)) && hash_char) {
		*hash_char = 0;
		char *rx_msg = rx_data + 2;
		return gdb_handle_command(f, emu_ctx, rx_msg);
	} else {
		fprintf(f, "-");
		return false;
	}
}

static bool handle_connection(struct emu_ctx_t *ctx, int fd) {
	FILE *f = fdopen(fd, "r+");
	if (!f) {
		close(fd);
		return false;
	}
	while (true) {
		char rx_data[4096];

		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);
		struct timeval timeout = {
			.tv_usec = 500000,
		};
		int ready_fds = select(fd + 1, &readfds, NULL, NULL, &timeout);
		if (ready_fds == -1) {
			if (errno == EINTR) {
				continue;
			} else {
				perror("select");
				break;
			}
		}

		if (ready_fds == 0) {
			continue;
		}
		size_t read_bytes = fread(rx_data, 1, sizeof(rx_data) - 1, f);

		/* Always guarantee zero termination */
		rx_data[sizeof(rx_data) - 1] = 0;
		if (read_bytes < 0) {
			break;
		} else if (read_bytes > 0) {
			interpret_rxdata(f, ctx, rx_data, read_bytes);
			fflush(f);
		}
	}
	fclose(f);
	return true;
}

static bool connect_gdb(struct emu_ctx_t *emu_ctx, const char *unix_socket) {
	int sd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sd == -1) {
		perror("socket");
		return false;
	}

	struct sockaddr_un sockaddr = {
		.sun_family = AF_UNIX,
	};
	strncpy(sockaddr.sun_path, unix_socket, sizeof(sockaddr.sun_path) - 1);
	if (connect(sd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == -1) {
		fprintf(stderr, "connect \"%s\": %s\n", sockaddr.sun_path, strerror(errno));
		close(sd);
		return false;
	}
/*
	struct timeval timeout = {
		.tv_sec = 1,
		.tv_usec = 0,
	};
	if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
		perror("setsockopt");
		close(sd);
		return false;
	}
	*/
	fcntl(sd, F_SETFL, O_NONBLOCK);

	return handle_connection(emu_ctx, sd);
}

int main(int argc, char **argv) {
	if (argc != 3) {
		fprintf(stderr, "%s [rom image] [gdb UNIX socket]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	const char *rom_image_filename = argv[1];
	const char *unix_socket_filename = argv[2];
	const struct hardware_params_t cpu_parameters = {
		.rom_size_bytes = 128 * 1024,
		.ram_size_bytes = 64 * 1024,
		.ivt_base_address = 0x0,
		.rom_base_address = 0x0,
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

	emu_ctx->cpu.psr |= FLAG_ZERO;	/* QEMU sets Z at reset */
	cpu_print_state(emu_ctx);
	connect_gdb(emu_ctx, unix_socket_filename);

#if 0
	cpu_run(emu_ctx);
	cpu_print_state(emu_ctx);
//	cpu_dump_file(emu_ctx, MULTI_FILE_RW_MEMORY, "/tmp");
#endif

	free_cortexm(emu_ctx);
	return 0;
}
