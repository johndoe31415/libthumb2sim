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
parser.add_argument("--bin-data", choices = [ "compressed", "uncompressed", "omit" ], default = "compressed", help = "For every trace step, include raw binary data. Can be in compressed form, uncompressed form or entirely ommited. Defaults to %(default)s.")
parser.add_argument("--bin-format", choices = [ "b64", "hex" ], default = "b64", help = "For binary data, chooses the representation inside JSON. Can be one of %(choices)s, defaults to %(default)s.")
parser.add_argument("--omit-raw-registers", action = "store_true", help = "By default, for every trace step, all register values as well as the PSR is included, not just the hashed values. This omits the raw data as well.")
parser.add_argument("--rom-base", metavar = "address", type = baseint, default = 0, help = "ROM base address, defaults to 0x%(default)x bytes.")
parser.add_argument("--ram-base", metavar = "address", type = baseint, default = 0x20000000, help = "RAM base address, defaults to 0x%(default)x bytes.")
parser.add_argument("--ram-size", metavar = "length", type = baseint, default = 0x10000, help = "RAM size, defaults to 0x%(default)x bytes.")
parser.add_argument("-d", "--decimation", metavar = "step", type = int, default = 1, help = "Record only every n-th step, i.e., decimate trace data by a factor of n. By default, n is %(default)d (i.e., no decimation occurs).")
parser.add_argument("--pretty-json", action = "store_true", help = "Output a nicely formatted, human-readable JSON document.")
parser.add_argument("img_filename", metavar = "image_filename", type = str, help = "Binary image which to load into QEMU for tracing")
parser.add_argument("trc_filename", metavar = "trace_filename", type = str, help = "JSON trc_filename to write")
args = parser.parse_args(sys.argv[1:])

class CustomJSONEncoderB64(json.JSONEncoder):
	def default(self, obj):
		if isinstance(obj, bytes):
			return base64.b64encode(obj).decode("ascii")
		return json.JSONEncoder.default(self, obj)

class CustomJSONEncoderHex(json.JSONEncoder):
	def default(self, obj):
		if isinstance(obj, bytes):
			return obj.hex()
		return json.JSONEncoder.default(self, obj)

class TraceType(object):
	def __init__(self, args):
		self._args = args

	def _add_data(self, result_dict, data):
		if self._args.bin_data == "compressed":
			result_dict.update({
				"raw":			zlib.compress(data),
			})
		elif self._args.bin_data == "uncompressed":
			result_dict.update({
				"raw":			data,
			})

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

	def get(self, target):
		data = target.read_memory(self._address, self._length)
		result = {
			"value":	hashlib.md5(data).digest()
		}
		self._add_data(result, data)
		return result

class TraceFile(object):
	def __init__(self, args, tracetypes):
		self._args = args
		self._previous_state = ConstantValue(self._args, "prev_state", bytes(16))
		self._tracetypes = [
			self._previous_state,
		] + list(tracetypes)
		self._trace = [ ]

	def set_prev_state(self, previous_state):
		self._previous_state.value = previous_state

	def record_state(self, target):
		all_state = bytearray()

		components = [ ]
		for tracetype in self._tracetypes:
			component = tracetype.get(target)
#			component["name"] = tracetype.name
			components.append(component)
			all_state += component["value"]
		state_hash = hashlib.md5(all_state).digest()
		self._previous_state.value = state_hash
		tracepoint = {
			"executed_insns":	len(self._trace),
			"components":		components,
			"state_hash":		state_hash,
		}
		self._trace.append(tracepoint)

	def write(self, filename):
		data = {
			"meta": {
				"trace_date_utc":		datetime.datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ"),
			},
			"structure": {
				# TODO: Only have repeating info (name, compression, etc) appear once
			},
			"trace": self._trace,
		}
		print(self._trace)
		with open(filename, "w") as f:
			json.dump(data, f, cls = CustomJSONEncoderB64 if (self._args.bin_format == "b64") else CustomJSONEncoderHex, sort_keys = self._args.pretty_json, indent = None if (not self._args.pretty_json) else 4)
			f.write("\n")

def get_filesize(filename):
	with open(filename, "rb") as f:
		f.seek(0, os.SEEK_END)
		return f.tell()

trace = TraceFile(args, [
	RegisterHash(args),
	MemoryHash(args, "rom", args.rom_base, get_filesize(args.img_filename), is_constant = True),
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
		qemu_process = subprocess.Popen([ "qemu-system-arm", "-S", "-machine", "lm3s6965evb", "-display", "none", "-monitor", "none", "-gdb", "unix:%s" % (f.name), "-kernel", args.img_filename ])

		# Accept debugging connection from QEMU
		(conn, peer_address) = sock.accept()
		conn.settimeout(0.1)

		with ARMDebuggingProtocol(conn) as dbg:
			trace.record_state(dbg)
			dbg.singlestep()
			trace.record_state(dbg)

	finally:
		qemu_process.kill()

trace.write(args.trc_filename)
