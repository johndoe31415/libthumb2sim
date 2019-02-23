<%def name="randomize_register(regname)">
	ldr ${regname}, =${hex(rand32())}
</%def>

<%def name="randomize_registers()">
	// Randomize all registers, including LR, except for SP or PC
%for i in range(13):
	ldr r${i}, =${hex(rand32())}
%endfor
	ldr r14, =${hex(rand32())}
	${place_literal_pool()}
</%def>


<%def name="set_psr(value, save_r0 = True)">
	// Set PSR to ${"nN"[bool(value & (1 << 31))]}${"zZ"[bool(value & (1 << 30))]}${"cC"[bool(value & (1 << 29))]}${"vV"[bool(value & (1 << 28))]}${"qQ"[bool(value & (1 << 27))]}
%if save_r0:
	push {r0}
%endif
	ldr r0, =${hex(value)}
	msr APSR_nzcvq, r0
%if save_r0:
	pop {r0}
%endif
</%def>


<%def name="clear_psr(save_r0 = True)">
	${set_psr(0, save_r0)}
</%def>


<%def name="xmas_psr(save_r0 = True)">
	${set_psr(0xffffffff, save_r0)}
</%def>


<%def name="rand_psr(save_r0 = True)">
	${set_psr(rand32(), save_r0)}
</%def>

<%def name="place_literal_pool()">
	b ${new_label()}
	// Generate literal table here so we don't run out of offset space
	.ltorg
	${label()}:
</%def>
