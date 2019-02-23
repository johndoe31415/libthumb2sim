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

import collections

class Instruction(object):
	_REGISTER_NAMES = set([ "Rd", "Rn", "Rm", "Rt" ])
	def __init__(self, opcode, variant, length, arguments = None):
		if arguments is None:
			arguments = { }
		self._opcode = opcode
		self._variant = variant
		self._length = length
		self._arguments = arguments

	@property
	def opcode(self):
		return self._opcode

	@property
	def length(self):
		return self._length

	@property
	def variant(self):
		return self._variant

	@classmethod
	def is_register(cls, regname):
		return regname in cls._REGISTER_NAMES

	def get_arg_string(self, argname):
		if self.is_register(argname):
			return "%s = r%d" % (argname, self._arguments[argname])
		else:
			return "%s = 0x%x" % (argname, self._arguments[argname])

	def __iter__(self):
		return iter(self._arguments.items())

	def __str__(self):
		return "%s(%s)" % (self.variant, ", ".join(self.get_arg_string(argname) for argname in self._arguments))


class InstructionSetDecoder(object):
	@classmethod
	def ror32(cls, in_value, rotate):
		rotate %= 32
		if rotate == 0:
			return in_value
		lo_mask = (1 << rotate) - 1
		hi_mask = ((1 << (32 - rotate)) - 1) << rotate
		return ((in_value & lo_mask) << (32 - rotate)) | ((in_value & hi_mask) >> rotate)

	@classmethod
	def sint32(cls, value):
		if value & 0x80000000:
			value -= 2 ** 32
		return value

	@classmethod
	def thumb_expand_imm(cls, imm):
		if ((imm & 0xc00) == 0x0):
			if ((imm & 0x300) == 0x0):
				return imm & 0xff
			elif ((imm & 0x300) == 0x100):
				return ((imm & 0xff) << 16) | (imm & 0xff)
			elif ((imm & 0x300) == 0x200):
				return cls.sint32(((imm & 0xff) << 24) | ((imm & 0xff) << 8))
			else:
				return cls.sint32(((imm & 0xff) << 24) | ((imm & 0xff) << 16) | ((imm & 0xff) << 8) | ((imm & 0xff) << 0))
		else:
			# Rotate thingy
			return cls.sint32(cls.ror32(0x80 | (imm & 0x7f), (imm & 0xf80) >> 7))

	@classmethod
	def thumb_sign_extend(cls, imm, bit_length):
		if (imm & (1 << (bit_length - 1))):
			return imm | (~((1 << bit_length) - 1) & 0xffffffff)
		else:
			return imm

	@classmethod
	def thumb_sign_extend20(cls, imm):
		return cls.sint32(cls.thumb_sign_extend(imm, 20))

	@classmethod
	def thumb_sign_extend24_EOR(cls, imm):
		imm ^= ((1 << 21) ^ ((imm & (1 << 23)) >> 2))
		imm ^= ((1 << 22) ^ ((imm & (1 << 23)) >> 1))
		return cls.sint32(cls.thumb_sign_extend(imm, 24))

	@classmethod
	def decode_opcode(cls, opcode):
		decoded_length = 0
		if 1 == 0:
			# Simplify code generation
			pass
%for opcode in i.getopcodes():
		elif (opcode & ${"%#x" % (opcode.bitfield.constantmask)}) == ${"%#x" % (opcode.bitfield.constantcmp)}:
			arguments = collections.OrderedDict([
%for (varname, variable) in opcode.itervars():
				("${varname}", ${variable.pyexpression("opcode")}),
%endfor
			])
			return Instruction(opcode = "${opcode.basename}", variant = "${opcode.name}", length = ${len(opcode) // 8}, arguments = arguments)
%endfor
		raise NotImplementedError("Unable to decode opcode 0x%x" % (opcode))
	
	@classmethod
	def decode_data(cls, data):
		if len(data) >= 4:
			opcode = (data[1] << 24) | (data[0] << 16) | (data[3] << 8) | (data[2] << 0)
		else:
			opcode = (data[1] << 24) | (data[0] << 16)
		return cls.decode_opcode(opcode)
# vim: set filetype=python:
