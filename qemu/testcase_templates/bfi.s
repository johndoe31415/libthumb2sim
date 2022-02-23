<%inherit file="template_base.s"/>
<%namespace file="template_helpers.s" import="*"/>
${rand_psr(save_r0 = False)}
${randomize_registers()}
%for i in range(length):
	<%
		width = (rand32() % 31) + 1
		lsb = (rand32() % (32 - width + 1))
		assert(width > 0)
		assert(width + lsb <= 32)
	%>
	bfi ${reg13()}, ${reg13()}, ${lsb}, ${width}
%endfor
