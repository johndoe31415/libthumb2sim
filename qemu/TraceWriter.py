#
#       libthumb2sim - Emulator for the Thumb-2 ISA (Cortex-M)
#       Copyright (C) 2019-2022 Johannes Bauer
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
from Tools import JSONTools
from BytesDiff import BytesDiff

class TraceComponent():
	@property
	def properties(self):
		return None

	def get(self, target):
		raise NotImplementedError()


class TraceRegisterSet(TraceComponent):
	@property
	def name(self):
		return "register_set"

	@property
	def properties(self):
		return {
			"name": self.name,
		}

	def get(self, target):
		return target.get_regs()


class TraceMemory(TraceComponent):
	def __init__(self, region_name, address, length, is_constant = False):
		self._region_name = region_name
		self._address = address
		self._length = length
		self._is_constant = is_constant
		self._cached_memory = None

	@property
	def name(self):
		return "memory/%s" % (self._region_name)

	@property
	def properties(self):
		return {
			"name":			self.name,
			"address":		self._address,
			"length":		self._length,
			"is_constant":	self._is_constant,
		}

	def _get_memory(self, target):
		return target.read_memory(self._address, self._length)

	def get(self, target):
		if not self._is_constant:
			return self._get_memory(target)
		else:
			if self._cached_memory is None:
				self._cached_memory = self._get_memory(target)
			return self._cached_memory

class TraceWriter():
	def __init__(self, args, rom_image, trace_components):
		self._args = args
		self._rom_image = rom_image
		self._trace_components = trace_components
		self._last_component_state = [ None for _ in range(len(self._trace_components)) ]
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

	def _dict_diff(self, old, new):
		diff_dict = { }
		for key in new.keys():
			if old[key] != new[key]:
				diff_dict[key] = new[key]
		return diff_dict

	def _diff_state(self, old, new):
		if old is None:
			return new
		if old == new:
			return None
		if isinstance(old, bytes):
			return BytesDiff.diff_data(old, new)
		else:
			return self._dict_diff(old, new)
		return new

	def record_state(self, target, append_to_trace = True, do_step = True):
		if do_step:
			target.singlestep()
			self._executed_insn_count += 1

		if append_to_trace:
			# We might want to step only and record every nth state
			component_state = [ component.get(target) for component in self._trace_components ]
			diffed_component_state = [ self._diff_state(old, new) for (old, new) in zip(self._last_component_state, component_state) ]
			tracepoint = {
				"executed_insns":	self._executed_insn_count,
				"state":			diffed_component_state,
			}
			self._last_component_state = component_state
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
				"emulator":							self._args.emulator,
				"version":							2,
				"components":						[ component.properties for component in self._trace_components ],
				"raw_rom_image":					self._rom_image,
			},
			"trace": self._trace,
		}
		with open(filename, "w") as f:
			JSONTools.dump(data, f)
