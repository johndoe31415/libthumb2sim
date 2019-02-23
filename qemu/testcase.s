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


	ldr r3, =0xffffffff
	ldr r3, =0x0
	adds r3, r3
	adds r8, r5, r2
	
	b .tclbl_2
	// Generate literal table here so we don't run out of offset space
	.ltorg
	.tclbl_2:


	ldr r11, =0xa32a891c
	ldr r9, =0x0
	adds r11, r9
	adds r6, r3, r3
	ldr r0, =0xce8d3a53
	ldr r0, =0x80000000
	add r0, r0
	adds r2, r9, r9
	ldr r2, =0x0
	ldr r6, =0x9b3371bd
	add r2, r6
	add r10, r5, r3
	ldr r4, =0x3523dd08
	ldr r2, =0xcadc22f7
	adds r4, r2
	adds r3, r12, r10
	ldr r12, =0x3140ee27
	ldr r2, =0xaa4d755
	adds r12, r2
	adds r11, r5, r6
	ldr r4, =0x0
	ldr r6, =0xf66538ed
	add r4, r6
	adds r10, r10, r0
	ldr r2, =0x5d1c323d
	ldr r7, =0xff53e3e3
	add r2, r7
	adds r9, r11, r11
	ldr r11, =0x80000000
	ldr r10, =0x0
	add r11, r10
	adds r2, r6, r10
	ldr r9, =0x80000000
	ldr r2, =0x72041fb8
	adds r9, r2
	adds r8, r11, r1
	ldr r6, =0x7fffffff
	ldr r10, =0x0
	add r6, r10
	adds r1, r6, r3
	ldr r1, =0x80000000
	ldr r4, =0xbeaf506
	add r1, r4
	add r10, r11, r1
	ldr r4, =0xecc8e3eb
	ldr r10, =0x1a40bada
	add r4, r10
	add r3, r0, r6
	ldr r3, =0x130a33f6
	ldr r4, =0x130a33f5
	add r3, r4
	adds r0, r13, r3
	ldr r0, =0x80000000
	ldr r4, =0x2ec5300c
	adds r0, r4
	add r11, r7, r12
	ldr r6, =0x408d037e
	ldr r4, =0xa924a901
	adds r6, r4
	add r9, r9, r4
	ldr r8, =0x80000000
	ldr r7, =0xf525c00f
	add r8, r7
	add r10, r0, r11
	ldr r12, =0x7fffffff
	ldr r12, =0x6d364d23
	adds r12, r12
	add r6, r13, r12
	ldr r1, =0x4c935408
	ldr r12, =0xb42bac4
	adds r1, r12
	adds r11, r14, r6
	ldr r1, =0x2a71c30
	ldr r8, =0xfd58e3cf
	adds r1, r8
	add r9, r12, r8
	ldr r2, =0x5ba758fe
	ldr r9, =0xa458a701
	adds r2, r9
	adds r0, r1, r11
	ldr r10, =0xf28f6af
	ldr r0, =0xf28f6ad
	adds r10, r0
	adds r5, r0, r9
	ldr r7, =0x46f4e5c2
	ldr r6, =0x724153ac
	add r7, r6
	add r11, r11, r3
	ldr r7, =0x7fffffff
	ldr r1, =0x5cbd56ee
	add r7, r1
	adds r4, r2, r10
	ldr r4, =0x9ecfb2da
	ldr r2, =0x9ecfb2d8
	add r4, r2
	adds r4, r8, r5
	ldr r2, =0x9c3d583d
	ldr r12, =0x9c3d583e
	add r2, r12
	adds r12, r8, r2
	ldr r6, =0xffffffff
	ldr r4, =0x0
	adds r6, r4
	add r8, r12, r6
	ldr r10, =0x3f49a8b3
	ldr r3, =0xdf519bfd
	adds r10, r3
	add r12, r11, r4
	ldr r5, =0x89b27715
	ldr r3, =0x8e75a6f6
	add r5, r3
	add r1, r8, r1
	ldr r4, =0xdba7d7a2
	ldr r0, =0xdba7d7a1
	adds r4, r0
	add r8, r12, r7
	ldr r3, =0x69c2acc9
	ldr r4, =0x963d5337
	adds r3, r4
	adds r9, r2, r8
	ldr r11, =0x98631cbd
	ldr r7, =0x7a0f7a62
	adds r11, r7
	add r5, r6, r0
	ldr r0, =0xffffffff
	ldr r11, =0x97e9e44b
	adds r0, r11
	adds r11, r4, r4
	ldr r4, =0x6ec755b2
	ldr r6, =0xdf6ed3ae
	adds r4, r6
	adds r1, r10, r11
	ldr r7, =0x813ef515
	ldr r3, =0x657c48a8
	adds r7, r3
	add r8, r0, r0
	ldr r1, =0xb8945507
	ldr r1, =0x476baaf9
	adds r1, r1
	add r7, r14, r0
	ldr r8, =0xffffffff
	ldr r9, =0x1fbbf1b3
	adds r8, r9
	adds r4, r12, r9
	ldr r8, =0xffffffff
	ldr r4, =0x7fffffff
	add r8, r4
	add r5, r7, r8
	ldr r4, =0x12adcb8b
	ldr r4, =0x80000000
	add r4, r4
	adds r9, r10, r1
	ldr r9, =0x7fffffff
	ldr r10, =0x3a2594e5
	adds r9, r10
	add r12, r8, r1
	ldr r7, =0xf0416f17
	ldr r11, =0x8abed9e2
	adds r7, r11
	add r6, r0, r11
	ldr r6, =0x74dfbd94
	ldr r12, =0x0
	adds r6, r12
	add r9, r3, r6
	ldr r1, =0x37d7ab9f
	ldr r12, =0x0
	adds r1, r12
	add r9, r10, r9
	ldr r8, =0xbbdc28f0
	ldr r9, =0xbbdc28f2
	add r8, r9
	add r11, r10, r7
	ldr r4, =0x80000000
	ldr r5, =0x7bfcc87f
	adds r4, r5
	adds r5, r6, r12
	ldr r6, =0x58b52460
	ldr r10, =0x0
	add r6, r10
	adds r0, r2, r3
	ldr r0, =0xbb4ee2a0
	ldr r8, =0xb6510e3b
	adds r0, r8
	add r0, r5, r12
	ldr r11, =0xffffffff
	ldr r8, =0x0
	add r11, r8
	add r1, r6, r8
	ldr r10, =0x1746847f
	ldr r10, =0x80000000
	adds r10, r10
	adds r10, r2, r1
	ldr r12, =0x52f189d3
	ldr r12, =0x45164b91
	adds r12, r12
	add r10, r8, r4
	ldr r10, =0xe8f99dfe
	ldr r6, =0x7fffffff
	add r10, r6
	add r7, r4, r11
	ldr r11, =0x7fffffff
	ldr r8, =0x603ad30a
	add r11, r8
	adds r11, r12, r3
	ldr r12, =0x7fffffff
	ldr r12, =0xe29bb174
	add r12, r12
	adds r3, r7, r2
	ldr r8, =0xf388f5cd
	ldr r0, =0xc770a32
	add r8, r0
	add r0, r8, r6
	ldr r6, =0x0
	ldr r11, =0x80000000
	add r6, r11
	adds r0, r4, r8
	ldr r6, =0x1d49dd10
	ldr r6, =0x1d49dd11
	add r6, r6
	add r7, r8, r2
	ldr r0, =0xffaee8e6
	ldr r5, =0xeafbe194
	adds r0, r5
	adds r2, r6, r4
	ldr r2, =0x61129148
	ldr r12, =0x14d8f6a5
	adds r2, r12
	add r6, r0, r6
	ldr r10, =0xe77e0122
	ldr r7, =0x80000000
	add r10, r7
	adds r3, r3, r10
	ldr r6, =0x17324d39
	ldr r11, =0xc44c3a67
	adds r6, r11
	adds r0, r13, r8
	ldr r3, =0xe5d99191
	ldr r12, =0xe5d99192
	add r3, r12
	adds r12, r6, r8
	ldr r10, =0x4d17e1fb
	ldr r10, =0x7fffffff
	adds r10, r10
	add r4, r9, r4
	ldr r2, =0x7736a3d4
	ldr r5, =0x88c95c2b
	adds r2, r5
	add r12, r11, r5
	ldr r1, =0x80000000
	ldr r5, =0x97a2e203
	add r1, r5
	adds r2, r3, r4
	ldr r6, =0x3409257a
	ldr r0, =0x34092578
	add r6, r0
	adds r5, r10, r11
	ldr r5, =0x12c5538f
	ldr r8, =0x7fffffff
	adds r5, r8
	add r4, r14, r8
	ldr r7, =0x7578c6dd
	ldr r9, =0x6490a95
	add r7, r9
	add r7, r8, r2
	ldr r2, =0x2728e6e7
	ldr r10, =0xd8d7191a
	add r2, r10
	adds r11, r5, r5
	ldr r7, =0x0
	ldr r8, =0x916effd9
	add r7, r8
	add r11, r4, r3
	ldr r11, =0x0
	ldr r5, =0x96440c2
	adds r11, r5
	add r1, r0, r7
	ldr r7, =0xffffffff
	ldr r12, =0xcf3bddd2
	adds r7, r12
	add r2, r9, r8
	ldr r7, =0xb523001
	ldr r5, =0xad622610
	adds r7, r5
	adds r5, r2, r1
	ldr r9, =0x9fdd3c7b
	ldr r2, =0xcfbc2357
	adds r9, r2
	adds r4, r8, r2
	ldr r2, =0x1b83a00b
	ldr r2, =0xa6af9021
	adds r2, r2
	adds r4, r5, r10
	ldr r6, =0x6706dd56
	ldr r9, =0xd764ab03
	adds r6, r9
	adds r5, r7, r2
	ldr r5, =0x7dac5bc5
	ldr r12, =0x9f2d4390
	add r5, r12
	adds r3, r10, r7
	ldr r4, =0xeadc8e2a
	ldr r6, =0xe864a5fb
	add r4, r6
	add r0, r8, r4
	ldr r9, =0x207ee271
	ldr r11, =0x0
	adds r9, r11
	add r1, r6, r10
	ldr r10, =0x5b5cdfe5
	ldr r1, =0xba492d7
	adds r10, r1
	add r9, r7, r2
	ldr r2, =0x63b32904
	ldr r11, =0x80000000
	add r2, r11
	adds r0, r0, r3
	ldr r12, =0x3c02efba
	ldr r1, =0xc3fd1045
	adds r12, r1
	add r11, r2, r4
	ldr r7, =0x0
	ldr r6, =0xeafdd475
	adds r7, r6
	add r10, r13, r10
	ldr r2, =0xa7061caa
	ldr r7, =0x74f96514
	adds r2, r7
	add r8, r13, r5
	ldr r1, =0xd80a48bd
	ldr r1, =0xa4ff6454
	add r1, r1
	adds r9, r5, r2
	ldr r3, =0xffffffff
	ldr r12, =0xe808bc32
	add r3, r12
	adds r7, r10, r0
	ldr r7, =0x6873e117
	ldr r2, =0x6873e116
	adds r7, r2
	add r10, r9, r5
	ldr r10, =0x0
	ldr r10, =0xffffffff
	adds r10, r10
	add r8, r6, r8
	ldr r3, =0x80000000
	ldr r5, =0xfaf0caab
	add r3, r5
	add r0, r6, r8
	ldr r10, =0x7fffffff
	ldr r9, =0x75611482
	add r10, r9
	adds r8, r3, r11
	ldr r1, =0x812b85f8
	ldr r1, =0x812b85fa
	adds r1, r1
	adds r6, r3, r7
	ldr r8, =0x10478c7
	ldr r8, =0x0
	adds r8, r8
	adds r6, r6, r7
	ldr r0, =0x80000000
	ldr r10, =0x7fffffff
	add r0, r10
	add r5, r10, r5
	ldr r2, =0x234000c6
	ldr r12, =0x80000000
	adds r2, r12
	adds r3, r3, r11
	ldr r7, =0x6cf124c5
	ldr r10, =0x0
	adds r7, r10
	adds r10, r4, r10
	ldr r6, =0xed4baa34
	ldr r1, =0x12b455ca
	adds r6, r1
	adds r1, r3, r3
	ldr r10, =0x80000000
	ldr r0, =0x76c0386e
	adds r10, r0
	add r1, r4, r7
	ldr r8, =0x0
	ldr r12, =0x7401342d
	add r8, r12
	add r2, r11, r5
	ldr r4, =0xc27c5564
	ldr r10, =0x19432331
	adds r4, r10
	add r11, r6, r4
	ldr r2, =0x2c2b5635
	ldr r6, =0x2c2b5637
	adds r2, r6
	add r3, r5, r10
	ldr r2, =0x9f1fbbe6
	ldr r8, =0x669e3f7a
	adds r2, r8
	adds r3, r2, r11
	ldr r5, =0xffffffff
	ldr r12, =0xcd4afd13
	add r5, r12
	add r1, r2, r9
	
	b .tclbl_3
	// Generate literal table here so we don't run out of offset space
	.ltorg
	.tclbl_3:


	ldr r11, =0x51f97d20
	ldr r11, =0x51f97d1f
	add r11, r11
	adds r7, r14, r3
	ldr r8, =0xecf6b35a
	ldr r1, =0x50102c02
	adds r8, r1
	add r12, r8, r1
	ldr r8, =0xb6840aff
	ldr r7, =0x497bf501
	adds r8, r7
	add r12, r6, r8
	ldr r0, =0xe12beedb
	ldr r3, =0x49f3e638
	add r0, r3
	adds r3, r4, r9
	ldr r0, =0x68c7bb45
	ldr r5, =0xc19afd18
	add r0, r5
	adds r0, r12, r4
	ldr r8, =0x80000000
	ldr r3, =0x7fffffff
	adds r8, r3
	adds r9, r10, r7
	ldr r6, =0x11948b4d
	ldr r2, =0x11948b4e
	add r6, r2
	add r8, r11, r2
	ldr r2, =0x676ec392
	ldr r12, =0x80000000
	adds r2, r12
	adds r3, r14, r8
	ldr r3, =0xfa93caa3
	ldr r10, =0x7fffffff
	adds r3, r10
	add r9, r2, r9
	ldr r3, =0x986137bb
	ldr r2, =0x0
	add r3, r2
	add r0, r1, r9
	ldr r9, =0x8b5a5454
	ldr r2, =0x8b5a5456
	adds r9, r2
	add r10, r2, r3
	ldr r4, =0xffffffff
	ldr r4, =0x8991e8f
	add r4, r4
	adds r9, r2, r10
	ldr r0, =0x53618aaa
	ldr r3, =0x80000000
	adds r0, r3
	add r7, r8, r2
	ldr r11, =0x0
	ldr r8, =0x4404073d
	adds r11, r8
	add r9, r2, r4
	ldr r11, =0xb9ce3b94
	ldr r8, =0x483c16bf
	add r11, r8
	adds r5, r12, r12
	ldr r6, =0xbeede76e
	ldr r8, =0x6797d893
	adds r6, r8
	add r10, r6, r3
	ldr r3, =0xd5e100
	ldr r5, =0xffffffff
	adds r3, r5
	adds r1, r1, r6
	ldr r11, =0xe132ab62
	ldr r8, =0x97463b1c
	add r11, r8
	adds r4, r2, r0
	ldr r8, =0x1146e3a9
	ldr r4, =0x9ba267c5
	adds r8, r4
	add r2, r3, r3
	ldr r1, =0x574ca70
	ldr r8, =0x7fffffff
	add r1, r8
	add r8, r5, r5
	ldr r5, =0xde5db2f5
	ldr r10, =0xc614ca8a
	add r5, r10
	add r1, r10, r10
	ldr r10, =0xe0963f4f
	ldr r6, =0xba27bdc2
	adds r10, r6
	adds r7, r0, r5
	ldr r2, =0xe150d602
	ldr r1, =0x80000000
	adds r2, r1
	adds r8, r2, r11
	ldr r4, =0x0
	ldr r5, =0x7dc4c99
	adds r4, r5
	adds r0, r4, r12
	ldr r12, =0x69cc762c
	ldr r11, =0x69cc762b
	add r12, r11
	add r7, r12, r4
	ldr r4, =0x80000000
	ldr r11, =0xd2f75b3a
	add r4, r11
	adds r2, r3, r4
	ldr r12, =0xffffffff
	ldr r10, =0xccac3071
	adds r12, r10
	add r11, r8, r12
	ldr r2, =0x80000000
	ldr r10, =0xffffffff
	add r2, r10
	adds r8, r1, r7
	ldr r0, =0xffffffff
	ldr r4, =0x7fffffff
	adds r0, r4
	adds r9, r10, r4
	ldr r11, =0xacf7f7f6
	ldr r7, =0xacf7f7f7
	adds r11, r7
	adds r6, r12, r3
	ldr r0, =0xdb9d112e
	ldr r11, =0x7fffffff
	adds r0, r11
	add r4, r10, r0
	ldr r11, =0x2a41c5f8
	ldr r8, =0x6874ca80
	adds r11, r8
	add r7, r5, r8
	ldr r9, =0x7fffffff
	ldr r0, =0xffffffff
	adds r9, r0
	adds r6, r2, r3
	ldr r8, =0x7ba8bb0b
	ldr r10, =0x46481058
	adds r8, r10
	adds r2, r1, r8
	ldr r11, =0x7fba202a
	ldr r4, =0x8045dfd6
	add r11, r4
	adds r10, r1, r1
	ldr r11, =0x0
	ldr r0, =0xd7cacc03
	adds r11, r0
	adds r3, r2, r1
	ldr r6, =0x68647507
	ldr r2, =0xc545bbda
	adds r6, r2
	adds r1, r6, r11
	ldr r2, =0x7fffffff
	ldr r8, =0x0
	add r2, r8
	adds r5, r11, r5
	ldr r12, =0xffffffff
	ldr r12, =0x7fffffff
	adds r12, r12
	add r4, r10, r4
	ldr r6, =0x8227995e
	ldr r2, =0x0
	add r6, r2
	add r6, r2, r8
	ldr r4, =0x171caafa
	ldr r11, =0xe8e35506
	add r4, r11
	add r6, r8, r8
	ldr r6, =0x57954ede
	ldr r11, =0x57954edd
	adds r6, r11
	add r3, r11, r4
	ldr r5, =0x5e47bc27
	ldr r8, =0xb16aa276
	adds r5, r8
	add r12, r2, r10
	ldr r1, =0x0
	ldr r12, =0xb3bb8847
	add r1, r12
	add r6, r10, r12
	ldr r7, =0x7fffffff
	ldr r12, =0x353b6fa4
	add r7, r12
	adds r3, r6, r9
	ldr r3, =0x7a0db919
	ldr r10, =0x8a34451d
	add r3, r10
	adds r10, r2, r0
	ldr r1, =0x70ddd95d
	ldr r2, =0xdb5fabd7
	add r1, r2
	add r11, r10, r9
	ldr r3, =0x6a84cc76
	ldr r0, =0x64d6c3db
	adds r3, r0
	add r0, r8, r12
	ldr r11, =0x9c71b74c
	ldr r10, =0x80000000
	adds r11, r10
	adds r11, r9, r3
	ldr r5, =0x7fffffff
	ldr r11, =0xa4c15fe2
	add r5, r11
	adds r7, r13, r8
	ldr r0, =0x7fffffff
	ldr r9, =0x39f55ca3
	adds r0, r9
	add r4, r0, r8
	ldr r5, =0x8e3092a5
	ldr r4, =0x3b007a31
	adds r5, r4
	adds r2, r9, r8
	ldr r6, =0xf58be6e0
	ldr r10, =0xf58be6e1
	adds r6, r10
	adds r12, r4, r12
	ldr r1, =0x381e9b33
	ldr r3, =0x5ec5006e
	adds r1, r3
	adds r5, r5, r4
	ldr r9, =0xeb50c6f5
	ldr r0, =0x50548898
	adds r9, r0
	adds r12, r9, r10
	ldr r9, =0x0
	ldr r0, =0x303eb132
	add r9, r0
	adds r4, r2, r3
	ldr r11, =0x54c679b6
	ldr r3, =0x500bae11
	add r11, r3
	add r4, r3, r8
	ldr r3, =0xe90a7488
	ldr r1, =0xc725b95d
	adds r3, r1
	add r6, r6, r2
	ldr r3, =0x0
	ldr r5, =0x397ea75a
	adds r3, r5
	adds r10, r3, r4
	ldr r9, =0x80000000
	ldr r10, =0x170cbbd3
	adds r9, r10
	add r12, r3, r5
	ldr r7, =0xc4eb7d6a
	ldr r6, =0x39c21c6b
	adds r7, r6
	adds r5, r1, r8
	ldr r5, =0x80000000
	ldr r4, =0x4ce3bd96
	adds r5, r4
	adds r5, r3, r0
	ldr r6, =0x0
	ldr r12, =0x7fffffff
	adds r6, r12
	add r0, r12, r5
	ldr r10, =0xd1caa8cb
	ldr r5, =0xd1caa8c9
	adds r10, r5
	adds r6, r3, r7
	ldr r0, =0xa81771c
	ldr r10, =0x532da1e6
	add r0, r10
	adds r11, r10, r11
	ldr r10, =0xb96e4d5e
	ldr r4, =0xffffffff
	add r10, r4
	adds r6, r9, r9
	ldr r4, =0x6f002e54
	ldr r12, =0x4e0999c1
	add r4, r12
	add r11, r3, r4
	ldr r5, =0x6c22029c
	ldr r12, =0x80000000
	adds r5, r12
	adds r4, r11, r8
	ldr r3, =0x90a7e7b5
	ldr r8, =0x90a7e7b4
	adds r3, r8
	adds r5, r13, r1
	ldr r9, =0x7fffffff
	ldr r0, =0xb11bf90f
	adds r9, r0
	add r10, r9, r7
	ldr r5, =0x6634088d
	ldr r8, =0x99cbf773
	adds r5, r8
	adds r2, r11, r1
	ldr r10, =0x7fffffff
	ldr r12, =0x80000000
	adds r10, r12
	add r0, r11, r4
	ldr r12, =0xba6f01b
	ldr r5, =0x33c109f3
	adds r12, r5
	add r7, r13, r10
	ldr r6, =0x461f0267
	ldr r10, =0xb9e0fd9a
	add r6, r10
	add r10, r10, r10
	ldr r6, =0xfe65822a
	ldr r11, =0x19a7dd5
	adds r6, r11
	adds r2, r3, r8
	ldr r10, =0x8e164583
	ldr r9, =0x71e9ba7d
	adds r10, r9
	add r10, r14, r5
	ldr r0, =0xffffffff
	ldr r3, =0x62b8f1ad
	add r0, r3
	add r2, r12, r9
	ldr r8, =0x18797ff9
	ldr r3, =0x5166c8bc
	add r8, r3
	add r4, r11, r7
	ldr r1, =0x8bb291c4
	ldr r8, =0x8bb291c6
	adds r1, r8
	adds r3, r6, r7
	ldr r2, =0x7fffffff
	ldr r3, =0x3ca02ff7
	add r2, r3
	add r3, r3, r0
	ldr r0, =0x50be1ae
	ldr r5, =0xd846e57c
	adds r0, r5
	add r12, r1, r1
	ldr r3, =0xe2f3b5ba
	ldr r7, =0xcf24b790
	adds r3, r7
	adds r0, r10, r11
	ldr r7, =0xa589d7c9
	ldr r12, =0x7fffffff
	adds r7, r12
	add r7, r5, r6
	ldr r3, =0x68950260
	ldr r11, =0xed0fcda9
	add r3, r11
	adds r3, r8, r0
	ldr r10, =0x80000000
	ldr r7, =0x11411a9c
	add r10, r7
	adds r9, r1, r4
	ldr r8, =0xedabbbff
	ldr r8, =0x0
	add r8, r8
	add r7, r6, r0
	ldr r6, =0xdbce7d4e
	ldr r0, =0xdbce7d4d
	adds r6, r0
	adds r0, r7, r3
	ldr r2, =0x9852b370
	ldr r7, =0xffffffff
	add r2, r7
	add r7, r6, r9
	ldr r3, =0x7fffffff
	ldr r6, =0x80000000
	add r3, r6
	add r10, r10, r11
	ldr r11, =0x5ab9f8be
	ldr r4, =0xb3f71704
	adds r11, r4
	adds r3, r13, r12
	ldr r3, =0x80000000
	ldr r7, =0x513497e2
	add r3, r7
	add r12, r5, r2
	ldr r4, =0xa3f21bc1
	ldr r6, =0xa3f21bc3
	add r4, r6
	add r8, r2, r7
	ldr r7, =0x7fffffff
	ldr r5, =0xc1a0fe9c
	add r7, r5
	adds r6, r12, r2
	ldr r6, =0x0
	ldr r1, =0x7fffffff
	adds r6, r1
	add r10, r3, r0
	ldr r3, =0xad8513dd
	ldr r7, =0x527aec22
	add r3, r7
	add r6, r8, r10
	ldr r5, =0x0
	ldr r3, =0x826fe6a1
	adds r5, r3
	adds r4, r6, r3
	ldr r2, =0xc945f476
	ldr r12, =0xebf27e25
	adds r2, r12
	adds r6, r10, r3
	ldr r8, =0x40e6722d
	ldr r8, =0x80000000
	add r8, r8
	adds r4, r3, r12
	ldr r1, =0xcf0ac11f
	ldr r12, =0x7fffffff
	adds r1, r12
	adds r0, r9, r7
	ldr r12, =0x7fffffff
	ldr r7, =0x6774c039
	add r12, r7
	add r12, r6, r10
	
	b .tclbl_4
	// Generate literal table here so we don't run out of offset space
	.ltorg
	.tclbl_4:


	ldr r10, =0x9307656
	ldr r12, =0xe811c04b
	adds r10, r12
	adds r8, r6, r5
	ldr r1, =0xffffffff
	ldr r4, =0x80000000
	add r1, r4
	adds r6, r9, r3
	ldr r2, =0xc4695736
	ldr r11, =0x80000000
	adds r2, r11
	add r5, r9, r1
	ldr r3, =0x994186b7
	ldr r1, =0x5822ee5f
	adds r3, r1
	add r0, r3, r8
	ldr r7, =0xbac232c7
	ldr r1, =0xbc7f4948
	add r7, r1
	adds r2, r2, r9
	ldr r4, =0xc9d24dbb
	ldr r0, =0xfeb3e612
	add r4, r0
	adds r4, r13, r9
	ldr r3, =0x7fffffff
	ldr r8, =0x32e6907a
	add r3, r8
	add r9, r2, r7
	ldr r9, =0x7fffffff
	ldr r7, =0xb92679ff
	adds r9, r7
	adds r12, r1, r4
	ldr r1, =0xa973243d
	ldr r1, =0x11de5d28
	add r1, r1
	add r1, r6, r1
	ldr r7, =0xb46bf6cf
	ldr r4, =0x4b94092f
	adds r7, r4
	add r7, r13, r2
	ldr r6, =0xd55aad10
	ldr r12, =0x80000000
	add r6, r12
	adds r4, r6, r6
	ldr r2, =0x295d1ce0
	ldr r11, =0xd6a2e31e
	add r2, r11
	adds r5, r11, r12
	ldr r2, =0x61f587c2
	ldr r10, =0x61f587c1
	add r2, r10
	adds r3, r12, r8
	ldr r9, =0xcc7bc1c
	ldr r10, =0xd8d0e23a
	adds r9, r10
	add r3, r5, r12
	ldr r7, =0x233768a8
	ldr r6, =0x7fffffff
	add r7, r6
	add r9, r2, r1
	ldr r9, =0x3cb37a8f
	ldr r1, =0xe8ba7fd
	adds r9, r1
	add r5, r8, r7
	ldr r5, =0xbea5dc13
	ldr r2, =0x0
	add r5, r2
	add r4, r0, r2
	ldr r12, =0x832bb580
	ldr r8, =0x7fffffff
	add r12, r8
	add r6, r0, r2
	ldr r8, =0x80000000
	ldr r4, =0xc2a63299
	add r8, r4
	adds r10, r5, r0
	ldr r0, =0x8b909ee5
	ldr r12, =0x746f611a
	add r0, r12
	add r11, r13, r4
	ldr r1, =0xbfa6a43b
	ldr r7, =0x7fffffff
	add r1, r7
	adds r12, r7, r12
	ldr r8, =0x7244c62f
	ldr r3, =0xf43d85bd
	add r8, r3
	adds r2, r3, r12
	ldr r2, =0x3b13a41d
	ldr r3, =0x11bcd3ac
	add r2, r3
	adds r12, r9, r12
	ldr r8, =0x0
	ldr r11, =0xffffffff
	add r8, r11
	adds r8, r10, r7
	ldr r4, =0x55192660
	ldr r0, =0xeeede4f4
	add r4, r0
	adds r12, r4, r1
	ldr r12, =0xb7dac18f
	ldr r4, =0x3d1f2b63
	adds r12, r4
	adds r0, r5, r11
	ldr r8, =0x7fffffff
	ldr r11, =0xa64be872
	adds r8, r11
	adds r3, r2, r10
	ldr r4, =0x5365af11
	ldr r1, =0x9ea61d6f
	adds r4, r1
	adds r3, r12, r0
	ldr r3, =0xffffffff
	ldr r0, =0xb5535cb0
	add r3, r0
	adds r7, r5, r9
	ldr r6, =0xf1c5498f
	ldr r2, =0x2a50f56e
	adds r6, r2
	adds r12, r0, r10
	ldr r11, =0xebc6947d
	ldr r11, =0x14396b82
	adds r11, r11
	add r12, r1, r10
	ldr r7, =0x7fffffff
	ldr r8, =0x560aae3e
	adds r7, r8
	add r0, r14, r0
	ldr r12, =0xaae3c0e4
	ldr r6, =0x0
	adds r12, r6
	add r10, r7, r2
	ldr r9, =0x7a6a4822
	ldr r7, =0x84a136db
	adds r9, r7
	add r3, r11, r10
	ldr r5, =0x80000000
	ldr r9, =0x62f97a61
	add r5, r9
	add r1, r9, r0
	ldr r2, =0xffffffff
	ldr r3, =0x53a6761d
	adds r2, r3
	adds r4, r14, r2
	ldr r7, =0xff89a28
	ldr r2, =0x3f792bda
	add r7, r2
	adds r9, r0, r5
	ldr r8, =0x778a011a
	ldr r12, =0x778a011b
	adds r8, r12
	adds r7, r7, r9
	ldr r11, =0xeae40e6b
	ldr r0, =0xae35cb9b
	add r11, r0
	add r11, r6, r8
	ldr r6, =0xffffffff
	ldr r5, =0x3625b34c
	adds r6, r5
	adds r9, r11, r4
	ldr r0, =0x43c7667
	ldr r9, =0xfbc38998
	add r0, r9
	adds r0, r0, r9
	ldr r8, =0x7fffffff
	ldr r2, =0x20f18984
	add r8, r2
	adds r5, r7, r7
	ldr r9, =0x1bde3b1d
	ldr r12, =0x80000000
	adds r9, r12
	add r12, r2, r1
	ldr r1, =0x0
	ldr r9, =0xccae97a7
	add r1, r9
	adds r9, r3, r10
	ldr r1, =0x84a267d8
	ldr r5, =0x84a267d9
	adds r1, r5
	add r5, r7, r7
	ldr r7, =0x76af4dd5
	ldr r8, =0x62fd513f
	add r7, r8
	adds r10, r7, r11
	ldr r10, =0xec754cef
	ldr r12, =0x0
	adds r10, r12
	adds r0, r2, r1
	ldr r11, =0x80000000
	ldr r3, =0x86576d70
	adds r11, r3
	add r1, r0, r10
	ldr r12, =0xa042f7
	ldr r6, =0x80000000
	adds r12, r6
	add r0, r12, r7
	ldr r11, =0x80000000
	ldr r11, =0xffffffff
	add r11, r11
	add r9, r7, r9
	ldr r8, =0xd540e8d2
	ldr r12, =0x2abf172d
	add r8, r12
	adds r1, r8, r1
	ldr r7, =0x0
	ldr r4, =0x56bd8cb8
	add r7, r4
	add r0, r13, r8
	ldr r5, =0x0
	ldr r4, =0x7fffffff
	add r5, r4
	add r3, r13, r8
	ldr r2, =0x80000000
	ldr r12, =0x478743c4
	add r2, r12
	add r3, r9, r7
	ldr r3, =0x6f96c05d
	ldr r7, =0xc8f63aa2
	add r3, r7
	add r3, r9, r12
	ldr r4, =0x91029324
	ldr r10, =0x80000000
	add r4, r10
	add r6, r11, r7
	ldr r2, =0x32d0b653
	ldr r7, =0x32d0b652
	adds r2, r7
	add r3, r13, r5
	ldr r3, =0xd7512eab
	ldr r4, =0x2641eff6
	adds r3, r4
	adds r4, r4, r8
	ldr r9, =0xd7c1205b
	ldr r3, =0xeb98b1b4
	adds r9, r3
	adds r2, r9, r2
	ldr r1, =0xffffffff
	ldr r7, =0x49f34419
	add r1, r7
	adds r1, r6, r1
	ldr r8, =0x13bc1b7c
	ldr r4, =0x820c67be
	add r8, r4
	add r4, r6, r10
	ldr r8, =0xffffffff
	ldr r12, =0xa1bea50d
	adds r8, r12
	adds r8, r5, r5
	ldr r5, =0x7fffffff
	ldr r8, =0x84c8b7af
	add r5, r8
	add r6, r6, r8
	ldr r4, =0x44f78727
	ldr r4, =0x44f78729
	adds r4, r4
	add r12, r9, r2
	ldr r12, =0x0
	ldr r10, =0xf8873b69
	add r12, r10
	add r8, r5, r3
	ldr r7, =0x0
	ldr r5, =0x6733f73f
	adds r7, r5
	adds r1, r7, r6
	ldr r5, =0x8a9ce70b
	ldr r11, =0x756318f6
	add r5, r11
	adds r0, r14, r2
	ldr r5, =0x604179b0
	ldr r5, =0xc9f8fc25
	adds r5, r5
	add r3, r4, r3
	ldr r10, =0x0
	ldr r0, =0x83e90610
	adds r10, r0
	adds r2, r6, r8
	ldr r10, =0xb05b90e5
	ldr r4, =0x8d43450
	add r10, r4
	adds r1, r8, r2
	ldr r4, =0x594af086
	ldr r0, =0x594af088
	add r4, r0
	adds r3, r14, r3
	ldr r6, =0xa2fb55f8
	ldr r1, =0x0
	adds r6, r1
	adds r2, r12, r6
	ldr r1, =0xb5c0454e
	ldr r11, =0x6a8e085e
	add r1, r11
	adds r9, r14, r7
	ldr r10, =0x7f0d1e83
	ldr r7, =0x80000000
	adds r10, r7
	add r3, r4, r2
	ldr r3, =0xf4c3ed0f
	ldr r3, =0x769277f5
	adds r3, r3
	add r8, r3, r12
	ldr r2, =0x4ad12c6a
	ldr r6, =0x2ff33401
	add r2, r6
	add r6, r13, r9
	ldr r3, =0x594ca802
	ldr r1, =0x17a2f7ef
	adds r3, r1
	adds r8, r7, r11
	ldr r2, =0x80bf354c
	ldr r9, =0x0
	add r2, r9
	add r9, r14, r7
	ldr r5, =0x6d209752
	ldr r5, =0x92df68ac
	add r5, r5
	adds r6, r2, r9
	ldr r2, =0x43c18737
	ldr r1, =0xbc3e78c7
	adds r2, r1
	adds r5, r2, r0
	ldr r8, =0x80000000
	ldr r7, =0x8d5755e6
	adds r8, r7
	adds r10, r10, r9
	ldr r3, =0x7fffffff
	ldr r1, =0x0
	add r3, r1
	add r10, r4, r10
	ldr r11, =0x3d23ddf2
	ldr r10, =0x7fffffff
	adds r11, r10
	adds r3, r10, r5
	ldr r7, =0xd3ec0bf0
	ldr r1, =0xd3ec0bf1
	adds r7, r1
	adds r5, r4, r7
	ldr r0, =0xa4895736
	ldr r9, =0xd51851d
	add r0, r9
	add r11, r5, r4
	ldr r3, =0xcb54cfd9
	ldr r9, =0xa7ec622f
	adds r3, r9
	add r10, r0, r3
	ldr r10, =0x2e5fe973
	ldr r4, =0x2e5fe975
	add r10, r4
	add r5, r3, r3
	ldr r8, =0xd07af67f
	ldr r4, =0xd07af67d
	add r8, r4
	adds r8, r0, r8
	ldr r3, =0x9fb3180b
	ldr r4, =0x604ce7f3
	add r3, r4
	add r1, r2, r5
	ldr r3, =0xf7942261
	ldr r3, =0x79602356
	add r3, r3
	add r5, r1, r6
	ldr r0, =0xffffffff
	ldr r9, =0x69feaf25
	add r0, r9
	add r1, r3, r1
	ldr r12, =0xd8219277
	ldr r3, =0x3410b2da
	adds r12, r3
	add r6, r12, r4
	ldr r0, =0x80000000
	ldr r6, =0x21f3a2aa
	add r0, r6
	add r5, r4, r11
	ldr r11, =0x9ab66204
	ldr r2, =0xf78da96e
	add r11, r2
	adds r11, r10, r3
	ldr r1, =0xffffffff
	ldr r10, =0x45127038
	add r1, r10
	add r9, r5, r7
	ldr r2, =0x9c1f7df6
	ldr r2, =0x63e08208
	adds r2, r2
	add r12, r5, r6
	ldr r7, =0xe4f6d5e0
	ldr r0, =0x9f01cc29
	adds r7, r0
	add r1, r9, r10
	ldr r5, =0x0
	ldr r7, =0x1ee83aa7
	adds r5, r7
	add r1, r11, r4
	ldr r6, =0x6e2dbc49
	ldr r7, =0x603b5ee8
	adds r6, r7
	add r10, r7, r10
	ldr r8, =0x0
	ldr r4, =0x4a83c1ec
	add r8, r4
	add r4, r1, r9
	
	b .tclbl_5
	// Generate literal table here so we don't run out of offset space
	.ltorg
	.tclbl_5:


	ldr r1, =0xb29fb170
	ldr r12, =0x38d1b62a
	add r1, r12
	adds r4, r11, r4
	ldr r4, =0x71e4cb24
	ldr r3, =0xffffffff
	adds r4, r3
	adds r10, r5, r0
	ldr r5, =0xc0966a0e
	ldr r1, =0x80000000
	adds r5, r1
	adds r10, r12, r9
	ldr r9, =0xffffffff
	ldr r7, =0x753660dd
	adds r9, r7
	add r6, r10, r4
	ldr r8, =0x714b6459
	ldr r0, =0xffffffff
	adds r8, r0
	add r4, r0, r4
	ldr r8, =0xc1b9c4cb
	ldr r11, =0x3e463b36
	adds r8, r11
	adds r8, r9, r10
	ldr r1, =0x36160941
	ldr r3, =0x5ccde84f
	add r1, r3
	add r12, r7, r1
	ldr r11, =0x7fffffff
	ldr r11, =0x0
	adds r11, r11
	add r11, r0, r11
	ldr r7, =0x286d35
	ldr r7, =0xe110daf3
	add r7, r7
	add r12, r12, r9
	ldr r6, =0xffffffff
	ldr r2, =0x2b02506f
	adds r6, r2
	adds r10, r1, r1
	ldr r2, =0x34e16de4
	ldr r6, =0xbaf9bdf3
	adds r2, r6
	adds r3, r6, r5
	ldr r11, =0x490b001a
	ldr r0, =0xb6f4ffe4
	add r11, r0
	add r0, r6, r2
	ldr r9, =0xd7c18be4
	ldr r1, =0xf47209ea
	adds r9, r1
	add r12, r6, r7
	ldr r0, =0xffffffff
	ldr r6, =0xe67db433
	add r0, r6
	add r4, r1, r8
	ldr r3, =0xc8f7085e
	ldr r10, =0xc8f7085d
	adds r3, r10
	add r0, r12, r6
	ldr r1, =0x965c68b6
	ldr r8, =0x965c68b5
	adds r1, r8
	adds r1, r10, r12
	ldr r12, =0xffffffff
	ldr r5, =0xb3716e22
	adds r12, r5
	adds r7, r2, r10
	ldr r6, =0x7fffffff
	ldr r5, =0xf8353889
	adds r6, r5
	add r1, r8, r6
	ldr r11, =0x9b0d38db
	ldr r0, =0x0
	add r11, r0
	add r5, r14, r11
	ldr r12, =0x5739da3f
	ldr r3, =0x7fffffff
	add r12, r3
	adds r2, r13, r2
	ldr r8, =0xf1f86482
	ldr r0, =0x40f539da
	adds r8, r0
	add r5, r12, r2
	ldr r6, =0x0
	ldr r8, =0xffffffff
	add r6, r8
	adds r6, r9, r3
	ldr r4, =0x69eabb1b
	ldr r0, =0x90efd21b
	add r4, r0
	adds r6, r1, r5
	ldr r8, =0x8ff8c0fc
	ldr r12, =0xb510ff
	add r8, r12
	add r5, r0, r5
	ldr r0, =0x0
	ldr r7, =0x1c257d20
	adds r0, r7
	add r5, r6, r0
	ldr r8, =0x80000000
	ldr r8, =0x10fb6877
	adds r8, r8
	adds r7, r14, r6
	ldr r0, =0x37be0239
	ldr r9, =0xffffffff
	adds r0, r9
	adds r1, r8, r10
	ldr r6, =0xd0f3965
	ldr r11, =0xf2f0c69b
	add r6, r11
	add r2, r11, r6
	ldr r4, =0xbe5d6fee
	ldr r6, =0xea22705e
	adds r4, r6
	add r11, r10, r7
	ldr r8, =0x31a8a82f
	ldr r9, =0x80000000
	add r8, r9
	adds r3, r12, r10
	ldr r2, =0x80000000
	ldr r7, =0x7fffffff
	add r2, r7
	add r5, r0, r11
	ldr r10, =0xffffffff
	ldr r5, =0x85a77f7
	adds r10, r5
	add r5, r10, r6
	ldr r2, =0xbbe2fbf8
	ldr r2, =0x80000000
	adds r2, r2
	add r3, r0, r10
	ldr r1, =0xd8bead1f
	ldr r0, =0xdf900c3e
	adds r1, r0
	adds r11, r14, r12
	ldr r9, =0x27ded95c
	ldr r4, =0x6bb386d7
	add r9, r4
	add r7, r13, r7
	ldr r6, =0x7fffffff
	ldr r1, =0x63170ed9
	add r6, r1
	adds r12, r7, r3
	ldr r12, =0xe45d760
	ldr r11, =0xffffffff
	adds r12, r11
	adds r3, r12, r12
	ldr r7, =0xecdf9901
	ldr r7, =0xecdf9902
	add r7, r7
	add r8, r12, r3
	ldr r7, =0x2aed47d3
	ldr r11, =0xffffffff
	add r7, r11
	add r4, r10, r8
	ldr r10, =0xddea5e7d
	ldr r11, =0xffffffff
	adds r10, r11
	add r3, r6, r1
	ldr r1, =0x248c3b2c
	ldr r0, =0x0
	adds r1, r0
	adds r8, r7, r1
	ldr r7, =0x80000000
	ldr r11, =0x65d79a9a
	adds r7, r11
	adds r0, r12, r9
	ldr r11, =0x5c119c2d
	ldr r2, =0xac7269df
	add r11, r2
	adds r2, r4, r12
	ldr r11, =0x80000000
	ldr r5, =0xae3b9eea
	add r11, r5
	add r5, r14, r7
	ldr r5, =0xc6b67a89
	ldr r2, =0x80000000
	add r5, r2
	adds r6, r5, r4
	ldr r10, =0x5d1ba1c6
	ldr r2, =0x5a5a69aa
	add r10, r2
	add r6, r3, r6
	ldr r2, =0x34533b80
	ldr r5, =0xcbacc481
	adds r2, r5
	adds r1, r12, r4
	ldr r12, =0x9fa7613e
	ldr r12, =0x60589ec1
	add r12, r12
	adds r10, r2, r8
	ldr r12, =0x7fffffff
	ldr r4, =0xd9898628
	adds r12, r4
	adds r11, r5, r12
	ldr r4, =0x0
	ldr r4, =0xd7be7209
	adds r4, r4
	add r1, r7, r8
	ldr r6, =0x975d6717
	ldr r1, =0xcd1cd671
	adds r6, r1
	adds r2, r13, r12
	ldr r8, =0xf67942e1
	ldr r12, =0x8dc944a9
	add r8, r12
	adds r6, r6, r8
	ldr r3, =0xe948552b
	ldr r7, =0x4afe6e46
	adds r3, r7
	add r1, r12, r4
	ldr r11, =0xffffffff
	ldr r6, =0x272d4a0a
	adds r11, r6
	adds r11, r2, r8
	ldr r8, =0x0
	ldr r6, =0x562a8031
	adds r8, r6
	add r10, r10, r2
	ldr r5, =0xd1af6165
	ldr r10, =0xd1af6166
	add r5, r10
	add r3, r14, r8
	ldr r4, =0x424836d7
	ldr r3, =0xbdb7c92a
	add r4, r3
	adds r2, r12, r2
	ldr r0, =0xb498c1d7
	ldr r2, =0xb498c1d9
	adds r0, r2
	add r3, r1, r11
	ldr r8, =0x7fffffff
	ldr r11, =0x9ddb9163
	adds r8, r11
	adds r9, r12, r5
	ldr r11, =0xce8cbc69
	ldr r2, =0x31734396
	add r11, r2
	add r10, r3, r0
	ldr r7, =0x0
	ldr r9, =0xbc472822
	adds r7, r9
	add r0, r12, r2
	ldr r5, =0xdeeb9c11
	ldr r6, =0xadc1ce40
	adds r5, r6
	add r0, r14, r7
	ldr r5, =0x1c668dfd
	ldr r11, =0x7fffffff
	add r5, r11
	add r10, r2, r9
	ldr r2, =0x36cd9b3c
	ldr r0, =0xc93264c2
	add r2, r0
	add r3, r4, r3
	ldr r12, =0x3616ac61
	ldr r0, =0x7fffffff
	add r12, r0
	add r11, r4, r9
	ldr r4, =0xf0ae6e14
	ldr r6, =0xf0ae6e13
	adds r4, r6
	add r4, r1, r8
	ldr r4, =0x0
	ldr r5, =0xee5ad8e7
	add r4, r5
	add r4, r2, r10
	ldr r3, =0xffffffff
	ldr r8, =0x7fffffff
	add r3, r8
	adds r2, r13, r6
	ldr r0, =0x468d47b9
	ldr r10, =0x80000000
	add r0, r10
	add r12, r0, r3
	ldr r1, =0x7bfd3d5d
	ldr r4, =0x80000000
	add r1, r4
	add r3, r11, r5
	ldr r0, =0xc3b88677
	ldr r5, =0x7fffffff
	add r0, r5
	adds r3, r3, r12
	ldr r8, =0x80000000
	ldr r12, =0x3e033c59
	add r8, r12
	add r10, r6, r6
	ldr r12, =0x3030c4a9
	ldr r3, =0x3030c4a8
	add r12, r3
	add r6, r14, r3
	ldr r9, =0x7fffffff
	ldr r1, =0xe3f03a2
	adds r9, r1
	add r7, r1, r3
	ldr r4, =0x7fffffff
	ldr r2, =0x66ff80db
	add r4, r2
	add r8, r1, r5
	ldr r6, =0x0
	ldr r4, =0xffffffff
	add r6, r4
	adds r5, r11, r8
	ldr r5, =0x63c6db55
	ldr r7, =0x80000000
	adds r5, r7
	add r2, r11, r5
	ldr r3, =0x80000000
	ldr r9, =0x510804d2
	adds r3, r9
	adds r4, r10, r9
	ldr r1, =0xa1869749
	ldr r1, =0x5e7968b8
	adds r1, r1
	adds r8, r6, r3
	ldr r0, =0x80000000
	ldr r9, =0xffffffff
	adds r0, r9
	add r2, r9, r4
	ldr r0, =0x80000000
	ldr r11, =0x7fffffff
	adds r0, r11
	add r12, r0, r10
	ldr r8, =0xaa243fad
	ldr r10, =0xfbcd1db9
	adds r8, r10
	add r5, r4, r9
	ldr r11, =0xb91d22b
	ldr r3, =0xa3011ef1
	add r11, r3
	add r8, r7, r7
	ldr r1, =0x80000000
	ldr r8, =0x0
	adds r1, r8
	adds r8, r0, r9
	ldr r3, =0xa301e56c
	ldr r2, =0x0
	adds r3, r2
	adds r8, r1, r5
	ldr r11, =0xb5629984
	ldr r11, =0xb5629983
	adds r11, r11
	adds r10, r13, r1
	ldr r12, =0xd5a4da55
	ldr r7, =0x7fffffff
	add r12, r7
	adds r0, r10, r6
	ldr r2, =0xd29c3f27
	ldr r1, =0xffffffff
	add r2, r1
	adds r3, r6, r10
	ldr r3, =0xcc5c9352
	ldr r4, =0xcc5c9353
	add r3, r4
	add r11, r8, r8
	ldr r3, =0xc3828680
	ldr r5, =0x80000000
	adds r3, r5
	add r1, r1, r9
	ldr r5, =0x3a6bb6d1
	ldr r11, =0x7fffffff
	adds r5, r11
	adds r1, r9, r6
	ldr r5, =0x7fffffff
	ldr r3, =0x8fa13dea
	adds r5, r3
	adds r7, r6, r6
	ldr r6, =0xffffffff
	ldr r7, =0x541ffb3
	add r6, r7
	adds r11, r7, r4
	ldr r11, =0x80000000
	ldr r10, =0x9d1be7e2
	add r11, r10
	adds r9, r4, r7
	ldr r11, =0x8fa15fad
	ldr r9, =0x705ea051
	add r11, r9
	add r12, r5, r5
	ldr r8, =0xa4673418
	ldr r1, =0x5b98cbe8
	adds r8, r1
	adds r8, r1, r1
	ldr r8, =0x5cf7c870
	ldr r5, =0xaac6b525
	add r8, r5
	adds r3, r13, r1
	ldr r7, =0x6613260a
	ldr r1, =0x7fffffff
	add r7, r1
	adds r10, r8, r5
	ldr r0, =0x7fffffff
	ldr r3, =0xffffffff
	adds r0, r3
	adds r0, r1, r11
	ldr r6, =0x500fa23f
	ldr r5, =0xffffffff
	adds r6, r5
	add r4, r4, r4
	
	b .tclbl_6
	// Generate literal table here so we don't run out of offset space
	.ltorg
	.tclbl_6:


	ldr r8, =0x4319ae4c
	ldr r3, =0xa73f316e
	add r8, r3
	add r8, r13, r8
	ldr r2, =0xf856577c
	ldr r12, =0xffffffff
	add r2, r12
	add r4, r3, r6
	ldr r1, =0x896722d6
	ldr r1, =0x896722d5
	adds r1, r1
	add r4, r1, r0
	ldr r8, =0x0
	ldr r3, =0xb2c9926
	add r8, r3
	adds r6, r8, r7
	ldr r5, =0x7fffffff
	ldr r4, =0xdb83b07f
	adds r5, r4
	adds r5, r7, r12
	ldr r3, =0x0
	ldr r10, =0x8f9ebd51
	adds r3, r10
	adds r7, r14, r12
	ldr r4, =0xcd6c22d
	ldr r4, =0xcd6c22e
	adds r4, r4
	adds r1, r5, r0
	ldr r8, =0xae7f1444
	ldr r5, =0x4d571e2c
	add r8, r5
	add r9, r1, r9
	ldr r5, =0x80000000
	ldr r6, =0xffffffff
	add r5, r6
	add r6, r4, r9
	ldr r11, =0x8183bf39
	ldr r0, =0x80000000
	adds r11, r0
	adds r5, r12, r4
	ldr r6, =0xa4a8606e
	ldr r5, =0x43d99746
	add r6, r5
	adds r0, r5, r6
	ldr r12, =0xce12432e
	ldr r9, =0xed5aa3fa
	adds r12, r9
	adds r6, r7, r4
	ldr r11, =0x8b3cb551
	ldr r8, =0x2cf22806
	add r11, r8
	add r3, r12, r8
	ldr r9, =0x772a0600
	ldr r8, =0x80000000
	adds r9, r8
	add r11, r11, r10
	ldr r5, =0xc66b3c4
	ldr r10, =0xf3994c3b
	adds r5, r10
	add r4, r14, r2
	ldr r0, =0x59253b43
	ldr r6, =0xa6dac4be
	add r0, r6
	add r7, r11, r11
	ldr r10, =0x0
	ldr r1, =0xffffffff
	adds r10, r1
	add r12, r9, r3
	ldr r2, =0xbadc22da
	ldr r2, =0xe86f71b4
	adds r2, r2
	adds r6, r9, r4
	ldr r10, =0xecaaaacf
	ldr r0, =0xecaaaad0
	adds r10, r0
	adds r12, r12, r9
	ldr r3, =0xf2a5f538
	ldr r2, =0x4ec39a57
	adds r3, r2
	add r3, r0, r9
	ldr r0, =0x24978730
	ldr r10, =0x8a2b6e36
	adds r0, r10
	adds r2, r9, r6
	ldr r10, =0xd7a1aa1f
	ldr r2, =0xed36569e
	adds r10, r2
	adds r7, r0, r4
	ldr r7, =0x9f81fe38
	ldr r5, =0xe66b3416
	add r7, r5
	adds r4, r8, r7
	ldr r12, =0xffffffff
	ldr r7, =0x0
	adds r12, r7
	adds r7, r8, r1
	ldr r11, =0x4025ac88
	ldr r5, =0xffffffff
	add r11, r5
	add r6, r6, r12
	ldr r5, =0x7fffffff
	ldr r7, =0x0
	add r5, r7
	adds r8, r12, r5
	ldr r9, =0x0
	ldr r7, =0xce9b67f6
	add r9, r7
	adds r10, r8, r5
	ldr r7, =0x761ecea0
	ldr r12, =0xa094e732
	adds r7, r12
	adds r8, r11, r3
	ldr r5, =0x69c4d9c1
	ldr r12, =0x7fffffff
	add r5, r12
	add r2, r9, r11
	ldr r11, =0x9a52ba09
	ldr r6, =0xc91f7c82
	adds r11, r6
	adds r6, r11, r11
	ldr r4, =0x6d9c3f1
	ldr r5, =0xbb3fe3c8
	add r4, r5
	adds r4, r9, r11
	ldr r2, =0x0
	ldr r5, =0x97854a51
	add r2, r5
	add r10, r0, r2
	ldr r8, =0x7fffffff
	ldr r10, =0x41eb8386
	add r8, r10
	add r7, r3, r4
	ldr r1, =0x9ebd5262
	ldr r0, =0x0
	adds r1, r0
	add r12, r3, r7
	ldr r8, =0x0
	ldr r7, =0x9b7bd68c
	add r8, r7
	adds r3, r11, r8
	ldr r12, =0x7fffffff
	ldr r0, =0x6e735124
	add r12, r0
	add r7, r14, r4
	ldr r6, =0x7fffffff
	ldr r10, =0x9aa032d1
	adds r6, r10
	adds r12, r1, r12
	ldr r10, =0xfa61059a
	ldr r6, =0x10cac59
	add r10, r6
	add r0, r9, r11
	ldr r10, =0x75c93a3
	ldr r2, =0xffffffff
	adds r10, r2
	add r4, r12, r4
	ldr r1, =0xbaf784c3
	ldr r2, =0xf6f442b1
	adds r1, r2
	add r12, r4, r7
	ldr r10, =0xd78d87b2
	ldr r2, =0xd78d87b1
	add r10, r2
	adds r12, r13, r0
	ldr r8, =0x7f0f65c5
	ldr r6, =0x7fffffff
	adds r8, r6
	add r0, r14, r2
	ldr r6, =0x8f4a12e0
	ldr r2, =0x0
	adds r6, r2
	adds r11, r10, r10
	ldr r9, =0xe36c9605
	ldr r10, =0x80000000
	add r9, r10
	add r3, r6, r2
	ldr r4, =0xa48d0f3a
	ldr r11, =0xffffffff
	add r4, r11
	add r10, r5, r6
	ldr r2, =0x9ab7501a
	ldr r6, =0x6548afe6
	add r2, r6
	adds r5, r12, r10
	ldr r2, =0xa47afb62
	ldr r10, =0xce87cb0e
	add r2, r10
	adds r10, r13, r4
	ldr r5, =0x92712b9d
	ldr r11, =0x92712b9c
	adds r5, r11
	adds r8, r2, r6
	ldr r11, =0x837c873b
	ldr r10, =0x837c873c
	adds r11, r10
	adds r6, r12, r4
	ldr r1, =0x0
	ldr r11, =0x5de27120
	adds r1, r11
	add r2, r14, r8
	ldr r3, =0x80000000
	ldr r6, =0x183c3773
	add r3, r6
	add r0, r0, r11
	ldr r9, =0x7c7cadd3
	ldr r10, =0x80000000
	adds r9, r10
	adds r4, r11, r12
	ldr r0, =0x6efa87a4
	ldr r3, =0xffffffff
	add r0, r3
	adds r11, r12, r0
	ldr r10, =0xffffffff
	ldr r7, =0x4f6af78
	add r10, r7
	add r5, r5, r3
	ldr r7, =0x6dd7399f
	ldr r6, =0x9228c660
	add r7, r6
	adds r12, r4, r9
	ldr r3, =0xf869a4a8
	ldr r5, =0x238f4ad6
	adds r3, r5
	add r11, r12, r5
	ldr r10, =0xffffffff
	ldr r1, =0xd6d9bfd8
	adds r10, r1
	add r1, r6, r8
	ldr r0, =0xced6079d
	ldr r8, =0x3b1dbd40
	adds r0, r8
	adds r4, r6, r3
	ldr r9, =0x86ecca80
	ldr r0, =0xb778e024
	add r9, r0
	add r0, r13, r5
	ldr r2, =0xd97e2b43
	ldr r2, =0x7f6253d8
	add r2, r2
	adds r10, r12, r10
	ldr r3, =0x3d4242c6
	ldr r8, =0x8d91f56f
	adds r3, r8
	adds r1, r7, r12
	ldr r3, =0xd338cddf
	ldr r1, =0xd338cddd
	adds r3, r1
	add r6, r9, r11
	ldr r5, =0x80000000
	ldr r4, =0xd45f66a1
	add r5, r4
	add r4, r4, r6
	ldr r12, =0x3581a393
	ldr r5, =0xca7e5c6d
	add r12, r5
	add r7, r5, r10
	ldr r7, =0xd7b53a5d
	ldr r12, =0x2e5fd220
	add r7, r12
	add r7, r14, r2
	ldr r12, =0x3303456e
	ldr r0, =0x3303456f
	adds r12, r0
	add r11, r5, r9
	ldr r9, =0x558ce33e
	ldr r2, =0xa155efd5
	add r9, r2
	adds r1, r7, r5
	ldr r9, =0x0
	ldr r4, =0xd7cf91ba
	add r9, r4
	add r10, r14, r12
	ldr r0, =0x7fffffff
	ldr r3, =0x5e58db8d
	adds r0, r3
	add r8, r6, r11
	ldr r2, =0x58733b40
	ldr r7, =0x80000000
	adds r2, r7
	add r11, r0, r9
	ldr r8, =0xf889128b
	ldr r4, =0x7fffffff
	add r8, r4
	add r0, r7, r6
	ldr r1, =0xd6595dc
	ldr r2, =0xd6595dd
	adds r1, r2
	adds r9, r12, r3
	ldr r12, =0x28d774de
	ldr r9, =0x0
	adds r12, r9
	adds r11, r13, r8
	ldr r6, =0x9893cad5
	ldr r5, =0xdcfa2339
	add r6, r5
	adds r4, r7, r5
	ldr r8, =0x21680112
	ldr r9, =0x80000000
	adds r8, r9
	add r3, r1, r10
	ldr r2, =0xf9cfaaaf
	ldr r6, =0x4c804dde
	add r2, r6
	adds r3, r11, r6
	ldr r4, =0x3d9fc5c8
	ldr r10, =0xbf054e66
	adds r4, r10
	adds r12, r4, r0
	ldr r1, =0x0
	ldr r8, =0xd2b43b07
	add r1, r8
	add r10, r10, r12
	ldr r3, =0xf4b86cca
	ldr r3, =0xf4b86ccb
	add r3, r3
	adds r4, r0, r8
	ldr r4, =0x7fffffff
	ldr r10, =0x69045fd9
	add r4, r10
	add r4, r10, r7
	ldr r12, =0xda9b5d59
	ldr r0, =0x80000000
	add r12, r0
	add r12, r2, r1
	ldr r7, =0x78e5071b
	ldr r6, =0x871af8e6
	add r7, r6
	adds r7, r0, r0
	ldr r9, =0xd2a27a64
	ldr r12, =0x7fffffff
	adds r9, r12
	add r2, r0, r4
	ldr r0, =0x70ffc982
	ldr r10, =0xffffffff
	adds r0, r10
	adds r6, r13, r3
	ldr r1, =0x6b36e6f1
	ldr r2, =0x76b00f6f
	adds r1, r2
	add r10, r7, r3
	ldr r9, =0xa2cdf7d9
	ldr r2, =0x5d320825
	add r9, r2
	adds r5, r7, r5
	ldr r4, =0x639212ef
	ldr r8, =0x639212ee
	add r4, r8
	add r5, r1, r11
	ldr r6, =0xbe49ba9b
	ldr r12, =0xda6c1461
	add r6, r12
	add r11, r7, r7
	ldr r11, =0xf6cc72c4
	ldr r9, =0xf075a1c0
	add r11, r9
	add r9, r12, r7
	ldr r10, =0x4e87f2e9
	ldr r2, =0xb1780d18
	add r10, r2
	adds r0, r14, r8
	ldr r8, =0x12c8c8b
	ldr r4, =0xfed37373
	add r8, r4
	add r12, r14, r12
	ldr r5, =0x80000000
	ldr r1, =0x7fffffff
	add r5, r1
	adds r1, r11, r7
	ldr r3, =0x2a79cc
	ldr r7, =0xffd58634
	adds r3, r7
	adds r7, r2, r4
	ldr r0, =0x7905d8d3
	ldr r9, =0x90ef6cea
	add r0, r9
	adds r8, r5, r0
	ldr r8, =0x7807198b
	ldr r11, =0xf6e71352
	adds r8, r11
	adds r1, r4, r9
	ldr r10, =0x79907f0f
	ldr r10, =0x80000000
	adds r10, r10
	add r12, r7, r7
	ldr r8, =0x647b6ba6
	ldr r10, =0x261283b
	add r8, r10
	adds r10, r12, r5
	ldr r2, =0x6c687bbc
	ldr r6, =0x93978444
	add r2, r6
	adds r12, r2, r2
	ldr r6, =0x80000000
	ldr r9, =0x0
	add r6, r9
	add r4, r8, r4
	ldr r11, =0x2996af28
	ldr r9, =0x20d9c887
	add r11, r9
	add r5, r14, r7
	
	b .tclbl_7
	// Generate literal table here so we don't run out of offset space
	.ltorg
	.tclbl_7:


	ldr r0, =0x78df16bd
	ldr r10, =0x8720e942
	adds r0, r10
	adds r10, r11, r8
	ldr r3, =0x88c7af82
	ldr r4, =0x88c7af80
	adds r3, r4
	add r7, r2, r1
	ldr r2, =0x1715e762
	ldr r8, =0x1715e761
	adds r2, r8
	adds r10, r14, r5
	ldr r6, =0x7fffffff
	ldr r1, =0xd8f89be0
	adds r6, r1
	add r6, r8, r7
	ldr r4, =0x443d856f
	ldr r1, =0x80000000
	add r4, r1
	adds r4, r2, r4
	ldr r6, =0xb5275507
	ldr r11, =0x80000000
	add r6, r11
	adds r9, r6, r9
	ldr r0, =0x51e3f2ad
	ldr r12, =0x5d63dae
	add r0, r12
	adds r2, r9, r9
	ldr r2, =0xa0ee064e
	ldr r11, =0x5f11f9b3
	add r2, r11
	adds r10, r9, r12
	ldr r12, =0x7c8cdf2a
	ldr r2, =0x51c24c5f
	adds r12, r2
	adds r6, r9, r2
	ldr r8, =0x1556bb85
	ldr r5, =0x217c638e
	adds r8, r5
	adds r10, r0, r0
	ldr r0, =0x7fd28f42
	ldr r11, =0x7fd28f40
	adds r0, r11
	adds r3, r12, r11
	ldr r10, =0x91f80830
	ldr r5, =0x0
	adds r10, r5
	adds r7, r10, r12
	ldr r3, =0xffffffff
	ldr r9, =0x7b82d6db
	add r3, r9
	adds r1, r3, r6
	ldr r1, =0xffffffff
	ldr r11, =0x2f286944
	add r1, r11
	add r9, r8, r10
	ldr r7, =0xadeef92c
	ldr r6, =0xadeef92b
	add r7, r6
	add r5, r0, r1
	ldr r8, =0xffffffff
	ldr r0, =0xc9244975
	add r8, r0
	adds r0, r7, r7
	ldr r8, =0x8750acec
	ldr r10, =0xd280ecd1
	adds r8, r10
	add r5, r8, r5
	ldr r8, =0x80000000
	ldr r9, =0xa95625fd
	add r8, r9
	adds r8, r4, r3
	ldr r0, =0x7fffffff
	ldr r5, =0x1f6af5d0
	adds r0, r5
	adds r11, r4, r8
	ldr r7, =0x14e908a0
	ldr r4, =0x80000000
	adds r7, r4
	add r12, r9, r0
	ldr r0, =0x7fffffff
	ldr r6, =0x37db3698
	adds r0, r6
	adds r12, r12, r3
	ldr r4, =0x68b0701d
	ldr r10, =0x974f8fe2
	adds r4, r10
	adds r2, r3, r4
	ldr r9, =0xbb1ee5da
	ldr r3, =0xa5f4174d
	adds r9, r3
	adds r0, r9, r12
	ldr r4, =0x30dc5155
	ldr r11, =0xffffffff
	adds r4, r11
	add r11, r10, r8
	ldr r8, =0x1037a61a
	ldr r6, =0xe8d76d19
	adds r8, r6
	adds r11, r9, r3
	ldr r0, =0x0
	ldr r7, =0x7807ccbb
	adds r0, r7
	adds r7, r7, r9
	ldr r7, =0x7fffffff
	ldr r5, =0x9f2e7515
	adds r7, r5
	adds r2, r9, r10
	ldr r8, =0x2be870ac
	ldr r4, =0x80000000
	add r8, r4
	adds r2, r13, r4
	ldr r3, =0x37ff1fbf
	ldr r8, =0xc800e03f
	adds r3, r8
	adds r5, r3, r7
	ldr r9, =0x5a4cfe14
	ldr r4, =0x5a4cfe12
	adds r9, r4
	add r4, r10, r5
	ldr r12, =0x5d8fb9fe
	ldr r4, =0x4d966b36
	adds r12, r4
	add r5, r14, r0
	ldr r5, =0x80000000
	ldr r2, =0xf74ec082
	adds r5, r2
	adds r10, r7, r6
	ldr r4, =0x0
	ldr r3, =0x39e2a192
	add r4, r3
	add r10, r7, r6
	ldr r0, =0xd2df3cb
	ldr r12, =0x4d28ec7b
	adds r0, r12
	adds r1, r4, r1
	ldr r8, =0x7ce7191b
	ldr r10, =0xa183d48d
	adds r8, r10
	adds r9, r7, r2
	ldr r6, =0xda116705
	ldr r2, =0x25ee98f9
	adds r6, r2
	adds r8, r6, r8
	ldr r6, =0xffffffff
	ldr r1, =0x61a7447e
	add r6, r1
	add r5, r8, r5
	ldr r6, =0x0
	ldr r9, =0xe1c1a0fd
	adds r6, r9
	add r10, r0, r9
	ldr r12, =0x21f159f1
	ldr r5, =0xb6ef4343
	adds r12, r5
	adds r10, r14, r10
	ldr r7, =0x80000000
	ldr r12, =0xbf9f4fdf
	adds r7, r12
	adds r4, r6, r5
	ldr r10, =0xc44e567d
	ldr r1, =0x7fffffff
	add r10, r1
	adds r4, r3, r6
	ldr r5, =0x45aa41c
	ldr r6, =0x26ad9918
	add r5, r6
	add r7, r2, r9
	ldr r12, =0xada8db1b
	ldr r11, =0xb9b9c665
	adds r12, r11
	add r12, r4, r10
	ldr r3, =0x94059b67
	ldr r1, =0xfe2b68e3
	add r3, r1
	adds r2, r6, r6
	ldr r4, =0xb0143b09
	ldr r7, =0x6b35091f
	add r4, r7
	add r4, r3, r1
	ldr r0, =0xb8fb8b17
	ldr r12, =0xfa9257c4
	adds r0, r12
	adds r12, r2, r10
	ldr r12, =0xa5efca44
	ldr r7, =0x80000000
	add r12, r7
	adds r0, r14, r5
	ldr r4, =0xdad86947
	ldr r4, =0xf3a1f046
	adds r4, r4
	add r6, r9, r0
	ldr r10, =0x7fffffff
	ldr r0, =0xf9139b78
	add r10, r0
	adds r5, r1, r2
	ldr r9, =0x8ad900c9
	ldr r8, =0x69aaa50c
	adds r9, r8
	add r12, r11, r9
	ldr r9, =0x7cfd2aa5
	ldr r8, =0xea869689
	adds r9, r8
	adds r3, r0, r12
	ldr r1, =0xdc37c0d9
	ldr r11, =0x0
	adds r1, r11
	adds r12, r12, r11
	ldr r2, =0xffffffff
	ldr r6, =0x3fa3d37c
	adds r2, r6
	adds r1, r10, r2
	ldr r8, =0xffffffff
	ldr r5, =0x739d3258
	add r8, r5
	adds r12, r1, r12
	ldr r4, =0x80000000
	ldr r4, =0xb84c2939
	add r4, r4
	add r6, r7, r10
	ldr r0, =0xf98d641b
	ldr r1, =0x80000000
	add r0, r1
	adds r5, r3, r8
	ldr r10, =0x7fffffff
	ldr r8, =0xccf45fec
	add r10, r8
	add r2, r11, r8
	ldr r6, =0x2cbc33c5
	ldr r2, =0x17c25613
	add r6, r2
	adds r7, r0, r8
	ldr r12, =0x48892fa8
	ldr r3, =0x7fffffff
	add r12, r3
	add r0, r1, r7
	ldr r6, =0xc2f4621
	ldr r5, =0xf3d0b9e0
	adds r6, r5
	add r12, r13, r7
	ldr r11, =0xffffffff
	ldr r10, =0xe3727ed3
	adds r11, r10
	adds r7, r13, r9
	ldr r2, =0x8dba5b51
	ldr r1, =0xb33dd92a
	add r2, r1
	add r3, r9, r6
	ldr r4, =0x89b64f10
	ldr r7, =0x7649b0ef
	adds r4, r7
	add r12, r5, r6
	ldr r8, =0xdcf0939f
	ldr r2, =0x73cd27ec
	add r8, r2
	adds r0, r13, r8
	ldr r4, =0x50eea87c
	ldr r2, =0xd8775007
	adds r4, r2
	add r0, r13, r7
	ldr r2, =0x2518ad6c
	ldr r9, =0x2518ad6e
	adds r2, r9
	adds r1, r6, r6
	ldr r1, =0x825f2771
	ldr r4, =0x7da0d88e
	adds r1, r4
	adds r11, r10, r1
	ldr r10, =0xffffffff
	ldr r5, =0x7fffffff
	add r10, r5
	adds r5, r12, r1
	ldr r12, =0x0
	ldr r2, =0x80000000
	add r12, r2
	adds r1, r5, r9
	ldr r3, =0xf09931bf
	ldr r8, =0x0
	add r3, r8
	adds r11, r1, r4
	ldr r2, =0xe07fc5ab
	ldr r9, =0xe07fc5aa
	adds r2, r9
	adds r5, r3, r7
	ldr r3, =0x6ad50c66
	ldr r4, =0x6ad50c65
	adds r3, r4
	add r8, r10, r8
	ldr r5, =0x80000000
	ldr r1, =0xc96f8ec5
	adds r5, r1
	adds r10, r3, r9
	ldr r1, =0x8b151dc6
	ldr r0, =0x74eae23a
	add r1, r0
	add r6, r14, r3
	ldr r10, =0x57895349
	ldr r7, =0xa876acb6
	adds r10, r7
	adds r6, r10, r11
	ldr r3, =0x69199b30
	ldr r3, =0x96e664d1
	adds r3, r3
	add r9, r13, r5
	ldr r12, =0x1b424ef5
	ldr r5, =0x3cf7d58f
	add r12, r5
	adds r1, r13, r3
	ldr r12, =0x7fffffff
	ldr r1, =0x0
	add r12, r1
	add r3, r1, r1
	ldr r4, =0x0
	ldr r11, =0xffffffff
	adds r4, r11
	adds r8, r11, r11
	ldr r9, =0x201d3683
	ldr r0, =0xb1ff0cf4
	add r9, r0
	add r10, r4, r4
	ldr r4, =0xe65ad440
	ldr r6, =0x2deb408b
	add r4, r6
	add r9, r10, r2
	ldr r8, =0x178e1c2a
	ldr r3, =0x3280513d
	adds r8, r3
	adds r10, r2, r2
	ldr r10, =0xeaa96a79
	ldr r1, =0x25636882
	adds r10, r1
	add r8, r12, r2
	ldr r10, =0xbba63dc9
	ldr r1, =0x68c87cf1
	add r10, r1
	add r12, r2, r7
	ldr r7, =0x6290adcf
	ldr r11, =0x9d6f5231
	adds r7, r11
	adds r5, r7, r2
	ldr r10, =0xdb1c9819
	ldr r6, =0xdb1c981a
	add r10, r6
	add r2, r14, r10
	ldr r1, =0x6a31ca7a
	ldr r5, =0x95ce3585
	add r1, r5
	adds r1, r0, r7
	ldr r9, =0xecb9b03a
	ldr r10, =0x13464fc6
	add r9, r10
	add r0, r4, r12
	ldr r4, =0xe6d9f165
	ldr r0, =0x19260e9c
	add r4, r0
	add r3, r11, r3
	ldr r1, =0xa07413ef
	ldr r5, =0x2b9a3aa5
	add r1, r5
	adds r3, r5, r12
	ldr r6, =0x5da924f4
	ldr r11, =0x5da924f5
	add r6, r11
	adds r5, r5, r7
	ldr r0, =0x80000000
	ldr r1, =0xffffffff
	add r0, r1
	adds r10, r6, r9
	ldr r7, =0x32b30a64
	ldr r12, =0x5cfeaecd
	add r7, r12
	adds r5, r10, r2
	ldr r4, =0x94187a8
	ldr r6, =0xffffffff
	adds r4, r6
	add r11, r1, r11
	ldr r6, =0x0
	ldr r5, =0x38a550f0
	add r6, r5
	add r5, r11, r1
	ldr r4, =0x7cd7ebd9
	ldr r4, =0x0
	adds r4, r4
	add r6, r0, r7
	ldr r1, =0x83b7bfb8
	ldr r2, =0x380492c1
	adds r1, r2
	add r0, r11, r0
	ldr r4, =0x2550022c
	ldr r5, =0x7fffffff
	add r4, r5
	add r6, r0, r7
	ldr r3, =0xb3d81d6e
	ldr r11, =0x9ab550f2
	adds r3, r11
	add r12, r0, r7
	ldr r6, =0x8c8b1407
	ldr r6, =0x8c8b1408
	adds r6, r6
	add r8, r1, r7
	
	b .tclbl_8
	// Generate literal table here so we don't run out of offset space
	.ltorg
	.tclbl_8:


	ldr r10, =0x7fffffff
	ldr r10, =0xf22eef4b
	adds r10, r10
	add r8, r3, r10
	ldr r5, =0x6e70d618
	ldr r0, =0x6e70d616
	add r5, r0
	adds r9, r13, r1
	ldr r3, =0x80000000
	ldr r2, =0x7fffffff
	add r3, r2
	adds r8, r3, r0
	ldr r10, =0xaea9513b
	ldr r1, =0x1f54d496
	adds r10, r1
	add r2, r5, r4
	ldr r7, =0x3543bb8a
	ldr r8, =0x4a9d714b
	add r7, r8
	add r6, r7, r6
	ldr r5, =0xe750993c
	ldr r0, =0x18af66c3
	add r5, r0
	add r5, r4, r4
	ldr r1, =0x7fffffff
	ldr r5, =0x0
	add r1, r5
	adds r4, r6, r9
	ldr r8, =0x80000000
	ldr r7, =0x2bcaf5cc
	add r8, r7
	adds r3, r6, r1
	ldr r10, =0x5f85c9f4
	ldr r2, =0xb174492d
	adds r10, r2
	adds r1, r5, r11
	ldr r7, =0x5ca418f1
	ldr r7, =0x83a5cbc1
	adds r7, r7
	adds r5, r9, r10
	ldr r5, =0xffffffff
	ldr r10, =0xb988c209
	adds r5, r10
	adds r0, r14, r12
	ldr r5, =0x80000000
	ldr r4, =0x0
	adds r5, r4
	adds r6, r11, r7
	ldr r0, =0x7fffffff
	ldr r6, =0xf69ea7f0
	add r0, r6
	add r9, r7, r8
	ldr r4, =0x80000000
	ldr r1, =0x227c3366
	add r4, r1
	adds r4, r8, r10
	ldr r12, =0x0
	ldr r2, =0x372fcbbe
	add r12, r2
	adds r10, r7, r4
	ldr r8, =0x96e52118
	ldr r7, =0x277aed51
	add r8, r7
	add r11, r4, r4
	ldr r8, =0xa1d8acf4
	ldr r7, =0xa2a0dc59
	add r8, r7
	add r10, r12, r10
	ldr r8, =0xdd86522a
	ldr r4, =0x44fa80db
	adds r8, r4
	adds r0, r4, r8
	ldr r4, =0x59627c1d
	ldr r9, =0xa69d83e4
	add r4, r9
	adds r11, r12, r11
	ldr r11, =0x7fffffff
	ldr r10, =0xd8f3c8b5
	add r11, r10
	adds r4, r2, r3
	ldr r3, =0x1cb1d247
	ldr r6, =0xdfc77b0f
	adds r3, r6
	add r6, r7, r3
	ldr r10, =0xc3c81fe0
	ldr r11, =0x7fffffff
	add r10, r11
	add r10, r8, r6
	ldr r0, =0x1baf6a84
	ldr r2, =0xe450957b
	adds r0, r2
	add r12, r10, r2
	ldr r12, =0xa7e52be2
	ldr r11, =0x80000000
	add r12, r11
	adds r9, r10, r12
	ldr r2, =0x438533b7
	ldr r10, =0xbc7acc48
	adds r2, r10
	adds r5, r12, r0
	ldr r0, =0x697db8f5
	ldr r9, =0x9682470b
	add r0, r9
	add r0, r13, r11
	ldr r3, =0x23b5ffe9
	ldr r12, =0x23b5ffea
	add r3, r12
	adds r8, r14, r9
	ldr r9, =0x7fffffff
	ldr r1, =0x80000000
	add r9, r1
	adds r0, r13, r10
	ldr r11, =0xb5a26b3
	ldr r10, =0x0
	add r11, r10
	add r12, r5, r8
	ldr r5, =0xd0fb3b5e
	ldr r9, =0x2f04c4a2
	adds r5, r9
	add r11, r0, r8
	ldr r6, =0x21d7dc29
	ldr r4, =0x33d473ec
	add r6, r4
	add r1, r11, r10
	ldr r1, =0x42711c5f
	ldr r11, =0xcca29d84
	add r1, r11
	adds r4, r4, r11
	ldr r12, =0x7fffffff
	ldr r1, =0xcf871d45
	adds r12, r1
	add r5, r4, r4
	ldr r4, =0xcc3d425
	ldr r9, =0x7fffffff
	adds r4, r9
	adds r1, r1, r9
	ldr r6, =0x1bf85224
	ldr r7, =0xe407addb
	add r6, r7
	adds r6, r4, r11
	ldr r5, =0x7db47d57
	ldr r4, =0x9c3ee254
	add r5, r4
	add r7, r5, r3
	ldr r2, =0x8b02474d
	ldr r2, =0x31439db
	add r2, r2
	add r1, r1, r7
	ldr r5, =0x80000000
	ldr r6, =0x67a38cd
	add r5, r6
	adds r8, r4, r9
	ldr r4, =0xc6a6f195
	ldr r2, =0x80000000
	add r4, r2
	adds r9, r0, r0
	ldr r11, =0xe555f863
	ldr r9, =0x0
	adds r11, r9
	adds r2, r13, r12
	ldr r1, =0x7fffffff
	ldr r5, =0xe61c509
	adds r1, r5
	adds r3, r14, r5
	ldr r7, =0x96e038a4
	ldr r11, =0x96e038a6
	add r7, r11
	adds r12, r7, r10
	ldr r5, =0x80000000
	ldr r4, =0x4cf5e46c
	adds r5, r4
	add r9, r9, r9
	ldr r11, =0x558f0de8
	ldr r1, =0x0
	adds r11, r1
	adds r0, r9, r10
	ldr r3, =0x30e133b9
	ldr r3, =0xbb9a2b3d
	adds r3, r3
	add r5, r13, r12
	ldr r10, =0x1736b57e
	ldr r4, =0x2ed3b2c0
	adds r10, r4
	adds r3, r3, r4
	ldr r2, =0x51855561
	ldr r12, =0x51855563
	add r2, r12
	add r7, r1, r10
	ldr r6, =0x77065de0
	ldr r4, =0x80000000
	add r6, r4
	adds r0, r8, r11
	ldr r3, =0x5ae2edc8
	ldr r8, =0x5ae2edc7
	add r3, r8
	adds r6, r6, r4
	ldr r2, =0x80000000
	ldr r5, =0x448e5a60
	add r2, r5
	adds r12, r12, r12
	ldr r8, =0x58461ad1
	ldr r5, =0xede2688a
	add r8, r5
	add r2, r9, r8
	ldr r1, =0xf812744e
	ldr r10, =0x80000000
	add r1, r10
	add r12, r6, r1
	ldr r3, =0x7fffffff
	ldr r0, =0x245c2584
	add r3, r0
	add r8, r7, r10
	ldr r0, =0x3b699037
	ldr r7, =0xc4966fc9
	adds r0, r7
	adds r5, r11, r11
	ldr r10, =0xb60da4f3
	ldr r5, =0x28d9a8bd
	adds r10, r5
	adds r1, r1, r0
	ldr r5, =0x2537452f
	ldr r9, =0x77ca51f3
	add r5, r9
	adds r2, r1, r2
	ldr r4, =0xed1a59f0
	ldr r2, =0xed1a59f2
	adds r4, r2
	add r12, r13, r8
	ldr r12, =0x88945fe4
	ldr r5, =0x80000000
	adds r12, r5
	adds r1, r11, r10
	ldr r4, =0x7fffffff
	ldr r0, =0xc1d21139
	adds r4, r0
	adds r4, r8, r5
	ldr r4, =0x5dc0ee6
	ldr r1, =0x7fffffff
	add r4, r1
	add r11, r13, r7
	ldr r1, =0x1083395d
	ldr r9, =0x87948b66
	add r1, r9
	add r0, r8, r4
	ldr r7, =0xfab139d0
	ldr r10, =0xfab139d1
	adds r7, r10
	adds r2, r9, r1
	ldr r0, =0xeebbe70
	ldr r12, =0xeebbe71
	add r0, r12
	add r1, r7, r4
	ldr r0, =0xd98f085
	ldr r3, =0xffffffff
	adds r0, r3
	add r5, r12, r5
	ldr r8, =0x701c1ca3
	ldr r7, =0x0
	adds r8, r7
	adds r4, r0, r1
	ldr r1, =0x64fa1561
	ldr r2, =0x80000000
	add r1, r2
	add r10, r0, r9
	ldr r7, =0x34279b0f
	ldr r10, =0x4cfa65d7
	add r7, r10
	adds r7, r8, r5
	ldr r9, =0x7fffffff
	ldr r5, =0xed75067f
	adds r9, r5
	add r4, r3, r9
	ldr r2, =0x98e3c799
	ldr r1, =0x184a18f
	adds r2, r1
	add r6, r6, r3
	ldr r4, =0x7fffffff
	ldr r11, =0x84decbb5
	add r4, r11
	add r5, r13, r7
	ldr r0, =0x14b83416
	ldr r7, =0x14b83417
	adds r0, r7
	add r0, r12, r11
	ldr r3, =0xd1acef60
	ldr r1, =0x0
	adds r3, r1
	add r4, r4, r5
	ldr r3, =0x32eac1df
	ldr r3, =0x32eac1de
	adds r3, r3
	adds r10, r9, r12
	ldr r5, =0xffffffff
	ldr r12, =0xdda9a963
	adds r5, r12
	add r12, r4, r10
	ldr r12, =0xfc99f7c6
	ldr r0, =0xa084ebd9
	add r12, r0
	add r9, r12, r0
	ldr r2, =0x9f878d7c
	ldr r10, =0x89e360f3
	adds r2, r10
	add r7, r2, r9
	ldr r8, =0x1e5e99f7
	ldr r9, =0xffffffff
	add r8, r9
	adds r9, r2, r9
	ldr r11, =0x7fffffff
	ldr r12, =0x7768873e
	add r11, r12
	add r10, r5, r11
	ldr r3, =0x80000000
	ldr r5, =0x5010ebef
	adds r3, r5
	adds r4, r10, r1
	ldr r4, =0x7fffffff
	ldr r11, =0xe4bfd667
	adds r4, r11
	add r11, r6, r12
	ldr r5, =0xbcdbf7a5
	ldr r6, =0x7fffffff
	add r5, r6
	adds r4, r12, r7
	ldr r5, =0x821cfc02
	ldr r3, =0xb40a881a
	add r5, r3
	add r11, r1, r10
	ldr r10, =0x7fffffff
	ldr r3, =0xb92204ac
	add r10, r3
	adds r1, r13, r11
	ldr r4, =0xe2fbd142
	ldr r1, =0x94e1d58
	add r4, r1
	add r3, r3, r4
	ldr r12, =0x85b8d22f
	ldr r10, =0x7a472dd1
	adds r12, r10
	add r10, r13, r2
	ldr r3, =0xa1bc6949
	ldr r12, =0x2899f688
	adds r3, r12
	adds r0, r5, r9
	ldr r8, =0x80000000
	ldr r8, =0x29be0211
	add r8, r8
	add r12, r5, r8
	ldr r5, =0x80000000
	ldr r3, =0xf7b4e446
	adds r5, r3
	adds r7, r2, r3
	ldr r5, =0x1cee7b9e
	ldr r7, =0xfcd1253b
	add r5, r7
	adds r4, r14, r9
	ldr r11, =0x7fffffff
	ldr r12, =0x76e872a4
	add r11, r12
	adds r2, r1, r1
	ldr r3, =0x3c7985cf
	ldr r6, =0x3c7985d0
	add r3, r6
	add r3, r5, r2
	ldr r1, =0x13576e93
	ldr r0, =0x29e9da67
	add r1, r0
	adds r2, r0, r12
	ldr r11, =0xffffffff
	ldr r11, =0x61c15d3d
	adds r11, r11
	adds r4, r11, r0
	ldr r9, =0x80000000
	ldr r12, =0xc0d2cca3
	add r9, r12
	adds r5, r12, r9
	ldr r0, =0xde89032e
	ldr r6, =0x2176fcd3
	add r0, r6
	add r2, r9, r5
	ldr r12, =0x1d011e59
	ldr r11, =0xcd0c6e4
	adds r12, r11
	adds r4, r13, r11
	ldr r0, =0xffffffff
	ldr r1, =0x450e9c0a
	adds r0, r1
	add r2, r7, r9
	ldr r12, =0x18a52b1b
	ldr r12, =0x80000000
	adds r12, r12
	add r12, r4, r10
	ldr r9, =0xaf43e7af
	ldr r8, =0xffffffff
	add r9, r8
	add r8, r4, r8
	ldr r11, =0xffffffff
	ldr r12, =0x53380ec4
	adds r11, r12
	adds r3, r14, r4
	
	b .tclbl_9
	// Generate literal table here so we don't run out of offset space
	.ltorg
	.tclbl_9:


	ldr r6, =0x0
	ldr r6, =0x2d5401c9
	add r6, r6
	add r8, r1, r8
	ldr r3, =0x0
	ldr r2, =0xffbedb7e
	adds r3, r2
	adds r4, r1, r7
	ldr r3, =0xfa6bd119
	ldr r8, =0x11b7ff74
	adds r3, r8
	adds r7, r5, r0
	ldr r5, =0x8ff0d4d3
	ldr r10, =0x9984feca
	adds r5, r10
	adds r10, r12, r0
	ldr r12, =0x6501a9a6
	ldr r10, =0x9afe565a
	add r12, r10
	add r7, r6, r4
	ldr r10, =0xffffffff
	ldr r1, =0x59cdf810
	add r10, r1
	adds r7, r10, r8
	ldr r8, =0xd2ea9901
	ldr r9, =0x80000000
	add r8, r9
	add r8, r1, r4
	ldr r10, =0x7a2433e
	ldr r12, =0x7fffffff
	adds r10, r12
	adds r11, r11, r3
	ldr r12, =0x495b8a69
	ldr r12, =0x7fffffff
	add r12, r12
	adds r10, r8, r7
	ldr r3, =0x7fffffff
	ldr r12, =0x93fc3cfe
	add r3, r12
	adds r9, r3, r6
	ldr r11, =0x31c1691b
	ldr r8, =0x7fffffff
	add r11, r8
	adds r5, r0, r4
	ldr r12, =0x8a4c1583
	ldr r3, =0x7fffffff
	add r12, r3
	add r9, r4, r4
	ldr r4, =0xe6035b85
	ldr r10, =0x7fffffff
	add r4, r10
	adds r5, r14, r2
	ldr r5, =0x7fffffff
	ldr r8, =0x0
	add r5, r8
	add r12, r4, r6
	ldr r0, =0xf315ed5e
	ldr r8, =0xc619fff1
	adds r0, r8
	add r5, r4, r3
	ldr r12, =0x7f239e51
	ldr r4, =0xc0f07e25
	adds r12, r4
	adds r12, r10, r3
	ldr r9, =0x474e6147
	ldr r0, =0xb8b19eb8
	adds r9, r0
	adds r6, r1, r12
	ldr r7, =0xa3bf7963
	ldr r2, =0x54f188f5
	add r7, r2
	add r5, r0, r8
	ldr r5, =0x0
	ldr r5, =0x7fffffff
	add r5, r5
	adds r7, r1, r6
	ldr r8, =0x0
	ldr r8, =0xba0606e5
	adds r8, r8
	adds r7, r5, r5
	ldr r7, =0x9e1881d5
	ldr r5, =0x82ce7411
	adds r7, r5
	add r0, r9, r12
	ldr r1, =0x0
	ldr r10, =0x75feaea
	add r1, r10
	add r3, r3, r10
	ldr r1, =0x3a44adce
	ldr r0, =0x6504299c
	add r1, r0
	adds r8, r6, r1
	ldr r11, =0x7c49fcf9
	ldr r9, =0x2552f174
	add r11, r9
	adds r3, r11, r0
	ldr r6, =0x4b16b149
	ldr r0, =0x7fffffff
	add r6, r0
	add r0, r7, r8
	ldr r1, =0x0
	ldr r4, =0x2040493c
	add r1, r4
	add r2, r0, r6
	ldr r7, =0x20ad0693
	ldr r7, =0xffffffff
	add r7, r7
	add r8, r6, r2
	ldr r5, =0xdfb36a2a
	ldr r9, =0xdfb36a29
	adds r5, r9
	adds r8, r6, r4
	ldr r4, =0x0
	ldr r6, =0x60688579
	add r4, r6
	adds r12, r0, r12
	ldr r0, =0xad093b21
	ldr r2, =0xd50382e2
	add r0, r2
	add r9, r14, r7
	ldr r11, =0x36fb0c62
	ldr r0, =0x51ebdf0b
	add r11, r0
	adds r11, r7, r6
	ldr r4, =0xffffffff
	ldr r0, =0xc376e091
	add r4, r0
	add r8, r9, r3
	ldr r6, =0xa2b6af64
	ldr r9, =0xa2b6af65
	add r6, r9
	adds r10, r9, r1
	ldr r10, =0x0
	ldr r9, =0x7fffffff
	adds r10, r9
	add r6, r2, r4
	ldr r11, =0x0
	ldr r0, =0x73353af8
	add r11, r0
	add r0, r4, r2
	ldr r1, =0x7fffffff
	ldr r2, =0x9f9193db
	adds r1, r2
	adds r6, r13, r9
	ldr r4, =0xceacc731
	ldr r1, =0x315338d0
	adds r4, r1
	add r8, r0, r9
	ldr r1, =0x80000000
	ldr r10, =0x3855ad72
	adds r1, r10
	adds r9, r13, r10
	ldr r2, =0x7fffffff
	ldr r9, =0xba9d1cec
	add r2, r9
	adds r8, r7, r8
	ldr r11, =0x2b204a44
	ldr r5, =0x0
	add r11, r5
	adds r1, r2, r4
	ldr r0, =0xffffffff
	ldr r11, =0x7fffffff
	add r0, r11
	add r11, r11, r5
	ldr r9, =0x80000000
	ldr r12, =0x7fffffff
	add r9, r12
	adds r7, r11, r1
	ldr r12, =0x8384df26
	ldr r0, =0x7c7b20d9
	adds r12, r0
	add r5, r0, r3
	ldr r12, =0x395d2ffc
	ldr r7, =0x7fffffff
	add r12, r7
	add r5, r12, r6
	ldr r3, =0x28527421
	ldr r2, =0x771d9eb6
	adds r3, r2
	adds r10, r2, r6
	ldr r12, =0xffffffff
	ldr r9, =0xcae9bd53
	add r12, r9
	adds r2, r13, r11
	ldr r0, =0x0
	ldr r7, =0x47dad2fe
	adds r0, r7
	add r3, r12, r9
	ldr r9, =0xc4976ce2
	ldr r10, =0x0
	adds r9, r10
	adds r11, r11, r4
	ldr r0, =0xe6db5a
	ldr r10, =0xe6db59
	adds r0, r10
	add r0, r3, r4
	ldr r6, =0xf12c2c8c
	ldr r10, =0xf12c2c8b
	add r6, r10
	add r11, r0, r12
	ldr r3, =0xb89f5add
	ldr r3, =0xb89f5adc
	add r3, r3
	adds r4, r7, r4
	ldr r3, =0x5d3770ad
	ldr r7, =0xbb3df5dd
	adds r3, r7
	adds r1, r8, r11
	ldr r9, =0x20b48ea
	ldr r6, =0xfb3e7d7
	add r9, r6
	adds r12, r0, r2
	ldr r8, =0x46b03e77
	ldr r3, =0x74ef1f81
	add r8, r3
	adds r4, r4, r4
	ldr r8, =0xa07b276a
	ldr r12, =0x6ae754c9
	adds r8, r12
	adds r9, r12, r6
	ldr r0, =0x73daa6a7
	ldr r0, =0x80000000
	add r0, r0
	adds r7, r6, r12
	ldr r11, =0x14fc78a1
	ldr r0, =0x0
	add r11, r0
	adds r10, r11, r7
	ldr r7, =0xffffffff
	ldr r6, =0xc37a259f
	adds r7, r6
	add r2, r3, r0
	ldr r6, =0xffffffff
	ldr r3, =0x24342ecf
	add r6, r3
	adds r9, r5, r5
	ldr r6, =0x80000000
	ldr r10, =0x29b4b262
	add r6, r10
	adds r9, r9, r2
	ldr r12, =0xc33dc28c
	ldr r10, =0xc33dc28b
	add r12, r10
	add r8, r3, r3
	ldr r10, =0x0
	ldr r1, =0xe7292023
	adds r10, r1
	add r1, r14, r10
	ldr r10, =0x6bb17701
	ldr r9, =0x7fffffff
	add r10, r9
	adds r5, r13, r6
	ldr r3, =0x80000000
	ldr r9, =0x9ca6997e
	add r3, r9
	adds r5, r12, r11
	ldr r11, =0xd13ec244
	ldr r7, =0xd2b3fbbc
	adds r11, r7
	adds r7, r3, r11
	ldr r6, =0x26ff9ddd
	ldr r0, =0x80000000
	adds r6, r0
	add r2, r10, r1
	ldr r11, =0x18997927
	ldr r11, =0x18997928
	adds r11, r11
	add r2, r10, r11
	ldr r9, =0xf3db3731
	ldr r7, =0x80000000
	add r9, r7
	adds r12, r12, r2
	ldr r8, =0xb358d493
	ldr r7, =0x4ca72b6d
	add r8, r7
	adds r6, r8, r3
	ldr r10, =0x7bf77017
	ldr r0, =0xc8fec22c
	adds r10, r0
	adds r1, r3, r7
	ldr r0, =0xef75a15b
	ldr r1, =0xef75a15c
	add r0, r1
	add r12, r12, r12
	ldr r3, =0x874438d6
	ldr r5, =0x4c591565
	adds r3, r5
	adds r4, r10, r6
	ldr r0, =0xc4b32e71
	ldr r2, =0xf99c8178
	adds r0, r2
	add r5, r7, r5
	ldr r12, =0x7fffffff
	ldr r4, =0xe32f8256
	add r12, r4
	adds r3, r9, r6
	ldr r9, =0xffffffff
	ldr r3, =0xdb929446
	adds r9, r3
	add r11, r12, r7
	ldr r7, =0xf7d96547
	ldr r8, =0xf7d96548
	add r7, r8
	adds r0, r3, r12
	ldr r5, =0x9431c67c
	ldr r6, =0x33a705ce
	add r5, r6
	adds r9, r12, r12
	ldr r0, =0x0
	ldr r2, =0xab93cee4
	adds r0, r2
	add r0, r7, r6
	ldr r9, =0xf5fe9b6
	ldr r1, =0xf05ac40a
	adds r9, r1
	adds r6, r14, r10
	ldr r1, =0xe22f744
	ldr r12, =0x1fa7d6b5
	add r1, r12
	adds r9, r1, r1
	ldr r9, =0x0
	ldr r0, =0x80000000
	add r9, r0
	adds r11, r4, r7
	ldr r4, =0x0
	ldr r3, =0x80000000
	add r4, r3
	add r3, r4, r0
	ldr r10, =0x80000000
	ldr r10, =0x6d4dca87
	add r10, r10
	adds r3, r7, r8
	ldr r12, =0x0
	ldr r9, =0x3e4a642d
	adds r12, r9
	add r12, r14, r7
	ldr r7, =0xd6cb76ff
	ldr r12, =0x3612a2c0
	adds r7, r12
	adds r7, r5, r2
	ldr r7, =0x7fffffff
	ldr r6, =0xeeee9a28
	adds r7, r6
	adds r6, r6, r2
	ldr r6, =0x412b6137
	ldr r1, =0x7902f097
	adds r6, r1
	add r8, r10, r5
	ldr r10, =0x2602777d
	ldr r3, =0x80000000
	add r10, r3
	adds r4, r14, r5
	ldr r1, =0x7fffffff
	ldr r5, =0x72996c9c
	adds r1, r5
	adds r9, r13, r7
	ldr r2, =0x26f511f7
	ldr r6, =0x52ef6832
	add r2, r6
	adds r1, r7, r10
	ldr r6, =0x7fffffff
	ldr r8, =0xffffffff
	adds r6, r8
	add r9, r3, r7
	ldr r3, =0x9bf96cfc
	ldr r10, =0x0
	adds r3, r10
	adds r11, r2, r1
	ldr r10, =0x2f722a25
	ldr r3, =0x2f722a26
	add r10, r3
	add r3, r13, r7
	ldr r2, =0x7d6deb07
	ldr r6, =0xe1e3d6cd
	adds r2, r6
	adds r11, r6, r10
	ldr r0, =0x0
	ldr r0, =0x7fd151c8
	adds r0, r0
	adds r1, r2, r12
	ldr r10, =0xf16b25d9
	ldr r4, =0xbeb378e0
	adds r10, r4
	add r8, r1, r3
	ldr r2, =0xe66bcdd1
	ldr r6, =0x69b20385
	add r2, r6
	add r9, r14, r12
	ldr r7, =0x59ee2ca
	ldr r0, =0x7fffffff
	add r7, r0
	add r8, r13, r0
	ldr r2, =0xffffffff
	ldr r8, =0x80000000
	adds r2, r8
	add r6, r2, r12
	ldr r5, =0x4b7b2ab2
	ldr r1, =0xb484d54f
	add r5, r1
	adds r11, r4, r10
	
	b .tclbl_10
	// Generate literal table here so we don't run out of offset space
	.ltorg
	.tclbl_10:


	ldr r1, =0x122b8ddc
	ldr r4, =0x122b8ddb
	adds r1, r4
	add r11, r12, r6
	ldr r10, =0x770bbadb
	ldr r0, =0x770bbadd
	add r10, r0
	add r1, r1, r3
	ldr r2, =0x1e28cf15
	ldr r6, =0x1e28cf16
	add r2, r6
	adds r9, r1, r3
	ldr r5, =0xcce2be9a
	ldr r5, =0x12daa3a8
	adds r5, r5
	adds r3, r6, r2
	ldr r5, =0x80000000
	ldr r4, =0x8c688de6
	adds r5, r4
	adds r4, r5, r3
	ldr r1, =0x26aac163
	ldr r11, =0x26aac164
	add r1, r11
	adds r1, r1, r11
	ldr r1, =0x7fffffff
	ldr r6, =0x6e78a70b
	adds r1, r6
	adds r3, r14, r0
	ldr r10, =0x0
	ldr r1, =0x54a0b1f9
	add r10, r1
	add r10, r12, r0
	ldr r5, =0x53929a95
	ldr r11, =0x7fffffff
	adds r5, r11
	add r10, r12, r5
	ldr r4, =0x34c7d666
	ldr r1, =0xd55b79bb
	adds r4, r1
	adds r12, r0, r12
	ldr r5, =0x7fffffff
	ldr r6, =0xd9774263
	adds r5, r6
	adds r4, r9, r11
	ldr r1, =0x3cec8775
	ldr r2, =0x7e8e5e95
	add r1, r2
	adds r8, r14, r7
	ldr r1, =0xeaf7857
	ldr r4, =0x80000000
	add r1, r4
	add r6, r4, r5
	ldr r9, =0x0
	ldr r6, =0xffffffff
	adds r9, r6
	adds r12, r8, r10
	ldr r6, =0xea6f91be
	ldr r8, =0xffffffff
	adds r6, r8
	add r7, r4, r1
	ldr r5, =0x70c97e07
	ldr r10, =0x77687e96
	add r5, r10
	add r3, r7, r7
	ldr r12, =0x0
	ldr r4, =0xf908e55f
	adds r12, r4
	add r7, r14, r9
	ldr r10, =0x78504d35
	ldr r12, =0x87afb2ca
	add r10, r12
	add r7, r10, r3
	ldr r6, =0xffffffff
	ldr r0, =0xef3858c1
	adds r6, r0
	add r8, r7, r6
	ldr r5, =0x987b3611
	ldr r4, =0x7497471d
	add r5, r4
	add r5, r14, r7
	ldr r2, =0x4f45500e
	ldr r7, =0xfccc2998
	add r2, r7
	adds r12, r4, r5
	ldr r12, =0x0
	ldr r1, =0x10ac6375
	add r12, r1
	adds r3, r5, r11
	ldr r1, =0x1f35858f
	ldr r8, =0x80000000
	adds r1, r8
	add r4, r11, r5
	ldr r0, =0x67025962
	ldr r8, =0xcf9e8e32
	add r0, r8
	add r11, r4, r2
	ldr r5, =0x9f364294
	ldr r12, =0x9ae2ec57
	adds r5, r12
	add r8, r9, r7
	ldr r2, =0x5dc7fd19
	ldr r3, =0x80000000
	adds r2, r3
	add r12, r6, r10
	ldr r0, =0x6a186ed9
	ldr r12, =0x0
	add r0, r12
	add r9, r3, r9
	ldr r11, =0xe0b5a2b9
	ldr r12, =0x1f4a5d45
	add r11, r12
	adds r2, r10, r5
	ldr r7, =0x4e04d583
	ldr r7, =0x7fffffff
	add r7, r7
	adds r4, r8, r2
	ldr r8, =0x80622d74
	ldr r2, =0x80622d76
	add r8, r2
	add r11, r13, r3
	ldr r2, =0x27b4ca53
	ldr r10, =0xffffffff
	add r2, r10
	adds r8, r8, r10
	ldr r2, =0x44c4cd4f
	ldr r10, =0xbb3b32b1
	adds r2, r10
	add r11, r10, r10
	ldr r11, =0x57f5f4e9
	ldr r11, =0x7fffffff
	add r11, r11
	add r0, r12, r0
	ldr r6, =0xff9055c0
	ldr r7, =0xb28c105c
	add r6, r7
	add r8, r14, r11
	ldr r3, =0x4600178a
	ldr r4, =0x4600178b
	add r3, r4
	adds r4, r1, r0
	ldr r2, =0x4f2539e2
	ldr r8, =0x80000000
	adds r2, r8
	add r1, r3, r3
	ldr r7, =0xffffffff
	ldr r2, =0xe8a940a1
	adds r7, r2
	add r1, r0, r10
	ldr r11, =0x68941ae5
	ldr r11, =0xffffffff
	add r11, r11
	adds r1, r5, r0
	ldr r3, =0xde0e1061
	ldr r3, =0x21f1efa0
	add r3, r3
	adds r6, r13, r12
	ldr r8, =0x0
	ldr r2, =0x4de34917
	adds r8, r2
	adds r5, r7, r3
	ldr r5, =0x129fed56
	ldr r9, =0xed6012aa
	add r5, r9
	add r8, r3, r0
	ldr r10, =0x7fffffff
	ldr r9, =0x5d3383d4
	adds r10, r9
	add r8, r9, r10
	ldr r7, =0x8cfff9f5
	ldr r10, =0x7300060c
	adds r7, r10
	add r9, r2, r8
	ldr r10, =0x0
	ldr r0, =0x8db9bf46
	add r10, r0
	adds r8, r4, r6
	ldr r12, =0xecca91f
	ldr r11, =0x1647ff97
	adds r12, r11
	adds r12, r3, r4
	ldr r8, =0x80000000
	ldr r12, =0x7fffffff
	adds r8, r12
	add r4, r5, r7
	ldr r10, =0x80000000
	ldr r4, =0x783e5c7e
	adds r10, r4
	add r8, r8, r4
	ldr r3, =0x82fc7d98
	ldr r9, =0x80000000
	adds r3, r9
	adds r5, r5, r9
	ldr r0, =0xbd2813c0
	ldr r5, =0xc30a7647
	adds r0, r5
	adds r0, r5, r5
	ldr r8, =0x94f01cbb
	ldr r0, =0x80000000
	add r8, r0
	add r5, r10, r1
	ldr r0, =0x7fffffff
	ldr r5, =0xdd6da6c3
	add r0, r5
	adds r3, r14, r12
	ldr r6, =0xc653522d
	ldr r11, =0xc653522e
	adds r6, r11
	adds r1, r2, r7
	ldr r2, =0x2dd64300
	ldr r10, =0xa21f2c35
	add r2, r10
	add r2, r5, r10
	ldr r1, =0x7fffffff
	ldr r12, =0x23824dd
	adds r1, r12
	add r7, r6, r3
	ldr r2, =0x0
	ldr r11, =0xffffffff
	adds r2, r11
	adds r5, r8, r5
	ldr r11, =0x7fffffff
	ldr r1, =0x13d0ba5f
	add r11, r1
	adds r6, r1, r2
	ldr r8, =0x327739a
	ldr r0, =0xa9cfd774
	add r8, r0
	adds r5, r4, r10
	ldr r3, =0xffffffff
	ldr r11, =0x80000000
	add r3, r11
	add r8, r0, r11
	ldr r9, =0x0
	ldr r12, =0x55c9826c
	add r9, r12
	adds r5, r0, r12
	ldr r0, =0x1a766ea8
	ldr r10, =0xe5899157
	adds r0, r10
	adds r8, r3, r9
	ldr r4, =0x0
	ldr r3, =0xc4eac623
	adds r4, r3
	adds r7, r12, r1
	ldr r11, =0x7faaa6dd
	ldr r2, =0x49708c4c
	adds r11, r2
	adds r4, r11, r12
	ldr r4, =0xd3f9f0c4
	ldr r6, =0xe32f9503
	add r4, r6
	add r3, r7, r1
	ldr r5, =0x0
	ldr r10, =0x9a7a9e0b
	adds r5, r10
	add r8, r4, r3
	ldr r7, =0x137832f8
	ldr r6, =0xfa65669
	add r7, r6
	add r7, r14, r8
	ldr r12, =0x7c195c3d
	ldr r6, =0xaad77843
	adds r12, r6
	add r2, r0, r12
	ldr r9, =0xa868d02d
	ldr r10, =0x80000000
	adds r9, r10
	adds r10, r6, r0
	ldr r4, =0x4bbe3288
	ldr r2, =0x4bbe3289
	add r4, r2
	adds r4, r6, r8
	ldr r8, =0xbba161db
	ldr r11, =0x7fffffff
	add r8, r11
	adds r10, r0, r4
	ldr r12, =0xffffffff
	ldr r9, =0x3580c853
	add r12, r9
	adds r7, r8, r8
	ldr r4, =0x64fa192d
	ldr r1, =0x8a494ed7
	adds r4, r1
	add r0, r10, r8
	ldr r6, =0x1110499d
	ldr r9, =0x1110499c
	adds r6, r9
	adds r11, r5, r9
	ldr r12, =0x7fffffff
	ldr r2, =0x1c46eb19
	adds r12, r2
	add r11, r4, r10
	ldr r3, =0x824c9400
	ldr r2, =0x21a1079d
	adds r3, r2
	adds r7, r9, r11
	ldr r8, =0x358723e8
	ldr r8, =0x358723e7
	add r8, r8
	add r9, r1, r9
	ldr r4, =0x2edd17ee
	ldr r12, =0x707e905
	adds r4, r12
	adds r5, r11, r12
	ldr r7, =0xef36598a
	ldr r5, =0xffffffff
	adds r7, r5
	adds r11, r6, r6
	ldr r1, =0xaa534bf
	ldr r2, =0x24b466a
	adds r1, r2
	add r3, r14, r5
	ldr r7, =0x14de89db
	ldr r11, =0x99c47e2f
	adds r7, r11
	add r11, r9, r0
	ldr r5, =0x0
	ldr r12, =0x49220287
	adds r5, r12
	adds r8, r1, r6
	ldr r7, =0x3c3e38be
	ldr r1, =0x3c3e38bf
	add r7, r1
	add r2, r3, r5
	ldr r10, =0xfd79cf0a
	ldr r3, =0xfd79cf0b
	add r10, r3
	adds r11, r5, r11
	ldr r12, =0x5400d515
	ldr r11, =0xabff2aea
	adds r12, r11
	add r6, r14, r5
	ldr r11, =0xc5cd67f5
	ldr r1, =0x3a32980b
	add r11, r1
	add r3, r6, r10
	ldr r9, =0xe1a4e8f
	ldr r12, =0x963b82d2
	add r9, r12
	adds r8, r6, r3
	ldr r2, =0xc39c77c6
	ldr r3, =0xbb3ebf52
	add r2, r3
	add r9, r6, r12
	ldr r10, =0x1162ac05
	ldr r12, =0x7fffffff
	adds r10, r12
	adds r10, r0, r7
	ldr r5, =0xffffffff
	ldr r7, =0x59902b60
	add r5, r7
	add r1, r13, r2
	ldr r12, =0x0
	ldr r7, =0xffffffff
	adds r12, r7
	add r8, r8, r12
	ldr r1, =0x8205603a
	ldr r9, =0x7fffffff
	add r1, r9
	add r6, r8, r8
	ldr r11, =0x0
	ldr r4, =0xfc12e89b
	adds r11, r4
	adds r2, r14, r4
	ldr r1, =0xffffffff
	ldr r6, =0xcc62625b
	adds r1, r6
	add r3, r0, r7
	ldr r7, =0x7fffffff
	ldr r10, =0x0
	add r7, r10
	add r6, r10, r1
	ldr r3, =0xdf3bc182
	ldr r9, =0xe2847d24
	add r3, r9
	adds r9, r3, r1
	ldr r2, =0xc923b323
	ldr r6, =0x36dc4cdd
	add r2, r6
	adds r1, r7, r2
	ldr r3, =0xffffffff
	ldr r9, =0x12b5c69d
	adds r3, r9
	adds r12, r14, r5
	ldr r1, =0x2401a578
	ldr r0, =0xdbfe5a86
	adds r1, r0
	adds r0, r9, r9
	ldr r2, =0xd4b7eb29
	ldr r9, =0x80000000
	adds r2, r9
	add r9, r0, r1
	ldr r10, =0x7fffffff
	ldr r7, =0x80000000
	adds r10, r7
	add r8, r3, r8
	ldr r4, =0x5fd0cdc2
	ldr r6, =0x6a80a68
	adds r4, r6
	adds r6, r13, r3
	
	b .tclbl_11
	// Generate literal table here so we don't run out of offset space
	.ltorg
	.tclbl_11:


	ldr r4, =0xf7d8cded
	ldr r4, =0x80000000
	add r4, r4
	adds r0, r6, r10
	ldr r2, =0x5a50b395
	ldr r3, =0xffffffff
	add r2, r3
	adds r2, r6, r5
	ldr r11, =0x3f7c7c79
	ldr r0, =0xc0838386
	adds r11, r0
	adds r4, r2, r9
	ldr r7, =0xcb54c658
	ldr r3, =0xffffffff
	adds r7, r3
	add r3, r1, r3
	ldr r8, =0xe1d60a40
	ldr r11, =0xe1d60a41
	add r8, r11
	adds r8, r9, r10
	ldr r4, =0x1fc2cfd2
	ldr r1, =0x1fc2cfd1
	adds r4, r1
	adds r3, r8, r6
	ldr r7, =0x383a3378
	ldr r6, =0xc7c5cc87
	adds r7, r6
	add r10, r3, r0
	ldr r11, =0x570d0a86
	ldr r11, =0x80000000
	add r11, r11
	adds r4, r13, r0
	ldr r9, =0xedd58db2
	ldr r10, =0x122a724f
	add r9, r10
	add r4, r4, r8
	ldr r10, =0x554d2d71
	ldr r3, =0x0
	add r10, r3
	add r1, r10, r0
	ldr r9, =0xffffffff
	ldr r0, =0x22bc4caf
	adds r9, r0
	add r9, r3, r4
	ldr r8, =0xa7bb6cf2
	ldr r11, =0x0
	add r8, r11
	adds r5, r14, r7
	ldr r7, =0xffffffff
	ldr r4, =0x4cbd5871
	adds r7, r4
	adds r1, r11, r8
	ldr r11, =0xffffffff
	ldr r0, =0x91f49b2e
	add r11, r0
	add r10, r0, r6
	ldr r1, =0xc9cd6d1
	ldr r10, =0x2baaf842
	add r1, r10
	add r10, r10, r5
	ldr r4, =0x405595ca
	ldr r3, =0x405595c8
	adds r4, r3
	adds r12, r7, r5
	ldr r4, =0x7a570b59
	ldr r8, =0xca7afa8
	adds r4, r8
	adds r5, r1, r1
	ldr r3, =0xa6bc2927
	ldr r2, =0x7fffffff
	add r3, r2
	adds r2, r8, r1
	ldr r4, =0x925e979
	ldr r4, =0x925e978
	adds r4, r4
	add r9, r12, r5
	ldr r12, =0x0
	ldr r7, =0x80000000
	adds r12, r7
	add r12, r4, r12
	ldr r12, =0x8449b
	ldr r6, =0xa455d3a
	adds r12, r6
	add r8, r1, r2
	ldr r9, =0x131d6d22
	ldr r5, =0xffffffff
	add r9, r5
	add r5, r14, r1
	ldr r5, =0x0
	ldr r3, =0x7fffffff
	adds r5, r3
	add r12, r7, r10
	ldr r7, =0x5d3b7e1d
	ldr r7, =0x7fffffff
	add r7, r7
	add r9, r3, r8
	ldr r9, =0x686a0019
	ldr r6, =0xb75de52b
	adds r9, r6
	adds r9, r11, r9
	ldr r8, =0xc05b55f1
	ldr r6, =0x1354a161
	adds r8, r6
	add r10, r13, r8
	ldr r7, =0x62b15c42
	ldr r9, =0xa2cc6a3d
	adds r7, r9
	add r12, r7, r1
	ldr r8, =0x80000000
	ldr r11, =0x0
	add r8, r11
	adds r12, r14, r8
	ldr r1, =0xf9967283
	ldr r1, =0xf9967284
	adds r1, r1
	add r1, r4, r6
	ldr r12, =0x0
	ldr r2, =0xffffffff
	adds r12, r2
	add r8, r4, r3
	ldr r6, =0x7c808aa3
	ldr r8, =0x837f755d
	adds r6, r8
	add r3, r4, r8
	ldr r1, =0x1c1f2cde
	ldr r4, =0xe3e0d320
	add r1, r4
	add r0, r6, r12
	ldr r1, =0x7fffffff
	ldr r3, =0xffffffff
	add r1, r3
	adds r3, r4, r0
	ldr r4, =0x80000000
	ldr r11, =0xea5b0176
	add r4, r11
	add r11, r8, r8
	ldr r8, =0xf353710f
	ldr r12, =0xa0e82267
	adds r8, r12
	add r0, r2, r8
	ldr r4, =0xecaee5e9
	ldr r6, =0xa7594ca5
	adds r4, r6
	adds r12, r7, r2
	ldr r4, =0xa5217a96
	ldr r8, =0x104f2e0e
	add r4, r8
	add r6, r6, r7
	ldr r1, =0x2d3efba2
	ldr r10, =0xffffffff
	add r1, r10
	adds r0, r10, r8
	ldr r10, =0x7fffffff
	ldr r5, =0x4e4b3ad3
	add r10, r5
	add r12, r14, r3
	ldr r4, =0xffffffff
	ldr r8, =0x1c1a0958
	adds r4, r8
	adds r1, r12, r8
	ldr r9, =0x63cb459
	ldr r2, =0x90224b00
	add r9, r2
	adds r1, r14, r7
	ldr r0, =0xfc701643
	ldr r11, =0x80000000
	add r0, r11
	adds r6, r3, r1
	ldr r9, =0x8246da42
	ldr r3, =0x5b9ba555
	add r9, r3
	adds r7, r5, r12
	ldr r12, =0x5dd7af99
	ldr r11, =0xe64affc7
	add r12, r11
	add r9, r4, r4
	ldr r5, =0x7fffffff
	ldr r1, =0x964519e7
	add r5, r1
	adds r5, r1, r4
	ldr r11, =0x9aff2464
	ldr r5, =0x6500db9d
	adds r11, r5
	adds r6, r10, r9
	ldr r5, =0x3f137df6
	ldr r4, =0x5d766caf
	add r5, r4
	add r6, r9, r0
	ldr r1, =0x115d83ef
	ldr r11, =0x244e8728
	adds r1, r11
	adds r4, r2, r3
	ldr r8, =0x22c3669b
	ldr r5, =0x0
	adds r8, r5
	add r7, r11, r12
	ldr r5, =0xa8d4f4ff
	ldr r3, =0x572b0b00
	adds r5, r3
	adds r12, r3, r10
	ldr r0, =0x64b5c3fb
	ldr r0, =0x64b5c3fd
	adds r0, r0
	add r8, r3, r9
	ldr r3, =0x9943e0e6
	ldr r8, =0x9943e0e5
	add r3, r8
	adds r1, r0, r6
	ldr r8, =0x365c76ca
	ldr r3, =0x0
	add r8, r3
	add r0, r4, r10
	ldr r6, =0x48fc076
	ldr r3, =0xfb703f8b
	adds r6, r3
	add r1, r10, r9
	ldr r11, =0x5e2f0cd2
	ldr r2, =0x1f2b1bb6
	adds r11, r2
	add r6, r9, r10
	ldr r2, =0x4fcb90b6
	ldr r1, =0x3204f20d
	add r2, r1
	add r10, r3, r9
	ldr r12, =0x0
	ldr r1, =0xdfd5e42
	add r12, r1
	adds r7, r5, r7
	ldr r3, =0x7fffffff
	ldr r0, =0xc24bc6a9
	adds r3, r0
	add r8, r10, r8
	ldr r2, =0x7fffffff
	ldr r7, =0xaf49acb3
	adds r2, r7
	adds r5, r13, r9
	ldr r1, =0xcbadb0d0
	ldr r5, =0x0
	adds r1, r5
	adds r8, r10, r5
	ldr r0, =0x9c96a61e
	ldr r6, =0x34ba4666
	adds r0, r6
	add r2, r1, r1
	ldr r10, =0x7882b593
	ldr r6, =0x80000000
	adds r10, r6
	add r8, r8, r3
	ldr r2, =0xd1fafe3d
	ldr r5, =0xca52d3e4
	adds r2, r5
	adds r10, r9, r1
	ldr r12, =0x96b4a70d
	ldr r2, =0x80000000
	add r12, r2
	add r10, r11, r7
	ldr r12, =0xb9aea63e
	ldr r5, =0x80000000
	add r12, r5
	add r6, r10, r7
	ldr r3, =0x98a37cef
	ldr r3, =0xd98fca36
	adds r3, r3
	adds r4, r2, r5
	ldr r6, =0x1cd1bb1
	ldr r6, =0xfe32e44d
	adds r6, r6
	adds r5, r13, r12
	ldr r4, =0x3755c166
	ldr r1, =0x201553d2
	adds r4, r1
	add r10, r2, r1
	ldr r7, =0x45091b45
	ldr r3, =0xffffffff
	add r7, r3
	adds r9, r3, r3
	ldr r6, =0x80000000
	ldr r4, =0x7fffffff
	add r6, r4
	adds r5, r12, r9
	ldr r11, =0xffffffff
	ldr r4, =0x7fffffff
	adds r11, r4
	add r10, r7, r12
	ldr r5, =0xb0f3cdda
	ldr r3, =0x80000000
	add r5, r3
	add r2, r2, r0
	ldr r3, =0x67412d6a
	ldr r2, =0x55fcb2cf
	adds r3, r2
	add r3, r8, r8
	ldr r9, =0xbc81edb0
	ldr r8, =0x437e124e
	adds r9, r8
	adds r7, r8, r8
	ldr r12, =0xf4940e25
	ldr r5, =0x4085f340
	add r12, r5
	adds r8, r0, r5
	ldr r8, =0x0
	ldr r8, =0x339375db
	add r8, r8
	add r11, r10, r3
	ldr r1, =0x92f2b728
	ldr r10, =0x456e81c2
	add r1, r10
	adds r7, r7, r6
	ldr r8, =0x69415f38
	ldr r12, =0xffffffff
	add r8, r12
	add r8, r13, r9
	ldr r12, =0x1b922c8c
	ldr r1, =0x80000000
	add r12, r1
	add r2, r13, r8
	ldr r2, =0x90009826
	ldr r3, =0x90009825
	adds r2, r3
	add r2, r9, r0
	ldr r3, =0xe630f694
	ldr r3, =0xe630f695
	adds r3, r3
	adds r7, r4, r7
	ldr r10, =0x45aa9739
	ldr r8, =0xec53a4f0
	adds r10, r8
	add r6, r10, r6
	ldr r7, =0x80000000
	ldr r1, =0x33972e1e
	add r7, r1
	add r5, r3, r9
	ldr r1, =0x5f3edcb7
	ldr r5, =0x5f3edcb5
	adds r1, r5
	adds r11, r4, r2
	ldr r0, =0xb953e36c
	ldr r7, =0xb953e36b
	adds r0, r7
	add r10, r3, r9
	ldr r1, =0x312343e2
	ldr r11, =0xcedcbc1c
	add r1, r11
	add r1, r14, r7
	ldr r8, =0x7e55226
	ldr r11, =0x7fffffff
	adds r8, r11
	adds r9, r12, r1
	ldr r0, =0x3bbb1899
	ldr r10, =0xc444e765
	adds r0, r10
	add r9, r2, r4
	ldr r3, =0x7fffffff
	ldr r6, =0x17433750
	add r3, r6
	adds r10, r9, r3
	ldr r11, =0x0
	ldr r3, =0xa3e26f32
	adds r11, r3
	add r5, r13, r1
	ldr r10, =0x54acee4a
	ldr r2, =0xab5311b5
	add r10, r2
	adds r3, r9, r8
	ldr r0, =0x7fffffff
	ldr r8, =0x6a9065ad
	add r0, r8
	adds r0, r5, r3
	ldr r9, =0x1271b739
	ldr r10, =0x80000000
	add r9, r10
	adds r4, r8, r2
	ldr r5, =0xdfb35006
	ldr r10, =0x7eeeb65c
	add r5, r10
	add r6, r3, r11
	ldr r7, =0x0
	ldr r0, =0x78807629
	adds r7, r0
	adds r6, r6, r1
	ldr r11, =0x80000000
	ldr r0, =0xdfddb754
	adds r11, r0
	add r3, r2, r0
	ldr r9, =0xa6c58606
	ldr r8, =0x593a79f9
	add r9, r8
	add r8, r4, r9
	ldr r12, =0xa689483e
	ldr r6, =0x819b8868
	add r12, r6
	add r3, r13, r6
	ldr r5, =0xdec93fd
	ldr r3, =0xf2136c02
	adds r5, r3
	adds r2, r1, r4


testcase_end:
	b testcase_end
.size main, .-main
.global main

.type default_fault_handler, %function
default_fault_handler:
	b default_fault_handler
.size default_fault_handler, .-default_fault_handler
.global default_fault_handler
