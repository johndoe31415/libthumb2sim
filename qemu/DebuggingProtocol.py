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

import socket
import time
import threading
import queue
import collections

class DebuggingProtocolResponse(object):
	def __init__(self, success, data, rx_cksum):
		self._success = success
		self._data = data
		self._rx_cksum = rx_cksum
		self._text = None

	@classmethod
	def from_response_line(cls, line):
		return cls()

	@property
	def success(self):
		return self._success

	@property
	def data(self):
		return self._data

	@property
	def text(self):
		if (self._text is None) and (self.data is not None):
			self._text = self.data.decode("ascii")
		return self._text

	def __str__(self):
		if self._success:
			return "OK: %s" % (str(self.data))
		else:
			return "ERR"

class RXBuffer(object):
	def __init__(self, tokenizer):
		self._tokenizer = tokenizer
		self._buffer = bytearray()
		self._lock = threading.Lock()
		self._run = True
		self._thread = None
		self._msgs = queue.Queue()

	def start_rx_thread(self, conn):
		assert(self._thread is None)
		self._thread = threading.Thread(target = self._recv_thread, args = (conn, ))
		self._thread.start()

	def stop_rx_thread(self):
		self._run = False
		self._thread.join()

	def _put(self, data):
		with self._lock:
			self._buffer += data
			(msgs, self._buffer) = self._tokenizer(self._buffer)
		for msg in msgs:
			self._msgs.put(msg)

	def _recv_thread(self, conn):
		while self._run:
			try:
				data = conn.recv(8192)
#				print("<=", data, len(data))
			except socket.timeout:
				continue
			if len(data) == 0:
				# Peer disconnected
				self._run = False
				break
			self._put(data)

	def wait(self, timeout = 1.0):
		return self._msgs.get(timeout = timeout)

class DebuggingProtocol(object):
	_MAX_MEMREQUEST_CHUNK = 2048

	def __init__(self, conn, close_conn = True):
		self._close_conn = close_conn
		self._conn = conn
		self._rxbuf = RXBuffer(tokenizer = self._tokenizer)
		self._rxbuf.start_rx_thread(self._conn)

	def close(self):
		self._rxbuf.stop_rx_thread()
		if self._close_conn:
			self._conn.close()

	@classmethod
	def listen_unix_socket(cls, filename, close_conn = True):
		sock = socket.socket(family = socket.AF_UNIX)
		sock.bind(filename)
		sock.listen(10)
		(conn, peer_address) = sock.accept()
		conn.settimeout(0.1)
		return cls(conn, close_conn = close_conn)

	@staticmethod
	def _tokenizer(data):
		msgs = [ ]
		remaining = data
		while len(remaining) > 0:
			if remaining[0 : 1] == b"+":
				hash_index = remaining.find(b"#")
				if (len(remaining) >= 5) and (remaining[0 : 2] == b"+$") and (hash_index != -1) and (len(remaining) >= hash_index + 3):
					# Successful message
					data = bytes(remaining[2 : hash_index])
					rx_cksum = remaining[hash_index : hash_index + 2]
					msg = DebuggingProtocolResponse(success = True, data = data, rx_cksum = rx_cksum)
					remaining = remaining[hash_index + 3 : ]
					msgs.append(msg)
				else:
					# Not yet complete
					break
			elif remaining[0 : 1] == b"-":
				msg = DebuggingProtocolResponse(success = False, data = None, rx_cksum = None)
				msgs.append(msg)
				remaining = remaining[1:]
			else:
				# Unknown message
				msg = DebuggingProtocolResponse(success = False, data = remaining[0 : 1], rx_cksum = None)
				msgs.append(msg)
				remaining = remaining[1:]
		return (msgs, remaining)

	@staticmethod
	def _checksum(data):
		cksum = 0
		for char in data:
			cksum += char
		cksum &= 0xff
		return cksum

	def send_cmd(self, command):
		command = command.encode("ascii")
		cksum = self._checksum(command)
		frame = b"+$" + command + ("#%02x" % (cksum)).encode("ascii")
		self._conn.send(frame)
#		print("->", command)
		reply = self._rxbuf.wait(timeout = 1)
#		print("<-", reply)
		return reply

	def singlestep(self):
		self.send_cmd("vCont;s:1;c")

	def _read_memory(self, start_address, length):
		data = self.send_cmd("m%x,%x" % (start_address, length))
		return bytes.fromhex(data.text)

	def read_memory(self, start_address, length):
		assert(0 <= start_address <= 0xffffffff)
		assert(length > 0)
		assert(start_address + length <= 0x100000000)
		result = bytearray()
		for offset in range(start_address, start_address + length, self._MAX_MEMREQUEST_CHUNK):
			chunk_length = start_address + length - offset
			if chunk_length > self._MAX_MEMREQUEST_CHUNK:
				chunk_length = self._MAX_MEMREQUEST_CHUNK
			result += self._read_memory(offset, chunk_length)
		return bytes(result)

	def __enter__(self):
		return self

	def __exit__(self, *args):
		self.close()

class ARMDebuggingProtocol(DebuggingProtocol):
	_REG_ORDER = collections.OrderedDict([
		("r0", "r0"),
		("r1", "r1"),
		("r2", "r2"),
		("r3", "r3"),
		("r4", "r4"),
		("r5", "r5"),
		("r6", "r6"),
		("r7", "r7"),
		("r8", "r8"),
		("r9", "r9"),
		("r10", "r10"),
		("r11", "r11"),
		("r12", "r12"),
		("r13", "r13"),
		("r14", "r14"),
		("r15", "pc"),
		("psr",	"PSR"),
	])

	@staticmethod
	def _byteorder(value):
		return (((value >> 0) & 0xff) << 24) | (((value >> 8) & 0xff) << 16) | (((value >> 16) & 0xff) << 8) | (((value >> 24) & 0xff) << 0)

	def get_regs(self):
		reply = self.send_cmd("g")
		values = [ int(reply.text[i : i + 8], 16) for i in range(0, len(reply.text), 8) ]
		result = { "r%d" % (i): self._byteorder(values[i]) for i in range(16) }
		result["psr"] = self._byteorder(values[41])
		return result

	def dump_regs(self):
		regs = self.get_regs()
		for (regname, show_regname) in self._REG_ORDER.items():
			value = regs[regname]
			print("%-3s %08x" % (show_regname, value))
		print()


if __name__ == "__main__":
	with ARMDebuggingProtocol.listen_unix_socket("foobar") as dbg:
		dbg.dump_regs()
		dbg.singlestep()
		dbg.dump_regs()
		dbg.singlestep()
		dbg.dump_regs()
		memory = dbg.read_memory(0x0, 0x100)
		print(memory)
		print(len(memory))
