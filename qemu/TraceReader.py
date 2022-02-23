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

from Tools import JSONTools

class TraceReaderRegisterSet():
	def __init__(self):
		self._regset = { "r%d" % (i): 0 for i in range(16) }
		self._regset["psr"] = 0

	@property
	def data(self):
		return self._regset

	def update(self, change):
		if change is None:
			return
		self._regset.update(change)

	def __repr__(self):
		return str(self._regset)

class TraceReaderMemory():
	def __init__(self, length):
		self._memory = None
		self._length = length

	@property
	def data(self):
		return self._memory

	def update(self, change):
		if change is None:
			return
		if self._memory is None:
			self._memory = change
		else:
			change.apply(self._memory)

class TraceReader():
	def __init__(self, filename):
		with open(filename) as f:
			self._trace = JSONTools.load(f)

	@property
	def rom_image(self):
		return self._trace["meta"]["raw_rom_image"]

	@property
	def rom_base(self):
		return self._trace["meta"]["rom_base"]

	@property
	def ram_base(self):
		return self._trace["meta"]["ram_base"]

	@property
	def ram_size(self):
		return self._trace["meta"]["ram_size"]

	@property
	def structure(self):
		return self._trace["meta"]["components"]

	def _initialize_states(self):
		states = [ ]
		for component in self.structure:
			if component["name"] == "register_set":
				state = TraceReaderRegisterSet()
			elif component["name"].startswith("memory/"):
				state = TraceReaderMemory(component["length"])
			else:
				raise NotImplementedError(component["name"])
			states.append(state)
		return states

	def get_state_at(self, at_insn_cnt):
		for (insn_cnt, state) in self:
			if insn_cnt == at_insn_cnt:
				return state
			elif insn_cnt > at_insn_cnt:
				return None
		return None

	def __iter__(self):
		states = self._initialize_states()
		for tracepoint in self._trace["trace"]:
			for (old_state, change) in zip(states, tracepoint["state"]):
				old_state.update(change)
			yield (tracepoint["executed_insns"], states)

	def align(self, other):
		iter1 = iter(self)
		iter2 = iter(other)

		try:
			cur1 = next(iter1)
			cur2 = next(iter2)
			while True:
				(insn1, state1) = cur1
				(insn2, state2) = cur2
				if insn1 == insn2:
					yield (insn1, state1, state2)
					cur1 = next(iter1)
					cur2 = next(iter2)
				elif insn1 < insn2:
					cur1 = next(iter1)
				elif insn2 < insn1:
					cur2 = next(iter2)
		except StopIteration:
			pass
