/**
 *	flightpanel - A Cortex-M4 based USB flight panel for flight simulators.
 *	Copyright (C) 2017-2017 Johannes Bauer
 *
 *	This file is part of flightpanel.
 *
 *	flightpanel is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; this program is ONLY licensed under
 *	version 3 of the License, later versions are explicitly excluded.
 *
 *	flightpanel is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with flightpanel; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *	Johannes Bauer <JohannesBauer@gmx.de>
**/

.syntax unified
.cpu cortex-m4
.fpu softvfp
.thumb

.section .text
.type main, %function
main:



	
	// Set PSR to NzcvQ
	ldr r0, =0x8a890eee
	msr APSR_nzcvq, r0



	// Randomize all registers, including LR, except for SP or PC
	ldr r0, =0xb5dee7a8
	ldr r1, =0xcf99e73f
	ldr r2, =0x35315388
	ldr r3, =0x10e83ce2
	ldr r4, =0xcf65f7fb
	ldr r5, =0x862c55e9
	ldr r6, =0x3da1a388
	ldr r7, =0xbaaa315b
	ldr r8, =0x7ffc8359
	ldr r9, =0x4ff5174b
	ldr r10, =0x75fd169a
	ldr r11, =0x7d175e99
	ldr r12, =0x629267b2
	ldr r14, =0xb060499
	
	b .tclbl_1
	// Generate literal table here so we don't run out of offset space
	.ltorg
	.tclbl_1:


		sdiv r3, r10, r12
		adc r0, r8
		sdiv r8, r4, r5
		sdiv r0, r7, r6
		adc r3, r3
		mvn r7, r8
		add r12, r9
		mvn r2, r7
		sub r1, r5
		sub r12, r8
		mvn r10, r7
		add r10, r9
		mul r1, r11
		adc r0, r0
		adc r12, r9
		add r2, r3
		mvn r6, r6
		add r7, r3
		sub r1, r4
		mvn r0, r11
		mul r4, r9
		sdiv r12, r0, r8
		mul r10, r3
		adc r2, r7
		mul r3, r3
		add r9, r10
		sdiv r0, r8, r6
		adc r2, r7
		add r1, r6
		mul r3, r9
		sub r3, r2
		mul r0, r10
		add r3, r11
		mvn r5, r9
		mvn r1, r7
		mul r6, r4
		mul r12, r11
		sub r1, r0
		mul r3, r0
		add r3, r11
		mvn r5, r1
		sub r4, r6
		mul r3, r10
		sub r9, r12
		mvn r12, r4
		mvn r10, r10
		adc r3, r8
		adc r8, r1
		add r0, r11
		sub r0, r9
		sdiv r8, r10, r6
		sub r5, r12
		mul r6, r7
		sub r2, r3
		mul r9, r11
		adc r11, r10
		sub r11, r8
		adc r12, r8
		adc r8, r12
		mul r3, r1
		sub r8, r4
		mul r3, r10
		add r11, r6
		sub r1, r1
		add r5, r10
		add r12, r12
		mul r12, r8
		mul r4, r5
		add r8, r8
		mul r6, r6
		sub r11, r8
		add r4, r5
		sdiv r1, r8, r12
		mul r9, r0
		mul r11, r5
		mvn r9, r2
		sdiv r10, r0, r8
		sdiv r10, r0, r7
		sub r2, r2
		add r7, r10
		add r3, r11
		sub r11, r2
		adc r11, r3
		sub r10, r4
		mvn r6, r1
		sdiv r8, r7, r12
		add r0, r2
		mul r2, r9
		sub r6, r2
		adc r4, r7
		adc r10, r2
		sdiv r6, r3, r11
		sdiv r5, r7, r3
		sub r5, r10
		sdiv r2, r6, r10
		sdiv r5, r5, r0
		add r2, r0
		sdiv r12, r2, r7
		mvn r6, r8
		sub r7, r1
		add r6, r7
		add r2, r7
		mvn r3, r3
		mvn r3, r6
		mvn r1, r3
		sub r7, r9
		adc r2, r10
		sub r12, r10
		sdiv r0, r12, r7
		sdiv r8, r7, r12
		mul r12, r9
		sub r9, r10
		sdiv r8, r5, r10
		adc r5, r3
		add r4, r8
		mul r12, r5
		mvn r2, r4
		adc r1, r8
		sub r4, r7
		mvn r10, r0
		mvn r10, r0
		sub r9, r9
		mvn r10, r2
		sdiv r12, r10, r8
		sdiv r4, r0, r3
		add r6, r8
		mul r12, r2
		adc r9, r6
		sub r4, r9
		sub r5, r4
		mvn r5, r9
		add r7, r9
		mul r5, r0
		sub r7, r0
		mul r11, r1
		sdiv r1, r11, r5
		mul r2, r9
		mul r1, r9
		adc r9, r6
		mul r4, r6
		add r5, r9
		sdiv r1, r8, r0
		add r7, r9
		add r2, r5
		mul r5, r6
		sub r5, r8
		mvn r4, r3
		adc r8, r0
		adc r3, r4
		sdiv r5, r12, r8
		mul r6, r7
		mul r1, r5
		adc r4, r10
		sub r5, r8
		sdiv r1, r5, r7
		mul r8, r9
		add r11, r6
		sub r9, r11
		sdiv r9, r4, r7
		mul r0, r2
		adc r4, r4
		adc r7, r6
		sdiv r1, r6, r12
		sub r4, r2
		add r5, r8
		adc r7, r3
		sdiv r11, r5, r11
		add r12, r0
		mul r10, r10
		adc r5, r11
		sdiv r7, r12, r3
		mvn r6, r9
		add r1, r0
		adc r12, r10
		adc r2, r4
		sub r4, r12
		add r0, r3
		mul r4, r2
		mvn r11, r6
		adc r4, r4
		sdiv r11, r6, r1
		sdiv r3, r0, r0
		sdiv r2, r11, r7
		mvn r8, r8
		mvn r12, r9
		sub r8, r10
		sub r11, r10
		mvn r9, r0
		adc r11, r7
		sdiv r5, r9, r11
		sub r3, r0
		mul r3, r12
		mul r10, r6
		sdiv r12, r10, r6
		sub r10, r4
		add r10, r12
		sub r3, r12
		mvn r12, r1
		mul r8, r11
		adc r1, r5
		sub r0, r1
		sdiv r1, r10, r5
		sdiv r0, r7, r10
		sub r3, r4
		mvn r2, r4
		adc r11, r0
		sdiv r6, r9, r7
		sub r9, r11
		sdiv r2, r1, r7
		add r5, r0
		sdiv r3, r0, r7
		sub r0, r10
		add r5, r5
		add r9, r12
		sdiv r3, r8, r8
		mul r7, r9
		add r0, r11
		sdiv r12, r10, r4
		sub r6, r8
		sub r1, r3
		mvn r2, r12
		sub r4, r3
		sub r12, r11
		sub r9, r6
		sub r3, r6
		mul r2, r7
		sub r9, r8
		mul r2, r4
		mul r0, r1
		mvn r9, r4
		sdiv r7, r3, r6
		add r2, r11
		mul r9, r7
		adc r3, r2
		mvn r4, r6
		sub r4, r6
		sub r9, r3
		sdiv r8, r1, r9
		add r3, r4
		add r6, r10
		add r0, r11
		adc r2, r11
		mul r2, r1
		sdiv r1, r5, r8
		sub r7, r3
		adc r10, r3
		add r5, r3
		add r2, r12
		mvn r0, r0
		sub r10, r1
		sub r11, r4
		mvn r2, r1
		mul r8, r9
		adc r11, r2
		adc r2, r8
		sub r0, r2
		adc r8, r7
		add r12, r1
		mvn r4, r3
		sub r9, r8
		mvn r12, r11
		mul r1, r8
		mul r7, r12
		mul r4, r11
		sdiv r2, r9, r12
		adc r7, r9
		adc r1, r8
		sub r1, r6
		mvn r2, r0
		sdiv r11, r4, r3
		mul r6, r10
		sub r6, r7
		add r0, r2
		adc r6, r1
		mul r0, r4
		sdiv r4, r4, r0
		mvn r5, r0
		adc r5, r3
		mvn r6, r6
		mul r2, r8
		mul r3, r0
		sdiv r4, r2, r0
		adc r1, r3
		sdiv r3, r0, r10
		sdiv r12, r2, r1
		sdiv r5, r11, r4
		adc r12, r7
		sdiv r2, r10, r2
		add r10, r8
		adc r9, r5
		add r12, r2
		mvn r6, r10
		adc r8, r11
		add r4, r5
		mvn r2, r8
		adc r2, r12
		sdiv r9, r7, r10
		mul r9, r8
		mul r5, r12
		mvn r6, r4
		mul r11, r5
		add r1, r10
		adc r4, r3
		adc r3, r6
		sub r5, r4
		sdiv r3, r7, r12
		sub r3, r12
		sdiv r12, r10, r11
		add r2, r9
		mul r4, r0
		mvn r11, r8
		mul r9, r0
		adc r12, r7
		add r6, r8
		adc r1, r1
		mul r0, r3
		sub r1, r1
		adc r12, r3
		add r3, r11
		sub r1, r12
		adc r11, r8
		adc r9, r1
		sdiv r0, r9, r11
		mul r8, r4
		adc r5, r12
		sdiv r12, r10, r10
		add r8, r0
		sub r0, r1
		add r4, r11
		sdiv r12, r6, r4
		sub r7, r5
		sub r4, r0
		sub r1, r5
		sub r6, r3
		sub r10, r3
		sdiv r8, r3, r11
		mvn r7, r9
		adc r8, r4
		add r9, r2
		add r6, r12
		adc r5, r2
		sdiv r9, r3, r9
		sdiv r12, r1, r9
		add r6, r0
		add r11, r0
		mul r11, r1
		sdiv r10, r7, r0
		mul r7, r12
		add r12, r11
		mvn r9, r7
		mvn r7, r9
		mul r8, r0
		mul r12, r12
		sub r7, r12
		mul r0, r10
		sub r6, r11
		sdiv r3, r10, r8
		mvn r1, r9
		mul r2, r1
		sub r2, r7
		add r2, r2
		sub r11, r4
		mvn r6, r2
		mul r3, r1
		add r6, r3
		mvn r5, r3
		add r6, r11
		mul r7, r2
		add r4, r3
		adc r3, r3
		sub r5, r12
		sdiv r1, r2, r7
		mul r8, r10
		adc r1, r0
		mvn r4, r7
		sdiv r9, r0, r2
		mul r6, r0
		sdiv r10, r12, r3
		sub r3, r5
		sub r8, r0
		sub r1, r7
		sdiv r3, r4, r0
		mul r1, r6
		adc r5, r6
		adc r0, r8
		adc r5, r4
		sub r4, r8
		adc r9, r1
		mvn r0, r9
		sdiv r0, r0, r2
		add r9, r6
		sdiv r1, r6, r5
		mul r0, r8
		sub r7, r4
		mvn r0, r8
		adc r7, r2
		mul r10, r2
		sub r0, r5
		sdiv r9, r4, r0
		mul r10, r10
		sub r12, r5
		sdiv r2, r5, r1
		mvn r0, r0
		sub r7, r9
		adc r10, r12
		adc r1, r2
		sdiv r5, r3, r12
		add r3, r0
		mul r11, r10
		sdiv r6, r9, r0
		mvn r3, r9
		mul r12, r1
		adc r12, r9
		mvn r3, r3
		sub r3, r7
		mvn r6, r11
		add r0, r9
		sub r5, r7
		mvn r2, r7
		adc r4, r8
		sub r2, r5
		mvn r11, r0
		add r6, r4
		sub r12, r9
		add r7, r10
		add r9, r12
		mvn r2, r4
		sdiv r8, r11, r8
		adc r4, r5
		adc r7, r4
		adc r1, r3
		sub r2, r8
		sdiv r6, r4, r1
		mul r6, r0
		mvn r0, r9
		mvn r6, r6
		adc r2, r6
		sdiv r5, r8, r1
		sdiv r4, r4, r2
		adc r12, r3
		sub r3, r9
		adc r5, r5
		mvn r5, r1
		adc r7, r0
		sdiv r0, r5, r9
		add r8, r2
		mvn r0, r1
		sdiv r7, r9, r9
		adc r0, r11
		sub r5, r1
		adc r2, r6
		adc r4, r12
		mul r6, r9
		sdiv r12, r6, r1
		adc r9, r7
		add r3, r3
		add r10, r10
		adc r8, r4
		add r7, r0
		sdiv r10, r0, r4
		mul r7, r7
		sub r12, r11
		mul r9, r3
		mvn r5, r2
		sdiv r12, r0, r3
		adc r1, r10
		mul r11, r5
		adc r6, r9
		sdiv r6, r4, r4
		add r10, r4
		adc r1, r1
		mul r5, r7
		sdiv r10, r3, r12
		mul r7, r1
		mvn r12, r7
		sdiv r4, r5, r10
		mvn r10, r4
		mul r6, r5
		add r8, r2
		adc r0, r11
		adc r12, r4
		mvn r2, r8
		mul r7, r3
		adc r1, r8
		add r6, r5
		add r8, r10
		mul r9, r5
		sdiv r3, r0, r9
		adc r5, r2
		add r4, r0
		sdiv r6, r8, r1
		add r2, r5
		adc r4, r1
		adc r1, r3
		sdiv r0, r7, r5
		mul r0, r3
		sub r9, r11
		add r8, r0
		adc r9, r2
		mvn r12, r1
		mvn r0, r7
		mul r6, r9
		mvn r10, r5
		adc r9, r12
		adc r2, r4
		sub r3, r7
		sdiv r9, r6, r1
		mvn r8, r9
		mvn r11, r8
		mvn r12, r12
		mvn r9, r1
		sub r2, r6
		add r2, r9
		adc r9, r5
		mvn r7, r2
		add r4, r6
		mul r5, r8
		adc r6, r11
		mul r8, r9
		mvn r1, r8
		mul r1, r6
		mul r0, r10
		mvn r4, r1
		mvn r6, r7
		mvn r6, r7
		sub r1, r8
		mul r10, r10
		mvn r6, r1
		sdiv r6, r12, r12
		sdiv r10, r1, r4
		sdiv r6, r6, r12
		mvn r4, r9
		adc r9, r10
		adc r5, r11
		adc r12, r0
		mvn r11, r9
		mul r0, r9
		sub r7, r6
		mul r11, r1
		mvn r12, r7
		sdiv r6, r0, r7
		sub r4, r0
		mul r4, r12
		adc r10, r5
		mul r12, r12
		adc r0, r10
		mul r2, r5
		sdiv r0, r6, r9
		mvn r10, r1
		mvn r7, r6
		sub r9, r12
		mvn r0, r9
		add r0, r3
		sub r7, r6
		mul r7, r0
		adc r1, r11
		adc r2, r2
		sub r11, r0
		sdiv r3, r1, r4
		add r11, r1
		sdiv r9, r12, r4
		sub r0, r0
		sub r1, r4
		add r6, r12
		add r5, r4
		add r4, r10
		add r1, r11
		add r1, r8
		sdiv r7, r8, r6
		sdiv r12, r7, r3
		mul r10, r12
		adc r5, r4
		add r12, r6
		add r12, r1
		mul r3, r11
		sub r5, r6
		sub r7, r0
		add r8, r1
		adc r6, r7
		mul r6, r8
		add r8, r6
		mvn r10, r10
		add r9, r5
		sub r2, r4
		sub r10, r5
		sub r10, r0
		sub r6, r3
		mvn r10, r10
		add r7, r10
		add r6, r1
		mvn r7, r10
		sdiv r7, r10, r2
		add r7, r4
		sub r2, r7
		mvn r2, r12
		mvn r2, r5
		mvn r7, r4
		sdiv r4, r9, r10
		add r7, r3
		mvn r8, r5
		sdiv r5, r11, r0
		sdiv r10, r1, r2
		sdiv r12, r1, r7
		adc r4, r10
		sdiv r6, r5, r5
		mvn r12, r0
		adc r6, r11
		sdiv r7, r12, r0
		adc r9, r3
		adc r7, r4
		sub r8, r8
		mul r3, r6
		sdiv r5, r0, r6
		mul r9, r6
		sub r3, r0
		mul r12, r3
		mvn r10, r6
		adc r2, r0
		mul r0, r12
		add r11, r6
		add r10, r10
		sdiv r12, r0, r3
		sub r12, r7
		adc r2, r6
		mul r12, r9
		add r11, r10
		adc r1, r1
		sub r7, r11
		add r8, r12
		sdiv r3, r3, r12
		mvn r1, r7
		add r2, r8
		sub r4, r10
		adc r11, r10
		mvn r10, r5
		sdiv r11, r0, r2
		mvn r5, r7
		adc r2, r6
		sub r6, r4
		mvn r9, r11
		sdiv r11, r4, r11
		sdiv r11, r7, r9
		adc r4, r12
		sub r2, r10
		adc r5, r0
		mul r10, r3
		add r11, r3
		adc r3, r9
		sdiv r9, r12, r5
		add r7, r9
		sdiv r8, r12, r8
		mul r9, r12
		sub r8, r12
		adc r11, r10
		sub r8, r10
		mvn r4, r1
		sub r5, r1
		sdiv r7, r7, r8
		mul r7, r4
		mvn r3, r3
		mul r5, r3
		sub r4, r9
		sdiv r8, r4, r4
		add r10, r7
		sdiv r2, r6, r0
		mvn r10, r0
		sub r1, r1
		sdiv r7, r7, r8
		sub r9, r7
		adc r8, r7
		mvn r2, r7
		adc r5, r0
		add r4, r2
		add r10, r5
		add r4, r6
		adc r1, r3
		sdiv r9, r5, r9
		sdiv r6, r5, r12
		sdiv r7, r8, r9
		adc r12, r9
		sub r1, r8
		adc r7, r12
		add r9, r9
		adc r2, r9
		sdiv r5, r10, r2
		mvn r3, r9
		adc r5, r10
		mul r8, r12
		sub r10, r3
		mvn r6, r0
		mul r0, r7
		sub r1, r6
		mvn r7, r1
		sdiv r2, r12, r2
		sdiv r2, r6, r8
		add r7, r0
		mvn r8, r0
		sdiv r9, r1, r3
		mvn r5, r0
		add r4, r8
		sub r3, r0
		adc r3, r7
		mul r4, r3
		sdiv r5, r5, r2
		sub r3, r2
		sdiv r1, r7, r7
		sdiv r6, r11, r6
		sub r10, r6
		mul r11, r6
		mvn r3, r9
		mvn r11, r3
		mul r7, r3
		adc r0, r8
		mul r5, r10
		add r10, r12
		mvn r11, r5
		sdiv r7, r10, r5
		sub r8, r3
		add r9, r6
		add r2, r6
		sub r1, r12
		adc r1, r5
		mul r2, r8
		sdiv r0, r5, r10
		sub r1, r12
		mul r8, r11
		adc r2, r2
		mul r6, r0
		sdiv r2, r1, r5
		mvn r11, r12
		mvn r2, r0
		add r12, r3
		add r0, r6
		mvn r2, r8
		mul r8, r11
		sdiv r2, r6, r0
		mvn r11, r3
		mvn r0, r6
		adc r3, r12
		sdiv r3, r0, r9
		add r12, r10
		sdiv r8, r9, r3
		sub r5, r11
		adc r11, r6
		mul r3, r2
		sub r4, r7
		sdiv r10, r2, r1
		adc r9, r12
		mul r7, r7
		sdiv r7, r2, r3
		sub r2, r4
		sub r5, r2
		mul r12, r8
		sdiv r3, r4, r8
		sdiv r3, r10, r10
		sdiv r6, r11, r3
		sdiv r10, r5, r4
		sub r0, r10
		mul r8, r1
		add r3, r6
		adc r4, r0
		adc r2, r4
		sub r0, r9
		add r6, r6
		sub r3, r3
		sub r9, r12
		adc r3, r6
		sdiv r10, r7, r11
		sdiv r11, r8, r0
		mvn r1, r5
		sdiv r11, r8, r12
		mul r1, r12
		sub r1, r11
		sub r11, r7
		sub r5, r8
		add r9, r7
		mvn r12, r10
		sub r9, r1
		add r11, r4
		adc r10, r5
		mvn r3, r7
		adc r4, r4
		mul r1, r11
		sdiv r12, r6, r6
		add r8, r10
		sdiv r3, r12, r10
		sub r9, r11
		mvn r5, r1
		mul r9, r6
		sub r8, r11
		sdiv r10, r4, r12
		mvn r1, r2
		adc r11, r6
		sdiv r8, r11, r0
		mul r10, r4
		sub r11, r2
		sub r10, r1
		add r11, r8
		sub r12, r12
		sdiv r8, r2, r6
		adc r3, r5
		mul r7, r3
		adc r7, r2
		adc r1, r9
		sub r12, r1
		mul r11, r12
		mul r4, r4
		sub r2, r3
		mul r10, r4
		mvn r6, r12
		mvn r4, r12
		add r1, r8
		sdiv r0, r6, r9
		sdiv r9, r0, r0
		mvn r7, r2
		mul r8, r10
		adc r2, r4
		mul r12, r4
		sdiv r12, r12, r2
		sub r0, r0
		mvn r0, r6
		adc r10, r1
		adc r1, r2
		sdiv r2, r2, r5
		sub r10, r6
		add r1, r11
		adc r1, r12
		sub r9, r12
		sdiv r0, r0, r2
		sub r0, r6
		mvn r1, r6
		sdiv r4, r7, r12
		mul r7, r0
		sdiv r8, r10, r1
		sdiv r7, r10, r11
		mul r7, r5
		sdiv r7, r7, r3
		adc r5, r1
		adc r2, r9
		mul r8, r10
		sdiv r4, r7, r8
		add r5, r12
		sub r1, r4
		sdiv r12, r3, r10
		mvn r9, r7
		add r0, r4
		adc r9, r0
		mul r2, r8
		mul r8, r8
		add r6, r2
		sdiv r1, r4, r11
		mvn r9, r7
		mul r2, r0
		adc r3, r6
		add r2, r3
		adc r8, r7
		mvn r5, r10
		sdiv r6, r4, r12
		adc r0, r0
		mvn r0, r9
		mul r1, r0
		mul r6, r6
		adc r7, r1
		adc r2, r10
		sdiv r2, r10, r4
		adc r3, r6
		adc r2, r10
		add r2, r7
		adc r8, r11
		mvn r3, r9
		mul r1, r9
		add r4, r6
		sdiv r12, r12, r12
		mul r9, r9
		adc r1, r9
		mvn r1, r3
		sub r0, r0
		mvn r4, r1
		sdiv r7, r4, r2
		sdiv r7, r1, r5
		sdiv r3, r9, r2
		adc r9, r0
		adc r1, r5
		mvn r8, r8
		add r0, r7
		adc r10, r5
		adc r9, r12
		sdiv r7, r3, r12
		sub r5, r11
		adc r10, r7
		add r10, r6
		adc r8, r8
		sub r1, r1
		adc r12, r11
		sdiv r1, r11, r1
		sdiv r11, r0, r4
		add r7, r9
		sub r3, r10
		sub r9, r8
		mvn r12, r4
		mul r0, r7
		sdiv r10, r2, r10
		sdiv r4, r10, r9
		sdiv r5, r4, r8
		mul r7, r11
		add r9, r12
		add r12, r11
		sub r6, r7
		add r6, r8
		sdiv r10, r5, r6
		mul r9, r4
		sdiv r3, r4, r0
		sub r3, r1
		mvn r2, r10
		mul r5, r12
		sub r3, r5
		add r1, r10
		mvn r11, r0
		add r9, r8
		add r8, r7
		sub r3, r8
		sdiv r10, r5, r6
		add r5, r9
		mvn r3, r0
		mul r3, r0
		add r0, r4
		mvn r7, r2
		sdiv r11, r9, r2
		sub r7, r2
		mvn r9, r2
		adc r2, r2
		sub r8, r12
		add r0, r1
		mul r2, r8
		adc r10, r1
		adc r6, r9
		add r11, r2
		adc r4, r4
		sdiv r1, r2, r2
		add r3, r4
		sub r3, r3
		adc r1, r2
		add r0, r9
		mvn r5, r7
		mul r5, r0
		adc r2, r3
		sdiv r5, r11, r6
		mvn r10, r11
		adc r7, r10
		add r10, r1
		mul r10, r1
		sub r10, r1
		adc r8, r11
		sdiv r0, r8, r11
		mul r0, r11
		mvn r11, r10
		mvn r12, r5
		sdiv r1, r12, r4
		add r6, r11
		adc r1, r1
		sdiv r8, r6, r3
		add r2, r7
		add r12, r11
		sub r7, r3
		add r12, r3
		sdiv r5, r7, r12
		add r6, r10
		adc r10, r11
		add r7, r6
		sub r8, r0
		mul r2, r12
		mvn r7, r10
		sdiv r12, r8, r8
		sub r1, r5
		mvn r6, r1
		adc r7, r7
		mvn r0, r8
		sub r4, r8
		mul r3, r7
		mul r10, r3
		sub r9, r2
		sdiv r7, r5, r7
		mul r8, r8
		adc r10, r11
		sdiv r4, r6, r5
		mul r3, r10
		mul r4, r7
		mvn r1, r1
		sub r12, r1
		adc r5, r8
		sdiv r3, r1, r11
		mul r0, r9
		mvn r3, r8
		adc r4, r1
		adc r3, r8
		sub r3, r5
		sub r6, r0
		mul r3, r8
		mul r0, r9
		mvn r3, r0


testcase_end:
	b testcase_end
.size main, .-main
.global main

.type default_fault_handler, %function
default_fault_handler:
	b default_fault_handler
.size default_fault_handler, .-default_fault_handler
.global default_fault_handler
