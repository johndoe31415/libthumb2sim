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

class TraceReader(object):
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
		return self._trace["structure"]

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
