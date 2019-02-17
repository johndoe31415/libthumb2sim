#!/usr/bin/python3
import gdb

class GDBHelper(object):
	def __init__(self):
		self._uint32_t = gdb.lookup_type("unsigned int")

	def set_register(self, regno, value):
		assert(0 <= regno <= 15)
		assert(0 <= value <= 0xffffffff)
		gdb.execute("set $r%d=0x%x" % (regno, value))

	def get_register(self, regno):
		assert(0 <= regno <= 15)
		value = gdb.parse_and_eval("$r%d" % (regno))

		# GDB complains if we take r13 or r15 as an integer, need to cast...
		value = value.cast(self._uint32_t)

		value = int(value)
		return value

	def get_registers(self):
		return { i: self.get_register(i) for i in range(16) }

	def step(self):
		gdb.execute("si")

	def quit(self):
		gdb.execute("detach")
		gdb.execute("quit")

helper = GDBHelper()
print(helper.get_registers())
helper.quit()
