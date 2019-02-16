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

.macro _asm_memset
	# r0: destination
	# r1: pattern
	# r2: size in bytes, must be >= 0 and divisible by 4
	ands r2, #~0x03
	cbz r2, _asm_memset_end_\@
	subs r2, #4

	_asm_memset_loop_\@:
		str r1, [r0, r2]
		cbz r2, _asm_memset_end_\@
		subs r2, #4
	b _asm_memset_loop_\@
	_asm_memset_end_\@:
.endm

.macro _asm_memcpy
	# r0: destination address
	# r1: source address
	# r2: size in bytes, must be >= 0 and divisible by 4
	ands r2, #~0x03
	cbz r2, _asm_memcpy_end_\@
	subs r2, #4

	_asm_memcpy_loop_\@:
		ldr r3, [r1, r2]
		str r3, [r0, r2]
		cbz r2, _asm_memcpy_end_\@
		subs r2, #4
	b _asm_memcpy_loop_\@
	_asm_memcpy_end_\@:
.endm

.macro _semihosting_exit
	ldr r0, =0x18
	ldr r1, =0x20026
	bkpt #0xab
.endm

.section .text
.type Reset_Handler, %function
Reset_Handler:
	# Painting of all RAM
	ldr r0, =_sram
	ldr r1, =0xdeadbeef
	ldr r2, =_eram
	subs r2, r0
	_asm_memset

	# Load .data section
	ldr r0, =_sdata
	ldr r1, =_sidata
	ldr r2, =_edata
	subs r2, r0
	_asm_memcpy

	# Zero .bss section
	ldr r0, =_sbss
	ldr r1, =0
	ldr r2, =_ebss
	subs r2, r0
	_asm_memset

#	_semihosting_exit

	ldr lr, =0x57a0057a
	bl SystemInit
	bl main

	_exit_loop:
	b _exit_loop
.size Reset_Handler, .-Reset_Handler
.global Reset_Handler

.section .text, "ax", %progbits
Default_Handler:
       b default_fault_handler
.size Default_Handler, .-Default_Handler
.global Default_Handler


.section .vectors, "a", %progbits
.type vectors, %object
vectors:
	.word		_eram
	.word		Reset_Handler
	.word		NMI_Handler
	.word		HardFault_Handler
	.word		MemManage_Handler
	.word		BusFault_Handler
	.word		UsageFault_Handler
	.word		0
	.word		0
	.word		0
	.word		0
	.word		SVC_Handler
	.word		DebugMon_Handler
	.word		0
	.word		PendSV_Handler
	.word		SysTick_Handler
	.word		WWDG_IRQHandler
	.word		PVD_IRQHandler
	.word		TAMP_STAMP_IRQHandler
	.word		RTC_WKUP_IRQHandler
	.word		FLASH_IRQHandler
	.word		RCC_IRQHandler
	.word		EXTI0_IRQHandler
	.word		EXTI1_IRQHandler
	.word		EXTI2_IRQHandler
	.word		EXTI3_IRQHandler
	.word		EXTI4_IRQHandler
	.word		DMA1_Stream0_IRQHandler
	.word		DMA1_Stream1_IRQHandler
	.word		DMA1_Stream2_IRQHandler
	.word		DMA1_Stream3_IRQHandler
	.word		DMA1_Stream4_IRQHandler
	.word		DMA1_Stream5_IRQHandler
	.word		DMA1_Stream6_IRQHandler
	.word		ADC_IRQHandler
	.word		CAN1_TX_IRQHandler
	.word		CAN1_RX0_IRQHandler
	.word		CAN1_RX1_IRQHandler
	.word		CAN1_SCE_IRQHandler
	.word		EXTI9_5_IRQHandler
	.word		TIM1_BRK_TIM9_IRQHandler
	.word		TIM1_UP_TIM10_IRQHandler
	.word		TIM1_TRG_COM_TIM11_IRQHandler
	.word		TIM1_CC_IRQHandler
	.word		TIM2_IRQHandler
	.word		TIM3_IRQHandler
	.word		TIM4_IRQHandler
	.word		I2C1_EV_IRQHandler
	.word		I2C1_ER_IRQHandler
	.word		I2C2_EV_IRQHandler
	.word		I2C2_ER_IRQHandler
	.word		SPI1_IRQHandler
	.word		SPI2_IRQHandler
	.word		USART1_IRQHandler
	.word		USART2_IRQHandler
	.word		USART3_IRQHandler
	.word		EXTI15_10_IRQHandler
	.word		RTC_Alarm_IRQHandler
	.word		OTG_FS_WKUP_IRQHandler
	.word		TIM8_BRK_TIM12_IRQHandler
	.word		TIM8_UP_TIM13_IRQHandler
	.word		TIM8_TRG_COM_TIM14_IRQHandler
	.word		TIM8_CC_IRQHandler
	.word		DMA1_Stream7_IRQHandler
	.word		FSMC_IRQHandler
	.word		SDIO_IRQHandler
	.word		TIM5_IRQHandler
	.word		SPI3_IRQHandler
	.word		UART4_IRQHandler
	.word		UART5_IRQHandler
	.word		TIM6_DAC_IRQHandler
	.word		TIM7_IRQHandler
	.word		DMA2_Stream0_IRQHandler
	.word		DMA2_Stream1_IRQHandler
	.word		DMA2_Stream2_IRQHandler
	.word		DMA2_Stream3_IRQHandler
	.word		DMA2_Stream4_IRQHandler
	.word		ETH_IRQHandler
	.word		ETH_WKUP_IRQHandler
	.word		CAN2_TX_IRQHandler
	.word		CAN2_RX0_IRQHandler
	.word		CAN2_RX1_IRQHandler
	.word		CAN2_SCE_IRQHandler
	.word		OTG_FS_IRQHandler
	.word		DMA2_Stream5_IRQHandler
	.word		DMA2_Stream6_IRQHandler
	.word		DMA2_Stream7_IRQHandler
	.word		USART6_IRQHandler
	.word		I2C3_EV_IRQHandler
	.word		I2C3_ER_IRQHandler
	.word		OTG_HS_EP1_OUT_IRQHandler
	.word		OTG_HS_EP1_IN_IRQHandler
	.word		OTG_HS_WKUP_IRQHandler
	.word		OTG_HS_IRQHandler
	.word		DCMI_IRQHandler
	.word		CRYP_IRQHandler
	.word		HASH_RNG_IRQHandler
	.word		FPU_IRQHandler

	.weak		NMI_Handler
	.thumb_set	NMI_Handler, Default_Handler

	.weak		HardFault_Handler
	.thumb_set	HardFault_Handler, Default_Handler

	.weak		MemManage_Handler
	.thumb_set	MemManage_Handler, Default_Handler

	.weak		BusFault_Handler
	.thumb_set	BusFault_Handler, Default_Handler

	.weak		UsageFault_Handler
	.thumb_set	UsageFault_Handler, Default_Handler

	.weak		SVC_Handler
	.thumb_set	SVC_Handler, Default_Handler

	.weak		DebugMon_Handler
	.thumb_set	DebugMon_Handler, Default_Handler

	.weak		PendSV_Handler
	.thumb_set	PendSV_Handler, Default_Handler

	.weak		SysTick_Handler
	.thumb_set	SysTick_Handler, Default_Handler

	.weak		WWDG_IRQHandler
	.thumb_set	WWDG_IRQHandler, Default_Handler

	.weak		PVD_IRQHandler
	.thumb_set	PVD_IRQHandler, Default_Handler

	.weak		TAMP_STAMP_IRQHandler
	.thumb_set	TAMP_STAMP_IRQHandler, Default_Handler

	.weak		RTC_WKUP_IRQHandler
	.thumb_set	RTC_WKUP_IRQHandler, Default_Handler

	.weak		FLASH_IRQHandler
	.thumb_set	FLASH_IRQHandler, Default_Handler

	.weak		RCC_IRQHandler
	.thumb_set	RCC_IRQHandler, Default_Handler

	.weak		EXTI0_IRQHandler
	.thumb_set	EXTI0_IRQHandler, Default_Handler

	.weak		EXTI1_IRQHandler
	.thumb_set	EXTI1_IRQHandler, Default_Handler

	.weak		EXTI2_IRQHandler
	.thumb_set	EXTI2_IRQHandler, Default_Handler

	.weak		EXTI3_IRQHandler
	.thumb_set	EXTI3_IRQHandler, Default_Handler

	.weak		EXTI4_IRQHandler
	.thumb_set	EXTI4_IRQHandler, Default_Handler

	.weak		DMA1_Stream0_IRQHandler
	.thumb_set	DMA1_Stream0_IRQHandler, Default_Handler

	.weak		DMA1_Stream1_IRQHandler
	.thumb_set	DMA1_Stream1_IRQHandler, Default_Handler

	.weak		DMA1_Stream2_IRQHandler
	.thumb_set	DMA1_Stream2_IRQHandler, Default_Handler

	.weak		DMA1_Stream3_IRQHandler
	.thumb_set	DMA1_Stream3_IRQHandler, Default_Handler

	.weak		DMA1_Stream4_IRQHandler
	.thumb_set	DMA1_Stream4_IRQHandler, Default_Handler

	.weak		DMA1_Stream5_IRQHandler
	.thumb_set	DMA1_Stream5_IRQHandler, Default_Handler

	.weak		DMA1_Stream6_IRQHandler
	.thumb_set	DMA1_Stream6_IRQHandler, Default_Handler

	.weak		ADC_IRQHandler
	.thumb_set	ADC_IRQHandler, Default_Handler

	.weak		CAN1_TX_IRQHandler
	.thumb_set	CAN1_TX_IRQHandler, Default_Handler

	.weak		CAN1_RX0_IRQHandler
	.thumb_set	CAN1_RX0_IRQHandler, Default_Handler

	.weak		CAN1_RX1_IRQHandler
	.thumb_set	CAN1_RX1_IRQHandler, Default_Handler

	.weak		CAN1_SCE_IRQHandler
	.thumb_set	CAN1_SCE_IRQHandler, Default_Handler

	.weak		EXTI9_5_IRQHandler
	.thumb_set	EXTI9_5_IRQHandler, Default_Handler

	.weak		TIM1_BRK_TIM9_IRQHandler
	.thumb_set	TIM1_BRK_TIM9_IRQHandler, Default_Handler

	.weak		TIM1_UP_TIM10_IRQHandler
	.thumb_set	TIM1_UP_TIM10_IRQHandler, Default_Handler

	.weak		TIM1_TRG_COM_TIM11_IRQHandler
	.thumb_set	TIM1_TRG_COM_TIM11_IRQHandler, Default_Handler

	.weak		TIM1_CC_IRQHandler
	.thumb_set	TIM1_CC_IRQHandler, Default_Handler

	.weak		TIM2_IRQHandler
	.thumb_set	TIM2_IRQHandler, Default_Handler

	.weak		TIM3_IRQHandler
	.thumb_set	TIM3_IRQHandler, Default_Handler

	.weak		TIM4_IRQHandler
	.thumb_set	TIM4_IRQHandler, Default_Handler

	.weak		I2C1_EV_IRQHandler
	.thumb_set	I2C1_EV_IRQHandler, Default_Handler

	.weak		I2C1_ER_IRQHandler
	.thumb_set	I2C1_ER_IRQHandler, Default_Handler

	.weak		I2C2_EV_IRQHandler
	.thumb_set	I2C2_EV_IRQHandler, Default_Handler

	.weak		I2C2_ER_IRQHandler
	.thumb_set	I2C2_ER_IRQHandler, Default_Handler

	.weak		SPI1_IRQHandler
	.thumb_set	SPI1_IRQHandler, Default_Handler

	.weak		SPI2_IRQHandler
	.thumb_set	SPI2_IRQHandler, Default_Handler

	.weak		USART1_IRQHandler
	.thumb_set	USART1_IRQHandler, Default_Handler

	.weak		USART2_IRQHandler
	.thumb_set	USART2_IRQHandler, Default_Handler

	.weak		USART3_IRQHandler
	.thumb_set	USART3_IRQHandler, Default_Handler

	.weak		EXTI15_10_IRQHandler
	.thumb_set	EXTI15_10_IRQHandler, Default_Handler

	.weak		RTC_Alarm_IRQHandler
	.thumb_set	RTC_Alarm_IRQHandler, Default_Handler

	.weak		OTG_FS_WKUP_IRQHandler
	.thumb_set	OTG_FS_WKUP_IRQHandler, Default_Handler

	.weak		TIM8_BRK_TIM12_IRQHandler
	.thumb_set	TIM8_BRK_TIM12_IRQHandler, Default_Handler

	.weak		TIM8_UP_TIM13_IRQHandler
	.thumb_set	TIM8_UP_TIM13_IRQHandler, Default_Handler

	.weak		TIM8_TRG_COM_TIM14_IRQHandler
	.thumb_set	TIM8_TRG_COM_TIM14_IRQHandler, Default_Handler

	.weak		TIM8_CC_IRQHandler
	.thumb_set	TIM8_CC_IRQHandler, Default_Handler

	.weak		DMA1_Stream7_IRQHandler
	.thumb_set	DMA1_Stream7_IRQHandler, Default_Handler

	.weak		FSMC_IRQHandler
	.thumb_set	FSMC_IRQHandler, Default_Handler

	.weak		SDIO_IRQHandler
	.thumb_set	SDIO_IRQHandler, Default_Handler

	.weak		TIM5_IRQHandler
	.thumb_set	TIM5_IRQHandler, Default_Handler

	.weak		SPI3_IRQHandler
	.thumb_set	SPI3_IRQHandler, Default_Handler

	.weak		UART4_IRQHandler
	.thumb_set	UART4_IRQHandler, Default_Handler

	.weak		UART5_IRQHandler
	.thumb_set	UART5_IRQHandler, Default_Handler

	.weak		TIM6_DAC_IRQHandler
	.thumb_set	TIM6_DAC_IRQHandler, Default_Handler

	.weak		TIM7_IRQHandler
	.thumb_set	TIM7_IRQHandler, Default_Handler

	.weak		DMA2_Stream0_IRQHandler
	.thumb_set	DMA2_Stream0_IRQHandler, Default_Handler

	.weak		DMA2_Stream1_IRQHandler
	.thumb_set	DMA2_Stream1_IRQHandler, Default_Handler

	.weak		DMA2_Stream2_IRQHandler
	.thumb_set	DMA2_Stream2_IRQHandler, Default_Handler

	.weak		DMA2_Stream3_IRQHandler
	.thumb_set	DMA2_Stream3_IRQHandler, Default_Handler

	.weak		DMA2_Stream4_IRQHandler
	.thumb_set	DMA2_Stream4_IRQHandler, Default_Handler

	.weak		ETH_IRQHandler
	.thumb_set	ETH_IRQHandler, Default_Handler

	.weak		ETH_WKUP_IRQHandler
	.thumb_set	ETH_WKUP_IRQHandler, Default_Handler

	.weak		CAN2_TX_IRQHandler
	.thumb_set	CAN2_TX_IRQHandler, Default_Handler

	.weak		CAN2_RX0_IRQHandler
	.thumb_set	CAN2_RX0_IRQHandler, Default_Handler

	.weak		CAN2_RX1_IRQHandler
	.thumb_set	CAN2_RX1_IRQHandler, Default_Handler

	.weak		CAN2_SCE_IRQHandler
	.thumb_set	CAN2_SCE_IRQHandler, Default_Handler

	.weak		OTG_FS_IRQHandler
	.thumb_set	OTG_FS_IRQHandler, Default_Handler

	.weak		DMA2_Stream5_IRQHandler
	.thumb_set	DMA2_Stream5_IRQHandler, Default_Handler

	.weak		DMA2_Stream6_IRQHandler
	.thumb_set	DMA2_Stream6_IRQHandler, Default_Handler

	.weak		DMA2_Stream7_IRQHandler
	.thumb_set	DMA2_Stream7_IRQHandler, Default_Handler

	.weak		USART6_IRQHandler
	.thumb_set	USART6_IRQHandler, Default_Handler

	.weak		I2C3_EV_IRQHandler
	.thumb_set	I2C3_EV_IRQHandler, Default_Handler

	.weak		I2C3_ER_IRQHandler
	.thumb_set	I2C3_ER_IRQHandler, Default_Handler

	.weak		OTG_HS_EP1_OUT_IRQHandler
	.thumb_set	OTG_HS_EP1_OUT_IRQHandler, Default_Handler

	.weak		OTG_HS_EP1_IN_IRQHandler
	.thumb_set	OTG_HS_EP1_IN_IRQHandler, Default_Handler

	.weak		OTG_HS_WKUP_IRQHandler
	.thumb_set	OTG_HS_WKUP_IRQHandler, Default_Handler

	.weak		OTG_HS_IRQHandler
	.thumb_set	OTG_HS_IRQHandler, Default_Handler

	.weak		DCMI_IRQHandler
	.thumb_set	DCMI_IRQHandler, Default_Handler

	.weak		CRYP_IRQHandler
	.thumb_set	CRYP_IRQHandler, Default_Handler

	.weak		HASH_RNG_IRQHandler
	.thumb_set	HASH_RNG_IRQHandler, Default_Handler

	.weak		FPU_IRQHandler
	.thumb_set	FPU_IRQHandler, Default_Handler


.size vectors, .-vectors
.global vectors
