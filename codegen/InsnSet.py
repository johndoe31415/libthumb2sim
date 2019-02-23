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
import itertools
import collections

from XMLParser import XMLParser
from BitfieldParser import BitfieldParser

class FieldShift(object):
	_FieldElem = collections.namedtuple("FieldElem", [ "srcmask", "shl" ])

	def __init__(self):
		self._components = { }
		self._len = 0
		self._shl = 0
		self._extend = None

	def add(self, srclobit, bitlen, dstlobit):
		shl = dstlobit - srclobit
		srcmask = ((1 << bitlen) - 1) << srclobit

		if shl not in self._components:
			self._components[shl] = self._FieldElem(shl = shl, srcmask = srcmask)
		else:
			prevcomp = self._components[shl]
			self._components[shl] = self._FieldElem(shl = shl, srcmask = srcmask | prevcomp.srcmask)
		self._len += bitlen

	def __len__(self):
		return self._len

	def _cshl(self, expr, shl):
		shl -= self._shl
		if shl > 0:
			return "((%s) << %d)" % (expr, shl)
		elif shl == 0:
			return "(%s)" % (expr)
		else:
			return "((%s) >> %d)" % (expr, -shl)

	def ctype(self):
		if self._extend is not None:
			return {
				"thumb_expand_imm":			"int32_t",
				"thumb_sign_extend20":		"int32_t",
				"thumb_sign_extend24":		"int32_t",
				"thumb_sign_extend24_EOR":	"int32_t",
			}[self._extend]

		length = len(self)
		if length == 1:
			return "bool"
		elif length <= 8:
			return "uint8_t"
		elif length <= 16:
			return "uint16_t"
		elif length <= 32:
			return "uint32_t"
		else:
			raise Exception("Not possible")

	def setshl(self, shlval):
		self._shl = shlval

	@property
	def staticmask(self):
		assert(len(self._components) == 1)
		assert(0 in self._components)
		return self._components[0].srcmask


	def origcexpression(self, varname = "inval"):
		# Group by shift first
		result = [ ]
		for (shl, component) in sorted(self._components.items()):
			result.append(self._cshl("%s & 0x%x" % (varname, component.srcmask << self._shl), shl))
		result = " | ".join(result)
		return result

	def cexpression(self, varname = "inval"):
		result = self.origcexpression(varname)
		if self._extend is not None:
			result = "%s(%s)" % (self._extend, result)
		return result

	def pyexpression(self, varname = "inval"):
		result = self.origcexpression(varname)
		if self._extend is not None:
			result = "cls._%s(%s)" % (self._extend, result)
		return result

	@property
	def hasextension(self):
		return self._extend is not None

	def setextend(self, extension):
		self._extend = extension

	def __str__(self):
		return self.cexpression("x")

class Bitfield(object):
	_REGISTER_ORDER = dict((value, index) for (index, value) in enumerate([
		"Rd", "Rdn", "Rdm",
		"Rt", "Rn", "Rm",
		"Rtx",
		"RdLo", "RdHi",
		"Vd", "Vn", "Vm",
		"rotate", "shift", "imm", "satimm", "op", "cond", "firstcond",
		"mask", "register_list",
		"SYSm",
		"type", "coproc",
		"CRd", "CRn", "Crn", "CRm", "opcA", "opcB", "Ra", "RtA", "RtB",
		"tb",
		"RM",
		"H", "D", "E", "P", "N", "M", "F", "I", "R", "S", "T", "U", "W",
		"j", "k",
		"sf", "sx", "sz",
		"msb", "option", "width",
		"Rmx",
	]))

	def __init__(self, bfstring):
		try:
			self._elements = list(BitfieldParser()(bfstring))
		except tpg.SyntacticError:
			raise Exception("Error: Cannot parse bitfield string '%s'." % (bfstring))
		self._bitlen = 0
		for element in self._elements:
			self._bitlen += len(element)

		variables = collections.defaultdict(int)
		for element in reversed(self._elements):
			if element.fieldtype == "variable":
				if element.position is None:
					newpos = variables[element.name]
					element.setposition(newpos)
					variables[element.name] += 1


		elements = [ ]
		shiftpos = 0
		for element in reversed(self._elements):
			elements.append((element, shiftpos))
			shiftpos += len(element)

		variables = collections.defaultdict(list)
		constants = [ ]
		for (element, shiftpos) in elements:
			if element.fieldtype == "constant":
				constants.append((element, shiftpos))
			else:
				variables[element.name].append((element, shiftpos))

		constantmask = FieldShift()
		self._constant_value = 0
		for (element, shiftpos) in constants:
			if not element.dontcare:
				constantmask.add(shiftpos, len(element), shiftpos)
				self._constant_value |= element.value << shiftpos
		self._constant_mask = constantmask.staticmask

		self._variables = { }
		for (varname, varcomponents) in variables.items():
			self._variables[varname] = FieldShift()
			varcomponents.sort(key = lambda x: x[0].position)
			dstpos = 0
			for (element, shiftpos) in varcomponents:
				self._variables[varname].add(shiftpos, len(element), dstpos)
				dstpos += len(element)
		self._widento = None

	def __getitem__(self, varname):
		variable = self._variables[varname]
		if self._widento is not None:
			variable.setshl(self._widenshiftleft())
		return variable

	@staticmethod
	def _regsortkey(regname):
		if regname not in Bitfield._REGISTER_ORDER:
			print("WARNING: No such register %s in order table." % (regname))
		return Bitfield._REGISTER_ORDER.get(regname, 999)

	@property
	def varnames(self):
		return sorted(self._variables.keys(), key = self._regsortkey)

	def _widenshiftleft(self):
		if self._widento is None:
			return 0
		else:
			return self._widento - len(self)

	@property
	def constantmask(self):
		return self._constant_mask << self._widenshiftleft()

	@property
	def constantcmp(self):
		return self._constant_value << self._widenshiftleft()

	def widen(self, widento):
		self._widento = widento

	def _get_dofmask(self):
		dofmask = (~self._constant_mask) & ((1 << len(self)) - 1)
		return dofmask

#	@staticmethod
#	def _all_combinations(inset):
#		for i in range(len(inset) + 1):
#			yield from itertools.combinations(inset, i)

	@staticmethod
	def _all_combinations(inbits):
		values = [ (0, 1 << bit) for bit in inbits ]
		yield from itertools.product(*values)

	def enumerate_all(self):
		dofmask = self._get_dofmask()
		dofbits = [ i for i in reversed(range(len(self))) if ((1 << i) & self._constant_mask) == 0 ]
		for setbits in self._all_combinations(dofbits):
			value = self._constant_value + sum(setbits)

			encoding = bytes((value >> (8 * i)) & 0xff for i in self._encoding_byteoffsets())
			yield encoding

	def _encoding_byteoffsets(self):
		if len(self) == 16:
			return [ 0, 1 ]
		elif len(self) == 32:
			return [ 2, 3, 0, 1 ]
		else:
			raise Exception(NotImplemented)

	def __len__(self):
		return self._bitlen

	def __str__(self):
		return "BitField<%d, %x, %x>" % (len(self), self.constantmask, self.constantcmp)

class _Opcode(object):
	def __init__(self, node):
		self._name = node["name"]
		self._variant = node["variant"]
		self._bitfield = Bitfield(node.encoding["bits"])
		if len(self._bitfield) not in [ 16, 32 ]:
			raise Exception("Bitfield length of %d bits invalid for opcode %s / %s encoded in line %d." % (len(self._bitfield), self._name, self._variant, node.getlinenumber()))
		self._bitfield.widen(32)
		self._insnlen = len(self._bitfield)
		if node.getchild("order") is not None:
			self._priority = int(node.order["priority"])
		else:
			self._priority = 0
		if node.getchild("extend") is not None:
			for extend in node.extend:
				(varname, exttype) = (extend["variable"], extend["type"])
				self.getvar(varname).setextend(exttype)

	@property
	def variablenames(self):
		return self._bitfield.varnames

	def getfullprototype(self):
		proto = [ "void *aCtx" ]
		for varname in self.variablenames:
			var = self.getvar(varname)
			proto.append("%s %s" % (var.ctype(), varname))
		return ", ".join(proto)

	@property
	def basename(self):
		return self._name

	@property
	def name(self):
		return self._variant

	@property
	def priority(self):
		return self._priority

	@property
	def bitfield(self):
		return self._bitfield

	def getvar(self, varname):
		return self._bitfield[varname]

	def itervars(self):
		for varname in self.variablenames:
			yield (varname, self.getvar(varname))

	def getstrmask(self):
		mask = self._bitfield.constantmask
		value = self._bitfield.constantcmp
		strmask = ""
		for i in reversed(range(32)):
			if mask & (1 << i):
				if value & (1 << i):
					strmask += "1"
				else:
					strmask += "0"
			else:
				strmask += " "
		return strmask

	def enumerate_all(self):
		return self._bitfield.enumerate_all()

	def _cmpkey(self):
		return (self.basename, self.name)

	def __hash__(self):
		return hash(self._cmpkey())

	def __lt__(self, other):
		return self._cmpkey() < other._cmpkey()

	def __len__(self):
		return self._insnlen

	def __str__(self):
		return "Opcode<%s, %s>" % (self.basename, self.name)

class InsnSet(object):
	def __init__(self, filename):
		xml = XMLParser().parsefile(filename)
		self._opcodes = [ ]
		for opcode in xml.opcode:
			if opcode.getchild("encoding") is None:
				print("Warning: Opcode %s has no encoding specificed in line %d." % (opcode["variant"], opcode.getlinenumber()))
				continue
			try:
				opcode = _Opcode(opcode)
				self._opcodes.append(opcode)
			except Exception as e:
				print("Opcode ignored, parsing error: %s" % (e))
		self._opcodes.sort(key = lambda x: (x.priority, x.name))
		self._opcodesbyname = { opcode.name: opcode for opcode in self._opcodes }

	def getopcode(self, name):
		return self._opcodesbyname[name]

	def getopcodes(self):
		return iter(self._opcodes)

if __name__ == "__main__":
	fs = FieldShift()
	fs.add(0, 4, 12)
	fs.add(4, 4, 8)
	fs.add(8, 4, 4)
	fs.add(12, 4, 0)
	print(fs.cexpression())

	fs = FieldShift()
	fs.add(0, 1, 3)
	fs.add(1, 1, 2)
	fs.add(2, 1, 1)
	fs.add(3, 1, 0)
	print(fs.cexpression())

	x = Bitfield("1010 Rd{4}.0 0101 Rd{4}.1")
	x.widen(32)
	print(x)
	print(x["Rd"])


