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

class TraceComparator(object):
	def __init__(self, args):
		self._args = args
		self._trace1 = TraceReader(args.trace1)
		self._trace2 = TraceReader(args.trace2)

	@staticmethod
	def _decode_psr(psr):
		psr_str = ""
		psr_str += " N"[bool(psr & (1 << 31))]
		psr_str += " Z"[bool(psr & (1 << 30))]
		psr_str += " C"[bool(psr & (1 << 29))]
		psr_str += " V"[bool(psr & (1 << 28))]
		#psr_str += " Q"[bool(psr & (1 << 27))]
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

	def _print_bytes_difference(self, data1, data2, fixed_offset = 0):
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

	def _print_deviation_constant(self, comp1, comp2):
		v1 = self._trace1.get_bytes(comp1["value"])
		v2 = self._trace2.get_bytes(comp2["value"])
		self._print_bytes_difference(v1, v2)

	def _print_deviation_registers(self, comp1, comp2):
		if ("regs" in comp1) and ("regs" in comp2):
			for regname in [ "r%d" % (i) for i in range(16) ]:
				if comp1["regs"][regname] != comp2["regs"][regname]:
					print("%3s %08x %08x" % (self._regname(regname), comp1["regs"][regname], comp2["regs"][regname]))
			if (comp1["regs"]["psr"] & 0xf8000000) != (comp2["regs"]["psr"] & 0xf8000000):
				print("PSR %08x %08x = %5s | %5s" % (comp1["regs"]["psr"], comp2["regs"]["psr"], self._decode_psr(comp1["regs"]["psr"]), self._decode_psr(comp2["regs"]["psr"])))

	def _print_deviation_memory(self, offset, comp1, comp2):
		if ("raw" in comp1) and ("raw" in comp2):
			data1 = self._trace1.getbytes(comp1["raw"])
			data2 = self._trace2.getbytes(comp2["raw"])
			self._print_bytes_difference(data1, data2, fixed_offset = offset)

	def _print_deviation(self, component_no, comp1, comp2):
		print(("=" * 45) + " Deviating %s " % (self._trace1.structure[component_no]["name"]) + ("=" * 45))
		if self._trace1.structure[component_no]["type"] == "ConstantValue":
			self._print_deviation_constant(comp1, comp2)
		elif self._trace1.structure[component_no]["type"] == "RegisterHash":
			self._print_deviation_registers(comp1, comp2)
		elif self._trace1.structure[component_no]["type"] == "MemoryHash":
			self._print_deviation_memory(self._trace1.structure[component_no]["address"], comp1, comp2)

	def _print_tracepoint(self, point):
		for component in point["components"]:
			if "regs" in component:
				regs = component["regs"]
				print("Register set:")
				register_values = [ ]
				for regname in [ "r%d" % (i) for i in range(16) ]:
					register_values.append("%3s %08x" % (self._regname(regname), regs[regname]))

				regs_per_line = 4
				for rowno in range(len(register_values) // regs_per_line):
					row = (register_values[rowno + (i * regs_per_line) ] for i in range(regs_per_line))
					print("   ".join(row))
				print("PSR %08x %s" % (regs["psr"], self._decode_psr(regs["psr"])))

				print()
				(rom_image, rom_base) = (self._trace1.rom_image, self._trace1.rom_base)
				if rom_image is None:
					(rom_image, rom_base) = (self._trace2.rom_image, self._trace2.rom_base)

				if rom_image is not None:
					rel_pc = regs["r15"] - rom_base
					if rel_pc >= 0:
						insn_data = rom_image[rel_pc : rel_pc + 4]

					insn_hex = " ".join("%02x" % (c) for c in insn_data)
					disassembled = disassemble_insn_objdump(rom_image, rel_pc)
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
				else:
					print("No flash ROM available to decode instruction at PC.")

	def _compare_tracepoint(self, trace1, pt1, trace2, pt2):
		deviation = [ ]
		for (component_no, (comp1, comp2)) in enumerate(zip(pt1["components"], pt2["components"])):
			v1 = self._trace1.getbytes(comp1["value"])
			v2 = self._trace2.getbytes(comp2["value"])
#			print(component_no, v1, v2)
			if v1 != v2:
				deviation.append(component_no)

		if len(deviation) > 0:
			print("Deviation in tracepoint at number of executed instructions: %d" % (pt1["executed_insns"]))
			for component_no in deviation:
				self._print_deviation(component_no, pt1["components"][component_no], pt2["components"][component_no])
			print()
			print("~" * 120)
			previous_tracepoint = trace1.get_tracepoint_by_insn_cnt(pt1["executed_insns"] - 1) or trace2.get_tracepoint_by_insn_cnt(pt2["executed_insns"] - 1)
			if previous_tracepoint is None:
				print("Unable to determine instruction that led to that deviation.")
			else:
				print("This was the cause of the deviation:")
				self._print_tracepoint(previous_tracepoint)
			raise DeviationException()

	def compare(self):
		self._trace1.align(self._trace2, self._compare_tracepoint)

parser = FriendlyArgumentParser()
parser.add_argument("trace1", metavar = "trace_filename", type = str, help = "First trace for comparison")
parser.add_argument("trace2", metavar = "trace_filename", type = str, help = "Second trace for comparison")
args = parser.parse_args(sys.argv[1:])

tc = TraceComparator(args)
try:
	tc.compare()
except DeviationException:
	sys.exit(1)
