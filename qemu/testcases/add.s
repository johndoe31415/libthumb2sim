<%inherit file="template_base.s"/>
<%namespace file="template_helpers.s" import="*"/>
${rand_psr(save_r0 = False)}
${randomize_registers()}
%for i in range(length):
<% rX = reg13(); rY = reg13(); (valX, valY) = spec32_2() %>\
	ldr ${rX}, =${hex(valX)}
	ldr ${rY}, =${hex(valY)}
	add${"s" if randb() else ""} ${rX}, ${rY}
<% rX = reg13(); rY = reg15(); rZ = reg13() %>\
	add${"s" if randb() else ""} ${rX}, ${rY}, ${rZ}
%if (i % 100) == 0:
	${place_literal_pool()}

%endif
%endfor
