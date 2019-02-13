#!/usr/bin/python3
#	libthumb2sim - Emulator for the Thumb-2 ISA (Cortex-M)
#	Copyright (C) 2014-2019 Johannes Bauer
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

import tpg

class FieldElement(object):
	def __init__(self):
		self._next = [ ]

	def __iter__(self):
		yield self
		yield from self._next

	def append(self, nextelement):
		self._next.append(nextelement)
		return self

	def __repr__(self):
		return str(self)

class ConstantField(FieldElement):
	def __init__(self, constant):
		FieldElement.__init__(self)
		self._dontcare = constant.endswith("?")
		self._constant = constant.replace(" ", "").rstrip("?")

	@property
	def fieldtype(self):
		return "constant"

	@property
	def value(self):
		return int(self._constant, 2)

	@property
	def dontcare(self):
		return self._dontcare

	def __len__(self):
		return len(self._constant)

	def __str__(self):
		return "Constant<%s>" % (self._constant)

class VariableField(FieldElement):
	def __init__(self, variable):
		FieldElement.__init__(self)
		self._variable = variable
		self._iter = 1
		self._pos = None

	@property
	def fieldtype(self):
		return "variable"

	@property
	def name(self):
		return self._variable

	@property
	def position(self):
		return self._pos

	def setiteration(self, iteration):
		self._iter = int(iteration[1:-1])
		return self

	def setposition(self, position):
		if isinstance(position, int):
			self._pos = position
		else:
			self._pos = int(position[1:])
		return self

	def __len__(self):
		return self._iter

	def __str__(self):
		return "Variable<%s, %s, %s>" % (self._variable, self._iter, self._pos)


class BitfieldParser(tpg.Parser, dict):
	r"""
		set lexer = ContextSensitiveLexer

		separator space		'\s+';

		token TOKEN_CONSTANT	'[0-1]+\??'												$ ConstantField
		token TOKEN_VARIABLE	'[A-Za-z_]+'											$ VariableField
		token TOKEN_ITERATION	'{\d+}'													$ str
		token TOKEN_POSITION	'\.\d+'													$ str

		START/f -> BitFields/f
		;

		BitFields/f -> BitField/f (BitField/n											$ f.append(n)
									)*
		;

		BitField/f -> ConstantBitField/f
						| VariableBitField/f
		;

		ConstantBitField/f -> TOKEN_CONSTANT/f
		;

		VariableBitField/f -> TOKEN_VARIABLE/f (TOKEN_ITERATION/i						$ f.setiteration(i)
												)?
												(TOKEN_POSITION/p						$ f.setposition(p)
												)?
		;

	"""
#	verbose = 2


if __name__ == "__main__":
	parser = BitfieldParser()
	#for l in [ "10010100 Foo{4} Blubb{9} Bar{3}.9 Bar.8 Bar.7 Bar.0 Moo 01 01 01 0011" ]:
	for l in [
			"10010100",
			"10010100 01",
			"10010100 01 01 01 0011",
			"10101 foo",
			"10101 foo{9} bar{3}",
			"10101 foo{9}.4 bar{3}.9",
			"10101 foo.4 foo.3 foo.1 foo.0 foo.2",
		]:
		print(l)
		try:
			parsed = parser(l)
			print("Parsed   : %s" % (str(parsed)))
			parsed = list(parsed)
			print("Parsed   : %s" % (str(parsed)))
		except tpg.Error as e:
			print("Error    : %s" % (str(e)))

		print("-" * 120)

