/*
	libthumb2sim - Emulator for the Thumb-2 ISA (Cortex-M)
	Copyright (C) 2014-2019 Johannes Bauer

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

#ifndef __THUMB2SIMGUEST_H__
#define __THUMB2SIMGUEST_H__

#include <stdio.h>
#include <stdint.h>

enum thumb2sim_syscall_t {
	SYSCALL_GUEST_READ = 0,
	SYSCALL_GUEST_WRITE = 1,
	SYSCALL_GUEST_PUTS = 2,
	SYSCALL_GUEST_EXIT = 3,
};

uint32_t thumb2sim_syscall(uint32_t syscall_no, void *data_ptr, uint32_t length);

#define thumb2sim_read(data, max_length)	thumb2sim_syscall(SYSCALL_GUEST_READ, (data), (max_length))
#define thumb2sim_write(data, length)		thumb2sim_syscall(SYSCALL_GUEST_WRITE, (void*)(data), (length))
#define thumb2sim_puts(msg)					thumb2sim_syscall(SYSCALL_GUEST_PUTS, (msg), 0)
#define thumb2sim_exit(retcode)				thumb2sim_syscall(SYSCALL_GUEST_EXIT, NULL, (retcode))

#endif
