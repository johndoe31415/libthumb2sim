/*
	libthumb2sim - Emulator for the Thumb-2 ISA (Cortex-M)
	Copyright (C) 2014-2019 Johannes Bauer

	This file is part of libthumb2sim.

	libthumb2sim is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; this program is ONLY licensed under
	version 3 of the License, later versions are explicitly excluded.

	libthumb2sim is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with libthumb2sim; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	Johannes Bauer <JohannesBauer@gmx.de>
*/

#ifndef __DECODER_H__
#define __DECODER_H__

#include <stdint.h>
#include <stdbool.h>

struct decoding_handler_t {
	void (*i32_adc_imm_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S);
	void (*i16_adc_reg_T1)(void *aCtx, uint8_t Rdn, uint8_t Rm);
	void (*i32_adc_reg_T2)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S);
	void (*i16_add_SPi_T1)(void *aCtx, uint8_t Rd, uint8_t imm);
	void (*i16_add_SPi_T2)(void *aCtx, uint8_t imm);
	void (*i32_add_SPi_T3)(void *aCtx, uint8_t Rd, int32_t imm, bool S);
	void (*i32_add_SPi_T4)(void *aCtx, uint8_t Rd, uint16_t imm);
	void (*i16_add_SPr_T1)(void *aCtx, uint8_t Rdm);
	void (*i16_add_SPr_T2)(void *aCtx, uint8_t Rm);
	void (*i32_add_SPr_T3)(void *aCtx, uint8_t Rd, uint8_t Rm, uint8_t imm, uint8_t type, bool S);
	void (*i16_add_imm_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t imm);
	void (*i16_add_imm_T2)(void *aCtx, uint8_t Rdn, uint8_t imm);
	void (*i32_add_imm_T3)(void *aCtx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S);
	void (*i32_add_imm_T4)(void *aCtx, uint8_t Rd, uint8_t Rn, uint16_t imm);
	void (*i16_add_reg_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i16_add_reg_T2)(void *aCtx, uint8_t Rdn, uint8_t Rm);
	void (*i32_add_reg_T3)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S);
	void (*i16_adr_T1)(void *aCtx, uint8_t Rd, uint8_t imm);
	void (*i32_adr_T2)(void *aCtx, uint8_t Rd, uint16_t imm);
	void (*i32_adr_T3)(void *aCtx, uint8_t Rd, uint16_t imm);
	void (*i32_and_imm_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S);
	void (*i16_and_reg_T1)(void *aCtx, uint8_t Rdn, uint8_t Rm);
	void (*i32_and_reg_T2)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S);
	void (*i16_asr_imm_T1)(void *aCtx, uint8_t Rd, uint8_t Rm, uint8_t imm);
	void (*i32_asr_imm_T2)(void *aCtx, uint8_t Rd, uint8_t Rm, uint8_t imm, bool S);
	void (*i16_asr_reg_T1)(void *aCtx, uint8_t Rdn, uint8_t Rm);
	void (*i32_asr_reg_T2)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, bool S);
	void (*i16_b_T1)(void *aCtx, uint8_t imm, uint8_t cond);
	void (*i16_b_T2)(void *aCtx, uint16_t imm);
	void (*i32_b_T3)(void *aCtx, int32_t imm, uint8_t cond);
	void (*i32_b_T4)(void *aCtx, int32_t imm);
	void (*i32_bfc_T1)(void *aCtx, uint8_t Rd, uint8_t imm, uint8_t msb);
	void (*i32_bfi_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t imm, uint8_t msb);
	void (*i32_bic_imm_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S);
	void (*i16_bic_reg_T1)(void *aCtx, uint8_t Rdn, uint8_t Rm);
	void (*i32_bic_reg_T2)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S);
	void (*i16_bkpt_T1)(void *aCtx, uint8_t imm);
	void (*i32_bl_T1)(void *aCtx, int32_t imm);
	void (*i16_blx_reg_T1)(void *aCtx, uint8_t Rm);
	void (*i16_bx_T1)(void *aCtx, uint8_t Rm);
	void (*i16_cbnz_T1)(void *aCtx, uint8_t Rn, uint8_t imm, bool op);
	void (*i32_cdp2_T2)(void *aCtx, uint8_t coproc, uint8_t CRd, uint8_t CRn, uint8_t CRm, uint8_t opcA, uint8_t opcB);
	void (*i32_cdp_T1)(void *aCtx, uint8_t coproc, uint8_t CRd, uint8_t CRn, uint8_t CRm, uint8_t opcA, uint8_t opcB);
	void (*i32_clrex_T1)(void *aCtx);
	void (*i32_clz_T1)(void *aCtx, uint8_t Rd, uint8_t Rm, uint8_t Rmx);
	void (*i32_cmn_imm_T1)(void *aCtx, uint8_t Rn, int32_t imm);
	void (*i16_cmn_reg_T1)(void *aCtx, uint8_t Rn, uint8_t Rm);
	void (*i32_cmn_reg_T2)(void *aCtx, uint8_t Rn, uint8_t Rm, int32_t imm, uint8_t type);
	void (*i16_cmp_imm_T1)(void *aCtx, uint8_t Rn, uint8_t imm);
	void (*i32_cmp_imm_T2)(void *aCtx, uint8_t Rn, int32_t imm);
	void (*i16_cmp_reg_T1)(void *aCtx, uint8_t Rn, uint8_t Rm);
	void (*i16_cmp_reg_T2)(void *aCtx, uint8_t Rn, uint8_t Rm);
	void (*i32_cmp_reg_T3)(void *aCtx, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type);
	void (*i16_cps_T1)(void *aCtx, bool imm, bool F, bool I);
	void (*i32_dbg_T1)(void *aCtx, uint8_t option);
	void (*i32_dmb_T1)(void *aCtx, uint8_t option);
	void (*i32_dsb_T1)(void *aCtx, uint8_t option);
	void (*i32_eor_imm_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S);
	void (*i16_eor_reg_T1)(void *aCtx, uint8_t Rdn, uint8_t Rm);
	void (*i32_eor_reg_T2)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S);
	void (*i32_isb_T1)(void *aCtx, uint8_t option);
	void (*i16_it_T1)(void *aCtx, uint8_t firstcond, uint8_t mask);
	void (*i32_ldc2_imm_T2)(void *aCtx, uint8_t Rn, uint8_t imm, uint8_t coproc, uint8_t CRd, bool D, bool P, bool U, bool W);
	void (*i32_ldc2_lit_T2)(void *aCtx, uint8_t imm, uint8_t coproc, uint8_t CRd, bool D, bool P, bool U, bool W);
	void (*i32_ldc_imm_T1)(void *aCtx, uint8_t Rn, uint8_t imm, uint8_t coproc, uint8_t CRd, bool D, bool P, bool U, bool W);
	void (*i32_ldc_lit_T1)(void *aCtx, uint8_t imm, uint8_t coproc, uint8_t CRd, bool D, bool P, bool U, bool W);
	void (*i16_ldm_T1)(void *aCtx, uint8_t Rn, uint8_t register_list);
	void (*i32_ldm_T2)(void *aCtx, uint8_t Rn, uint16_t register_list, bool P, bool M, bool W);
	void (*i32_ldmdb_T1)(void *aCtx, uint8_t Rn, uint16_t register_list, bool P, bool M, bool W);
	void (*i16_ldr_imm_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t imm);
	void (*i16_ldr_imm_T2)(void *aCtx, uint8_t Rt, uint8_t imm);
	void (*i32_ldr_imm_T3)(void *aCtx, uint8_t Rt, uint8_t Rn, uint16_t imm);
	void (*i32_ldr_imm_T4)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t imm, bool P, bool U, bool W);
	void (*i16_ldr_lit_T1)(void *aCtx, uint8_t Rt, uint8_t imm);
	void (*i32_ldr_lit_T2)(void *aCtx, uint8_t Rt, uint16_t imm, bool U);
	void (*i16_ldr_reg_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t Rm);
	void (*i32_ldr_reg_T2)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t Rm, uint8_t imm);
	void (*i16_ldrb_imm_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t imm);
	void (*i32_ldrb_imm_T2)(void *aCtx, uint8_t Rt, uint8_t Rn, uint16_t imm);
	void (*i32_ldrb_imm_T3)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t imm, bool P, bool U, bool W);
	void (*i32_ldrb_lit_T1)(void *aCtx, uint8_t Rt, uint16_t imm, bool U);
	void (*i16_ldrb_reg_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t Rm);
	void (*i32_ldrb_reg_T2)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t Rm, uint8_t imm);
	void (*i32_ldrbt_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t imm);
	void (*i32_ldrd_imm_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t Rtx, uint8_t imm, bool P, bool U, bool W);
	void (*i32_ldrd_lit_T1)(void *aCtx, uint8_t Rt, uint8_t Rtx, uint8_t imm, bool P, bool U, bool W);
	void (*i32_ldrex_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t imm);
	void (*i32_ldrexb_T1)(void *aCtx, uint8_t Rt, uint8_t Rn);
	void (*i32_ldrexh_T1)(void *aCtx, uint8_t Rt, uint8_t Rn);
	void (*i16_ldrh_imm_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t imm);
	void (*i32_ldrh_imm_T2)(void *aCtx, uint8_t Rt, uint8_t Rn, uint16_t imm);
	void (*i32_ldrh_imm_T3)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t imm, bool P, bool U, bool W);
	void (*i32_ldrh_lit_T1)(void *aCtx, uint8_t Rt, uint16_t imm, bool U);
	void (*i16_ldrh_reg_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t Rm);
	void (*i32_ldrh_reg_T2)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t Rm, uint8_t imm);
	void (*i32_ldrht_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t imm);
	void (*i32_ldrsb_imm_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint16_t imm);
	void (*i32_ldrsb_imm_T2)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t imm, bool P, bool U, bool W);
	void (*i32_ldrsb_lit_T1)(void *aCtx, uint8_t Rt, uint16_t imm, bool U);
	void (*i16_ldrsb_reg_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t Rm);
	void (*i32_ldrsb_reg_T2)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t Rm, uint8_t imm);
	void (*i32_ldrsbt_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t imm);
	void (*i32_ldrsh_imm_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint16_t imm);
	void (*i32_ldrsh_imm_T2)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t imm, bool P, bool U, bool W);
	void (*i32_ldrsh_lit_T1)(void *aCtx, uint8_t Rt, uint16_t imm, bool U);
	void (*i16_ldrsh_reg_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t Rm);
	void (*i32_ldrsh_reg_T2)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t Rm, uint8_t imm);
	void (*i32_ldrsh_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t imm);
	void (*i32_ldrt_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t imm);
	void (*i16_lsl_imm_T1)(void *aCtx, uint8_t Rd, uint8_t Rm, uint8_t imm);
	void (*i32_lsl_imm_T2)(void *aCtx, uint8_t Rd, uint8_t Rm, uint8_t imm, bool S);
	void (*i16_lsl_reg_T1)(void *aCtx, uint8_t Rdn, uint8_t Rm);
	void (*i32_lsl_reg_T2)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, bool S);
	void (*i16_lsr_imm_T1)(void *aCtx, uint8_t Rd, uint8_t Rm, uint8_t imm);
	void (*i32_lsr_imm_T2)(void *aCtx, uint8_t Rd, uint8_t Rm, uint8_t imm, bool S);
	void (*i16_lsr_reg_T1)(void *aCtx, uint8_t Rdn, uint8_t Rm);
	void (*i32_lsr_reg_T2)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, bool S);
	void (*i32_mcr2_T2)(void *aCtx, uint8_t Rt, uint8_t coproc, uint8_t CRn, uint8_t CRm, uint8_t opcA, uint8_t opcB);
	void (*i32_mcr_T1)(void *aCtx, uint8_t Rt, uint8_t coproc, uint8_t CRn, uint8_t CRm, uint8_t opcA, uint8_t opcB);
	void (*i32_mcrr2_T2)(void *aCtx, uint8_t Rt, uint8_t coproc, uint8_t CRm, uint8_t opcA, uint8_t RtB);
	void (*i32_mcrr_T1)(void *aCtx, uint8_t Rt, uint8_t coproc, uint8_t CRm, uint8_t opcA, uint8_t RtB);
	void (*i32_mla_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t Ra);
	void (*i32_mls_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t Ra);
	void (*i16_mov_imm_T1)(void *aCtx, uint8_t Rd, uint8_t imm);
	void (*i32_mov_imm_T2)(void *aCtx, uint8_t Rd, int32_t imm, bool S);
	void (*i32_mov_imm_T3)(void *aCtx, uint8_t Rd, uint16_t imm);
	void (*i16_mov_reg_T1)(void *aCtx, uint8_t Rd, uint8_t Rm);
	void (*i16_mov_reg_T2)(void *aCtx, uint8_t Rd, uint8_t Rm);
	void (*i32_mov_reg_T3)(void *aCtx, uint8_t Rd, uint8_t Rm, bool S);
	void (*i32_movt_T1)(void *aCtx, uint8_t Rd, uint16_t imm);
	void (*i32_mrc2_T2)(void *aCtx, uint8_t Rt, uint8_t coproc, uint8_t Crn, uint8_t CRm, uint8_t opcA, uint8_t opcB);
	void (*i32_mrc_T1)(void *aCtx, uint8_t Rt, uint8_t coproc, uint8_t Crn, uint8_t CRm, uint8_t opcA, uint8_t opcB);
	void (*i32_mrrc2_T2)(void *aCtx, uint8_t Rt, uint8_t coproc, uint8_t CRm, uint8_t opcA, uint8_t RtB);
	void (*i32_mrrc_T1)(void *aCtx, uint8_t Rt, uint8_t coproc, uint8_t CRm, uint8_t opcA, uint8_t RtB);
	void (*i32_mrs_T1)(void *aCtx, uint8_t Rd, uint8_t SYSm);
	void (*i32_msr_T1)(void *aCtx, uint8_t Rn, uint8_t mask, uint8_t SYSm);
	void (*i16_mul_T1)(void *aCtx, uint8_t Rdm, uint8_t Rn);
	void (*i32_mul_T2)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_mvn_imm_T1)(void *aCtx, uint8_t Rd, int32_t imm, bool S);
	void (*i16_mvn_reg_T1)(void *aCtx, uint8_t Rd, uint8_t Rm);
	void (*i32_mvn_reg_T2)(void *aCtx, uint8_t Rd, uint8_t Rm, int32_t imm, uint8_t type, bool S);
	void (*i16_nop_T1)(void *aCtx);
	void (*i32_nop_T2)(void *aCtx);
	void (*i32_orn_imm_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint16_t imm, bool S);
	void (*i32_orn_reg_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S);
	void (*i32_orr_imm_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S);
	void (*i16_orr_reg_T1)(void *aCtx, uint8_t Rdn, uint8_t Rm);
	void (*i32_orr_reg_T2)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S);
	void (*i32_pkhbt_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, bool tb, bool S, bool T);
	void (*i32_pld_imm_T1)(void *aCtx, uint8_t Rn, uint16_t imm);
	void (*i32_pld_imm_T2)(void *aCtx, uint8_t Rn, uint8_t imm);
	void (*i32_pld_lit_T1)(void *aCtx, uint16_t imm, bool U);
	void (*i32_pld_reg_T1)(void *aCtx, uint8_t Rn, uint8_t Rm, uint8_t imm);
	void (*i32_pld_immlit_T1)(void *aCtx, uint8_t Rn, uint16_t imm);
	void (*i32_pld_immlit_T2)(void *aCtx, uint8_t Rn, uint8_t imm);
	void (*i32_pld_immlit_T3)(void *aCtx, uint16_t imm, bool U);
	void (*i16_pop_T1)(void *aCtx, uint8_t register_list, bool P);
	void (*i32_pop_T2)(void *aCtx, uint16_t register_list, bool P, bool M);
	void (*i32_pop_T3)(void *aCtx, uint8_t Rt);
	void (*i16_push_T1)(void *aCtx, uint8_t register_list, bool M);
	void (*i32_push_T2)(void *aCtx, uint16_t register_list, bool M);
	void (*i32_push_T3)(void *aCtx, uint8_t Rt);
	void (*i32_qadd16_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_qadd8_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_qadd_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_qasx_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_qdadd_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_qdsub_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_qsax_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_qsub16_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_qsub8_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_qsub_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_rbit_T1)(void *aCtx, uint8_t Rd, uint8_t Rm);
	void (*i16_rev16_T1)(void *aCtx, uint8_t Rd, uint8_t Rm);
	void (*i32_rev16_T2)(void *aCtx, uint8_t Rd, uint8_t Rm);
	void (*i16_rev_T1)(void *aCtx, uint8_t Rd, uint8_t Rm);
	void (*i32_rev_T2)(void *aCtx, uint8_t Rd, uint8_t Rm, uint8_t Rmx);
	void (*i16_revsh_T1)(void *aCtx, uint8_t Rd, uint8_t Rm);
	void (*i32_revsh_T2)(void *aCtx, uint8_t Rd, uint8_t Rm);
	void (*i32_ror_imm_T1)(void *aCtx, uint8_t Rd, uint8_t Rm, uint8_t imm, bool S);
	void (*i16_ror_reg_T1)(void *aCtx, uint8_t Rdn, uint8_t Rm);
	void (*i32_ror_reg_T2)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, bool S);
	void (*i32_rrx_T1)(void *aCtx, uint8_t Rd, uint8_t Rm, bool S);
	void (*i16_rsb_imm_T1)(void *aCtx, uint8_t Rd, uint8_t Rn);
	void (*i32_rsb_imm_T2)(void *aCtx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S);
	void (*i32_rsb_reg_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S);
	void (*i32_sadd16_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_sadd8_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_sasx_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_sbc_imm_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S);
	void (*i16_sbc_reg_T1)(void *aCtx, uint8_t Rdn, uint8_t Rm);
	void (*i32_sbc_reg_T2)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S);
	void (*i32_sbfx_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t imm, uint8_t width);
	void (*i32_sdiv_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_sel_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i16_sev_T1)(void *aCtx);
	void (*i32_sev_T2)(void *aCtx);
	void (*i32_shadd16_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_shadd8_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_shasx_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_shsax_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_shsub16_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_shsub8_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_smlabb_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t Ra, bool N, bool M);
	void (*i32_smlad_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t Ra, bool M);
	void (*i32_smlal_T1)(void *aCtx, uint8_t Rn, uint8_t Rm, uint8_t RdLo, uint8_t RdHi);
	void (*i32_smlalbb_T1)(void *aCtx, uint8_t Rn, uint8_t Rm, uint8_t RdLo, uint8_t RdHi, bool N, bool M);
	void (*i32_smlald_T1)(void *aCtx, uint8_t Rn, uint8_t Rm, uint8_t RdLo, uint8_t RdHi, bool M);
	void (*i32_smlawb_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t Ra, bool M);
	void (*i32_smlsd_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t Ra, bool M);
	void (*i32_smlsld_T1)(void *aCtx, uint8_t Rn, uint8_t Rm, uint8_t RdLo, uint8_t RdHi, bool M);
	void (*i32_smmla_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t Ra, bool R);
	void (*i32_smmls_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t Ra, bool R);
	void (*i32_smmul_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, bool R);
	void (*i32_smuad_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, bool M);
	void (*i32_smulbb_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, bool N, bool M);
	void (*i32_smull_T1)(void *aCtx, uint8_t Rn, uint8_t Rm, uint8_t RdLo, uint8_t RdHi);
	void (*i32_smulwb_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, bool M);
	void (*i32_smusd_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, bool M);
	void (*i32_ssat16_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t satimm);
	void (*i32_ssat_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, bool shift, uint8_t imm, uint8_t satimm);
	void (*i32_ssax_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_ssub16_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_ssub8_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_stc2_T2)(void *aCtx, uint8_t Rn, uint8_t imm, uint8_t coproc, uint8_t CRd, bool P, bool N, bool U, bool W);
	void (*i32_stc_T1)(void *aCtx, uint8_t Rn, uint8_t imm, uint8_t coproc, uint8_t CRd, bool P, bool N, bool U, bool W);
	void (*i16_stm_T1)(void *aCtx, uint8_t Rn, uint8_t register_list);
	void (*i32_stm_T2)(void *aCtx, uint8_t Rn, uint16_t register_list, bool M, bool W);
	void (*i32_stmdb_T1)(void *aCtx, uint8_t Rn, uint16_t register_list, bool M, bool W);
	void (*i16_str_imm_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t imm);
	void (*i16_str_imm_T2)(void *aCtx, uint8_t Rt, uint8_t imm);
	void (*i32_str_imm_T3)(void *aCtx, uint8_t Rt, uint8_t Rn, uint16_t imm);
	void (*i32_str_imm_T4)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t imm, bool P, bool U, bool W);
	void (*i16_str_reg_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t Rm);
	void (*i32_str_reg_T2)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t Rm, uint8_t imm);
	void (*i16_strb_imm_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t imm);
	void (*i32_strb_imm_T2)(void *aCtx, uint8_t Rt, uint8_t Rn, uint16_t imm);
	void (*i32_strb_imm_T3)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t imm, bool P, bool U, bool W);
	void (*i16_strb_reg_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t Rm);
	void (*i32_strb_reg_T2)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t Rm, uint8_t imm);
	void (*i32_strb_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t imm);
	void (*i32_strd_imm_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t Rtx, uint8_t imm, bool P, bool U, bool W);
	void (*i32_strex_T1)(void *aCtx, uint8_t Rd, uint8_t Rt, uint8_t Rn, uint8_t imm);
	void (*i32_strexb_T1)(void *aCtx, uint8_t Rd, uint8_t Rt, uint8_t Rn);
	void (*i32_strexh_T1)(void *aCtx, uint8_t Rd, uint8_t Rt, uint8_t Rn);
	void (*i16_strh_imm_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t imm);
	void (*i32_strh_imm_T2)(void *aCtx, uint8_t Rt, uint8_t Rn, uint16_t imm);
	void (*i32_strh_imm_T3)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t imm, bool P, bool U, bool W);
	void (*i16_strh_reg_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t Rm);
	void (*i32_strh_reg_T2)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t Rm, uint8_t imm);
	void (*i32_strht_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t imm);
	void (*i32_strt_T1)(void *aCtx, uint8_t Rt, uint8_t Rn, uint8_t imm);
	void (*i16_sub_SPimm_T1)(void *aCtx, uint8_t imm);
	void (*i32_sub_SPimm_T2)(void *aCtx, uint8_t Rd, uint16_t imm, bool S);
	void (*i32_sub_SPimm_T3)(void *aCtx, uint8_t Rd, uint16_t imm);
	void (*i32_sub_SPreg_T1)(void *aCtx, uint8_t Rd, uint8_t Rm, uint8_t imm, uint8_t type, bool S);
	void (*i16_sub_imm_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t imm);
	void (*i16_sub_imm_T2)(void *aCtx, uint8_t Rdn, uint8_t imm);
	void (*i32_sub_imm_T3)(void *aCtx, uint8_t Rd, uint8_t Rn, int32_t imm, bool S);
	void (*i32_sub_imm_T4)(void *aCtx, uint8_t Rd, uint8_t Rn, uint16_t imm);
	void (*i16_sub_reg_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_sub_reg_T2)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type, bool S);
	void (*i16_svc_T1)(void *aCtx, uint8_t imm);
	void (*i32_sxtab_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t rotate);
	void (*i32_sxtab16_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t rotate);
	void (*i32_sxtah_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t rotate);
	void (*i32_sxtb16_T1)(void *aCtx, uint8_t Rd, uint8_t Rm, uint8_t rotate);
	void (*i16_sxtb_T1)(void *aCtx, uint8_t Rd, uint8_t Rm);
	void (*i32_sxtb_T2)(void *aCtx, uint8_t Rd, uint8_t Rm, uint8_t rotate);
	void (*i16_sxth_T1)(void *aCtx, uint8_t Rd, uint8_t Rm);
	void (*i32_sxth_T2)(void *aCtx, uint8_t Rd, uint8_t Rm, uint8_t rotate);
	void (*i32_tbb_T1)(void *aCtx, uint8_t Rn, uint8_t Rm, bool H);
	void (*i32_teq_imm_T1)(void *aCtx, uint8_t Rn, uint16_t imm);
	void (*i32_teq_reg_T1)(void *aCtx, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type);
	void (*i32_tst_imm_T1)(void *aCtx, uint8_t Rn, int32_t imm);
	void (*i16_tst_reg_T1)(void *aCtx, uint8_t Rn, uint8_t Rm);
	void (*i32_tst_reg_T2)(void *aCtx, uint8_t Rn, uint8_t Rm, uint8_t imm, uint8_t type);
	void (*i32_uadd16_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_uadd8_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_uasx_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_ubfx_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t imm, uint8_t width);
	void (*i16_udf_T1)(void *aCtx, uint8_t imm);
	void (*i32_udf_T2)(void *aCtx, uint16_t imm);
	void (*i32_udiv_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_uhadd16_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_uhadd8_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_uhasx_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_uhsax_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_uhsub16_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_uhsub8_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_umaal_T1)(void *aCtx, uint8_t Rn, uint8_t Rm, uint8_t RdLo, uint8_t RdHi);
	void (*i32_umlal_T1)(void *aCtx, uint8_t Rn, uint8_t Rm, uint8_t RdLo, uint8_t RdHi);
	void (*i32_umull_T1)(void *aCtx, uint8_t Rn, uint8_t Rm, uint8_t RdLo, uint8_t RdHi);
	void (*i32_uqadd16_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_uqadd8_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_uqasx_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_uqsax_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_uqsub16_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_uqsub8_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_usad8_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_usada8_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t Ra);
	void (*i32_usat16_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t satimm);
	void (*i32_usat_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, bool shift, uint8_t imm, uint8_t satimm);
	void (*i32_usax_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_usub16_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_usub8_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm);
	void (*i32_uxtab16_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t rotate);
	void (*i32_uxtab_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t rotate);
	void (*i32_uxtah_T1)(void *aCtx, uint8_t Rd, uint8_t Rn, uint8_t Rm, uint8_t rotate);
	void (*i32_uxtb16_T1)(void *aCtx, uint8_t Rd, uint8_t Rm, uint8_t rotate);
	void (*i16_uxtb_T1)(void *aCtx, uint8_t Rd, uint8_t Rm);
	void (*i32_uxtb_T2)(void *aCtx, uint8_t Rd, uint8_t Rm, uint8_t rotate);
	void (*i16_uxth_T1)(void *aCtx, uint8_t Rd, uint8_t Rm);
	void (*i32_uxth_T2)(void *aCtx, uint8_t Rd, uint8_t Rm, uint8_t rotate);
	void (*i32_vabs_T1)(void *aCtx, uint8_t Vd, uint8_t Vm, bool D, bool M, bool sz);
	void (*i32_vadd_T1)(void *aCtx, uint8_t Vd, uint8_t Vn, uint8_t Vm, bool D, bool N, bool M, bool sz);
	void (*i32_vcmp_T1)(void *aCtx, uint8_t Vd, uint8_t Vm, bool D, bool E, bool M, bool sz);
	void (*i32_vcmp_T2)(void *aCtx, uint8_t Vd, bool D, bool E, bool sz);
	void (*i32_vcvt_dpsp_T1)(void *aCtx, uint8_t Vd, uint8_t Vm, bool D, bool M, bool sz);
	void (*i32_vcvt_fltfpt_T1)(void *aCtx, uint8_t Vd, uint8_t imm, bool op, bool D, bool U, bool sf, bool sx);
	void (*i32_vcvt_fltint_T1)(void *aCtx, uint8_t Vd, uint8_t Vm, bool op, uint8_t RM, bool D, bool M, bool sz);
	void (*i32_vcvta_T1)(void *aCtx, uint8_t Vd, uint8_t Vm, bool op, uint8_t RM, bool D, bool M, bool sz);
	void (*i32_vcvtb_T1)(void *aCtx, uint8_t Vd, uint8_t Vm, bool op, bool D, bool M, bool T, bool sz);
	void (*i32_vdiv_T1)(void *aCtx, uint8_t Vd, uint8_t Vn, uint8_t Vm, bool D, bool N, bool M, bool sz);
	void (*i32_vfma_T1)(void *aCtx, uint8_t Vd, uint8_t Vn, uint8_t Vm, bool op, bool D, bool N, bool M, bool sz);
	void (*i32_vfnma_T1)(void *aCtx, uint8_t Vd, uint8_t Vn, uint8_t Vm, bool op, bool D, bool N, bool M, bool sz);
	void (*i32_vldm_T1)(void *aCtx, uint8_t Rn, uint8_t Vd, uint8_t imm, bool D, bool P, bool U, bool W);
	void (*i32_vldm_T2)(void *aCtx, uint8_t Rn, uint8_t Vd, uint8_t imm, bool D, bool P, bool U, bool W);
	void (*i32_vldr_T1)(void *aCtx, uint8_t Rn, uint8_t Vd, uint8_t imm, bool D, bool U);
	void (*i32_vldr_T2)(void *aCtx, uint8_t Rn, uint8_t Vd, uint8_t imm, bool D, bool U);
	void (*i32_vmaxnm_T1)(void *aCtx, uint8_t Vd, uint8_t Vn, uint8_t Vm, bool op, bool D, bool N, bool M, bool sz);
	void (*i32_vmla_T1)(void *aCtx, uint8_t Vd, uint8_t Vn, uint8_t Vm, bool op, bool D, bool N, bool M, bool sz);
	void (*i32_vmov_2arm2sp_T1)(void *aCtx, uint8_t Rt, uint8_t Rtx, uint8_t Vm, bool op, bool M);
	void (*i32_vmov_2armdw_T1)(void *aCtx, uint8_t Rt, uint8_t Rtx, uint8_t Vm, bool op, bool M);
	void (*i32_vmov_armscl_T1)(void *aCtx, uint8_t Rt, uint8_t Vd, bool H, bool D);
	void (*i32_vmov_armsp_T1)(void *aCtx, uint8_t Rt, uint8_t Vn, bool op, bool N);
	void (*i32_vmov_imm_T1)(void *aCtx, uint8_t Vd, uint8_t imm, bool D, bool sz);
	void (*i32_vmov_reg_T1)(void *aCtx, uint8_t Vd, uint8_t Vm, bool D, bool M, bool sz);
	void (*i32_vmov_sclarm_T1)(void *aCtx, uint8_t Rt, uint8_t Vd, bool H, bool D);
	void (*i32_vmrs_T1)(void *aCtx, uint8_t Rt);
	void (*i32_vmsr_T1)(void *aCtx, uint8_t Rt);
	void (*i32_vmul_T1)(void *aCtx, uint8_t Vd, uint8_t Vn, uint8_t Vm, bool D, bool N, bool M, bool sz);
	void (*i32_vneg_T1)(void *aCtx, uint8_t Vd, uint8_t Vm, bool D, bool M, bool sz);
	void (*i32_vnmla_T1)(void *aCtx, uint8_t Vd, uint8_t Vn, uint8_t Vm, bool op, bool D, bool N, bool M, bool sz);
	void (*i32_vnmla_T2)(void *aCtx, uint8_t Vd, uint8_t Vn, uint8_t Vm, bool D, bool N, bool M, bool sz);
	void (*i32_vpop_T1)(void *aCtx, uint8_t Vd, uint8_t imm, bool D);
	void (*i32_vpop_T2)(void *aCtx, uint8_t Vd, uint8_t imm, bool D);
	void (*i32_vpush_T1)(void *aCtx, uint8_t Vd, uint8_t imm, bool D);
	void (*i32_vpush_T2)(void *aCtx, uint8_t Vd, uint8_t imm, bool D);
	void (*i32_vrinta_T1)(void *aCtx, uint8_t Vd, uint8_t Vm, uint8_t RM, bool D, bool M, bool sz);
	void (*i32_vrintx_T1)(void *aCtx, uint8_t Vd, uint8_t Vm, bool D, bool M, bool sz);
	void (*i32_vrintz_T1)(void *aCtx, uint8_t Vd, uint8_t Vm, bool op, bool D, bool M, bool sz);
	void (*i32_vsel_T1)(void *aCtx, uint8_t Vd, uint8_t Vn, uint8_t Vm, uint8_t cond, bool D, bool N, bool M, bool sz);
	void (*i32_vsqrt_T1)(void *aCtx, uint8_t Vd, uint8_t Vm, bool D, bool M, bool sz);
	void (*i32_vstm_T1)(void *aCtx, uint8_t Rn, uint8_t Vd, uint8_t imm, bool D, bool P, bool U, bool W);
	void (*i32_vstm_T2)(void *aCtx, uint8_t Rn, uint8_t Vd, uint8_t imm, bool D, bool P, bool U, bool W);
	void (*i32_vstr_T1)(void *aCtx, uint8_t Rn, uint8_t Vd, uint8_t imm, bool D, bool U);
	void (*i32_vstr_T2)(void *aCtx, uint8_t Rn, uint8_t Vd, uint8_t imm, bool D, bool U);
	void (*i32_vsub_T1)(void *aCtx, uint8_t Vd, uint8_t Vn, uint8_t Vm, bool D, bool N, bool M, bool sz);
	void (*i16_wfe_T1)(void *aCtx);
	void (*i32_wfe_T2)(void *aCtx);
	void (*i16_wfi_T1)(void *aCtx);
	void (*i32_wfi_T2)(void *aCtx);
	void (*i16_yield_T1)(void *aCtx);
	void (*i32_yield_T2)(void *aCtx);
};

int decode_insn(void *vctx, uint32_t opcode, const struct decoding_handler_t *handler, FILE *insn_debugging_info);

#endif
// vim: set filetype=c:
