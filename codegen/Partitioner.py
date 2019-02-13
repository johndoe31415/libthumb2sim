import collections
import itertools

PartitionElement = collections.namedtuple("PartitionElement", [ "identifier", "bitmask", "bitvalue" ])

class Partition(object):
	def __init__(self, criterion, if_elements, else_elements):
		self._criterion = criterion
		self._if_set = if_elements
		self._else_set = else_elements
		self._len = len(self._if_set) + len(self._else_set)
		
	def partition(self):
		print("Partition: %s %s" % (type(self._if_set), type(self._else_set)))

		# Try to partition if set
		if len(self._if_set) > 4:
			subpart = self._if_set.partition()
			print("IF SUB", subpart)
			if subpart is not None:
				self._if_set = subpart
				self._if_set.partition()

		# Try to partition if set
		if len(self._else_set) > 4:
			subpart = self._else_set.partition()
			print("ELSE SUB", subpart)
			if subpart is not None:
				self._else_set = subpart
				self._else_set.partition()

	def dump(self, indent = 0):
		indent_str = ("   " * indent)
		print(indent_str + "Partition 0x%x <%d + %d> {" % (self._criterion, len(self._if_set), len(self._else_set)))
		self._if_set.dump(indent + 1)
		print(indent_str + "} else {")
		self._else_set.dump(indent + 1)
		print(indent_str + "}")

	def __len__(self):
		return self._len

	def __str__(self):
		return "Partition<0x%x, %d / %d>" % (self._criterion, len(self._if_set), len(self._else_set))

class PartitionableSet(object):
	def __init__(self, initialitems = None):
		if initialitems is None:
			self._elements = [ ]
		else:
			self._elements = list(initialitems)

	@property
	def commonmask(self):
		common_mask = self._elements[0].bitmask
		for element in self._elements:
			common_mask &= element.bitmask
		return common_mask

	def add(self, element):
		self._elements.append(element)

	@staticmethod
	def _iterbits(value):
		bitno = 0
		while value > 0:
			if (value & 1) != 0:
				yield 1 << bitno
			value >>= 1
			bitno += 1

	def __len__(self):
		return len(self._elements)

	def discriminatorcnt(self, mask):
		return len(list(element for element in self._elements if ((element.bitvalue & mask) == 0)))
	
	def discriminate(self, mask):
		return [ PartitionableSet(element for element in self._elements if ((element.bitvalue & mask) == 0)),
				PartitionableSet(element for element in self._elements if ((element.bitvalue & mask) != 0))
		]

	@staticmethod
	def _all_combinations(values):
		for i in range(1, len(values)):
			yield from itertools.combinations(values, i)

	def partition(self):
		common = self.commonmask
		if common == 0:
			raise Exception("Cannot partition set with no common mask.")

		bitdict = { }

		(minscore, minmask) = (None, None)
		relevant_bits = list(self._iterbits(common))
		for mask in self._all_combinations(relevant_bits):
			mask = sum(mask)
			discriminatorcnt = self.discriminatorcnt(mask)
			remaining = len(self) - discriminatorcnt
			
			if (discriminatorcnt == 0) or (remaining == 0):
				# Unfit!
				continue

			if discriminatorcnt >= remaining:
				score = discriminatorcnt / remaining
			else:
				score = remaining / discriminatorcnt

			if (minscore is None) or (score < minscore):
				minscore = score
				minmask = mask

		if (minscore is not None) and (minscore < 65):
			# Partition found!
			(ifpart, elsepart) = self.discriminate(minmask)
			return Partition(minmask, ifpart, elsepart)
		else:
			if minscore is None:
				print("Cannot further partition set of %d elements, no solution.")
			else:
				print("Cannot further partition set of %d elements, minscore %.1f" % (len(self), minscore))
	
	def dump(self, indent = 0):
		indent_str = ("   " * indent)
		print(indent_str + "%d: [ %s ]" % (len(self), ", ".join(x.identifier for x in self._elements)))

class Partitioner(object):
	def __init__(self, bitwidth):
		self._bits = bitwidth
		self._rootset = PartitionableSet()

	def add(self, identifier, bitmask, bitvalue):
		self._rootset.add(PartitionElement(identifier = identifier, bitmask = bitmask, bitvalue = bitvalue))

	def partition(self):
		root_partition = self._rootset.partition()
#		print(root_partition)
		root_partition.partition()
		return root_partition

if __name__ == "__main__":
	from InsnSet import InsnSet
	iset = InsnSet("instructions.xml")

	partitioner = Partitioner(32)
	for opcode in iset.getopcodes(16):
		mask = opcode.bitfield.constant.staticvalue << 16
		value = opcode.bitfield.constantcmp << 16
		partitioner.add(opcode.name, mask, value)
	for opcode in iset.getopcodes(32):
		mask = opcode.bitfield.constant.staticvalue
		value = opcode.bitfield.constantcmp
		partitioner.add(opcode.name, mask, value)

	result = partitioner.partition()
	result.dump()



