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


	
	bfi r10, r12, 1, 1
	
	bfi r8, r2, 4, 28
	
	bfi r5, r5, 1, 22
	
	bfi r6, r5, 3, 22
	
	bfi r6, r7, 8, 9
	
	bfi r12, r9, 9, 23
	
	bfi r7, r5, 9, 11
	
	bfi r5, r12, 13, 8
	
	bfi r10, r7, 3, 4
	
	bfi r9, r1, 1, 30
	
	bfi r10, r0, 9, 23
	
	bfi r12, r9, 6, 23
	
	bfi r3, r6, 1, 22
	
	bfi r6, r7, 14, 8
	
	bfi r1, r4, 8, 18
	
	bfi r11, r3, 3, 26
	
	bfi r6, r12, 1, 9
	
	bfi r2, r10, 0, 30
	
	bfi r2, r7, 6, 23
	
	bfi r3, r12, 0, 31
	
	bfi r6, r0, 10, 20
	
	bfi r4, r2, 4, 26
	
	bfi r1, r6, 11, 7
	
	bfi r9, r3, 6, 12
	
	bfi r10, r0, 2, 25
	
	bfi r3, r11, 8, 2
	
	bfi r9, r1, 0, 1
	
	bfi r2, r6, 9, 21
	
	bfi r12, r11, 10, 13
	
	bfi r0, r12, 8, 21
	
	bfi r3, r3, 1, 25
	
	bfi r5, r1, 29, 1
	
	bfi r6, r4, 18, 3
	
	bfi r2, r9, 9, 9
	
	bfi r12, r4, 9, 1
	
	bfi r10, r11, 7, 9
	
	bfi r6, r8, 1, 29
	
	bfi r0, r11, 0, 14
	
	bfi r9, r9, 4, 19
	
	bfi r6, r1, 13, 18
	
	bfi r7, r6, 13, 16
	
	bfi r2, r3, 9, 23
	
	bfi r11, r7, 5, 22
	
	bfi r11, r11, 10, 5
	
	bfi r12, r8, 15, 2
	
	bfi r12, r12, 0, 27
	
	bfi r5, r8, 7, 12
	
	bfi r3, r10, 8, 21
	
	bfi r6, r4, 5, 17
	
	bfi r3, r5, 13, 11
	
	bfi r12, r12, 1, 27
	
	bfi r8, r10, 12, 20
	
	bfi r9, r8, 5, 27
	
	bfi r6, r6, 2, 4
	
	bfi r8, r10, 1, 15
	
	bfi r0, r1, 16, 1
	
	bfi r9, r9, 14, 14
	
	bfi r11, r5, 14, 15
	
	bfi r2, r6, 2, 18
	
	bfi r8, r4, 1, 11
	
	bfi r7, r5, 3, 24
	
	bfi r2, r7, 2, 29
	
	bfi r3, r11, 9, 5
	
	bfi r2, r0, 1, 31
	
	bfi r4, r10, 26, 3
	
	bfi r6, r1, 3, 19
	
	bfi r7, r12, 4, 18
	
	bfi r2, r3, 2, 23
	
	bfi r0, r6, 22, 10
	
	bfi r4, r7, 18, 1
	
	bfi r2, r9, 8, 24
	
	bfi r11, r2, 25, 2
	
	bfi r3, r6, 1, 30
	
	bfi r12, r2, 2, 30
	
	bfi r8, r5, 6, 13
	
	bfi r4, r2, 1, 11
	
	bfi r12, r2, 6, 25
	
	bfi r6, r8, 15, 13
	
	bfi r1, r5, 11, 16
	
	bfi r0, r2, 5, 13
	
	bfi r3, r3, 23, 5
	
	bfi r6, r0, 0, 3
	
	bfi r6, r7, 5, 16
	
	bfi r2, r10, 6, 21
	
	bfi r10, r9, 6, 8
	
	bfi r7, r5, 17, 15
	
	bfi r12, r5, 0, 8
	
	bfi r12, r9, 0, 18
	
	bfi r8, r5, 0, 5
	
	bfi r5, r3, 6, 20
	
	bfi r8, r7, 14, 9
	
	bfi r8, r2, 8, 12
	
	bfi r1, r8, 0, 24
	
	bfi r7, r8, 18, 3
	
	bfi r7, r10, 11, 13
	
	bfi r9, r9, 27, 5
	
	bfi r2, r1, 10, 20
	
	bfi r8, r5, 6, 19
	
	bfi r3, r11, 15, 14
	
	bfi r5, r12, 5, 11
	
	bfi r9, r6, 12, 17
	
	bfi r9, r7, 15, 15
	
	bfi r6, r5, 18, 3
	
	bfi r7, r9, 1, 11
	
	bfi r0, r3, 5, 26
	
	bfi r7, r11, 20, 1
	
	bfi r1, r11, 2, 13
	
	bfi r2, r9, 4, 23
	
	bfi r9, r6, 3, 19
	
	bfi r4, r4, 1, 28
	
	bfi r5, r9, 2, 16
	
	bfi r8, r0, 13, 10
	
	bfi r9, r7, 7, 16
	
	bfi r11, r5, 6, 12
	
	bfi r5, r8, 11, 15
	
	bfi r3, r9, 1, 31
	
	bfi r9, r3, 9, 21
	
	bfi r5, r12, 16, 2
	
	bfi r6, r7, 14, 9
	
	bfi r5, r8, 11, 8
	
	bfi r10, r5, 6, 24
	
	bfi r1, r5, 20, 11
	
	bfi r8, r9, 10, 1
	
	bfi r6, r4, 1, 30
	
	bfi r4, r9, 13, 6
	
	bfi r7, r0, 15, 11
	
	bfi r4, r4, 16, 8
	
	bfi r6, r4, 2, 16
	
	bfi r12, r3, 1, 27
	
	bfi r11, r5, 2, 20
	
	bfi r7, r3, 15, 6
	
	bfi r5, r11, 1, 28
	
	bfi r0, r2, 13, 3
	
	bfi r11, r5, 9, 14
	
	bfi r7, r12, 10, 21
	
	bfi r6, r9, 6, 18
	
	bfi r0, r6, 17, 6
	
	bfi r5, r2, 10, 7
	
	bfi r4, r12, 23, 7
	
	bfi r3, r6, 20, 2
	
	bfi r3, r11, 1, 28
	
	bfi r4, r4, 6, 9
	
	bfi r6, r1, 1, 28
	
	bfi r0, r0, 3, 20
	
	bfi r11, r7, 11, 17
	
	bfi r8, r1, 3, 24
	
	bfi r6, r8, 17, 9
	
	bfi r11, r10, 11, 9
	
	bfi r0, r8, 9, 20
	
	bfi r3, r5, 5, 19
	
	bfi r5, r3, 0, 31
	
	bfi r3, r12, 6, 17
	
	bfi r6, r8, 6, 3
	
	bfi r6, r10, 27, 4
	
	bfi r5, r10, 1, 27
	
	bfi r3, r12, 2, 24
	
	bfi r1, r10, 5, 26
	
	bfi r10, r1, 1, 29
	
	bfi r0, r1, 3, 25
	
	bfi r10, r5, 5, 19
	
	bfi r7, r10, 0, 31
	
	bfi r4, r5, 0, 26
	
	bfi r4, r11, 19, 8
	
	bfi r6, r9, 8, 8
	
	bfi r9, r11, 23, 7
	
	bfi r1, r7, 8, 23
	
	bfi r0, r10, 23, 2
	
	bfi r7, r11, 7, 24
	
	bfi r11, r5, 10, 20
	
	bfi r9, r12, 13, 15
	
	bfi r8, r8, 0, 31
	
	bfi r9, r8, 0, 22
	
	bfi r1, r12, 10, 22
	
	bfi r4, r6, 5, 23
	
	bfi r1, r3, 5, 25
	
	bfi r12, r12, 6, 22
	
	bfi r0, r12, 0, 27
	
	bfi r9, r6, 3, 29
	
	bfi r6, r9, 8, 24
	
	bfi r6, r9, 7, 23
	
	bfi r2, r4, 10, 17
	
	bfi r1, r12, 0, 20
	
	bfi r0, r7, 1, 10
	
	bfi r3, r2, 0, 14
	
	bfi r9, r7, 17, 8
	
	bfi r2, r12, 18, 9
	
	bfi r0, r4, 1, 27
	
	bfi r9, r3, 23, 3
	
	bfi r1, r9, 3, 21
	
	bfi r4, r10, 15, 12
	
	bfi r12, r0, 17, 9
	
	bfi r2, r11, 8, 17
	
	bfi r1, r0, 4, 27
	
	bfi r8, r9, 4, 26
	
	bfi r1, r10, 0, 24
	
	bfi r5, r3, 29, 1
	
	bfi r12, r5, 1, 21
	
	bfi r12, r10, 5, 16
	
	bfi r11, r4, 5, 27
	
	bfi r1, r0, 3, 8
	
	bfi r0, r11, 8, 2
	
	bfi r2, r8, 0, 26
	
	bfi r2, r4, 1, 28
	
	bfi r0, r12, 7, 20
	
	bfi r4, r3, 5, 22
	
	bfi r8, r7, 10, 11
	
	bfi r10, r1, 18, 3
	
	bfi r7, r12, 10, 13
	
	bfi r11, r5, 7, 22
	
	bfi r12, r0, 2, 19
	
	bfi r11, r1, 1, 30
	
	bfi r1, r6, 8, 12
	
	bfi r0, r3, 5, 26
	
	bfi r3, r9, 4, 23
	
	bfi r8, r6, 10, 7
	
	bfi r0, r2, 5, 19
	
	bfi r1, r9, 5, 16
	
	bfi r4, r4, 17, 7
	
	bfi r9, r5, 0, 31
	
	bfi r5, r3, 22, 5
	
	bfi r6, r10, 3, 29
	
	bfi r12, r3, 4, 4
	
	bfi r4, r2, 2, 28
	
	bfi r1, r3, 0, 9
	
	bfi r0, r10, 29, 1
	
	bfi r2, r1, 0, 27
	
	bfi r11, r4, 14, 3
	
	bfi r7, r8, 0, 29
	
	bfi r2, r2, 12, 6
	
	bfi r0, r9, 0, 8
	
	bfi r12, r2, 23, 4
	
	bfi r10, r7, 5, 27
	
	bfi r1, r4, 4, 28
	
	bfi r2, r8, 19, 11
	
	bfi r12, r2, 0, 27
	
	bfi r10, r3, 0, 26
	
	bfi r4, r5, 6, 23
	
	bfi r6, r4, 15, 9
	
	bfi r5, r6, 1, 30
	
	bfi r2, r4, 7, 16
	
	bfi r3, r6, 3, 18
	
	bfi r4, r7, 1, 31
	
	bfi r12, r9, 0, 19
	
	bfi r3, r12, 11, 19
	
	bfi r5, r2, 11, 21
	
	bfi r4, r0, 2, 27
	
	bfi r8, r3, 7, 14
	
	bfi r8, r12, 24, 6
	
	bfi r6, r8, 9, 4
	
	bfi r1, r9, 4, 22
	
	bfi r7, r1, 8, 4
	
	bfi r12, r3, 5, 26
	
	bfi r11, r4, 5, 15
	
	bfi r11, r11, 11, 18
	
	bfi r9, r1, 1, 16
	
	bfi r9, r11, 9, 4
	
	bfi r4, r1, 4, 21
	
	bfi r7, r12, 22, 1
	
	bfi r2, r8, 9, 1
	
	bfi r0, r1, 2, 28
	
	bfi r11, r7, 7, 15
	
	bfi r4, r9, 0, 11
	
	bfi r7, r4, 8, 21
	
	bfi r1, r5, 14, 14
	
	bfi r3, r9, 9, 1
	
	bfi r4, r8, 7, 21
	
	bfi r0, r7, 8, 10
	
	bfi r8, r4, 6, 6
	
	bfi r2, r10, 4, 15
	
	bfi r2, r5, 10, 15
	
	bfi r9, r3, 30, 2
	
	bfi r12, r1, 6, 25
	
	bfi r6, r0, 4, 24
	
	bfi r0, r10, 11, 20
	
	bfi r11, r10, 0, 28
	
	bfi r10, r7, 8, 21
	
	bfi r12, r11, 1, 29
	
	bfi r7, r5, 2, 5
	
	bfi r4, r8, 16, 13
	
	bfi r12, r12, 4, 22
	
	bfi r12, r3, 13, 3
	
	bfi r2, r6, 7, 14
	
	bfi r3, r10, 7, 10
	
	bfi r1, r9, 10, 4
	
	bfi r1, r0, 1, 26
	
	bfi r10, r2, 14, 12
	
	bfi r11, r4, 7, 23
	
	bfi r2, r3, 8, 16
	
	bfi r10, r6, 8, 16
	
	bfi r5, r3, 9, 15
	
	bfi r11, r8, 12, 16
	
	bfi r12, r4, 19, 10
	
	bfi r3, r3, 7, 22
	
	bfi r12, r7, 8, 23
	
	bfi r7, r6, 13, 13
	
	bfi r7, r1, 11, 1
	
	bfi r4, r7, 27, 3
	
	bfi r0, r2, 9, 6
	
	bfi r0, r6, 6, 17
	
	bfi r3, r6, 1, 14
	
	bfi r0, r8, 12, 8
	
	bfi r1, r7, 5, 20
	
	bfi r4, r0, 3, 20
	
	bfi r6, r12, 4, 21
	
	bfi r9, r0, 13, 19
	
	bfi r5, r4, 0, 30
	
	bfi r8, r4, 0, 23
	
	bfi r7, r0, 14, 15
	
	bfi r0, r0, 0, 23
	
	bfi r9, r6, 23, 2
	
	bfi r6, r5, 2, 27
	
	bfi r8, r10, 3, 16
	
	bfi r3, r0, 3, 10
	
	bfi r7, r2, 0, 14
	
	bfi r2, r7, 10, 7
	
	bfi r10, r9, 13, 19
	
	bfi r8, r10, 6, 23
	
	bfi r12, r5, 1, 31
	
	bfi r5, r1, 1, 31
	
	bfi r0, r9, 4, 1
	
	bfi r2, r10, 21, 11
	
	bfi r1, r2, 0, 27
	
	bfi r3, r12, 18, 12
	
	bfi r0, r1, 20, 10
	
	bfi r0, r6, 2, 29
	
	bfi r12, r3, 9, 22
	
	bfi r12, r1, 2, 9
	
	bfi r9, r3, 24, 5
	
	bfi r1, r3, 1, 30
	
	bfi r6, r11, 0, 2
	
	bfi r9, r4, 13, 4
	
	bfi r4, r2, 8, 23
	
	bfi r4, r8, 10, 5
	
	bfi r5, r11, 2, 20
	
	bfi r1, r6, 4, 2
	
	bfi r12, r9, 2, 18
	
	bfi r10, r3, 10, 9
	
	bfi r5, r2, 8, 15
	
	bfi r8, r11, 16, 5
	
	bfi r4, r5, 0, 19
	
	bfi r4, r6, 0, 30
	
	bfi r5, r2, 3, 27
	
	bfi r6, r4, 1, 30
	
	bfi r6, r0, 6, 25
	
	bfi r9, r0, 4, 8
	
	bfi r2, r2, 5, 19
	
	bfi r5, r8, 8, 11
	
	bfi r4, r4, 10, 15
	
	bfi r12, r3, 6, 6
	
	bfi r9, r0, 5, 26
	
	bfi r11, r5, 2, 19
	
	bfi r7, r0, 6, 6
	
	bfi r5, r9, 15, 8
	
	bfi r2, r5, 10, 19
	
	bfi r9, r7, 13, 11
	
	bfi r8, r0, 0, 23
	
	bfi r5, r1, 3, 13
	
	bfi r6, r9, 23, 3
	
	bfi r11, r6, 0, 31
	
	bfi r12, r6, 6, 25
	
	bfi r9, r7, 2, 14
	
	bfi r3, r1, 3, 27
	
	bfi r6, r8, 4, 26
	
	bfi r7, r0, 13, 15
	
	bfi r0, r4, 2, 22
	
	bfi r7, r8, 15, 4
	
	bfi r9, r9, 12, 13
	
	bfi r5, r2, 15, 15
	
	bfi r0, r3, 5, 21
	
	bfi r10, r0, 15, 10
	
	bfi r4, r6, 4, 28
	
	bfi r6, r4, 22, 4
	
	bfi r10, r4, 1, 28
	
	bfi r1, r12, 6, 16
	
	bfi r0, r10, 7, 20
	
	bfi r5, r7, 4, 22
	
	bfi r12, r7, 3, 27
	
	bfi r5, r10, 13, 15
	
	bfi r4, r5, 3, 26
	
	bfi r10, r8, 13, 3
	
	bfi r0, r11, 4, 7
	
	bfi r4, r9, 1, 12
	
	bfi r5, r7, 5, 18
	
	bfi r1, r8, 12, 19
	
	bfi r5, r4, 6, 20
	
	bfi r8, r9, 23, 1
	
	bfi r3, r0, 10, 15
	
	bfi r5, r2, 2, 29
	
	bfi r0, r8, 0, 31
	
	bfi r1, r1, 6, 10
	
	bfi r1, r4, 0, 28
	
	bfi r1, r3, 15, 12
	
	bfi r7, r5, 26, 2
	
	bfi r3, r8, 4, 13
	
	bfi r4, r8, 8, 22
	
	bfi r9, r2, 6, 24
	
	bfi r1, r10, 4, 10
	
	bfi r1, r6, 5, 27
	
	bfi r10, r5, 13, 14
	
	bfi r12, r12, 10, 17
	
	bfi r1, r3, 5, 27
	
	bfi r9, r6, 22, 7
	
	bfi r8, r9, 21, 2
	
	bfi r8, r0, 15, 9
	
	bfi r8, r9, 0, 30
	
	bfi r2, r6, 2, 22
	
	bfi r9, r9, 11, 16
	
	bfi r1, r7, 1, 31
	
	bfi r4, r6, 1, 30
	
	bfi r8, r8, 0, 31
	
	bfi r10, r8, 0, 11
	
	bfi r1, r8, 23, 1
	
	bfi r6, r11, 1, 20
	
	bfi r9, r4, 0, 29
	
	bfi r6, r7, 15, 15
	
	bfi r7, r8, 1, 21
	
	bfi r8, r10, 12, 14
	
	bfi r6, r1, 15, 17
	
	bfi r12, r12, 8, 17
	
	bfi r1, r4, 4, 23
	
	bfi r6, r12, 9, 9
	
	bfi r9, r3, 20, 11
	
	bfi r0, r5, 2, 29
	
	bfi r12, r0, 8, 23
	
	bfi r9, r12, 22, 1
	
	bfi r5, r7, 8, 22
	
	bfi r11, r1, 0, 25
	
	bfi r7, r7, 22, 5
	
	bfi r7, r0, 0, 9
	
	bfi r1, r4, 1, 30
	
	bfi r10, r5, 6, 17
	
	bfi r12, r4, 18, 2
	
	bfi r12, r2, 31, 1
	
	bfi r0, r6, 10, 21
	
	bfi r10, r1, 0, 28
	
	bfi r6, r9, 5, 27
	
	bfi r7, r0, 13, 18
	
	bfi r0, r3, 7, 17
	
	bfi r6, r5, 3, 29
	
	bfi r0, r1, 19, 8
	
	bfi r2, r2, 10, 17
	
	bfi r0, r4, 16, 5
	
	bfi r4, r11, 16, 5
	
	bfi r7, r9, 1, 20
	
	bfi r12, r0, 2, 6
	
	bfi r1, r4, 5, 27
	
	bfi r12, r3, 7, 25
	
	bfi r7, r4, 3, 17
	
	bfi r1, r11, 3, 19
	
	bfi r8, r9, 28, 2
	
	bfi r6, r5, 15, 5
	
	bfi r3, r12, 24, 1
	
	bfi r10, r5, 15, 14
	
	bfi r12, r6, 25, 5
	
	bfi r1, r11, 9, 19
	
	bfi r10, r5, 10, 4
	
	bfi r7, r0, 15, 17
	
	bfi r1, r4, 3, 13
	
	bfi r1, r6, 16, 3
	
	bfi r8, r6, 1, 15
	
	bfi r10, r8, 23, 7
	
	bfi r11, r2, 14, 16
	
	bfi r10, r5, 9, 16
	
	bfi r0, r5, 3, 16
	
	bfi r10, r10, 28, 2
	
	bfi r7, r10, 13, 5
	
	bfi r1, r11, 13, 18
	
	bfi r4, r7, 3, 28
	
	bfi r5, r7, 3, 13
	
	bfi r2, r7, 19, 8
	
	bfi r12, r2, 0, 29
	
	bfi r11, r7, 9, 14
	
	bfi r4, r9, 16, 9
	
	bfi r7, r3, 1, 27
	
	bfi r5, r4, 25, 1
	
	bfi r0, r3, 6, 16
	
	bfi r2, r3, 8, 5
	
	bfi r7, r2, 0, 30
	
	bfi r2, r6, 10, 2
	
	bfi r12, r12, 1, 31
	
	bfi r6, r11, 3, 12
	
	bfi r12, r0, 9, 15
	
	bfi r3, r2, 9, 7
	
	bfi r2, r8, 2, 24
	
	bfi r3, r6, 9, 2
	
	bfi r0, r6, 5, 22
	
	bfi r6, r9, 0, 24
	
	bfi r2, r12, 9, 22
	
	bfi r10, r6, 2, 8
	
	bfi r0, r8, 4, 28
	
	bfi r9, r11, 16, 12
	
	bfi r10, r10, 3, 1
	
	bfi r0, r3, 4, 26
	
	bfi r7, r5, 9, 17
	
	bfi r2, r12, 12, 12
	
	bfi r11, r10, 1, 19
	
	bfi r1, r1, 3, 29
	
	bfi r0, r8, 7, 23
	
	bfi r3, r3, 16, 15
	
	bfi r1, r7, 6, 26
	
	bfi r8, r3, 4, 28
	
	bfi r4, r11, 2, 9
	
	bfi r10, r5, 15, 4
	
	bfi r0, r2, 15, 4
	
	bfi r7, r3, 30, 2
	
	bfi r1, r6, 8, 22
	
	bfi r9, r11, 6, 24
	
	bfi r4, r11, 0, 29
	
	bfi r7, r9, 1, 30
	
	bfi r12, r12, 29, 2
	
	bfi r0, r5, 7, 5
	
	bfi r10, r3, 10, 7
	
	bfi r3, r4, 1, 31
	
	bfi r11, r9, 7, 21
	
	bfi r12, r7, 0, 21
	
	bfi r8, r12, 3, 2
	
	bfi r9, r12, 8, 21
	
	bfi r12, r2, 11, 17
	
	bfi r6, r8, 11, 16
	
	bfi r4, r1, 0, 30
	
	bfi r1, r7, 5, 21
	
	bfi r8, r8, 20, 5
	
	bfi r11, r3, 6, 26
	
	bfi r5, r3, 8, 8
	
	bfi r9, r7, 13, 19
	
	bfi r4, r6, 27, 4
	
	bfi r7, r2, 4, 18
	
	bfi r1, r10, 2, 23
	
	bfi r1, r1, 13, 17
	
	bfi r7, r8, 2, 23
	
	bfi r7, r3, 10, 21
	
	bfi r6, r2, 6, 12
	
	bfi r5, r0, 0, 30
	
	bfi r2, r4, 0, 30
	
	bfi r5, r4, 16, 4
	
	bfi r1, r3, 6, 2
	
	bfi r5, r9, 22, 7
	
	bfi r5, r12, 7, 17
	
	bfi r8, r9, 7, 20
	
	bfi r9, r2, 6, 12
	
	bfi r12, r7, 1, 25
	
	bfi r9, r9, 10, 2
	
	bfi r9, r3, 2, 20
	
	bfi r2, r6, 9, 6
	
	bfi r10, r5, 14, 14
	
	bfi r8, r12, 8, 11
	
	bfi r3, r1, 15, 12
	
	bfi r5, r0, 22, 8
	
	bfi r1, r6, 2, 24
	
	bfi r1, r0, 0, 24
	
	bfi r2, r3, 5, 17
	
	bfi r8, r1, 6, 14
	
	bfi r10, r8, 13, 17
	
	bfi r9, r1, 4, 3
	
	bfi r5, r0, 15, 9
	
	bfi r8, r7, 3, 13
	
	bfi r4, r3, 13, 3
	
	bfi r4, r3, 11, 14
	
	bfi r5, r2, 16, 10
	
	bfi r2, r10, 12, 9
	
	bfi r7, r2, 17, 1
	
	bfi r6, r0, 0, 22
	
	bfi r4, r11, 5, 22
	
	bfi r3, r9, 3, 5
	
	bfi r3, r9, 0, 28
	
	bfi r6, r0, 1, 22
	
	bfi r5, r10, 15, 14
	
	bfi r12, r6, 2, 2
	
	bfi r12, r7, 2, 23
	
	bfi r1, r8, 1, 31
	
	bfi r9, r6, 11, 20
	
	bfi r6, r3, 3, 24
	
	bfi r5, r1, 0, 31
	
	bfi r2, r8, 6, 20
	
	bfi r5, r10, 1, 30
	
	bfi r12, r3, 8, 8
	
	bfi r2, r2, 9, 19
	
	bfi r6, r0, 11, 6
	
	bfi r1, r5, 5, 27
	
	bfi r12, r7, 0, 31
	
	bfi r2, r12, 0, 31
	
	bfi r0, r6, 3, 23
	
	bfi r8, r3, 9, 23
	
	bfi r12, r2, 10, 15
	
	bfi r9, r11, 13, 15
	
	bfi r0, r6, 1, 26
	
	bfi r12, r1, 3, 27
	
	bfi r9, r10, 4, 26
	
	bfi r12, r8, 11, 9
	
	bfi r10, r5, 9, 3
	
	bfi r11, r6, 20, 1
	
	bfi r2, r2, 5, 14
	
	bfi r9, r10, 3, 3
	
	bfi r5, r9, 8, 2
	
	bfi r7, r7, 16, 2
	
	bfi r2, r3, 1, 1
	
	bfi r4, r10, 18, 5
	
	bfi r10, r12, 2, 29
	
	bfi r3, r4, 23, 2
	
	bfi r3, r10, 7, 4
	
	bfi r6, r11, 10, 5
	
	bfi r10, r5, 28, 1
	
	bfi r0, r10, 5, 15
	
	bfi r1, r3, 4, 12
	
	bfi r3, r4, 2, 27
	
	bfi r2, r4, 3, 14
	
	bfi r9, r3, 24, 2
	
	bfi r0, r3, 4, 11
	
	bfi r9, r12, 1, 28
	
	bfi r6, r12, 1, 31
	
	bfi r11, r3, 0, 28
	
	bfi r0, r7, 9, 16
	
	bfi r8, r11, 14, 15
	
	bfi r9, r1, 10, 13
	
	bfi r1, r11, 2, 24
	
	bfi r7, r10, 24, 7
	
	bfi r8, r9, 4, 17
	
	bfi r12, r10, 11, 7
	
	bfi r1, r9, 2, 21
	
	bfi r2, r10, 13, 6
	
	bfi r3, r7, 3, 9
	
	bfi r4, r8, 3, 28
	
	bfi r0, r12, 19, 10
	
	bfi r6, r8, 1, 31
	
	bfi r3, r12, 1, 18
	
	bfi r9, r11, 14, 16
	
	bfi r1, r12, 0, 12
	
	bfi r2, r8, 3, 15
	
	bfi r10, r4, 2, 25
	
	bfi r1, r2, 3, 29
	
	bfi r6, r12, 6, 23
	
	bfi r0, r11, 11, 20
	
	bfi r9, r11, 24, 5
	
	bfi r10, r1, 5, 27
	
	bfi r8, r11, 8, 8
	
	bfi r8, r8, 8, 18
	
	bfi r4, r3, 2, 29
	
	bfi r7, r3, 10, 22
	
	bfi r2, r3, 11, 8
	
	bfi r12, r12, 5, 24
	
	bfi r11, r12, 16, 11
	
	bfi r4, r7, 9, 16
	
	bfi r10, r10, 4, 19
	
	bfi r6, r12, 2, 28
	
	bfi r12, r6, 8, 1
	
	bfi r6, r0, 5, 25
	
	bfi r1, r9, 9, 20
	
	bfi r6, r7, 2, 22
	
	bfi r8, r10, 2, 29
	
	bfi r4, r6, 3, 29
	
	bfi r4, r12, 10, 14
	
	bfi r1, r0, 0, 31
	
	bfi r0, r6, 1, 20
	
	bfi r1, r12, 3, 16
	
	bfi r7, r2, 4, 26
	
	bfi r6, r10, 2, 30
	
	bfi r1, r11, 26, 4
	
	bfi r12, r2, 13, 10
	
	bfi r4, r0, 4, 8
	
	bfi r5, r0, 26, 6
	
	bfi r1, r6, 23, 1
	
	bfi r7, r12, 10, 21
	
	bfi r0, r1, 21, 1
	
	bfi r1, r8, 8, 1
	
	bfi r11, r10, 23, 7
	
	bfi r0, r7, 15, 9
	
	bfi r4, r5, 16, 4
	
	bfi r2, r9, 14, 13
	
	bfi r10, r5, 15, 10
	
	bfi r8, r3, 1, 24
	
	bfi r12, r1, 2, 8
	
	bfi r12, r3, 12, 1
	
	bfi r9, r7, 5, 25
	
	bfi r4, r12, 4, 23
	
	bfi r11, r2, 6, 9
	
	bfi r8, r8, 14, 9
	
	bfi r2, r12, 18, 11
	
	bfi r11, r5, 3, 23
	
	bfi r6, r2, 4, 23
	
	bfi r3, r6, 18, 5
	
	bfi r3, r1, 3, 24
	
	bfi r10, r5, 1, 21
	
	bfi r6, r4, 10, 21
	
	bfi r0, r0, 1, 26
	
	bfi r9, r6, 12, 12
	
	bfi r2, r6, 2, 13
	
	bfi r7, r1, 15, 16
	
	bfi r10, r11, 5, 25
	
	bfi r4, r9, 2, 9
	
	bfi r8, r2, 7, 11
	
	bfi r2, r7, 7, 24
	
	bfi r11, r3, 0, 28
	
	bfi r7, r1, 28, 2
	
	bfi r4, r6, 1, 31
	
	bfi r12, r12, 25, 7
	
	bfi r9, r2, 18, 2
	
	bfi r6, r1, 13, 19
	
	bfi r0, r0, 1, 22
	
	bfi r1, r11, 1, 31
	
	bfi r2, r3, 9, 11
	
	bfi r5, r2, 5, 26
	
	bfi r2, r5, 4, 24
	
	bfi r5, r1, 0, 7
	
	bfi r8, r8, 13, 14
	
	bfi r7, r6, 4, 15
	
	bfi r6, r9, 13, 10
	
	bfi r7, r3, 21, 10
	
	bfi r5, r11, 17, 9
	
	bfi r7, r12, 10, 7
	
	bfi r11, r8, 7, 18
	
	bfi r1, r1, 4, 22
	
	bfi r11, r10, 8, 9
	
	bfi r1, r11, 21, 9
	
	bfi r4, r6, 2, 15
	
	bfi r6, r3, 19, 10
	
	bfi r9, r8, 2, 30
	
	bfi r4, r6, 1, 9
	
	bfi r4, r10, 5, 9
	
	bfi r5, r4, 16, 5
	
	bfi r9, r5, 3, 26
	
	bfi r10, r7, 11, 8
	
	bfi r9, r12, 7, 24
	
	bfi r11, r5, 10, 17
	
	bfi r2, r6, 7, 8
	
	bfi r10, r5, 6, 5
	
	bfi r9, r4, 17, 6
	
	bfi r4, r0, 19, 6
	
	bfi r1, r10, 8, 4
	
	bfi r5, r5, 5, 10
	
	bfi r3, r5, 27, 1
	
	bfi r10, r6, 5, 27
	
	bfi r3, r9, 0, 1
	
	bfi r8, r7, 13, 12
	
	bfi r8, r6, 7, 9
	
	bfi r6, r6, 0, 27
	
	bfi r6, r3, 2, 11
	
	bfi r3, r0, 0, 31
	
	bfi r4, r1, 0, 31
	
	bfi r10, r11, 10, 21
	
	bfi r6, r7, 8, 15
	
	bfi r9, r2, 25, 5
	
	bfi r2, r2, 2, 30
	
	bfi r3, r0, 3, 26
	
	bfi r2, r8, 10, 17
	
	bfi r1, r0, 12, 8
	
	bfi r7, r11, 0, 27
	
	bfi r4, r4, 10, 2
	
	bfi r2, r2, 2, 5
	
	bfi r4, r10, 20, 8
	
	bfi r4, r1, 8, 23
	
	bfi r0, r9, 2, 26
	
	bfi r7, r5, 0, 26
	
	bfi r6, r2, 1, 28
	
	bfi r5, r11, 4, 6
	
	bfi r10, r11, 0, 30
	
	bfi r10, r12, 20, 5
	
	bfi r3, r10, 4, 28
	
	bfi r10, r1, 11, 17
	
	bfi r11, r5, 10, 15
	
	bfi r11, r0, 6, 19
	
	bfi r4, r11, 1, 12
	
	bfi r12, r5, 28, 4
	
	bfi r12, r4, 0, 8
	
	bfi r11, r1, 1, 31
	
	bfi r4, r8, 1, 25
	
	bfi r5, r2, 8, 16
	
	bfi r12, r11, 1, 21
	
	bfi r3, r1, 6, 19
	
	bfi r8, r5, 8, 18
	
	bfi r8, r6, 4, 28
	
	bfi r10, r11, 3, 16
	
	bfi r6, r8, 8, 23
	
	bfi r5, r2, 0, 5
	
	bfi r7, r10, 5, 1
	
	bfi r8, r8, 10, 3
	
	bfi r5, r10, 2, 28
	
	bfi r11, r7, 2, 19
	
	bfi r0, r8, 4, 28
	
	bfi r8, r9, 5, 16
	
	bfi r2, r10, 22, 5
	
	bfi r9, r2, 3, 25
	
	bfi r5, r7, 22, 2
	
	bfi r8, r1, 6, 24
	
	bfi r12, r4, 17, 6
	
	bfi r7, r3, 14, 1
	
	bfi r4, r7, 7, 20
	
	bfi r1, r12, 14, 5
	
	bfi r6, r5, 0, 31
	
	bfi r3, r1, 4, 9
	
	bfi r0, r9, 6, 23
	
	bfi r8, r8, 1, 19
	
	bfi r11, r3, 10, 8
	
	bfi r3, r5, 5, 12
	
	bfi r0, r2, 18, 2
	
	bfi r8, r0, 13, 6
	
	bfi r3, r0, 0, 24
	
	bfi r0, r4, 7, 11
	
	bfi r3, r4, 29, 2
	
	bfi r6, r7, 1, 31
	
	bfi r5, r1, 10, 21
	
	bfi r7, r5, 4, 23
	
	bfi r4, r8, 2, 26
	
	bfi r7, r7, 5, 19
	
	bfi r12, r6, 22, 4
	
	bfi r2, r4, 5, 16
	
	bfi r9, r8, 11, 18
	
	bfi r11, r9, 20, 8
	
	bfi r8, r12, 3, 29
	
	bfi r10, r0, 9, 6
	
	bfi r2, r7, 1, 26
	
	bfi r5, r8, 30, 1
	
	bfi r11, r3, 7, 8
	
	bfi r6, r2, 9, 8
	
	bfi r0, r10, 22, 7
	
	bfi r9, r3, 2, 25
	
	bfi r6, r0, 30, 2
	
	bfi r9, r9, 8, 13
	
	bfi r11, r11, 11, 16
	
	bfi r2, r3, 2, 30
	
	bfi r5, r1, 6, 25
	
	bfi r7, r3, 0, 16
	
	bfi r6, r10, 22, 2
	
	bfi r0, r2, 5, 26
	
	bfi r5, r2, 3, 28
	
	bfi r0, r8, 0, 12
	
	bfi r12, r7, 16, 8
	
	bfi r8, r6, 6, 22
	
	bfi r10, r1, 4, 9
	
	bfi r6, r2, 13, 17
	
	bfi r6, r3, 25, 2
	
	bfi r4, r2, 6, 22
	
	bfi r6, r3, 12, 12
	
	bfi r4, r6, 7, 21
	
	bfi r10, r5, 5, 9
	
	bfi r9, r6, 0, 15
	
	bfi r9, r11, 4, 15
	
	bfi r6, r5, 15, 17
	
	bfi r8, r6, 1, 21
	
	bfi r9, r0, 12, 2
	
	bfi r5, r6, 3, 3
	
	bfi r11, r8, 0, 26
	
	bfi r2, r3, 8, 20
	
	bfi r3, r5, 9, 1
	
	bfi r1, r11, 2, 25
	
	bfi r0, r11, 1, 22
	
	bfi r11, r12, 16, 15
	
	bfi r5, r2, 1, 30
	
	bfi r12, r4, 5, 18
	
	bfi r2, r4, 3, 24
	
	bfi r1, r1, 0, 30
	
	bfi r10, r11, 3, 29
	
	bfi r5, r8, 3, 28
	
	bfi r7, r6, 16, 15
	
	bfi r10, r7, 14, 11
	
	bfi r4, r9, 16, 14
	
	bfi r9, r4, 9, 18
	
	bfi r2, r12, 5, 5
	
	bfi r0, r11, 14, 11
	
	bfi r11, r3, 12, 13
	
	bfi r5, r12, 8, 14
	
	bfi r0, r3, 20, 12
	
	bfi r5, r6, 5, 4
	
	bfi r4, r11, 9, 21
	
	bfi r1, r0, 4, 8
	
	bfi r7, r9, 3, 28
	
	bfi r6, r5, 8, 5
	
	bfi r10, r11, 12, 19
	
	bfi r4, r1, 17, 15
	
	bfi r6, r10, 0, 28
	
	bfi r4, r2, 20, 8
	
	bfi r7, r5, 14, 18
	
	bfi r7, r1, 13, 6
	
	bfi r9, r12, 6, 10
	
	bfi r7, r12, 6, 19
	
	bfi r10, r5, 2, 11
	
	bfi r9, r3, 1, 19
	
	bfi r8, r5, 12, 8
	
	bfi r0, r10, 3, 29
	
	bfi r10, r10, 7, 8
	
	bfi r8, r6, 8, 24
	
	bfi r1, r7, 0, 30
	
	bfi r10, r5, 16, 8
	
	bfi r6, r5, 16, 16
	
	bfi r9, r4, 6, 10
	
	bfi r4, r12, 4, 28
	
	bfi r4, r6, 14, 10
	
	bfi r7, r4, 3, 16
	
	bfi r1, r7, 1, 23
	
	bfi r6, r2, 26, 1
	
	bfi r1, r1, 8, 16
	
	bfi r0, r0, 4, 2
	
	bfi r1, r4, 20, 11
	
	bfi r4, r2, 4, 25
	
	bfi r3, r12, 12, 20
	
	bfi r5, r9, 4, 8
	
	bfi r11, r7, 8, 16
	
	bfi r11, r10, 3, 23
	
	bfi r12, r2, 14, 16
	
	bfi r7, r4, 12, 19
	
	bfi r3, r12, 19, 4
	
	bfi r9, r2, 6, 16
	
	bfi r1, r5, 2, 26
	
	bfi r2, r4, 10, 14
	
	bfi r4, r2, 2, 29
	
	bfi r12, r7, 1, 29
	
	bfi r8, r5, 6, 24
	
	bfi r11, r8, 11, 18
	
	bfi r9, r7, 1, 13
	
	bfi r0, r12, 0, 17
	
	bfi r0, r12, 8, 21
	
	bfi r9, r3, 25, 5
	
	bfi r10, r0, 1, 26
	
	bfi r9, r9, 1, 18
	
	bfi r5, r9, 9, 2
	
	bfi r4, r1, 16, 6
	
	bfi r6, r5, 14, 4
	
	bfi r2, r3, 22, 2
	
	bfi r0, r8, 8, 7
	
	bfi r11, r0, 2, 28
	
	bfi r11, r1, 19, 1
	
	bfi r12, r6, 0, 28
	
	bfi r2, r10, 11, 10
	
	bfi r10, r5, 5, 27
	
	bfi r4, r7, 17, 5
	
	bfi r10, r12, 15, 9
	
	bfi r6, r8, 25, 1
	
	bfi r4, r9, 0, 10
	
	bfi r2, r8, 2, 26
	
	bfi r5, r5, 12, 14
	
	bfi r2, r7, 7, 23
	
	bfi r9, r4, 26, 4
	
	bfi r1, r5, 3, 28
	
	bfi r4, r10, 10, 13
	
	bfi r4, r11, 20, 11
	
	bfi r12, r3, 16, 9
	
	bfi r10, r10, 2, 25
	
	bfi r9, r9, 5, 8
	
	bfi r8, r5, 0, 29
	
	bfi r1, r11, 23, 7
	
	bfi r9, r10, 11, 7
	
	bfi r8, r9, 0, 31
	
	bfi r1, r9, 9, 22
	
	bfi r4, r11, 5, 26
	
	bfi r8, r9, 19, 11
	
	bfi r4, r5, 8, 21
	
	bfi r2, r10, 0, 17
	
	bfi r10, r7, 17, 9
	
	bfi r2, r9, 12, 5
	
	bfi r5, r5, 7, 3
	
	bfi r2, r12, 4, 27
	
	bfi r9, r7, 5, 2
	
	bfi r8, r10, 10, 5
	
	bfi r6, r9, 0, 25
	
	bfi r12, r9, 5, 20
	
	bfi r6, r11, 6, 23
	
	bfi r3, r7, 3, 26
	
	bfi r2, r8, 2, 29
	
	bfi r12, r9, 21, 7
	
	bfi r10, r0, 2, 29
	
	bfi r10, r3, 16, 7
	
	bfi r11, r4, 21, 10
	
	bfi r4, r9, 1, 16
	
	bfi r2, r11, 13, 6
	
	bfi r0, r7, 2, 24
	
	bfi r12, r10, 0, 7
	
	bfi r11, r11, 1, 31
	
	bfi r12, r4, 0, 27
	
	bfi r2, r3, 0, 30
	
	bfi r11, r1, 4, 12
	
	bfi r9, r12, 1, 31
	
	bfi r11, r1, 13, 3
	
	bfi r5, r0, 0, 18
	
	bfi r6, r0, 5, 24
	
	bfi r6, r9, 10, 20
	
	bfi r8, r11, 12, 15
	
	bfi r2, r12, 10, 6
	
	bfi r10, r4, 3, 24
	
	bfi r5, r11, 27, 4
	
	bfi r7, r8, 15, 17
	
	bfi r5, r6, 17, 2
	
	bfi r3, r5, 3, 28
	
	bfi r0, r4, 11, 17
	
	bfi r0, r0, 8, 5
	
	bfi r2, r11, 1, 31
	
	bfi r2, r10, 0, 30
	
	bfi r3, r5, 8, 6
	
	bfi r9, r5, 14, 12
	
	bfi r7, r10, 8, 15
	
	bfi r4, r9, 20, 2
	
	bfi r2, r7, 7, 14
	
	bfi r12, r3, 15, 16
	
	bfi r6, r8, 5, 25
	
	bfi r8, r2, 0, 31
	
	bfi r2, r6, 5, 17
	
	bfi r2, r0, 2, 27
	
	bfi r2, r8, 1, 13
	
	bfi r3, r9, 4, 8
	
	bfi r10, r9, 7, 2
	
	bfi r1, r11, 13, 19
	
	bfi r1, r8, 6, 25


testcase_end:
	b testcase_end
.size main, .-main
.global main

.type default_fault_handler, %function
default_fault_handler:
	b default_fault_handler
.size default_fault_handler, .-default_fault_handler
.global default_fault_handler
