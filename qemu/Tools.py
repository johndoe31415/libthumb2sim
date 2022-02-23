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

import zlib
import enum
import base64
import json
from BytesDiff import BytesDiff

class JSONTypes(enum.Enum):
	BytesUncompressed = "buc"
	BytesCompressedZlib = "bz"
	BytesDiff = "bd"

class JSONTools():
	@classmethod
	def _json_encode_default(cls, obj):
		if isinstance(obj, (bytes, bytearray)):
			if len(obj) < 8:
				# Output raw object
				return {
					"__t": JSONTypes.BytesUncompressed.value,
					"data": base64.b64encode(obj).decode("ascii")
				}
			else:
				# Output raw object
				return {
					"__t": JSONTypes.BytesCompressedZlib.value,
					"data": base64.b64encode(zlib.compress(obj)).decode("ascii")
				}
		elif isinstance(obj, BytesDiff):
			return {
				"__t": JSONTypes.BytesDiff.value,
				"data": obj.patch_data,
			}
		else:
			raise NotImplementedError(obj)

	@classmethod
	def _json_decode_object_hook(cls, obj):
		if isinstance(obj, dict) and ("__t" in obj):
			json_type = JSONTypes(obj["__t"])
			if json_type == JSONTypes.BytesUncompressed:
				return base64.b64decode(obj["data"])
			elif json_type == JSONTypes.BytesCompressedZlib:
				return zlib.decompress(base64.b64decode(obj["data"]))
			elif json_type == JSONTypes.BytesDiff:
				return BytesDiff(obj["data"])
			else:
				raise NotImplementedError(json_type)
		else:
			return obj
		return obj

	@classmethod
	def load(cls, f):
		return json.load(f, object_hook = cls._json_decode_object_hook)

	@classmethod
	def dump(cls, data, f):
		json.dump(data, f, default = cls._json_encode_default, separators = (",", ":"))
