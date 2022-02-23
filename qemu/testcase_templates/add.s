<%inherit file="template_base.s"/>
<%namespace file="template_helpers.s" import="*"/>
<%namespace file="template_arith.s" import="*"/>
${rand_psr(save_r0 = False)}
${randomize_registers()}
%for i in range(length):
	${testcase_arith_insn("add")}
%if (i % 200) == 0:
	${place_literal_pool()}

%endif
%endfor
