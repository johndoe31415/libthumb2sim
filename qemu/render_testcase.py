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

import os
import sys
import hashlib
import mako.lookup
import mako.template
from FriendlyArgumentParser import FriendlyArgumentParser

base_path = os.path.realpath(os.path.dirname(__file__))

class PRNG(object):
	def __init__(self, seed = 0):
		self._seed = hashlib.md5(seed.to_bytes(length = 8, byteorder = "little")).digest()
		self._block = 0
		self._buffer = bytearray()

	def _add_block(self):
		indata = self._seed + self._block.to_bytes(length = 8, byteorder = "little")
		self._buffer += hashlib.md5(indata).digest()
		self._block += 1

	def _fill(self, min_length):
		while len(self._buffer) < min_length:
			self._add_block()

	def _get_data(self, length):
		self._fill(length)
		return_data = self._buffer[ : length]
		self._buffer = self._buffer[length : ]
		return return_data

	def randbool(self):
		return bool(self.uint8() & 1)

	def uint8(self):
		return int.from_bytes(self._get_data(1), byteorder = "little")

	def uint16(self):
		return int.from_bytes(self._get_data(2), byteorder = "little")

	def uint32(self):
		return int.from_bytes(self._get_data(4), byteorder = "little")

	def randint(self, top):
		return self.uint32() % top

	def shuffle(self, array):
		l = len(array)
		order = list(range(l))
		for i in range(l):
			j = self.randint(i + 1)
			(order[i], order[j]) = (order[j], order[i])
		return [ array[order[i]] for i in range(l) ]

	def uint32_special(self, candidate_count = 2, pattern = "d~-<>0f78"):
		orig_candidates = [ self.uint32() for _ in range(candidate_count) ]
		candidates = list(orig_candidates)
		if "d" in pattern:
			# Duplication
			candidates += orig_candidates
		if "~" in pattern:
			# Bitwise negation
			candidates += [ (~c) & 0xffffffff for c in orig_candidates ]
		if "-" in pattern:
			# Arithmetic inversion
			candidates += [ (-c) & 0xffffffff for c in orig_candidates ]
		if "<" in pattern:
			# Decremented
			candidates += [ (c - 1) & 0xffffffff for c in orig_candidates ]
		if ">" in pattern:
			# Incremented
			candidates += [ (c + 1) & 0xffffffff for c in orig_candidates ]
		if "0" in pattern:
			# Zero
			candidates += [ 0 ]
		if "f" in pattern:
			# 0xffffffff
			candidates += [ 0xffffffff ]
		if "f" in pattern:
			# 0x7fffffff
			candidates += [ 0x7fffffff ]
		if "8" in pattern:
			# 0x80000000
			candidates += [ 0x80000000 ]

		candidates = list(sorted(set(candidates)))
		candidates = self.shuffle(candidates)
		return candidates

	def choices(self, values):
		if not isinstance(values, dict):
			values = { key: 1 for key in values }

		value_sum = sum(values.values())
		choice_value = self.randint(value_sum)
		s = 0
		for (key, value) in sorted(values.items()):
			s += value
			if choice_value < s:
				return key

class ThumbGenerator(object):
	def __init__(self, prng):
		self._label = 0
		self._prng = prng

	def reg8(self):
		return "r%d" % (self._prng.randint(8))

	def reg13(self):
		# Exclude SP, LR and PC
		return "r%d" % (self._prng.randint(13))

	def reg16(self):
		return "r%d" % (self._prng.randint(16))

	def new_label(self, count = 1):
		self._label += count
		return self.label()

	def label(self, number = 0):
		return ".tclbl_%d" % (self._label - number)

parser = FriendlyArgumentParser()
parser.add_argument("-s", "--seed", metavar = "value", type = int, default = 0, help = "Seed value to use for PRNG. Defaults to %(default)s. Different seed will generate different testcases.")
parser.add_argument("-l", "--length", metavar = "count", type = int, default = 1000, help = "Length of code to be generated. Defaults to %(default)d. Exact interpretation dependent on template, but usually correlates with the number of test cases.")
parser.add_argument("-f", "--force", action = "store_true", help = "Overwrite output file if it exists.")
parser.add_argument("-o", "--outfile", metavar = "filename", type = str, default = "testcase.s", help = "Output file to write rendered template to. Defaults to %(default)s.")
parser.add_argument("infile", metavar = "filename", help = "Input testcase body file to render.")
args = parser.parse_args(sys.argv[1:])

if (not args.force) and os.path.exists(args.outfile):
	print("Output file \"%s\" already exists, aborting." % (args.outfile))
	sys.exit(1)

lookup = mako.lookup.TemplateLookup(directories = [ base_path, base_path + "/templates" ], input_encoding = "utf-8", strict_undefined = True)
with open(args.infile) as f:
	template_src = f.read()
template = mako.template.Template(template_src, lookup = lookup)

prng = PRNG(args.seed)
thumbgen = ThumbGenerator(prng)
template_args = {
	"randb":		prng.randbool,
	"rand8":		prng.uint8,
	"rand16":		prng.uint16,
	"rand32":		prng.uint32,
	"spec32":		prng.uint32_special,
	"spec32_2":		lambda: tuple(prng.uint32_special(candidate_count = 2)[:2]),
	"reg8":			thumbgen.reg8,
	"reg13":		thumbgen.reg13,
	"reg16":		thumbgen.reg16,
	"new_label":	thumbgen.new_label,
	"label":		thumbgen.label,
	"length":		args.length,
	"seed":			args.seed,
	"infile":		args.infile,
	"choices":		prng.choices,
}
result = template.render(**template_args)

with open(args.outfile, "w") as f:
	f.write(result)
