#!/usr/bin/python3
import gdb

class GDBHelper(object):
	def __init__(self):
		self._uint32_t = gdb.lookup_type("unsigned int")

	def set_register(self, regno, value):
		assert(0 <= regno <= 15)
		assert(0 <= value <= 0xffffffff)
		gdb.execute("set $r%d=0x%x" % (regno, value))

	def get_register_by_name(self, regname):
		value = gdb.parse_and_eval("$%s" % (regname))

		# GDB complains if we take r13 or r15 as an integer, need to cast...
		value = value.cast(self._uint32_t)

		value = int(value)
		return value

	def get_register(self, regno):
		assert(0 <= regno <= 15)
		return self.get_register_by_name("r%d" % (regno))

	def get_registers(self):
		return { "r%d" % (i): self.get_register(i) for i in range(16) }

	def get_psr(self):
		psr = self.get_register_by_name("cpsr")
		flags = ""
		flags += "nN"[bool(psr & (1 << 31))]		# Negative
		flags += "zZ"[bool(psr & (1 << 30))]		# Zero
		flags += "cC"[bool(psr & (1 << 29))]		# Carry/borrow
		flags += "vV"[bool(psr & (1 << 28))]		# Overflow
		flags += "qQ"[bool(psr & (1 << 27))]		# DSP overflow
		return {
			"value":	psr,
			"flags":	flags,
		}

	def get_memory(self, address, length):
		data = gdb.selected_inferior().read_memory(address, length)
		data = bytes(data)
		return data

	def get_cpu_state(self):
		return {
			"regs":		self.get_registers(),
			"psr":		self.get_psr(),
		}

	def step(self):
		gdb.execute("si")

	def quit(self):
		gdb.execute("detach")
		gdb.execute("quit")

helper = GDBHelper()
print(helper.get_cpu_state())
print(helper.get_memory(0x20000000, 128))
helper.quit()
