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
		elif (opcode & 0xfbe08000) == 0xf1400000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", cls._thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff))),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "adc", variant = "adc_imm_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffc00000) == 0x41400000:
			arguments = collections.OrderedDict([
				("Rdn", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
			])
			return Instruction(opcode = "adc", variant = "adc_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffe08000) == 0xeb400000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("type", ((opcode & 0x30) >> 4)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "adc", variant = "adc_reg_T2", length = 4, arguments = arguments)
		elif (opcode & 0xf8000000) == 0xa8000000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0x7000000) >> 24)),
				("imm", ((opcode & 0xff0000) >> 16)),
			])
			return Instruction(opcode = "add", variant = "add_SPi_T1", length = 2, arguments = arguments)
		elif (opcode & 0xff800000) == 0xb0000000:
			arguments = collections.OrderedDict([
				("imm", ((opcode & 0x7f0000) >> 16)),
			])
			return Instruction(opcode = "add", variant = "add_SPi_T2", length = 2, arguments = arguments)
		elif (opcode & 0xfe000000) == 0x1c000000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0x70000) >> 16)),
				("Rn", ((opcode & 0x380000) >> 19)),
				("imm", ((opcode & 0x1c00000) >> 22)),
			])
			return Instruction(opcode = "add", variant = "add_imm_T1", length = 2, arguments = arguments)
		elif (opcode & 0xf8000000) == 0x30000000:
			arguments = collections.OrderedDict([
				("Rdn", ((opcode & 0x7000000) >> 24)),
				("imm", ((opcode & 0xff0000) >> 16)),
			])
			return Instruction(opcode = "add", variant = "add_imm_T2", length = 2, arguments = arguments)
		elif (opcode & 0xfe000000) == 0x18000000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0x70000) >> 16)),
				("Rn", ((opcode & 0x380000) >> 19)),
				("Rm", ((opcode & 0x1c00000) >> 22)),
			])
			return Instruction(opcode = "add", variant = "add_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xf8000000) == 0xa0000000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0x7000000) >> 24)),
				("imm", ((opcode & 0xff0000) >> 16)),
			])
			return Instruction(opcode = "adr", variant = "adr_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffc00000) == 0x40000000:
			arguments = collections.OrderedDict([
				("Rdn", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
			])
			return Instruction(opcode = "and", variant = "and_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xf8000000) == 0x10000000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
				("imm", ((opcode & 0x7c00000) >> 22)),
			])
			return Instruction(opcode = "asr", variant = "asr_imm_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffc00000) == 0x41000000:
			arguments = collections.OrderedDict([
				("Rdn", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
			])
			return Instruction(opcode = "asr", variant = "asr_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffe0f0f0) == 0xfa40f000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "asr", variant = "asr_reg_T2", length = 4, arguments = arguments)
		elif (opcode & 0xf8000000) == 0xe0000000:
			arguments = collections.OrderedDict([
				("imm", ((opcode & 0x7ff0000) >> 16)),
			])
			return Instruction(opcode = "b", variant = "b_T2", length = 2, arguments = arguments)
		elif (opcode & 0xfbe08000) == 0xf0200000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", cls._thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff))),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "bic", variant = "bic_imm_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffc00000) == 0x43800000:
			arguments = collections.OrderedDict([
				("Rdn", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
			])
			return Instruction(opcode = "bic", variant = "bic_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffe00000) == 0xea200000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("type", ((opcode & 0x30) >> 4)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "bic", variant = "bic_reg_T2", length = 4, arguments = arguments)
		elif (opcode & 0xff000000) == 0xbe000000:
			arguments = collections.OrderedDict([
				("imm", ((opcode & 0xff0000) >> 16)),
			])
			return Instruction(opcode = "bkpt", variant = "bkpt_T1", length = 2, arguments = arguments)
		elif (opcode & 0xff800000) == 0x47800000:
			arguments = collections.OrderedDict([
				("Rm", ((opcode & 0x780000) >> 19)),
			])
			return Instruction(opcode = "blx", variant = "blx_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xff800000) == 0x47000000:
			arguments = collections.OrderedDict([
				("Rm", ((opcode & 0x780000) >> 19)),
			])
			return Instruction(opcode = "bx", variant = "bx_T1", length = 2, arguments = arguments)
		elif (opcode & 0xf5000000) == 0xb1000000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0x70000) >> 16)),
				("imm", ((opcode & 0x2000000) >> 20) | ((opcode & 0xf80000) >> 19)),
				("op", ((opcode & 0x8000000) >> 27)),
			])
			return Instruction(opcode = "cbnz", variant = "cbnz_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfab0f080:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rm", (opcode & 0xf)),
				("Rmx", ((opcode & 0xf0000) >> 16)),
			])
			return Instruction(opcode = "clz", variant = "clz_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffc00000) == 0x42c00000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
			])
			return Instruction(opcode = "cmn", variant = "cmn_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xf8000000) == 0x28000000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0x7000000) >> 24)),
				("imm", ((opcode & 0xff0000) >> 16)),
			])
			return Instruction(opcode = "cmp", variant = "cmp_imm_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffc00000) == 0x42800000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
			])
			return Instruction(opcode = "cmp", variant = "cmp_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xff000000) == 0x45000000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0x800000) >> 20) | ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x780000) >> 19)),
			])
			return Instruction(opcode = "cmp", variant = "cmp_reg_T2", length = 2, arguments = arguments)
		elif (opcode & 0xffe00000) == 0xb6600000:
			arguments = collections.OrderedDict([
				("imm", ((opcode & 0x100000) >> 20)),
				("F", ((opcode & 0x10000) >> 16)),
				("I", ((opcode & 0x20000) >> 17)),
			])
			return Instruction(opcode = "cps", variant = "cps_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffc00000) == 0x40400000:
			arguments = collections.OrderedDict([
				("Rdn", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
			])
			return Instruction(opcode = "eor", variant = "eor_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xf8000000) == 0xc8000000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0x7000000) >> 24)),
				("register_list", ((opcode & 0xff0000) >> 16)),
			])
			return Instruction(opcode = "ldm", variant = "ldm_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffd02000) == 0xe9100000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("register_list", (opcode & 0x1fff)),
				("P", ((opcode & 0x8000) >> 15)),
				("M", ((opcode & 0x4000) >> 14)),
				("W", ((opcode & 0x200000) >> 21)),
			])
			return Instruction(opcode = "ldmdb", variant = "ldmdb_T1", length = 4, arguments = arguments)
		elif (opcode & 0xf8000000) == 0x68000000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0x70000) >> 16)),
				("Rn", ((opcode & 0x380000) >> 19)),
				("imm", ((opcode & 0x7c00000) >> 22)),
			])
			return Instruction(opcode = "ldr", variant = "ldr_imm_T1", length = 2, arguments = arguments)
		elif (opcode & 0xf8000000) == 0x98000000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0x7000000) >> 24)),
				("imm", ((opcode & 0xff0000) >> 16)),
			])
			return Instruction(opcode = "ldr", variant = "ldr_imm_T2", length = 2, arguments = arguments)
		elif (opcode & 0xf8000000) == 0x48000000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0x7000000) >> 24)),
				("imm", ((opcode & 0xff0000) >> 16)),
			])
			return Instruction(opcode = "ldr", variant = "ldr_lit_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfe000000) == 0x58000000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0x70000) >> 16)),
				("Rn", ((opcode & 0x380000) >> 19)),
				("Rm", ((opcode & 0x1c00000) >> 22)),
			])
			return Instruction(opcode = "ldr", variant = "ldr_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xf8000000) == 0x78000000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0x70000) >> 16)),
				("Rn", ((opcode & 0x380000) >> 19)),
				("imm", ((opcode & 0x7c00000) >> 22)),
			])
			return Instruction(opcode = "ldrb", variant = "ldrb_imm_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfe000000) == 0x5c000000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0x70000) >> 16)),
				("Rn", ((opcode & 0x380000) >> 19)),
				("Rm", ((opcode & 0x1c00000) >> 22)),
			])
			return Instruction(opcode = "ldrb", variant = "ldrb_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xf8000000) == 0x88000000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0x70000) >> 16)),
				("Rn", ((opcode & 0x380000) >> 19)),
				("imm", ((opcode & 0x7c00000) >> 22)),
			])
			return Instruction(opcode = "ldrh", variant = "ldrh_imm_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfe000000) == 0x5a000000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0x70000) >> 16)),
				("Rn", ((opcode & 0x380000) >> 19)),
				("Rm", ((opcode & 0x1c00000) >> 22)),
			])
			return Instruction(opcode = "ldrh", variant = "ldrh_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfe000000) == 0x56000000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0x70000) >> 16)),
				("Rn", ((opcode & 0x380000) >> 19)),
				("Rm", ((opcode & 0x1c00000) >> 22)),
			])
			return Instruction(opcode = "ldrsb", variant = "ldrsb_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfe000000) == 0x5e000000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0x70000) >> 16)),
				("Rn", ((opcode & 0x380000) >> 19)),
				("Rm", ((opcode & 0x1c00000) >> 22)),
			])
			return Instruction(opcode = "ldrsh", variant = "ldrsh_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffc00000) == 0x40800000:
			arguments = collections.OrderedDict([
				("Rdn", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
			])
			return Instruction(opcode = "lsl", variant = "lsl_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffe0f0f0) == 0xfa00f000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "lsl", variant = "lsl_reg_T2", length = 4, arguments = arguments)
		elif (opcode & 0xf8000000) == 0x8000000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
				("imm", ((opcode & 0x7c00000) >> 22)),
			])
			return Instruction(opcode = "lsr", variant = "lsr_imm_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffc00000) == 0x40c00000:
			arguments = collections.OrderedDict([
				("Rdn", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
			])
			return Instruction(opcode = "lsr", variant = "lsr_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffe0f0f0) == 0xfa20f000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "lsr", variant = "lsr_reg_T2", length = 4, arguments = arguments)
		elif (opcode & 0xff100010) == 0xfe000010:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("coproc", ((opcode & 0xf00) >> 8)),
				("CRn", ((opcode & 0xf0000) >> 16)),
				("CRm", (opcode & 0xf)),
				("opcA", ((opcode & 0xe00000) >> 21)),
				("opcB", ((opcode & 0xe0) >> 5)),
			])
			return Instruction(opcode = "mcr", variant = "mcr2_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfff000f0) == 0xfb000010:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("Ra", ((opcode & 0xf000) >> 12)),
			])
			return Instruction(opcode = "mls", variant = "mls_T1", length = 4, arguments = arguments)
		elif (opcode & 0xf8000000) == 0x20000000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0x7000000) >> 24)),
				("imm", ((opcode & 0xff0000) >> 16)),
			])
			return Instruction(opcode = "mov", variant = "mov_imm_T1", length = 2, arguments = arguments)
		elif (opcode & 0xff000000) == 0x46000000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0x800000) >> 20) | ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x780000) >> 19)),
			])
			return Instruction(opcode = "mov", variant = "mov_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfbf08000) == 0xf2c00000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("imm", ((opcode & 0x4000000) >> 15) | ((opcode & 0xf7000) >> 4) | (opcode & 0xff)),
			])
			return Instruction(opcode = "movt", variant = "movt_T1", length = 4, arguments = arguments)
		elif (opcode & 0xff100010) == 0xfe100010:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("coproc", ((opcode & 0xf00) >> 8)),
				("Crn", ((opcode & 0xf0000) >> 16)),
				("CRm", (opcode & 0xf)),
				("opcA", ((opcode & 0xe00000) >> 21)),
				("opcB", ((opcode & 0xe0) >> 5)),
			])
			return Instruction(opcode = "mrc", variant = "mrc2_T2", length = 4, arguments = arguments)
		elif (opcode & 0xffc00000) == 0x43400000:
			arguments = collections.OrderedDict([
				("Rdm", ((opcode & 0x70000) >> 16)),
				("Rn", ((opcode & 0x380000) >> 19)),
			])
			return Instruction(opcode = "mul", variant = "mul_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffc00000) == 0x43c00000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
			])
			return Instruction(opcode = "mvn", variant = "mvn_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffc00000) == 0x43000000:
			arguments = collections.OrderedDict([
				("Rdn", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
			])
			return Instruction(opcode = "orr", variant = "orr_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffe00000) == 0xeac00000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("tb", ((opcode & 0x20) >> 5)),
				("S", ((opcode & 0x100000) >> 20)),
				("T", ((opcode & 0x10) >> 4)),
			])
			return Instruction(opcode = "pkhbt", variant = "pkhbt_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfe000000) == 0xbc000000:
			arguments = collections.OrderedDict([
				("register_list", ((opcode & 0xff0000) >> 16)),
				("P", ((opcode & 0x1000000) >> 24)),
			])
			return Instruction(opcode = "pop", variant = "pop_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfe000000) == 0xb4000000:
			arguments = collections.OrderedDict([
				("register_list", ((opcode & 0xff0000) >> 16)),
				("M", ((opcode & 0x1000000) >> 24)),
			])
			return Instruction(opcode = "push", variant = "push_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfa90f010:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "qadd", variant = "qadd16_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfa80f010:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "qadd", variant = "qadd8_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfa80f080:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "qadd", variant = "qadd_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfa80f090:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "qdadd", variant = "qdadd_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfa80f0b0:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "qdsub", variant = "qdsub_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfad0f010:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "qsub", variant = "qsub16_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfac0f010:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "qsub", variant = "qsub8_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfa80f0a0:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "qsub", variant = "qsub_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfa90f0a0:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rm", ((opcode & 0xf0000) >> 12) | (opcode & 0xf)),
			])
			return Instruction(opcode = "rbit", variant = "rbit_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffc00000) == 0xba400000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
			])
			return Instruction(opcode = "rev", variant = "rev16_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfa90f090:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rm", ((opcode & 0xf0000) >> 12) | (opcode & 0xf)),
			])
			return Instruction(opcode = "rev", variant = "rev16_T2", length = 4, arguments = arguments)
		elif (opcode & 0xffc00000) == 0xba000000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
			])
			return Instruction(opcode = "rev", variant = "rev_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfa90f080:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rm", (opcode & 0xf)),
				("Rmx", ((opcode & 0xf0000) >> 16)),
			])
			return Instruction(opcode = "rev", variant = "rev_T2", length = 4, arguments = arguments)
		elif (opcode & 0xffc00000) == 0xbac00000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
			])
			return Instruction(opcode = "revsh", variant = "revsh_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfa90f0b0:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rm", ((opcode & 0xf0000) >> 12) | (opcode & 0xf)),
			])
			return Instruction(opcode = "revsh", variant = "revsh_T2", length = 4, arguments = arguments)
		elif (opcode & 0xffc00000) == 0x41c00000:
			arguments = collections.OrderedDict([
				("Rdn", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
			])
			return Instruction(opcode = "ror", variant = "ror_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffe0f0f0) == 0xfa60f000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "ror", variant = "ror_reg_T2", length = 4, arguments = arguments)
		elif (opcode & 0xffc00000) == 0x42400000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0x70000) >> 16)),
				("Rn", ((opcode & 0x380000) >> 19)),
			])
			return Instruction(opcode = "rsb", variant = "rsb_imm_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfbe08000) == 0xf1c00000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", cls._thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff))),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "rsb", variant = "rsb_imm_T2", length = 4, arguments = arguments)
		elif (opcode & 0xffe00000) == 0xebc00000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("type", ((opcode & 0x30) >> 4)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "rsb", variant = "rsb_reg_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfa90f000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "sadd", variant = "sadd16_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfa80f000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "sadd", variant = "sadd8_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfaa0f000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "sasx", variant = "sasx_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfbe08000) == 0xf1600000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", cls._thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff))),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "sbc", variant = "sbc_imm_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffc00000) == 0x41800000:
			arguments = collections.OrderedDict([
				("Rdn", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
			])
			return Instruction(opcode = "sbc", variant = "sbc_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffe00000) == 0xeb600000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("type", ((opcode & 0x30) >> 4)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "sbc", variant = "sbc_reg_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfbf08000) == 0xf3400000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("width", (opcode & 0x1f)),
			])
			return Instruction(opcode = "sbfx", variant = "sbfx_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff000f0) == 0xfb9000f0:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "sdiv", variant = "sdiv_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfaa0f080:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "sel", variant = "sel_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfa90f020:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "shadd", variant = "shadd16_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfa80f020:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "shadd", variant = "shadd8_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfaa0f020:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "shasx", variant = "shasx_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfae0f020:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "shsax", variant = "shsax_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfad0f020:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "shsub", variant = "shsub16_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfac0f020:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "shsub", variant = "shsub8_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff000f0) == 0xfbc00000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("RdLo", ((opcode & 0xf000) >> 12)),
				("RdHi", ((opcode & 0xf00) >> 8)),
			])
			return Instruction(opcode = "smlal", variant = "smlal_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff000c0) == 0xfbc00080:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("RdLo", ((opcode & 0xf000) >> 12)),
				("RdHi", ((opcode & 0xf00) >> 8)),
				("N", ((opcode & 0x20) >> 5)),
				("M", ((opcode & 0x10) >> 4)),
			])
			return Instruction(opcode = "smlalbb", variant = "smlalbb_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff000e0) == 0xfbc000c0:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("RdLo", ((opcode & 0xf000) >> 12)),
				("RdHi", ((opcode & 0xf00) >> 8)),
				("M", ((opcode & 0x10) >> 4)),
			])
			return Instruction(opcode = "smlald", variant = "smlald_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff000e0) == 0xfbd000c0:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("RdLo", ((opcode & 0xf000) >> 12)),
				("RdHi", ((opcode & 0xf00) >> 8)),
				("M", ((opcode & 0x10) >> 4)),
			])
			return Instruction(opcode = "smlsld", variant = "smlsld_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff000e0) == 0xfb600000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("Ra", ((opcode & 0xf000) >> 12)),
				("R", ((opcode & 0x10) >> 4)),
			])
			return Instruction(opcode = "smmls", variant = "smmls_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff000f0) == 0xfb800000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("RdLo", ((opcode & 0xf000) >> 12)),
				("RdHi", ((opcode & 0xf00) >> 8)),
			])
			return Instruction(opcode = "smull", variant = "smull_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfae0f000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "ssax", variant = "ssax_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfad0f000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "ssub", variant = "ssub16_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfac0f000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "ssub", variant = "ssub8_T1", length = 4, arguments = arguments)
		elif (opcode & 0xf8000000) == 0xc0000000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0x7000000) >> 24)),
				("register_list", ((opcode & 0xff0000) >> 16)),
			])
			return Instruction(opcode = "stm", variant = "stm_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffd0a000) == 0xe8800000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("register_list", (opcode & 0x1fff)),
				("M", ((opcode & 0x4000) >> 14)),
				("W", ((opcode & 0x200000) >> 21)),
			])
			return Instruction(opcode = "stm", variant = "stm_T2", length = 4, arguments = arguments)
		elif (opcode & 0xf8000000) == 0x60000000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0x70000) >> 16)),
				("Rn", ((opcode & 0x380000) >> 19)),
				("imm", ((opcode & 0x7c00000) >> 22)),
			])
			return Instruction(opcode = "str", variant = "str_imm_T1", length = 2, arguments = arguments)
		elif (opcode & 0xf8000000) == 0x90000000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0x7000000) >> 24)),
				("imm", ((opcode & 0xff0000) >> 16)),
			])
			return Instruction(opcode = "str", variant = "str_imm_T2", length = 2, arguments = arguments)
		elif (opcode & 0xfff00000) == 0xf8c00000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xfff)),
			])
			return Instruction(opcode = "str", variant = "str_imm_T3", length = 4, arguments = arguments)
		elif (opcode & 0xfe000000) == 0x50000000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0x70000) >> 16)),
				("Rn", ((opcode & 0x380000) >> 19)),
				("Rm", ((opcode & 0x1c00000) >> 22)),
			])
			return Instruction(opcode = "str", variant = "str_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfff00fc0) == 0xf8400000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "str", variant = "str_reg_T2", length = 4, arguments = arguments)
		elif (opcode & 0xf8000000) == 0x70000000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0x70000) >> 16)),
				("Rn", ((opcode & 0x380000) >> 19)),
				("imm", ((opcode & 0x7c00000) >> 22)),
			])
			return Instruction(opcode = "strb", variant = "strb_imm_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfff00000) == 0xf8800000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xfff)),
			])
			return Instruction(opcode = "strb", variant = "strb_imm_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfe000000) == 0x54000000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0x70000) >> 16)),
				("Rn", ((opcode & 0x380000) >> 19)),
				("Rm", ((opcode & 0x1c00000) >> 22)),
			])
			return Instruction(opcode = "strb", variant = "strb_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfff00fc0) == 0xf8000000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "strb", variant = "strb_reg_T2", length = 4, arguments = arguments)
		elif (opcode & 0xf8000000) == 0x80000000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0x70000) >> 16)),
				("Rn", ((opcode & 0x380000) >> 19)),
				("imm", ((opcode & 0x7c00000) >> 22)),
			])
			return Instruction(opcode = "strh", variant = "strh_imm_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfff00000) == 0xf8a00000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xfff)),
			])
			return Instruction(opcode = "strh", variant = "strh_imm_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfe000000) == 0x52000000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0x70000) >> 16)),
				("Rn", ((opcode & 0x380000) >> 19)),
				("Rm", ((opcode & 0x1c00000) >> 22)),
			])
			return Instruction(opcode = "strh", variant = "strh_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfff00fc0) == 0xf8200000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "strh", variant = "strh_reg_T2", length = 4, arguments = arguments)
		elif (opcode & 0xff800000) == 0xb0800000:
			arguments = collections.OrderedDict([
				("imm", ((opcode & 0x7f0000) >> 16)),
			])
			return Instruction(opcode = "sub", variant = "sub_SPimm_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfe000000) == 0x1e000000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0x70000) >> 16)),
				("Rn", ((opcode & 0x380000) >> 19)),
				("imm", ((opcode & 0x1c00000) >> 22)),
			])
			return Instruction(opcode = "sub", variant = "sub_imm_T1", length = 2, arguments = arguments)
		elif (opcode & 0xf8000000) == 0x38000000:
			arguments = collections.OrderedDict([
				("Rdn", ((opcode & 0x7000000) >> 24)),
				("imm", ((opcode & 0xff0000) >> 16)),
			])
			return Instruction(opcode = "sub", variant = "sub_imm_T2", length = 2, arguments = arguments)
		elif (opcode & 0xfe000000) == 0x1a000000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0x70000) >> 16)),
				("Rn", ((opcode & 0x380000) >> 19)),
				("Rm", ((opcode & 0x1c00000) >> 22)),
			])
			return Instruction(opcode = "sub", variant = "sub_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffc00000) == 0xb2400000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
			])
			return Instruction(opcode = "sxtb", variant = "sxtb_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffc00000) == 0xb2000000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
			])
			return Instruction(opcode = "sxth", variant = "sxth_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffc00000) == 0x42000000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
			])
			return Instruction(opcode = "tst", variant = "tst_reg_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfa90f040:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "uadd", variant = "uadd16_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfa80f040:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "uadd", variant = "uadd8_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfaa0f040:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "uasx", variant = "uasx_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfbf08000) == 0xf3c00000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("width", (opcode & 0x1f)),
			])
			return Instruction(opcode = "ubfx", variant = "ubfx_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff000f0) == 0xfbb000f0:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "udiv", variant = "udiv_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfa90f060:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "uhadd", variant = "uhadd16_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfa80f060:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "uhadd", variant = "uhadd8_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfaa0f060:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "uhasx", variant = "uhasx_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfae0f060:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "uhsax", variant = "uhsax_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfad0f060:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "uhsub", variant = "uhsub16_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfac0f060:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "uhsub", variant = "uhsub8_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff000f0) == 0xfbe00060:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("RdLo", ((opcode & 0xf000) >> 12)),
				("RdHi", ((opcode & 0xf00) >> 8)),
			])
			return Instruction(opcode = "umaal", variant = "umaal_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff000f0) == 0xfbe00000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("RdLo", ((opcode & 0xf000) >> 12)),
				("RdHi", ((opcode & 0xf00) >> 8)),
			])
			return Instruction(opcode = "umlal", variant = "umlal_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff000f0) == 0xfba00000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("RdLo", ((opcode & 0xf000) >> 12)),
				("RdHi", ((opcode & 0xf00) >> 8)),
			])
			return Instruction(opcode = "umull", variant = "umull_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfa90f050:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "uqadd", variant = "uqadd16_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfa80f050:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "uqadd", variant = "uqadd8_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfaa0f050:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "uqasx", variant = "uqasx_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfae0f050:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "uqsax", variant = "uqsax_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfad0f050:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "uqsub", variant = "uqsub16_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfac0f050:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "uqsub", variant = "uqsub8_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfae0f040:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "usax", variant = "usax_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfad0f040:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "usub", variant = "usub16_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfac0f040:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "usub", variant = "usub8_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffc00000) == 0xb2c00000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
			])
			return Instruction(opcode = "uxtb", variant = "uxtb_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffc00000) == 0xb2800000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
			])
			return Instruction(opcode = "uxth", variant = "uxth_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfbff8000) == 0xf20d0000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("imm", ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff)),
			])
			return Instruction(opcode = "add", variant = "add_SPi_T4", length = 4, arguments = arguments)
		elif (opcode & 0xff870000) == 0x44850000:
			arguments = collections.OrderedDict([
				("Rm", ((opcode & 0x780000) >> 19)),
			])
			return Instruction(opcode = "add", variant = "add_SPr_T2", length = 2, arguments = arguments)
		elif (opcode & 0xfbff8000) == 0xf2af0000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("imm", ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff)),
			])
			return Instruction(opcode = "adr", variant = "adr_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfbff8000) == 0xf20f0000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("imm", ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff)),
			])
			return Instruction(opcode = "adr", variant = "adr_T3", length = 4, arguments = arguments)
		elif (opcode & 0xfbff8000) == 0xf36f0000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("msb", (opcode & 0x1f)),
			])
			return Instruction(opcode = "bfc", variant = "bfc_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfbf08f00) == 0xf1100f00:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", cls._thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff))),
			])
			return Instruction(opcode = "cmn", variant = "cmn_imm_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff00f00) == 0xeb100f00:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("imm", cls._thumb_expand_imm(((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6))),
				("type", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "cmn", variant = "cmn_reg_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfff08f00) == 0xebb00f00:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("type", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "cmp", variant = "cmp_reg_T3", length = 4, arguments = arguments)
		elif (opcode & 0xfff00fff) == 0xe8d00f4f:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
			])
			return Instruction(opcode = "ldrexb", variant = "ldrexb_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff00fff) == 0xe8d00f5f:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
			])
			return Instruction(opcode = "ldrexh", variant = "ldrexh_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff00fc0) == 0xf8300000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "ldrh", variant = "ldrh_reg_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfff00f00) == 0xf9300e00:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xff)),
			])
			return Instruction(opcode = "ldrsht", variant = "ldrsh_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff00fc0) == 0xf9300000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "ldrsh", variant = "ldrsh_reg_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfff00000) == 0xfc400000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("coproc", ((opcode & 0xf00) >> 8)),
				("CRm", (opcode & 0xf)),
				("opcA", ((opcode & 0xf0) >> 4)),
				("RtB", ((opcode & 0xf0000) >> 16)),
			])
			return Instruction(opcode = "mcrr", variant = "mcrr2_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfbef8000) == 0xf04f0000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("imm", cls._thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff))),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "mov", variant = "mov_imm_T2", length = 4, arguments = arguments)
		elif (opcode & 0xffc00000) == 0x0:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
			])
			return Instruction(opcode = "mov", variant = "mov_reg_T2", length = 2, arguments = arguments)
		elif (opcode & 0xffeff0f0) == 0xf24f0000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rm", (opcode & 0xf)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "mov", variant = "mov_reg_T3", length = 4, arguments = arguments)
		elif (opcode & 0xfff00000) == 0xfc500000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("coproc", ((opcode & 0xf00) >> 8)),
				("CRm", (opcode & 0xf)),
				("opcA", ((opcode & 0xf0) >> 4)),
				("RtB", ((opcode & 0xf0000) >> 16)),
			])
			return Instruction(opcode = "mrrc", variant = "mrrc2_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfb00f000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "mul", variant = "mul_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfbef8000) == 0xf06f0000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("imm", cls._thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff))),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "mvn", variant = "mvn_imm_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffef8000) == 0xea6f0000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rm", (opcode & 0xf)),
				("imm", cls._thumb_expand_imm(((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6))),
				("type", ((opcode & 0x30) >> 4)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "mvn", variant = "mvn_reg_T2", length = 4, arguments = arguments)
		elif (opcode & 0xffff0000) == 0xbf000000:
			arguments = collections.OrderedDict([
			])
			return Instruction(opcode = "nop", variant = "nop_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffffffff) == 0xf3af8000:
			arguments = collections.OrderedDict([
			])
			return Instruction(opcode = "nop", variant = "nop_T2", length = 4, arguments = arguments)
		elif (opcode & 0xff7ff000) == 0xf91ff000:
			arguments = collections.OrderedDict([
				("imm", (opcode & 0xfff)),
				("U", ((opcode & 0x800000) >> 23)),
			])
			return Instruction(opcode = "pli", variant = "pld_immlit_T3", length = 4, arguments = arguments)
		elif (opcode & 0xff7ff000) == 0xf81ff000:
			arguments = collections.OrderedDict([
				("imm", (opcode & 0xfff)),
				("U", ((opcode & 0x800000) >> 23)),
			])
			return Instruction(opcode = "pld", variant = "pld_lit_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffff2000) == 0xe8bd0000:
			arguments = collections.OrderedDict([
				("register_list", (opcode & 0x1fff)),
				("P", ((opcode & 0x8000) >> 15)),
				("M", ((opcode & 0x4000) >> 14)),
			])
			return Instruction(opcode = "pop", variant = "pop_T2", length = 4, arguments = arguments)
		elif (opcode & 0xffff0fff) == 0xf85d0b04:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
			])
			return Instruction(opcode = "pop", variant = "pop_T3", length = 4, arguments = arguments)
		elif (opcode & 0xffffa000) == 0xe92d0000:
			arguments = collections.OrderedDict([
				("register_list", (opcode & 0x1fff)),
				("M", ((opcode & 0x4000) >> 14)),
			])
			return Instruction(opcode = "push", variant = "push_T2", length = 4, arguments = arguments)
		elif (opcode & 0xffff0fff) == 0xf84d0d04:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
			])
			return Instruction(opcode = "push", variant = "push_T3", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfae0f010:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "qasx", variant = "qasx_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfae0f010:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "qsax", variant = "qsax_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffef70f0) == 0xea4f0030:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rm", (opcode & 0xf)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "rrx", variant = "rrx_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffff0000) == 0xbf400000:
			arguments = collections.OrderedDict([
			])
			return Instruction(opcode = "sev", variant = "sev_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfff0f0e0) == 0xfb50f000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("R", ((opcode & 0x10) >> 4)),
			])
			return Instruction(opcode = "smmul", variant = "smmul_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0e0) == 0xfb20f000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("M", ((opcode & 0x10) >> 4)),
			])
			return Instruction(opcode = "smuad", variant = "smuad_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0c0) == 0xfb10f000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("N", ((opcode & 0x20) >> 5)),
				("M", ((opcode & 0x10) >> 4)),
			])
			return Instruction(opcode = "smulbb", variant = "smulbb_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0e0) == 0xfb30f000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("M", ((opcode & 0x10) >> 4)),
			])
			return Instruction(opcode = "smulwb", variant = "smulwb_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f0e0) == 0xfb40f000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("M", ((opcode & 0x10) >> 4)),
			])
			return Instruction(opcode = "smusd", variant = "smusd_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfbf0f0c0) == 0xf3200000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("satimm", (opcode & 0xf)),
			])
			return Instruction(opcode = "ssat", variant = "ssat16_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff00f00) == 0xf8000e00:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xff)),
			])
			return Instruction(opcode = "strbt", variant = "strb_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff000f0) == 0xe8c00040:
			arguments = collections.OrderedDict([
				("Rd", (opcode & 0xf)),
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
			])
			return Instruction(opcode = "strexb", variant = "strexb_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff000f0) == 0xe8c00050:
			arguments = collections.OrderedDict([
				("Rd", (opcode & 0xf)),
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
			])
			return Instruction(opcode = "strexh", variant = "strexh_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff00f00) == 0xf8200e00:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xff)),
			])
			return Instruction(opcode = "strht", variant = "strht_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfbff8000) == 0xf2ad0000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("imm", ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff)),
			])
			return Instruction(opcode = "sub", variant = "sub_SPimm_T3", length = 4, arguments = arguments)
		elif (opcode & 0xff000000) == 0xdf000000:
			arguments = collections.OrderedDict([
				("imm", ((opcode & 0xff0000) >> 16)),
			])
			return Instruction(opcode = "svc", variant = "svc_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfffff0c0) == 0xfa2ff080:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rm", (opcode & 0xf)),
				("rotate", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "sxtb", variant = "sxtb16_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfffff0c0) == 0xfa4ff080:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rm", (opcode & 0xf)),
				("rotate", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "sxtb", variant = "sxtb_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfffff0c0) == 0xfa0ff080:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rm", (opcode & 0xf)),
				("rotate", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "sxth", variant = "sxth_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfbf08f00) == 0xf0900f00:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff)),
			])
			return Instruction(opcode = "teq", variant = "teq_imm_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff00f00) == 0xea900f00:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("type", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "teq", variant = "teq_reg_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfbf08f00) == 0xf0100f00:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", cls._thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff))),
			])
			return Instruction(opcode = "tst", variant = "tst_imm_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff08f00) == 0xea100f00:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("type", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "tst", variant = "tst_reg_T2", length = 4, arguments = arguments)
		elif (opcode & 0xff000000) == 0xde000000:
			arguments = collections.OrderedDict([
				("imm", ((opcode & 0xff0000) >> 16)),
			])
			return Instruction(opcode = "udf", variant = "udf_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfff0f0f0) == 0xfb70f000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
			])
			return Instruction(opcode = "usad8", variant = "usad8_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfbf0f0c0) == 0xf3a00000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("satimm", (opcode & 0xf)),
			])
			return Instruction(opcode = "usat", variant = "usat16_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfffff0c0) == 0xfa3ff080:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rm", (opcode & 0xf)),
				("rotate", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "uxtb", variant = "uxtb16_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfffff0c0) == 0xfa5ff080:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rm", (opcode & 0xf)),
				("rotate", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "uxtb", variant = "uxtb_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfffff0c0) == 0xfa1ff080:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rm", (opcode & 0xf)),
				("rotate", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "uxth", variant = "uxth_T2", length = 4, arguments = arguments)
		elif (opcode & 0xffbf0ed0) == 0xeeb00ac0:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("Vm", (opcode & 0xf)),
				("D", ((opcode & 0x400000) >> 22)),
				("M", ((opcode & 0x20) >> 5)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vabs", variant = "vabs_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffbf0ed0) == 0xeeb70ac0:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("Vm", (opcode & 0xf)),
				("D", ((opcode & 0x400000) >> 22)),
				("M", ((opcode & 0x20) >> 5)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vcvt", variant = "vcvt_dpsp_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffe00fd0) == 0xec400a10:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rtx", ((opcode & 0xf0000) >> 16)),
				("Vm", (opcode & 0xf)),
				("op", ((opcode & 0x100000) >> 20)),
				("M", ((opcode & 0x20) >> 5)),
			])
			return Instruction(opcode = "vmov", variant = "vmov_2arm2sp_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffe00fd0) == 0xec400b10:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rtx", ((opcode & 0xf0000) >> 16)),
				("Vm", (opcode & 0xf)),
				("op", ((opcode & 0x100000) >> 20)),
				("M", ((opcode & 0x20) >> 5)),
			])
			return Instruction(opcode = "vmov", variant = "vmov_2armdw_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffd00f70) == 0xee000b10:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Vd", ((opcode & 0xf0000) >> 16)),
				("H", ((opcode & 0x200000) >> 21)),
				("D", ((opcode & 0x80) >> 7)),
			])
			return Instruction(opcode = "vmov", variant = "vmov_armscl_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffbf0ed0) == 0xeeb00a40:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("Vm", (opcode & 0xf)),
				("D", ((opcode & 0x400000) >> 22)),
				("M", ((opcode & 0x20) >> 5)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vmov", variant = "vmov_reg_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffd00f70) == 0xee100b10:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Vd", ((opcode & 0xf0000) >> 16)),
				("H", ((opcode & 0x200000) >> 21)),
				("D", ((opcode & 0x80) >> 7)),
			])
			return Instruction(opcode = "vmov", variant = "vmov_sclarm_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffbf0ed0) == 0xeeb10a40:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("Vm", (opcode & 0xf)),
				("D", ((opcode & 0x400000) >> 22)),
				("M", ((opcode & 0x20) >> 5)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vneg", variant = "vneg_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffbc0ed0) == 0xfeb80a40:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("Vm", (opcode & 0xf)),
				("RM", ((opcode & 0x30000) >> 16)),
				("D", ((opcode & 0x400000) >> 22)),
				("M", ((opcode & 0x20) >> 5)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vrinta", variant = "vrinta_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffbf0ed0) == 0xeeb70a40:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("Vm", (opcode & 0xf)),
				("D", ((opcode & 0x400000) >> 22)),
				("M", ((opcode & 0x20) >> 5)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vrintx", variant = "vrintx_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffbf0ed0) == 0xeeb10ac0:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("Vm", (opcode & 0xf)),
				("D", ((opcode & 0x400000) >> 22)),
				("M", ((opcode & 0x20) >> 5)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vsqrt", variant = "vsqrt_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffff0000) == 0xbf200000:
			arguments = collections.OrderedDict([
			])
			return Instruction(opcode = "wfe", variant = "wfe_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffff0000) == 0xbf300000:
			arguments = collections.OrderedDict([
			])
			return Instruction(opcode = "wfi", variant = "wfi_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffff0000) == 0xbf100000:
			arguments = collections.OrderedDict([
			])
			return Instruction(opcode = "yield", variant = "yield_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfbef8000) == 0xf10d0000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("imm", cls._thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff))),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "add", variant = "add_SPi_T3", length = 4, arguments = arguments)
		elif (opcode & 0xff780000) == 0x44680000:
			arguments = collections.OrderedDict([
				("Rdm", ((opcode & 0x800000) >> 20) | ((opcode & 0x70000) >> 16)),
			])
			return Instruction(opcode = "add", variant = "add_SPr_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffef8000) == 0xeb0d0000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("type", ((opcode & 0x30) >> 4)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "add", variant = "add_SPr_T3", length = 4, arguments = arguments)
		elif (opcode & 0xfbf08000) == 0xf2000000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff)),
			])
			return Instruction(opcode = "add", variant = "add_imm_T4", length = 4, arguments = arguments)
		elif (opcode & 0xfbe08000) == 0xf0000000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", cls._thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff))),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "and", variant = "and_imm_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffe08000) == 0xea000000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("type", ((opcode & 0x30) >> 4)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "and", variant = "and_reg_T2", length = 4, arguments = arguments)
		elif (opcode & 0xffef8030) == 0xea4f0020:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "asr", variant = "asr_imm_T2", length = 4, arguments = arguments)
		elif (opcode & 0xf0000000) == 0xd0000000:
			arguments = collections.OrderedDict([
				("imm", ((opcode & 0xff0000) >> 16)),
				("cond", ((opcode & 0xf000000) >> 24)),
			])
			return Instruction(opcode = "b", variant = "b_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfbf08000) == 0xf3600000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("msb", (opcode & 0x1f)),
			])
			return Instruction(opcode = "bfi", variant = "bfi_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfbe08000) == 0xf0800000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", cls._thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff))),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "eor", variant = "eor_imm_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffe08000) == 0xea800000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("type", ((opcode & 0x30) >> 4)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "eor", variant = "eor_reg_T2", length = 4, arguments = arguments)
		elif (opcode & 0xff000000) == 0xbf000000:
			arguments = collections.OrderedDict([
				("firstcond", ((opcode & 0xf00000) >> 20)),
				("mask", ((opcode & 0xf0000) >> 16)),
			])
			return Instruction(opcode = "it", variant = "it_T1", length = 2, arguments = arguments)
		elif (opcode & 0xfe1f0000) == 0xfc1f0000:
			arguments = collections.OrderedDict([
				("imm", (opcode & 0xff)),
				("coproc", ((opcode & 0xf00) >> 8)),
				("CRd", ((opcode & 0xf000) >> 12)),
				("D", ((opcode & 0x400000) >> 22)),
				("P", ((opcode & 0x1000000) >> 24)),
				("U", ((opcode & 0x800000) >> 23)),
				("W", ((opcode & 0x200000) >> 21)),
			])
			return Instruction(opcode = "ldc", variant = "ldc2_lit_T2", length = 4, arguments = arguments)
		elif (opcode & 0xffd02000) == 0xe8900000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("register_list", (opcode & 0x1fff)),
				("P", ((opcode & 0x8000) >> 15)),
				("M", ((opcode & 0x4000) >> 14)),
				("W", ((opcode & 0x200000) >> 21)),
			])
			return Instruction(opcode = "ldm", variant = "ldm_T2", length = 4, arguments = arguments)
		elif (opcode & 0xff7f0000) == 0xf85f0000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("imm", (opcode & 0xfff)),
				("U", ((opcode & 0x800000) >> 23)),
			])
			return Instruction(opcode = "ldr", variant = "ldr_lit_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfff00f00) == 0xe8500f00:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xff)),
			])
			return Instruction(opcode = "ldrex", variant = "ldrex_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff00f00) == 0xf8300e00:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xff)),
			])
			return Instruction(opcode = "ldrht", variant = "ldrht_T1", length = 4, arguments = arguments)
		elif (opcode & 0xff7f0000) == 0xf91f0000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("imm", (opcode & 0xfff)),
				("U", ((opcode & 0x800000) >> 23)),
			])
			return Instruction(opcode = "ldrsb", variant = "ldrsb_lit_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff00800) == 0xf9300800:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xff)),
				("P", ((opcode & 0x400) >> 10)),
				("U", ((opcode & 0x200) >> 9)),
				("W", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "ldrsh", variant = "ldrsh_imm_T2", length = 4, arguments = arguments)
		elif (opcode & 0xf8000000) == 0x0:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x380000) >> 19)),
				("imm", ((opcode & 0x7c00000) >> 22)),
			])
			return Instruction(opcode = "lsl", variant = "lsl_imm_T1", length = 2, arguments = arguments)
		elif (opcode & 0xffef8030) == 0xea4f0000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "lsl", variant = "lsl_imm_T2", length = 4, arguments = arguments)
		elif (opcode & 0xffef8030) == 0xea4f0010:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "lsr", variant = "lsr_imm_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfff000f0) == 0xfb000000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("Ra", ((opcode & 0xf000) >> 12)),
			])
			return Instruction(opcode = "mla", variant = "mla_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfbf08000) == 0xf2400000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("imm", ((opcode & 0x4000000) >> 15) | ((opcode & 0xf7000) >> 4) | (opcode & 0xff)),
			])
			return Instruction(opcode = "mov", variant = "mov_imm_T3", length = 4, arguments = arguments)
		elif (opcode & 0xfbe08000) == 0xf0600000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "orn", variant = "orn_imm_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffe08000) == 0xea600000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("type", ((opcode & 0x30) >> 4)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "orn", variant = "orn_reg_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfbe08000) == 0xf0400000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", cls._thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff))),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "orr", variant = "orr_imm_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0ffc0) == 0xf810f000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "pld", variant = "pld_reg_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffef0030) == 0xea4f0030:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "ror", variant = "ror_imm_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff000c0) == 0xfb100000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("Ra", ((opcode & 0xf000) >> 12)),
				("N", ((opcode & 0x20) >> 5)),
				("M", ((opcode & 0x10) >> 4)),
			])
			return Instruction(opcode = "smlabb", variant = "smlabb_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff000e0) == 0xfb200000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("Ra", ((opcode & 0xf000) >> 12)),
				("M", ((opcode & 0x10) >> 4)),
			])
			return Instruction(opcode = "smlad", variant = "smlad_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff000e0) == 0xfb300000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("Ra", ((opcode & 0xf000) >> 12)),
				("M", ((opcode & 0x10) >> 4)),
			])
			return Instruction(opcode = "smlawb", variant = "smlawb_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff000e0) == 0xfb400000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("Ra", ((opcode & 0xf000) >> 12)),
				("M", ((opcode & 0x10) >> 4)),
			])
			return Instruction(opcode = "smlsd", variant = "smlsd_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff000e0) == 0xfb500000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("Ra", ((opcode & 0xf000) >> 12)),
				("R", ((opcode & 0x10) >> 4)),
			])
			return Instruction(opcode = "smmla", variant = "smmla_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfbd08000) == 0xf3000000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("shift", ((opcode & 0x200000) >> 21)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("satimm", (opcode & 0x1f)),
			])
			return Instruction(opcode = "ssat", variant = "ssat_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfe100000) == 0xfc000000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xff)),
				("coproc", ((opcode & 0xf00) >> 8)),
				("CRd", ((opcode & 0xf000) >> 12)),
				("P", ((opcode & 0x1000000) >> 24)),
				("N", ((opcode & 0x400000) >> 22)),
				("U", ((opcode & 0x800000) >> 23)),
				("W", ((opcode & 0x200000) >> 21)),
			])
			return Instruction(opcode = "stc", variant = "stc2_T2", length = 4, arguments = arguments)
		elif (opcode & 0xffd0a000) == 0xe9000000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("register_list", (opcode & 0x1fff)),
				("M", ((opcode & 0x4000) >> 14)),
				("W", ((opcode & 0x200000) >> 21)),
			])
			return Instruction(opcode = "stmdb", variant = "stmdb_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff00800) == 0xf8000800:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xff)),
				("P", ((opcode & 0x400) >> 10)),
				("U", ((opcode & 0x200) >> 9)),
				("W", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "strb", variant = "strb_imm_T3", length = 4, arguments = arguments)
		elif (opcode & 0xfff00000) == 0xe8400000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xff)),
			])
			return Instruction(opcode = "strex", variant = "strex_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff00800) == 0xf8200800:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xff)),
				("P", ((opcode & 0x400) >> 10)),
				("U", ((opcode & 0x200) >> 9)),
				("W", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "strh", variant = "strh_imm_T3", length = 4, arguments = arguments)
		elif (opcode & 0xfff00f00) == 0xf8400e00:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xff)),
			])
			return Instruction(opcode = "strt", variant = "strt_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffef8000) == 0xebad0000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("type", ((opcode & 0x30) >> 4)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "sub", variant = "sub_SPreg_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfbf08000) == 0xf2a00000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff)),
			])
			return Instruction(opcode = "sub", variant = "sub_imm_T4", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f080) == 0xfa20f080:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("rotate", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "sxtab16", variant = "sxtab16_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f080) == 0xfa40f080:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("rotate", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "sxtab", variant = "sxtab_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f080) == 0xfa00f080:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("rotate", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "sxtah", variant = "sxtah_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f000) == 0xf7f0a000:
			arguments = collections.OrderedDict([
				("imm", ((opcode & 0xf0000) >> 4) | (opcode & 0xfff)),
			])
			return Instruction(opcode = "udf", variant = "udf_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfff000f0) == 0xfb700000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("Ra", ((opcode & 0xf000) >> 12)),
			])
			return Instruction(opcode = "usada8", variant = "usada8_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfbd08000) == 0xf3800000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("shift", ((opcode & 0x200000) >> 21)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("satimm", (opcode & 0x1f)),
			])
			return Instruction(opcode = "usat", variant = "usat_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f080) == 0xfa30f080:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("rotate", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "uxtab", variant = "uxtab16_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f080) == 0xfa50f080:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("rotate", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "uxtab", variant = "uxtab_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f080) == 0xfa10f080:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("rotate", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "uxtah", variant = "uxtah_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffbf0e50) == 0xeeb40a40:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("Vm", (opcode & 0xf)),
				("D", ((opcode & 0x400000) >> 22)),
				("E", ((opcode & 0x80) >> 7)),
				("M", ((opcode & 0x20) >> 5)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vcmp", variant = "vcmp_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffbf0e50) == 0xeeb50a40:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("D", ((opcode & 0x400000) >> 22)),
				("E", ((opcode & 0x80) >> 7)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vcmp", variant = "vcmp_T2", length = 4, arguments = arguments)
		elif (opcode & 0xffbc0e50) == 0xfebc0a40:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("Vm", (opcode & 0xf)),
				("op", ((opcode & 0x80) >> 7)),
				("RM", ((opcode & 0x30000) >> 16)),
				("D", ((opcode & 0x400000) >> 22)),
				("M", ((opcode & 0x20) >> 5)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vcvt", variant = "vcvt_fltint_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffbc0e50) == 0xfebc0a40:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("Vm", (opcode & 0xf)),
				("op", ((opcode & 0x80) >> 7)),
				("RM", ((opcode & 0x30000) >> 16)),
				("D", ((opcode & 0x400000) >> 22)),
				("M", ((opcode & 0x20) >> 5)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vcvta", variant = "vcvta_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff00f10) == 0xeef00a10:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
			])
			return Instruction(opcode = "vmrs", variant = "vmrs_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff00f10) == 0xeee00a10:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
			])
			return Instruction(opcode = "vmsr", variant = "vmsr_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffbf0f00) == 0xecbd0b00:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("imm", (opcode & 0xff)),
				("D", ((opcode & 0x400000) >> 22)),
			])
			return Instruction(opcode = "vpop", variant = "vpop_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffbf0f00) == 0xecbd0a00:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("imm", (opcode & 0xff)),
				("D", ((opcode & 0x400000) >> 22)),
			])
			return Instruction(opcode = "vpop", variant = "vpop_T2", length = 4, arguments = arguments)
		elif (opcode & 0xffbf0f00) == 0xed2d0b00:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("imm", (opcode & 0xff)),
				("D", ((opcode & 0x400000) >> 22)),
			])
			return Instruction(opcode = "vpush", variant = "vpush_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffbf0f00) == 0xed2d0a00:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("imm", (opcode & 0xff)),
				("D", ((opcode & 0x400000) >> 22)),
			])
			return Instruction(opcode = "vpush", variant = "vpush_T2", length = 4, arguments = arguments)
		elif (opcode & 0xffbf0e50) == 0xeeb60a40:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("Vm", (opcode & 0xf)),
				("op", ((opcode & 0x80) >> 7)),
				("D", ((opcode & 0x400000) >> 22)),
				("M", ((opcode & 0x20) >> 5)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vrintz", variant = "vrintz_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfbe08000) == 0xf1000000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", cls._thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff))),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "add", variant = "add_imm_T3", length = 4, arguments = arguments)
		elif (opcode & 0xff000000) == 0x44000000:
			arguments = collections.OrderedDict([
				("Rdn", ((opcode & 0x800000) >> 20) | ((opcode & 0x70000) >> 16)),
				("Rm", ((opcode & 0x780000) >> 19)),
			])
			return Instruction(opcode = "add", variant = "add_reg_T2", length = 2, arguments = arguments)
		elif (opcode & 0xffe08000) == 0xeb000000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("type", ((opcode & 0x30) >> 4)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "add", variant = "add_reg_T3", length = 4, arguments = arguments)
		elif (opcode & 0xfe100000) == 0xfc100000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xff)),
				("coproc", ((opcode & 0xf00) >> 8)),
				("CRd", ((opcode & 0xf000) >> 12)),
				("D", ((opcode & 0x400000) >> 22)),
				("P", ((opcode & 0x1000000) >> 24)),
				("U", ((opcode & 0x800000) >> 23)),
				("W", ((opcode & 0x200000) >> 21)),
			])
			return Instruction(opcode = "ldc", variant = "ldc2_imm_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfff00fc0) == 0xf8500000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "ldr", variant = "ldr_reg_T2", length = 4, arguments = arguments)
		elif (opcode & 0xff7f0000) == 0xf83f0000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("imm", (opcode & 0xfff)),
				("U", ((opcode & 0x800000) >> 23)),
			])
			return Instruction(opcode = "ldrh", variant = "ldrh_lit_T1", length = 4, arguments = arguments)
		elif (opcode & 0xff7f0000) == 0xf93f0000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("imm", (opcode & 0xfff)),
				("U", ((opcode & 0x800000) >> 23)),
			])
			return Instruction(opcode = "ldrsh", variant = "ldrsh_lit_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffe08000) == 0xea400000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("type", ((opcode & 0x30) >> 4)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "orr", variant = "orr_reg_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfff0ff00) == 0xf810fc00:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xff)),
			])
			return Instruction(opcode = "pld", variant = "pld_imm_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfff0ff00) == 0xf910fc00:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xff)),
			])
			return Instruction(opcode = "pli", variant = "pld_immlit_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfff0d7ff) == 0xf3a08004:
			arguments = collections.OrderedDict([
			])
			return Instruction(opcode = "sev", variant = "sev_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfff00800) == 0xf8400800:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xff)),
				("P", ((opcode & 0x400) >> 10)),
				("U", ((opcode & 0x200) >> 9)),
				("W", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "str", variant = "str_imm_T4", length = 4, arguments = arguments)
		elif (opcode & 0xfe500000) == 0xe8400000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rtx", ((opcode & 0xf00) >> 8)),
				("imm", (opcode & 0xff)),
				("P", ((opcode & 0x1000000) >> 24)),
				("U", ((opcode & 0x800000) >> 23)),
				("W", ((opcode & 0x200000) >> 21)),
			])
			return Instruction(opcode = "strd", variant = "strd_imm_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffe08000) == 0xeba00000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x7000) >> 10) | ((opcode & 0xc0) >> 6)),
				("type", ((opcode & 0x30) >> 4)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "sub", variant = "sub_reg_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfff000e0) == 0xe8d00000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("H", ((opcode & 0x10) >> 4)),
			])
			return Instruction(opcode = "tbb", variant = "tbb_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffbe0e50) == 0xeeb20a40:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("Vm", (opcode & 0xf)),
				("op", ((opcode & 0x10000) >> 16)),
				("D", ((opcode & 0x400000) >> 22)),
				("M", ((opcode & 0x20) >> 5)),
				("T", ((opcode & 0x80) >> 7)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vcvtb", variant = "vcvtb_T1", length = 4, arguments = arguments)
		elif (opcode & 0xff300f00) == 0xed100b00:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Vd", ((opcode & 0xf000) >> 12)),
				("imm", (opcode & 0xff)),
				("D", ((opcode & 0x400000) >> 22)),
				("U", ((opcode & 0x800000) >> 23)),
			])
			return Instruction(opcode = "vldr", variant = "vldr_T1", length = 4, arguments = arguments)
		elif (opcode & 0xff300f00) == 0xed100a00:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Vd", ((opcode & 0xf000) >> 12)),
				("imm", (opcode & 0xff)),
				("D", ((opcode & 0x400000) >> 22)),
				("U", ((opcode & 0x800000) >> 23)),
			])
			return Instruction(opcode = "vldr", variant = "vldr_T2", length = 4, arguments = arguments)
		elif (opcode & 0xffb00e10) == 0xfe800a00:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("Vn", ((opcode & 0xf0000) >> 16)),
				("Vm", (opcode & 0xf)),
				("op", ((opcode & 0x40) >> 6)),
				("D", ((opcode & 0x400000) >> 22)),
				("N", ((opcode & 0x80) >> 7)),
				("M", ((opcode & 0x20) >> 5)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vmaxnm", variant = "vmaxnm_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffe00f10) == 0xee000a10:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Vn", ((opcode & 0xf0000) >> 16)),
				("op", ((opcode & 0x100000) >> 20)),
				("N", ((opcode & 0x80) >> 7)),
			])
			return Instruction(opcode = "vmov", variant = "vmov_armsp_T1", length = 4, arguments = arguments)
		elif (opcode & 0xff300f00) == 0xed000b00:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Vd", ((opcode & 0xf000) >> 12)),
				("imm", (opcode & 0xff)),
				("D", ((opcode & 0x400000) >> 22)),
				("U", ((opcode & 0x800000) >> 23)),
			])
			return Instruction(opcode = "vstr", variant = "vstr_T1", length = 4, arguments = arguments)
		elif (opcode & 0xff300f00) == 0xed000a00:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Vd", ((opcode & 0xf000) >> 12)),
				("imm", (opcode & 0xff)),
				("D", ((opcode & 0x400000) >> 22)),
				("U", ((opcode & 0x800000) >> 23)),
			])
			return Instruction(opcode = "vstr", variant = "vstr_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfff0d7ff) == 0xf3a08002:
			arguments = collections.OrderedDict([
			])
			return Instruction(opcode = "wfe", variant = "wfe_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfff0d7ff) == 0xf3a08003:
			arguments = collections.OrderedDict([
			])
			return Instruction(opcode = "wfi", variant = "wfi_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfff0d7ff) == 0xf3a08001:
			arguments = collections.OrderedDict([
			])
			return Instruction(opcode = "yield", variant = "yield_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfff0d7f0) == 0xf3a080f0:
			arguments = collections.OrderedDict([
				("option", (opcode & 0xf)),
			])
			return Instruction(opcode = "dbg", variant = "dbg_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfe5f0000) == 0xe85f0000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rtx", ((opcode & 0xf00) >> 8)),
				("imm", (opcode & 0xff)),
				("P", ((opcode & 0x1000000) >> 24)),
				("U", ((opcode & 0x800000) >> 23)),
				("W", ((opcode & 0x200000) >> 21)),
			])
			return Instruction(opcode = "ldrd", variant = "ldrd_lit_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff00800) == 0xf8300800:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xff)),
				("P", ((opcode & 0x400) >> 10)),
				("U", ((opcode & 0x200) >> 9)),
				("W", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "ldrh", variant = "ldrh_imm_T3", length = 4, arguments = arguments)
		elif (opcode & 0xfff00000) == 0xf9b00000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xfff)),
			])
			return Instruction(opcode = "ldrsh", variant = "ldrsh_imm_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff00f00) == 0xf8500e00:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xff)),
			])
			return Instruction(opcode = "ldrt", variant = "ldrt_T1", length = 4, arguments = arguments)
		elif (opcode & 0xff100010) == 0xee000010:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("coproc", ((opcode & 0xf00) >> 8)),
				("CRn", ((opcode & 0xf0000) >> 16)),
				("CRm", (opcode & 0xf)),
				("opcA", ((opcode & 0xe00000) >> 21)),
				("opcB", ((opcode & 0xe0) >> 5)),
			])
			return Instruction(opcode = "mcr", variant = "mcr_T1", length = 4, arguments = arguments)
		elif (opcode & 0xff100010) == 0xee100010:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("coproc", ((opcode & 0xf00) >> 8)),
				("Crn", ((opcode & 0xf0000) >> 16)),
				("CRm", (opcode & 0xf)),
				("opcA", ((opcode & 0xe00000) >> 21)),
				("opcB", ((opcode & 0xe0) >> 5)),
			])
			return Instruction(opcode = "mrc", variant = "mrc_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f000) == 0xf890f000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xfff)),
			])
			return Instruction(opcode = "pld", variant = "pld_imm_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0f000) == 0xf990f000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xfff)),
			])
			return Instruction(opcode = "pli", variant = "pld_immlit_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffba0e50) == 0xeeba0a40:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("imm", ((opcode & 0x20) >> 5) | ((opcode & 0xf) << 1)),
				("op", ((opcode & 0x40000) >> 18)),
				("D", ((opcode & 0x400000) >> 22)),
				("U", ((opcode & 0x10000) >> 16)),
				("sf", ((opcode & 0x100) >> 8)),
				("sx", ((opcode & 0x80) >> 7)),
			])
			return Instruction(opcode = "vcvt", variant = "vcvt_fltfpt_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfe100f00) == 0xec100b00:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Vd", ((opcode & 0xf000) >> 12)),
				("imm", (opcode & 0xff)),
				("D", ((opcode & 0x400000) >> 22)),
				("P", ((opcode & 0x1000000) >> 24)),
				("U", ((opcode & 0x800000) >> 23)),
				("W", ((opcode & 0x200000) >> 21)),
			])
			return Instruction(opcode = "vldm", variant = "vldm_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfe100f00) == 0xec100a00:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Vd", ((opcode & 0xf000) >> 12)),
				("imm", (opcode & 0xff)),
				("D", ((opcode & 0x400000) >> 22)),
				("P", ((opcode & 0x1000000) >> 24)),
				("U", ((opcode & 0x800000) >> 23)),
				("W", ((opcode & 0x200000) >> 21)),
			])
			return Instruction(opcode = "vldm", variant = "vldm_T2", length = 4, arguments = arguments)
		elif (opcode & 0xff800e50) == 0xfe000a00:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("Vn", ((opcode & 0xf0000) >> 16)),
				("Vm", (opcode & 0xf)),
				("cond", ((opcode & 0x300000) >> 20)),
				("D", ((opcode & 0x400000) >> 22)),
				("N", ((opcode & 0x80) >> 7)),
				("M", ((opcode & 0x20) >> 5)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vsel", variant = "vsel_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfe100f00) == 0xec000b00:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Vd", ((opcode & 0xf000) >> 12)),
				("imm", (opcode & 0xff)),
				("D", ((opcode & 0x400000) >> 22)),
				("P", ((opcode & 0x1000000) >> 24)),
				("U", ((opcode & 0x800000) >> 23)),
				("W", ((opcode & 0x200000) >> 21)),
			])
			return Instruction(opcode = "vstm", variant = "vstm_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfe100f00) == 0xec000a00:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Vd", ((opcode & 0xf000) >> 12)),
				("imm", (opcode & 0xff)),
				("D", ((opcode & 0x400000) >> 22)),
				("P", ((opcode & 0x1000000) >> 24)),
				("U", ((opcode & 0x800000) >> 23)),
				("W", ((opcode & 0x200000) >> 21)),
			])
			return Instruction(opcode = "vstm", variant = "vstm_T2", length = 4, arguments = arguments)
		elif (opcode & 0xff000010) == 0xfe000000:
			arguments = collections.OrderedDict([
				("coproc", ((opcode & 0xf00) >> 8)),
				("CRd", ((opcode & 0xf000) >> 12)),
				("CRn", ((opcode & 0xf0000) >> 16)),
				("CRm", (opcode & 0xf)),
				("opcA", ((opcode & 0xf00000) >> 20)),
				("opcB", ((opcode & 0xe0) >> 5)),
			])
			return Instruction(opcode = "cdp", variant = "cdp2_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfff0d0f0) == 0xf3b08020:
			arguments = collections.OrderedDict([
			])
			return Instruction(opcode = "clrex", variant = "clrex_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0d0f0) == 0xf3b08050:
			arguments = collections.OrderedDict([
				("option", (opcode & 0xf)),
			])
			return Instruction(opcode = "dmb", variant = "dmb_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0d0f0) == 0xf3b08040:
			arguments = collections.OrderedDict([
				("option", (opcode & 0xf)),
			])
			return Instruction(opcode = "dsb", variant = "dsb_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff0d0f0) == 0xf3b08060:
			arguments = collections.OrderedDict([
				("option", (opcode & 0xf)),
			])
			return Instruction(opcode = "isb", variant = "isb_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfe1f0000) == 0xec1f0000:
			arguments = collections.OrderedDict([
				("imm", (opcode & 0xff)),
				("coproc", ((opcode & 0xf00) >> 8)),
				("CRd", ((opcode & 0xf000) >> 12)),
				("D", ((opcode & 0x400000) >> 22)),
				("P", ((opcode & 0x1000000) >> 24)),
				("U", ((opcode & 0x800000) >> 23)),
				("W", ((opcode & 0x200000) >> 21)),
			])
			return Instruction(opcode = "ldc", variant = "ldc_lit_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff00800) == 0xf8500800:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xff)),
				("P", ((opcode & 0x400) >> 10)),
				("U", ((opcode & 0x200) >> 9)),
				("W", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "ldr", variant = "ldr_imm_T4", length = 4, arguments = arguments)
		elif (opcode & 0xfff00fc0) == 0xf8100000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "ldrb", variant = "ldrb_reg_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfe500000) == 0xe8500000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rtx", ((opcode & 0xf00) >> 8)),
				("imm", (opcode & 0xff)),
				("P", ((opcode & 0x1000000) >> 24)),
				("U", ((opcode & 0x800000) >> 23)),
				("W", ((opcode & 0x200000) >> 21)),
			])
			return Instruction(opcode = "ldrd", variant = "ldrd_imm_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff00000) == 0xf8b00000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xfff)),
			])
			return Instruction(opcode = "ldrh", variant = "ldrh_imm_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfff00fc0) == 0xf9100000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("Rm", (opcode & 0xf)),
				("imm", ((opcode & 0x30) >> 4)),
			])
			return Instruction(opcode = "ldrsb", variant = "ldrsb_reg_T2", length = 4, arguments = arguments)
		elif (opcode & 0xffb00e50) == 0xee300a00:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("Vn", ((opcode & 0xf0000) >> 16)),
				("Vm", (opcode & 0xf)),
				("D", ((opcode & 0x400000) >> 22)),
				("N", ((opcode & 0x80) >> 7)),
				("M", ((opcode & 0x20) >> 5)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vadd", variant = "vadd_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffb00e50) == 0xee800a00:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("Vn", ((opcode & 0xf0000) >> 16)),
				("Vm", (opcode & 0xf)),
				("D", ((opcode & 0x400000) >> 22)),
				("N", ((opcode & 0x80) >> 7)),
				("M", ((opcode & 0x20) >> 5)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vdiv", variant = "vdiv_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffb00e50) == 0xeeb00a00:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("imm", ((opcode & 0xf0000) >> 12) | (opcode & 0xf)),
				("D", ((opcode & 0x400000) >> 22)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vmov", variant = "vmov_imm_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffb00e50) == 0xee200a00:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("Vn", ((opcode & 0xf0000) >> 16)),
				("Vm", (opcode & 0xf)),
				("D", ((opcode & 0x400000) >> 22)),
				("N", ((opcode & 0x80) >> 7)),
				("M", ((opcode & 0x20) >> 5)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vmul", variant = "vmul_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffb00e50) == 0xee200a40:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("Vn", ((opcode & 0xf0000) >> 16)),
				("Vm", (opcode & 0xf)),
				("D", ((opcode & 0x400000) >> 22)),
				("N", ((opcode & 0x80) >> 7)),
				("M", ((opcode & 0x20) >> 5)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vnmla", variant = "vnmla_T2", length = 4, arguments = arguments)
		elif (opcode & 0xffb00e50) == 0xee300a40:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("Vn", ((opcode & 0xf0000) >> 16)),
				("Vm", (opcode & 0xf)),
				("D", ((opcode & 0x400000) >> 22)),
				("N", ((opcode & 0x80) >> 7)),
				("M", ((opcode & 0x20) >> 5)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vsub", variant = "vsub_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff00000) == 0xf8d00000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xfff)),
			])
			return Instruction(opcode = "ldr", variant = "ldr_imm_T3", length = 4, arguments = arguments)
		elif (opcode & 0xfff00f00) == 0xf8100e00:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xff)),
			])
			return Instruction(opcode = "ldrbt", variant = "ldrbt_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff00f00) == 0xf9100e00:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xff)),
			])
			return Instruction(opcode = "ldrsbt", variant = "ldrsbt_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff00000) == 0xec400000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("coproc", ((opcode & 0xf00) >> 8)),
				("CRm", (opcode & 0xf)),
				("opcA", ((opcode & 0xf0) >> 4)),
				("RtB", ((opcode & 0xf0000) >> 16)),
			])
			return Instruction(opcode = "mcrr", variant = "mcrr_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff00000) == 0xec500000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("coproc", ((opcode & 0xf00) >> 8)),
				("CRm", (opcode & 0xf)),
				("opcA", ((opcode & 0xf0) >> 4)),
				("RtB", ((opcode & 0xf0000) >> 16)),
			])
			return Instruction(opcode = "mrrc", variant = "mrrc_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffe0d000) == 0xf3e08000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("SYSm", (opcode & 0xff)),
			])
			return Instruction(opcode = "mrs", variant = "mrs_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffe0d000) == 0xf3808000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("mask", ((opcode & 0xc00) >> 10)),
				("SYSm", (opcode & 0xff)),
			])
			return Instruction(opcode = "msr", variant = "msr_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffb00e10) == 0xeea00a00:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("Vn", ((opcode & 0xf0000) >> 16)),
				("Vm", (opcode & 0xf)),
				("op", ((opcode & 0x40) >> 6)),
				("D", ((opcode & 0x400000) >> 22)),
				("N", ((opcode & 0x80) >> 7)),
				("M", ((opcode & 0x20) >> 5)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vfma", variant = "vfma_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffb00e10) == 0xee900a00:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("Vn", ((opcode & 0xf0000) >> 16)),
				("Vm", (opcode & 0xf)),
				("op", ((opcode & 0x40) >> 6)),
				("D", ((opcode & 0x400000) >> 22)),
				("N", ((opcode & 0x80) >> 7)),
				("M", ((opcode & 0x20) >> 5)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vfnma", variant = "vfnma_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffb00e10) == 0xee000a00:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("Vn", ((opcode & 0xf0000) >> 16)),
				("Vm", (opcode & 0xf)),
				("op", ((opcode & 0x40) >> 6)),
				("D", ((opcode & 0x400000) >> 22)),
				("N", ((opcode & 0x80) >> 7)),
				("M", ((opcode & 0x20) >> 5)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vmla", variant = "vmla_T1", length = 4, arguments = arguments)
		elif (opcode & 0xffb00e10) == 0xee100a00:
			arguments = collections.OrderedDict([
				("Vd", ((opcode & 0xf000) >> 12)),
				("Vn", ((opcode & 0xf0000) >> 16)),
				("Vm", (opcode & 0xf)),
				("op", ((opcode & 0x40) >> 6)),
				("D", ((opcode & 0x400000) >> 22)),
				("N", ((opcode & 0x80) >> 7)),
				("M", ((opcode & 0x20) >> 5)),
				("sz", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "vnmla", variant = "vnmla_T1", length = 4, arguments = arguments)
		elif (opcode & 0xff000010) == 0xee000000:
			arguments = collections.OrderedDict([
				("coproc", ((opcode & 0xf00) >> 8)),
				("CRd", ((opcode & 0xf000) >> 12)),
				("CRn", ((opcode & 0xf0000) >> 16)),
				("CRm", (opcode & 0xf)),
				("opcA", ((opcode & 0xf00000) >> 20)),
				("opcB", ((opcode & 0xe0) >> 5)),
			])
			return Instruction(opcode = "cdp", variant = "cdp_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfe100000) == 0xec100000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xff)),
				("coproc", ((opcode & 0xf00) >> 8)),
				("CRd", ((opcode & 0xf000) >> 12)),
				("D", ((opcode & 0x400000) >> 22)),
				("P", ((opcode & 0x1000000) >> 24)),
				("U", ((opcode & 0x800000) >> 23)),
				("W", ((opcode & 0x200000) >> 21)),
			])
			return Instruction(opcode = "ldc", variant = "ldc_imm_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff00800) == 0xf8100800:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xff)),
				("P", ((opcode & 0x400) >> 10)),
				("U", ((opcode & 0x200) >> 9)),
				("W", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "ldrb", variant = "ldrb_imm_T3", length = 4, arguments = arguments)
		elif (opcode & 0xfff00800) == 0xf9100800:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xff)),
				("P", ((opcode & 0x400) >> 10)),
				("U", ((opcode & 0x200) >> 9)),
				("W", ((opcode & 0x100) >> 8)),
			])
			return Instruction(opcode = "ldrsb", variant = "ldrsb_imm_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfe100000) == 0xec000000:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xff)),
				("coproc", ((opcode & 0xf00) >> 8)),
				("CRd", ((opcode & 0xf000) >> 12)),
				("P", ((opcode & 0x1000000) >> 24)),
				("N", ((opcode & 0x400000) >> 22)),
				("U", ((opcode & 0x800000) >> 23)),
				("W", ((opcode & 0x200000) >> 21)),
			])
			return Instruction(opcode = "stc", variant = "stc_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfbef8000) == 0xf1ad0000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("imm", ((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff)),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "sub", variant = "sub_SPimm_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfbf00f00) == 0xf1b00f00:
			arguments = collections.OrderedDict([
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", cls._thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff))),
			])
			return Instruction(opcode = "cmp", variant = "cmp_imm_T2", length = 4, arguments = arguments)
		elif (opcode & 0xff7f0000) == 0xf81f0000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("imm", (opcode & 0xfff)),
				("U", ((opcode & 0x800000) >> 23)),
			])
			return Instruction(opcode = "ldrb", variant = "ldrb_lit_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff00000) == 0xf9900000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xfff)),
			])
			return Instruction(opcode = "ldrsb", variant = "ldrsb_imm_T1", length = 4, arguments = arguments)
		elif (opcode & 0xfff00000) == 0xf8900000:
			arguments = collections.OrderedDict([
				("Rt", ((opcode & 0xf000) >> 12)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", (opcode & 0xfff)),
			])
			return Instruction(opcode = "ldrb", variant = "ldrb_imm_T2", length = 4, arguments = arguments)
		elif (opcode & 0xfbe08000) == 0xf1a00000:
			arguments = collections.OrderedDict([
				("Rd", ((opcode & 0xf00) >> 8)),
				("Rn", ((opcode & 0xf0000) >> 16)),
				("imm", cls._thumb_expand_imm(((opcode & 0x4000000) >> 15) | ((opcode & 0x7000) >> 4) | (opcode & 0xff))),
				("S", ((opcode & 0x100000) >> 20)),
			])
			return Instruction(opcode = "sub", variant = "sub_imm_T3", length = 4, arguments = arguments)
		elif (opcode & 0xf800d000) == 0xf0008000:
			arguments = collections.OrderedDict([
				("imm", cls._thumb_sign_extend20(((opcode & 0x4000000) >> 7) | ((opcode & 0x3f0000) >> 5) | (opcode & 0x7ff) | ((opcode & 0x2000) << 4) | ((opcode & 0x800) << 7))),
				("cond", ((opcode & 0x3c00000) >> 22)),
			])
			return Instruction(opcode = "b", variant = "b_T3", length = 4, arguments = arguments)
		elif (opcode & 0xf800d000) == 0xf0009000:
			arguments = collections.OrderedDict([
				("imm", cls._thumb_sign_extend24_EOR(((opcode & 0x3ff0000) >> 5) | ((opcode & 0x4000000) >> 3) | (opcode & 0x7ff) | ((opcode & 0x2000) << 9) | ((opcode & 0x800) << 10))),
			])
			return Instruction(opcode = "b", variant = "b_T4", length = 4, arguments = arguments)
		elif (opcode & 0xf800d000) == 0xf000d000:
			arguments = collections.OrderedDict([
				("imm", cls._thumb_sign_extend24_EOR(((opcode & 0x3ff0000) >> 5) | ((opcode & 0x4000000) >> 3) | (opcode & 0x7ff) | ((opcode & 0x2000) << 9) | ((opcode & 0x800) << 10))),
			])
			return Instruction(opcode = "bl", variant = "bl_T1", length = 4, arguments = arguments)
		raise NotImplementedError("Unable to decode opcode 0x%x" % (opcode))
	
	@classmethod
	def decode_data(cls, data):
		if len(data) >= 4:
			opcode = (data[1] << 24) | (data[0] << 16) | (data[3] << 8) | (data[2] << 0)
		else:
			opcode = (data[1] << 24) | (data[0] << 16)
		return cls.decode_opcode(opcode)
# vim: set filetype=python:
