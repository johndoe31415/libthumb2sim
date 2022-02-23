#!/usr/bin/python3
#
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

import sys
import json
import zlib
import subprocess
import tempfile
import re
import base64
from FriendlyArgumentParser import FriendlyArgumentParser
from TraceReader import TraceReader
from InstructionSetDecoder import InstructionSetDecoder

def disassemble_insn_objdump(rom_image, pc):
	with tempfile.NamedTemporaryFile(suffix = ".bin") as f:
		f.write(rom_image)
		f.flush()
		disas = subprocess.check_output([ "arm-none-eabi-objdump", "-D", f.name, "-b", "binary", "-m", "arm", "-M", "force-thumb" ])
		lines = disas.decode("ascii").split("\n")

		search_re = re.compile("\s*%x:" % (pc))
		for (lineno, line) in enumerate(lines):
			if search_re.match(line):
				return (lines[lineno - 3 : lineno], line, lines[lineno + 1: lineno + 4])

class DeviationException(Exception): pass

class TraceComparator():
	_REGNAMES = [ "r%d" % (i) for i in range(16) ] + [ "psr" ]

	def __init__(self, args):
		self._args = args
		self._trace1 = TraceReader(args.trace1)
		self._trace2 = TraceReader(args.trace2)
		if self._trace1.structure != self._trace2.structure:
			raise Exception("Traces are structurally incompatible, cannot proceed with compare.")

	@property
	def structure(self):
		return self._trace1.structure

	@staticmethod
	def _decode_psr(psr):
		psr_str = ""
		psr_str += " N"[bool(psr & (1 << 31))]
		psr_str += " Z"[bool(psr & (1 << 30))]
		psr_str += " C"[bool(psr & (1 << 29))]
		psr_str += " V"[bool(psr & (1 << 28))]
		psr_str += " Q"[bool(psr & (1 << 27))]
		return psr_str

	@staticmethod
	def _regname(regname):
		return {
			"r12":		"ip",
			"r13":		"sp",
			"r14":		"lr",
			"r15":		"pc",
		}.get(regname, regname)

	@staticmethod
	def _hexdump(data):
		hex_data = " ".join("%02x" % (c) for c in data)
		return hex_data

	def TODO_print_bytes_difference(self, data1, data2, fixed_offset = 0):
		if len(data1) != len(data2):
			print("Length different: %d bytes / %d bytes" % (len(data1), len(data2)))

		cmp_len = min(len(data1), len(data2))
		bytes_per_row = 16
		for i in range(0, cmp_len, bytes_per_row):
			address = fixed_offset + i
			row1 = data1[i : i + bytes_per_row]
			row2 = data2[i : i + bytes_per_row]
			if row1 != row2:
				print("%8x: %s     %s" % (address, self._hexdump(row1), self._hexdump(row2)))

	def TODO_print_deviation_constant(self, comp1, comp2):
		v1 = self._trace1.getbytes(comp1["value"])
		v2 = self._trace2.getbytes(comp2["value"])
		self._print_bytes_difference(v1, v2)

	def TODO_print_deviation_registers(self, comp1, comp2):
		if ("regs" in comp1) and ("regs" in comp2):
			for regname in [ "r%d" % (i) for i in range(16) ]:
				if comp1["regs"][regname] != comp2["regs"][regname]:
					print("%3s %08x %08x" % (self._regname(regname), comp1["regs"][regname], comp2["regs"][regname]))
			if (comp1["regs"]["psr"] & 0xf8000000) != (comp2["regs"]["psr"] & 0xf8000000):
				print("PSR %08x %08x = %5s | %5s" % (comp1["regs"]["psr"], comp2["regs"]["psr"], self._decode_psr(comp1["regs"]["psr"]), self._decode_psr(comp2["regs"]["psr"])))

	def TODO_print_deviation_memory(self, offset, comp1, comp2):
		if ("raw" in comp1) and ("raw" in comp2):
			data1 = self._trace1.getbytes(comp1["raw"])
			data2 = self._trace2.getbytes(comp2["raw"])
			self._print_bytes_difference(data1, data2, fixed_offset = offset)

	def _print_tracepoint(self, point):
		for (structure, state) in zip(self.structure, point):
			if structure["name"] == "register_set":
				regstr = [ ]
				for regname in self._REGNAMES:
					regstr.append("%-3s %08x" % (self._regname(regname), state.data[regname]))
				for i in range(0, len(regstr), 4):
					print("  ".join(regstr[i : i + 4]))

				rel_pc = state.data["r15"] - self._trace1.rom_base
				if rel_pc >= 0:
					print()
					insn_data = self._trace1.rom_image[rel_pc : rel_pc + 4]
					disassembled = disassemble_insn_objdump(self._trace1.rom_image, rel_pc)
					if disassembled is not None:
						(prefix, insn, suffix) = disassembled
						for line in prefix:
							print("    %s" % (line))
						print()
						print(">>> %-60s  <<< deviating instruction" % (insn))
						print()
						for line in suffix:
							print("    %s" % (line))

				insn = InstructionSetDecoder.decode_data(insn_data)
				print()
				print("Instruction details: %s" % (insn))
				for (param_name, value) in insn:
					if insn.is_register(param_name):
						reg_value = state.data["r%d" % (value)]
						if reg_value & 0x80000000:
							# Negative
							signed_reg_value = reg_value - (2 ** 32)
							print("%5s r%-2d 0x%x / %d    signed: -%0x%x / %d" % (param_name, value, reg_value, reg_value, signed_reg_value, signed_reg_value))
						else:
							# Positive
							print("%5s r%-2d 0x%x / %d" % (param_name, value, reg_value, reg_value))
				print()

	def _print_deviation(self, struct_definition, state1_data, state2_data):
		if struct_definition["name"] == "register_set":
			for regname in self._REGNAMES:
				value1 = state1_data[regname]
				value2 = state2_data[regname]
				if value1 != value2:
					regname = self._regname(regname)
					if regname != "psr":
						print("%-5s   %08x  |  %08x    (XOR %08x)" % (regname, value1, value2, (value1 ^ value2) & 0xffffffff))
					else:
						print("%-5s   %-8s  |  %-80s" % (regname, self._decode_psr(value1), self._decode_psr(value2)))


			pass
		else:
			raise NotImplementedError(struct_definition["name"])

	def _compare_regset(self, regset):
		regset = dict(regset)
		#regset["psr"] = regset["psr"] & 0xf8000000
		regset["psr"] = regset["psr"] & 0xf0000000			# TODO THIS DOES NOT CHECK THE Q FLAG
		return regset

	def _compare_tracepoint(self, executed_insn_cnt, state1, state2):
		deviation = [ ]

		for (struct_definition, state1_element, state2_element) in zip(self._trace1.structure, state1, state2):
			state1_data = state1_element.data
			state2_data = state2_element.data
			if struct_definition["name"] == "register_set":
				state1_data = self._compare_regset(state1_data)
				state2_data = self._compare_regset(state2_data)
			if state1_data != state2_data:
				deviation.append((struct_definition, state1_data, state2_data))

		if len(deviation) > 0:
			print("Deviation in tracepoint after %d executed instructions in %d structural elements." % (executed_insn_cnt, len(deviation)))
			for (struct_definition, state1_data, state2_data) in deviation:
				self._print_deviation(struct_definition, state1_data, state2_data)
			print()
			print("~" * 120)
			previous_tracepoint = self._trace1.get_state_at(executed_insn_cnt - 1) or self._trace2.get_state_at(executed_insn_cnt - 1)
			if previous_tracepoint is None:
				print("Unable to determine instruction that led to that deviation.")
			else:
				print("This was the cause of the deviation:")
				self._print_tracepoint(previous_tracepoint)
			raise DeviationException()

	def compare(self):
		for (executed_insn_cnt, state1, state2) in self._trace1.align(self._trace2):
			self._compare_tracepoint(executed_insn_cnt, state1, state2)

parser = FriendlyArgumentParser(description = "Compare two trace files; where they differ, show the instruction and deviation details.")
parser.add_argument("trace1", metavar = "trace1_filename", help = "First trace for comparison")
parser.add_argument("trace2", metavar = "trace2_filename", help = "Second trace for comparison")
args = parser.parse_args(sys.argv[1:])

tc = TraceComparator(args)
try:
	tc.compare()
except DeviationException:
	sys.exit(1)
