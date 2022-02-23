<%inherit file="template_base.s"/>
<%namespace file="template_helpers.s" import="*"/>
${rand_psr(save_r0 = False)}
${randomize_registers()}
%for i in range(length):
	eor ${reg13()}, ${reg13()}
%endfor
