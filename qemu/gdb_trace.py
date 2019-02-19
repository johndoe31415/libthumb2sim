#!/usr/bin/python3
#
#	libthumb2sim - Emulator for the Thumb-2 ISA (Cortex-M)
#	Copyright (C) 2019-2019 Johannes Bauer
#
#	This file is part of libthumb2sim.
#
#	libthumb2sim is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; this program is ONLY licensed under
#	version 3 of the License, later versions are explicitly excluded.
#
#	libthumb2sim is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with libthumb2sim; if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#	Johannes Bauer <JohannesBauer@gmx.de>
#

import sys
import gdb

class GDBHelper(object):
	def __init__(self):
		self._uint32_t = gdb.lookup_type("unsigned int")

	def set_register(self, regno, value):
		assert(0 <= regno <= 15)
		assert(0 <= value <= 0xffffffff)
		gdb.execute("set $r%d=0x%x" % (regno, value))

	def get_register_by_name(self, regname):
		value = gdb.parse_and_eval("$%s" % (regname))

		# GDB complains if we take r13 or r15 as an integer, need to cast...
		value = value.cast(self._uint32_t)

		value = int(value)
		return value

	def get_register(self, regno):
		assert(0 <= regno <= 15)
		return self.get_register_by_name("r%d" % (regno))

	def get_registers(self):
		return { "r%d" % (i): self.get_register(i) for i in range(16) }

	def get_psr(self):
		psr = self.get_register_by_name("cpsr")
		flags = ""
		flags += "nN"[bool(psr & (1 << 31))]		# Negative
		flags += "zZ"[bool(psr & (1 << 30))]		# Zero
		flags += "cC"[bool(psr & (1 << 29))]		# Carry/borrow
		flags += "vV"[bool(psr & (1 << 28))]		# Overflow
		flags += "qQ"[bool(psr & (1 << 27))]		# DSP overflow
		return {
			"value":	psr,
			"flags":	flags,
		}

	def get_memory(self, address, length):
		data = gdb.selected_inferior().read_memory(address, length)
		data = bytes(data)
		return data

	def get_cpu_state(self):
		return {
			"regs":		self.get_registers(),
			"psr":		self.get_psr(),
		}

	def step(self):
		gdb.execute("si")

	def quit(self):
		gdb.execute("detach")
		gdb.execute("quit")

print(sys.argv)
helper = GDBHelper()
print(helper.get_cpu_state())
print(helper.get_memory(0x20000000, 128))
helper.quit()

