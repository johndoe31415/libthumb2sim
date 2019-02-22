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
import sys
import time
import subprocess
import tempfile
import socket
import hashlib
import struct
import zlib
import json
import datetime
import base64
from FriendlyArgumentParser import FriendlyArgumentParser, baseint
from DebuggingProtocol import ARMDebuggingProtocol

parser = FriendlyArgumentParser()
parser.add_argument("--bin-format", choices = [ "b64", "hex" ], default = "b64", help = "For binary data, chooses the representation inside JSON. Can be one of %(choices)s, defaults to %(default)s.")
parser.add_argument("--include-raw-data", action = "store_true", help = "By default, only the hashes of large amounts of data (such as ROM or RAM) are recorded. This option forces all that data to be included verbatim as well.")
parser.add_argument("--omit-raw-registers", action = "store_true", help = "By default, for every trace step, all register values as well as the PSR is included, not just the hashed values. This omits the raw data as well.")
parser.add_argument("--omit-rom-image", action = "store_true", help = "Do not include the binary ROM image into the trace file output.")
parser.add_argument("--rom-base", metavar = "address", type = baseint, default = 0, help = "ROM base address, defaults to 0x%(default)x bytes.")
parser.add_argument("--ram-base", metavar = "address", type = baseint, default = 0x20000000, help = "RAM base address, defaults to 0x%(default)x bytes.")
parser.add_argument("--ram-size", metavar = "length", type = baseint, default = 0x10000, help = "RAM size, defaults to 0x%(default)x bytes.")
parser.add_argument("-d", "--decimation", metavar = "step", type = int, default = 1, help = "Record only every n-th step, i.e., decimate trace data by a factor of n. By default, n is %(default)d (i.e., no decimation occurs).")
parser.add_argument("--max-insn-cnt", metavar = "cnt", type = int, default = 0, help = "Abort after a maximum of n executed instructions.")
parser.add_argument("--no-compression", action = "store_true", help = "When long raw binary is included, it is by default compressed using zlib. This option forces inclusion as uncompressed data. Short pieces (less or equal to 16 bytes) are never compressed.")
parser.add_argument("--pretty-json", action = "store_true", help = "Output a nicely formatted, human-readable JSON document.")
parser.add_argument("-e", "--emulator", choices = [ "t2sim", "qemu" ], default = "t2sim", help = "Decides whether to use libthumb2sim or QEMU as the underlying emulator.")
parser.add_argument("--emulator-binary", metavar = "filename", help = "Full path to the emulator binary. Defaults to  t2sim-gdbserver for thumb2sim and qemu-system-arm for QEMU emulation.")
parser.add_argument("-v", "--verbose", action = "count", default = 0, help = "Show more verbose output. Can be specified multiple times.")
parser.add_argument("img_filename", metavar = "image_filename", type = str, help = "Binary image which to load into QEMU for tracing")
parser.add_argument("trc_filename", metavar = "trace_filename", type = str, help = "JSON trc_filename to write")
args = parser.parse_args(sys.argv[1:])


class CustomJSONEncoder(json.JSONEncoder):
	def __init__(self, cmdline_args, **kwargs):
		json.JSONEncoder.__init__(self, **kwargs)
		self._cmdline_args = args

	def default(self, obj):
		if isinstance(obj, bytes):
			data = obj
			if len(data) <= 16:
				# Short fragments always uncompressed and as hex, prefixed with
				# ">" to indicate special handling
				data = ">" + data.hex()
			else:
				if not self._cmdline_args.no_compression:
					data = zlib.compress(data)
				if self._cmdline_args.bin_format == "b64":
					data = base64.b64encode(data).decode("ascii")
				elif self._cmdline_args.bin_format == "hex":
					data = data.hex()
				else:
					raise NotImplementedError(self._cmdline_args.bin_format)
			return data
		return json.JSONEncoder.default(self, obj)

class TraceType(object):
	def __init__(self, args):
		self._args = args

	def _custom_to_dict(self):
		return { }

	def to_dict(self):
		result = {
			"type":		self.__class__.__name__,
			"name":		self.name,
		}
		result.update(self._custom_to_dict())
		return result

	def _add_data(self, result_dict, data):
		if self._args.include_raw_data:
			result_dict["raw"] = data

class ConstantValue(TraceType):
	def __init__(self, args, name, value):
		TraceType.__init__(self, args)
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
			"value":	self._value,
		}

class RegisterHash(TraceType):
	_RegStruct = struct.Struct("< 17L")

	@property
	def name(self):
		return "register_set"

	def _custom_to_dict(self):
		return {
			"order":	"r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,r15,psr&0xf8000000",
		}

	def get(self, target):
		regs = target.get_regs()
		packed = [ regs["r%d" % (i)] for i in range(16) ]
		packed.append(regs["psr"] & 0xf8000000)
		data = self._RegStruct.pack(*packed)
		result = {
			"value":	hashlib.md5(data).digest()
		}
		self._add_data(result, data)
		if not self._args.omit_raw_registers:
			result["regs"] = regs
		return result

class MemoryHash(TraceType):
	def __init__(self, args, region_name, address, length, is_constant):
		TraceType.__init__(self, args)
		self._region_name = region_name
		self._address = address
		self._length = length
		self._is_constant = is_constant

	@property
	def name(self):
		return "memory_hash/%s" % (self._region_name)

	def _custom_to_dict(self):
		return {
			"address":		self._address,
			"length":		self._length,
			"is_constant":	self._is_constant,
		}

	def get(self, target):
		data = target.read_memory(self._address, self._length)
		result = {
			"value":	hashlib.md5(data).digest()
		}
		self._add_data(result, data)
		return result

class TraceFile(object):
	def __init__(self, args, rom_image, tracetypes):
		self._args = args
		self._rom_image = rom_image
		self._previous_state = ConstantValue(self._args, "prev_state_hash", bytes(16))
		self._tracetypes = [
			self._previous_state,
		] + list(tracetypes)
		self._trace = [ ]
		self._executed_insn_count = 0

	@property
	def trace_length(self):
		return len(self._trace)

	@property
	def executed_insn_cnt(self):
		return self._executed_insn_count

	def set_prev_state_hash(self, previous_state):
		self._previous_state.value = previous_state

	def record_state(self, target, append_to_trace = True, do_step = True):
		if do_step:
			target.singlestep()
			self._executed_insn_count += 1

		all_state = bytearray()

		components = [ ]
		for tracetype in self._tracetypes:
			component = tracetype.get(target)
			components.append(component)
			all_state += component["value"]
		state_hash = hashlib.md5(all_state).digest()
		tracepoint = {
			"executed_insns":	self._executed_insn_count,
			"components":		components,
			"state_hash":		state_hash,
		}
		if do_step:
			# Only advance the previous state when we actually did stepping.
			# Intermediate snapshots do not modify state.
			self._previous_state.value = state_hash
		else:
			tracepoint["did_step"] = False

		if append_to_trace:
			# If we omit a step, but record a tracepoint, this modifies the
			# internal state, so we need to record it.
			self._trace.append(tracepoint)

	def write(self, filename):
		data = {
			"meta": {
				"trace_date_utc":					datetime.datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ"),
				"rom_image_md5":					hashlib.md5(self._rom_image).digest(),
				"rom_image_length":					len(self._rom_image),
				"compression":						not self._args.no_compression,
				"binary_format":					self._args.bin_format,
				"emulator":							self._args.emulator,
			},
			"structure": [ component_structure.to_dict() for component_structure in self._tracetypes	],
			"trace": self._trace,
		}
		if not self._args.omit_rom_image:
			data["meta"]["raw_rom_image"] = self._rom_image

		with open(filename, "w") as f:
			encoder = CustomJSONEncoder(self._args, indent = None if (not self._args.pretty_json) else 4)
			json_data = encoder.encode(data)
			f.write(json_data)
			f.write("\n")

with open(args.img_filename, "rb") as f:
	rom_image = f.read()

trace = TraceFile(args, rom_image, [
	RegisterHash(args),
	MemoryHash(args, "rom", args.rom_base, len(rom_image), is_constant = True),
	MemoryHash(args, "ram", args.ram_base, args.ram_size, is_constant = False),
])

with tempfile.NamedTemporaryFile(prefix = "qemu_gdb_") as f:
	os.unlink(f.name)

	# Listen for QEMU debugging connection on UNIX socket
	sock = socket.socket(family = socket.AF_UNIX)
	sock.bind(f.name)
	sock.listen(1)

	# Then fire up QEMU and have it connect to socket
	try:
		if args.emulator == "qemu":
			cmd = [ args.emulator_binary or "qemu-system-arm", "-S", "-machine", "lm3s6965evb", "-display", "none", "-monitor", "none", "-gdb", "unix:%s" % (f.name), "-kernel", args.img_filename ]
		elif args.emulator == "t2sim":
			cmd = [ args.emulator_binary or "t2sim-gdbserver", args.img_filename,  f.name ]
		else:
			raise NotImplementedError(args.emulator)
		emu_process = None
		emu_process = subprocess.Popen(cmd)

		# Accept debugging connection from QEMU
		(conn, peer_address) = sock.accept()
		conn.settimeout(0.1)

		last_pc = None
		with ARMDebuggingProtocol(conn) as dbg:
			t0 = time.time()

			# Before stepping, record state initially
			trace.record_state(dbg, append_to_trace = True, do_step = False)

			while True:
				trace.record_state(dbg, append_to_trace = (trace.executed_insn_cnt % args.decimation) == 0)
				regs = dbg.get_regs()
				current_pc = regs["r15"]

				if (trace.executed_insn_cnt % 100) == 0:
					now = time.time()
					tdiff = now - t0
					if args.verbose >= 1:
						print("%d instructions, PC 0x%x, %.1f insns/sec, %d tracepoints." % (trace.executed_insn_cnt, current_pc, trace.executed_insn_cnt / tdiff, trace.trace_length), file = sys.stderr)
				if current_pc == last_pc:
					# Infinite loop (probably? what about looping instructions?), end trace.
					if args.verbose >= 1:
						print("Exiting tracing at PC 0x%x, infinite loop detected." % (current_pc), file = sys.stderr)
					break
				elif (args.max_insn_cnt != 0) and (trace.executed_insn_cnt >= args.max_insn_cnt):
					if args.verbose >= 1:
						print("Exiting tracing at PC 0x%x, %d instructions executed." % (current_pc, trace.executed_insn_cnt), file = sys.stderr)
					break
				last_pc = current_pc

			# After exiting, record state one last time
			trace.record_state(dbg, append_to_trace = True, do_step = False)

			if args.verbose >= 1:
				print("Trace finished: %d executed instructions, %d tracepoints." % (trace.executed_insn_cnt, trace.trace_length), file = sys.stderr)

	finally:
		if emu_process is not None:
			emu_process.kill()

trace.write(args.trc_filename)
