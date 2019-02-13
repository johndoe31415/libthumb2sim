import sys

import mako.template, mako.exceptions
from mako.lookup import TemplateLookup

class Template():
	def __init__(self, filename):
		lookupdir = "./"
		lookup = TemplateLookup([ lookupdir ])
		self._template = mako.lookup.Template(open(lookupdir + filename, "r").read(), lookup = lookup)		

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
