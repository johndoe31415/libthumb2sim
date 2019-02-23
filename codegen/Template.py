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

import sys

import mako.template, mako.exceptions
import mako.lookup

class Template():
	def __init__(self, filename):
		lookup = mako.lookup.TemplateLookup([ "." ], strict_undefined = True)
		print(filename)
		self._template = lookup.get_template(filename)
		#self._template = mako.lookup.Template(open(lookupdir + filename, "r").read(), lookup = lookup)

	def render(self, **args):
		parameters = {
		}
		parameters.update(args)
		try:
			renderresult = self._template.render(**parameters)
		except:
			print("Templating error, cannot continue. Mako-decoded stacktrace follows:")
			print(mako.exceptions.text_error_template().render())
			sys.exit(1)

		return renderresult
