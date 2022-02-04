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

import os
import sys
import time
import subprocess
import tempfile
import socket
from FriendlyArgumentParser import FriendlyArgumentParser, baseint
from DebuggingProtocol import ARMDebuggingProtocol
from TraceWriter import TraceWriter, TraceRegisterSet, TraceMemory
from TraceReader import TraceReader

parser = FriendlyArgumentParser(description = "Execute a binary either in libthumb2sim or in qemu and write a JSON trace file.")
parser.add_argument("--rom-base", metavar = "address", type = baseint, default = 0, help = "ROM base address, defaults to 0x%(default)x bytes. Only relevant when a binary is specified as input file.")
parser.add_argument("--ram-base", metavar = "address", type = baseint, default = 0x20000000, help = "RAM base address, defaults to 0x%(default)x bytes. Only relevant when a binary is specified as input file.")
parser.add_argument("--ram-size", metavar = "length", type = baseint, default = 8 * 1024, help = "RAM size, defaults to 0x%(default)x bytes. Only relevant when a binary is specified as input file.")
parser.add_argument("-d", "--decimation", metavar = "step", type = int, default = 1, help = "Record only every n-th step, i.e., decimate trace data by a factor of n. By default, n is %(default)d (i.e., no decimation occurs).")
parser.add_argument("--max-insn-cnt", metavar = "cnt", type = int, default = 0, help = "Abort after a maximum of n executed instructions.")
parser.add_argument("-e", "--emulator", choices = [ "t2sim", "qemu", "user" ], default = "t2sim", help = "Decides whether to use libthumb2sim or QEMU as the underlying emulator. When 'user' is given, no emulator call is forked; instead, the created listening UNIX socket is printed and the user can connect to it.")
parser.add_argument("--emulator-binary", metavar = "filename", help = "Full path to the emulator binary. Defaults to  t2sim-gdbserver for thumb2sim and qemu-system-arm for QEMU emulation.")
parser.add_argument("--unix-socket", metavar = "filename", help = "By default, the UNIX socket is created temporarily. This allows specifying a filename for it.")
parser.add_argument("-i", "--img-format", choices = [ "bin", "trace" ], default = "bin", help = "Specify the input image format. Can be one of %(choices)s, defaults to %(default)s.")
parser.add_argument("-v", "--verbose", action = "count", default = 0, help = "Show more verbose output. Can be specified multiple times.")
parser.add_argument("img_filename", metavar = "image_filename", type = str, help = "Binary image or trace containing a binary image, depending on the input image format, to load into the emulator for tracing")
parser.add_argument("trc_filename", metavar = "trace_filename", type = str, help = "JSON trace filename to write")
args = parser.parse_args(sys.argv[1:])

class ArgumentWrapper(object):
	def __init__(self, args):
		self._args = args
		if self._args.img_format == "trace":
			self._trace = TraceReader(self._args.img_filename)
		else:
			self._trace = None
		self._image = None

	@property
	def rom_image(self):
		print("ROMY")
		try:
			if self._image is None:
				if self._trace is None:
					with open(self._args.img_filename, "rb") as f:
						self._image = f.read()
				else:
					self._image = self._trace.rom_image
			return self._image
		except Exception as e:
			print(e)

	@property
	def rom_base(self):
		if self._trace is None:
			return self._args.rom_base
		else:
			return self._trace.rom_base

	@property
	def ram_base(self):
		if self._trace is None:
			return self._args.ram_base
		else:
			return self._trace.ram_base

	@property
	def ram_size(self):
		if self._trace is None:
			return self._args.ram_size
		else:
			return self._trace.ram_size

	def __getattr__(self, key):
		return getattr(self._args, key)

args = ArgumentWrapper(args)
trace = TraceWriter(args, args.rom_image, [
	TraceRegisterSet(),
	TraceMemory("rom", args.rom_base, len(args.rom_image)),
	TraceMemory("ram", args.ram_base, args.ram_size),
])

with tempfile.NamedTemporaryFile(prefix = "qemu_gdb_") as f, tempfile.NamedTemporaryFile(prefix = "rom_", suffix = ".bin") as rom_img:
	rom_img.write(args.rom_image)
	rom_img.flush()

	if args.unix_socket is None:
		os.unlink(f.name)
		socket_filename = f.name
	else:
		socket_filename = args.unix_socket

	# Listen for QEMU debugging connection on UNIX socket
	sock = socket.socket(family = socket.AF_UNIX)
	sock.bind(socket_filename)
	sock.listen(1)

	# Then fire up QEMU and have it connect to socket
	try:
		qemu_cmd = [ args.emulator_binary or "qemu-system-arm", "-S", "-machine", "lm3s6965evb", "-display", "none", "-monitor", "none", "-gdb", "unix:%s" % (socket_filename), "-kernel", rom_img.name ]
		t2sim_cmd = [ args.emulator_binary or "t2sim-gdbserver", rom_img.name, socket_filename ]
		if args.emulator == "qemu":
			cmd = qemu_cmd
		elif args.emulator == "t2sim":
			cmd = t2sim_cmd
		elif args.emulator == "user":
			cmd = None
			print("Listening on UNIX socket for emulator: %s" % (socket_filename), file = sys.stderr)
			print("T2SIM commandline: %s" % (" ".join(qemu_cmd)))
			print("QEMU commandline : %s" % (" ".join(t2sim_cmd)))
		else:
			raise NotImplementedError(args.emulator)

		emu_process = None
		if cmd is not None:
			emu_process = subprocess.Popen(cmd)

		# Accept debugging connection from the emulator
		(conn, peer_address) = sock.accept()
		conn.settimeout(0.1)

		last_pc = None
		with ARMDebuggingProtocol(conn) as dbg:
			t0 = time.time()

			# Before stepping, record state initially
			trace.record_state(dbg, append_to_trace = True, do_step = False)

			while True:
				trace.record_state(dbg, append_to_trace = ((trace.executed_insn_cnt % args.decimation) == 0))
				regs = dbg.get_regs()
				current_pc = regs["r15"]

				if (trace.executed_insn_cnt % 100) == 0:
					now = time.time()
					tdiff = now - t0
					if args.verbose >= 1:
						print("%d instructions, PC 0x%x, %.1f insns/sec, %d tracepoints." % (trace.executed_insn_cnt, current_pc, trace.executed_insn_cnt / tdiff, trace.trace_length), file = sys.stderr)
				if current_pc == last_pc:
					# Infinite loop (probably? what about looping instructions?), end trace.
					if args.verbose >= 1:
						print("Exiting tracing at PC 0x%x, infinite loop detected." % (current_pc), file = sys.stderr)
					break
				elif (args.max_insn_cnt != 0) and (trace.executed_insn_cnt >= args.max_insn_cnt):
					if args.verbose >= 1:
						print("Exiting tracing at PC 0x%x, %d instructions executed." % (current_pc, trace.executed_insn_cnt), file = sys.stderr)
					break
				last_pc = current_pc

			# After exiting, record state one last time
			trace.record_state(dbg, append_to_trace = True, do_step = False)

			if args.verbose >= 1:
				print("Trace finished: %d executed instructions, %d tracepoints." % (trace.executed_insn_cnt, trace.trace_length), file = sys.stderr)

			# Send kill request to emulator to shut it down gracefully so we
			# can get profiling information
			dbg.kill_request()
	finally:
		if emu_process is not None:
			emu_process.kill()

trace.write(args.trc_filename)
