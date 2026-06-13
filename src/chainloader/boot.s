// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2021-2022 Andre Richter <andre.o.richter@gmail.com>

.macro ADR_REL register, symbol
	adrp	\register, \symbol
	add	\register, \register, #:lo12:\symbol
.endm

.section .text._start

_start:
	mrs	x1, MPIDR_EL1
	and	x1, x1, 0b11
	ldr	x2, BOOT_CORE_ID
	cmp	x1, x2
	b.ne	.L_parking_loop

	mrs     x0, CurrentEL
	and     x0, x0, #12

	cmp     x0, #12
	bne     ._L_EL2
	mov     x2, #0x5b1
	msr     scr_el3, x2
	mov     x2, #0x3c9
	msr     spsr_el3, x2
	adr     x2, ._L_EL2
	msr     elr_el3, x2
	eret

._L_EL2:
	ADR_REL	x0, __bss_start
	ADR_REL x1, __bss_end_exclusive

.L_bss_init_loop:
	cmp	x0, x1
	b.eq	.L_prepare_cpp
	stp	xzr, xzr, [x0], #16
	b	.L_bss_init_loop

.L_prepare_cpp:
	ADR_REL	x0, __boot_core_stack_end_exclusive
	mov	sp, x0

	ADR_REL	x1, ARCH_TIMER_COUNTER_FREQUENCY
	mrs	x2, CNTFRQ_EL0
	cmp	x2, xzr
	b.eq	.L_parking_loop
	str	w2, [x1]

	b	_start_cpp

.L_parking_loop:
	wfe
	b	.L_parking_loop

.size	_start, . - _start
.type	_start, function
.global	_start
