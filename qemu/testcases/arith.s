<%inherit file="template_base.s"/>
<%namespace file="template_helpers.s" import="*"/>
${rand_psr(save_r0 = False)}
${randomize_registers()}
%for i in range(length):
	<% choice = choices([ "add", "adc", "sub", "mul", "sdiv", "mvn" ]) %>\
	%if choice == "add":
	add ${reg13()}, ${reg13()}
	%elif choice == "adc":
	adc ${reg13()}, ${reg13()}
	%elif choice == "sub":
	sub ${reg13()}, ${reg13()}
	%elif choice == "mul":
	mul ${reg13()}, ${reg13()}
	%elif choice == "sdiv":
	sdiv ${reg13()}, ${reg13()}, ${reg13()}
	%elif choice == "mvn":
	mvn ${reg13()}, ${reg13()}
	%else:
	.err
	%endif
%endfor
