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

import zlib
import json
import base64

class TraceReader(object):
	def __init__(self, filename):
		with open(filename) as f:
			self._trace = json.load(f)
		self._tracepoint_by_insn_cnt = { tracepoint["executed_insns"]: tracepoint for tracepoint in self._trace["trace"] }

	@property
	def rom_image(self):
		rom_image = self._trace["meta"].get("raw_rom_image")
		if rom_image is not None:
			return self.getbytes(rom_image)
		else:
			return None

	@property
	def rom_base(self):
		return self._trace["meta"].get("rom_base")

	@property
	def ram_base(self):
		return self._trace["meta"].get("ram_base")

	@property
	def ram_size(self):
		return self._trace["meta"].get("ram_size")

	@property
	def structure(self):
		return self._trace["structure"]

	def get_tracepoint_by_insn_cnt(self, insn_cnt):
		return self._tracepoint_by_insn_cnt.get(insn_cnt)

	def getbytes(self, text):
		if text.startswith(">"):
			# Short literal
			data = bytes.fromhex(text[1:])
		else:
			if self._trace["meta"]["binary_format"] == "hex":
				data = bytes.fromhex(text)
			elif self._trace["meta"]["binary_format"] == "b64":
				data = base64.b64decode(text)
			else:
				raise NotImplementedError(self._trace["meta"]["binary_format"])

			if self._trace["meta"]["compression"]:
				data = zlib.decompress(data)
		return data

	def align(self, other, callback):
		(i, j) = (0, 0)
		while (i < len(self)) and (j < len(other)):
			pt1 = self[i]
			pt2 = other[j]
			(key1, key2) = (pt1["executed_insns"], pt2["executed_insns"])
			if key1 == key2:
				callback(self, pt1, other, pt2)
				i += 1
				j += 1
			elif key1 < key2:
				# No alignment an i is lagging behind
				i += 1
			elif key2 < key1:
				# No alignment an j is lagging behind
				j += 1

	def __getitem__(self, index):
		return self._trace["trace"][index]

	def __len__(self):
		return len(self._trace["trace"])
