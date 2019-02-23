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
import os
import subprocess
import shutil
import contextlib
from FriendlyArgumentParser import FriendlyArgumentParser

parser = FriendlyArgumentParser()
parser.add_argument("tc_dir", metavar = "tc_dir", type = str, help = "Testcase directory for input data")
parser.add_argument("trace_dir", metavar = "trace_dir", type = str, help = "Trace output directory")
args = parser.parse_args(sys.argv[1:])

subprocess.check_call([ "make", "clean" ])

def create_trace(input_filename, output_filename):
	if os.path.exists(output_filename):
		input_mtime = os.stat(input_filename).st_mtime
		output_mtime = os.stat(output_filename).st_mtime
		if input_mtime < output_mtime:
			return

	print("%s -> %s" % (input_filename, output_filename))
	with contextlib.suppress(FileExistsError):
		os.makedirs(os.path.dirname(output_filename))
	subprocess.check_call([ "./render_testcase.py", "-f", "-o", "testcase.s", input_filename ])
	subprocess.check_call([ "make", "testcase.bin" ])
	subprocess.check_call([ "./run_testcase.py", "-e", "qemu", "testcase.bin", output_filename ])

input_dir = args.tc_dir
output_dir = args.trace_dir
if not input_dir.endswith("/"):
	input_dir += "/"
if not output_dir.endswith("/"):
	output_dir += "/"

for (dirname, subdirs, files) in os.walk(input_dir):
	if not dirname.endswith("/"):
		dirname += "/"
	for filename in files:
		if not filename.endswith(".s"):
			continue
		rel_dirname = dirname[len(args.tc_dir) + 1:]
		input_filename = dirname + filename
		output_filename = output_dir + rel_dirname + filename[:-2] + ".json"
		create_trace(input_filename, output_filename)
