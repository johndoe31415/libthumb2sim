#
#       libthumb2sim - Emulator for the Thumb-2 ISA (Cortex-M)
#       Copyright (C) 2019-2019 Johannes Bauer
#
#       This file is part of libthumb2sim.
#
#       libthumb2sim is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; this program is ONLY licensed under
#       version 3 of the License, later versions are explicitly excluded.
#
#       libthumb2sim is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#
#       You should have received a copy of the GNU General Public License
#       along with libthumb2sim; if not, write to the Free Software
#       Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#       Johannes Bauer <JohannesBauer@gmx.de>
#

import struct
import json
import hashlib
import datetime
import zlib
import base64

class CustomJSONEncoder(json.JSONEncoder):
	def __init__(self, cmdline_args, **kwargs):
		json.JSONEncoder.__init__(self, **kwargs)
		self._cmdline_args = cmdline_args

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
			"order":	"r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,r15,psr&0xf0000000",
		}

	def get(self, target):
		regs = target.get_regs()
		packed = [ regs["r%d" % (i)] for i in range(16) ]
		packed.append(regs["psr"] & 0xf0000000)
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
		if self._args.include_raw_data or (self._args.emulator == "qemu"):
			data = target.read_memory(self._address, self._length)
			result = {
				"value":	hashlib.md5(data).digest()
			}
			self._add_data(result, data)
		else:
			# We use the t2sim-specific hashing call for faster evaluation of
			# MD5 directly inside the target.
			result = {
				"value":	target.hash_memory(self._address, self._length)
			}
		return result

class TraceWriter(object):
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
				"rom_base":							self._args.rom_base,
				"ram_base":							self._args.ram_base,
				"ram_size":							self._args.ram_size,
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
