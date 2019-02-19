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

import os
import time
import subprocess
import tempfile
import socket
import hashlib
import struct
from DebuggingProtocol import ARMDebuggingProtocol

class ConstantValue(object):
	def __init__(self, name, value):
		assert(isinstance(value, bytes))
		self._name = name
		self._value = value

	@property
	def name(self):
		return self._name

	@property
	def value(self):
		return self._value

	@value.setter
	def value(self, value):
		assert(isinstance(value, bytes))
		self._value = value

	def get(self, target):
		return {
			"result":	self._value,
		}

class RegisterHash(object):
	_RegStruct = struct.Struct("< 17L")
	def __init__(self):
		pass

	@property
	def name(self):
		return "register_set"

	def get(self, target):
		regs = target.get_regs()
		packed = [ regs["r%d" % (i)] for i in range(16) ]
		packed.append(regs["psr"] & 0xf8000000)
		data = self._RegStruct.pack(*packed)
		return {
			"regs":		regs,
			"data":		data,
			"result":	hashlib.md5(data).digest()
		}

class MemoryHash(object):
	def __init__(self, name, address, length, is_constant):
		self._name = name
		self._address = address
		self._length = length
		self._is_constant = is_constant

	@property
	def name(self):
		return "memory_hash"

	def get(self, target):
		data = target.read_memory(self._address, self._length)
		return {
			"regs":		regs,
			"data":		data,
			"result":	hashlib.md5(data).digest()
		}


class TraceFile(object):
	def __init__(self, tracetypes):
		self._previous_state = ConstantValue("prev_state", bytes(16))
		self._tracetypes = [
			self._previous_state,
		] + list(tracetypes)
		self._trace = [ ]

	def set_prev_state(self, previous_state):
		self._previous_state.value = previous_state

	def determine_state(self, target):
		all_state = bytearray()
		for tracetype in self._tracetypes:
			tracehash = tracetype.get(target)
			print(tracehash)
			#all_state += tracehash
		print(all_state)
		state_hash = hashlib.md5(all_state).digest()
		print(state_hash)
		return state_hash

trace = TraceFile([
	RegisterHash(),
	MemoryHash("rom", 0x0, 0x800, is_constant = True),
	MemoryHash("ram", 0x20000000, 0x10000, is_constant = False),
])

with tempfile.NamedTemporaryFile(prefix = "qemu_gdb_") as f:
	os.unlink(f.name)

	# Listen for QEMU debugging connection on UNIX socket
	sock = socket.socket(family = socket.AF_UNIX)
	sock.bind(f.name)
	sock.listen(1)

	# Then fire up QEMU and have it connect to socket
	try:
		qemu_process = subprocess.Popen([ "qemu-system-arm", "-S", "-machine", "lm3s6965evb", "-display", "none", "-monitor", "none", "-gdb", "unix:%s" % (f.name), "-kernel", "qemu_test.bin" ])

		# Accept debugging connection from QEMU
		(conn, peer_address) = sock.accept()
		conn.settimeout(0.1)

		with ARMDebuggingProtocol(conn) as dbg:
			trace.determine_state(dbg)
#			dbg.dump_regs()
#			dbg.singlestep()

	finally:
		qemu_process.kill()
