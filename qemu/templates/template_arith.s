<%def name="testcase_arith_insn(opcode)">
<% rX = reg14(); rY = reg13(); (valX, valY) = spec32_2() %>\
	ldr ${rX}, =${hex(valX)}
	ldr ${rY}, =${hex(valY)}
	${opcode}${"s" if randb() else ""} ${rX}, ${rY}
<% rX = reg13(); rY = reg15(); rZ = reg13() %>\
	${opcode}${"s" if randb() else ""} ${rX}, ${rY}, ${rZ}
</%def>
