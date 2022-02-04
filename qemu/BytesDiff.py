#	libthumb2sim - Emulator for the Thumb-2 ISA (Cortex-M)
#	Copyright (C) 2019-2022 Johannes Bauer
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

import struct

class BytesDiff():
	_Header = struct.Struct("< L L")

	def __init__(self, patch_data):
		self._patch_data = patch_data

	@property
	def patch_data(self):
		return self._patch_data

	def apply(self, old):
		new_data = bytearray(old)
		patch_offset = 0
		while patch_offset < len(self._patch_data):
			(offset, length) = self._Header.unpack(self._patch_data[patch_offset : patch_offset + self._Header.size])
			patchdata = self._patch_data[patch_offset + 8 : patch_offset + 8 + length]
			new_data[offset : offset + len(patchdata)] = patchdata
			patch_offset += 8 + len(patchdata)
		return new_data

	@classmethod
	def diff_data(cls, old, new):
		def _emit_patch(offset, data):
			header = cls._Header.pack(offset, len(data))
			return header + data

		patch = bytes()
		start_offset = None
		end_offset = None
		for (offset, (old_byte, new_byte)) in enumerate(zip(old, new)):
			if old_byte != new_byte:
				if start_offset is None:
					# Start a new patch
					start_offset = offset
					end_offset = offset
				else:
					if offset - end_offset < 8:
						# Just continue the patch
						end_offset = offset
					else:
						# Emit patch and start new
						patch += _emit_patch(start_offset, self._new[start_offset : end_offset + 1])
						start_offset = offset
						end_offset = offset

		if start_offset is not None:
			patch += _emit_patch(start_offset, new[start_offset : end_offset + 1])
		return cls(patch)

if __name__ == "__main__":
	import os

	for i in range(10):
		q1 = os.urandom(10000)
		q2 = os.urandom(10000)
		diff = BytesDiff.diff_data(q1, q2)
		q2x = diff.apply(q1)
		assert(q2 == q2x)
