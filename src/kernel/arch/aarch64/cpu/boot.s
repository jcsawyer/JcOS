// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2021-2022 Andre Richter <andre.o.richter@gmail.com>

//--------------------------------------------------------------------------------------------------
// Definitions
//--------------------------------------------------------------------------------------------------

// Load the address of a symbol into a register, PC-relative.
//
// The symbol must lie within +/- 4 GiB of the Program Counter.
//
// # Resources
//
// - https://sourceware.org/binutils/docs-2.36/as/AArch64_002dRelocations.html
.macro ADR_REL register, symbol
	adrp	\register, \symbol
	add	\register, \register, #:lo12:\symbol
.endm

// Load the address of a symbol into a register, absolute.
.macro ADR_ABS register, symbol
	movz	\register, #:abs_g3:\symbol
	movk	\register, #:abs_g2_nc:\symbol
	movk	\register, #:abs_g1_nc:\symbol
	movk	\register, #:abs_g0_nc:\symbol
.endm

//--------------------------------------------------------------------------------------------------
// Public Code
//--------------------------------------------------------------------------------------------------
.section .text._start

//------------------------------------------------------------------------------
// fn _start()
//------------------------------------------------------------------------------
_start:
	mov	x19, x0

	// Only proceed on the boot core. Park it otherwise.
	mrs	x20, MPIDR_EL1
	and	x20, x20, 0b11
	ldr	x2, BOOT_CORE_ID
	cmp	x20, x2
	b.ne	.L_secondary_parking_loop

	// If execution reaches here, it is the boot core.
	// set up EL1
    mrs     x0, CurrentEL
    and     x0, x0, #12 // clear reserved bits

    // running at EL3?
    cmp     x0, #12
    bne     ._L_EL2
    // should never be executed, just for completeness
    mov     x2, #0x5b1
    msr     scr_el3, x2
    mov     x2, #0x3c9
    msr     spsr_el3, x2
    adr     x2, ._L_EL2
    msr     elr_el3, x2
    eret

._L_EL2:
	// Initialize DRAM.
	ADR_REL	x0, __bss_start
	ADR_REL x1, __bss_end_exclusive

.L_bss_init_loop:
	cmp	x0, x1
	b.eq	.L_prepare_cpp
	stp	xzr, xzr, [x0], #16
	b	.L_bss_init_loop

	// Prepare the jump to cpp code.
.L_prepare_cpp:
	// Load the base address of the kernel's translation tables.
	ldr	x0, PHYS_KERNEL_TABLES_BASE_ADDR // patched post-link

	// Load the linked virtual addresses for EL1.
	ADR_ABS	x1, __boot_core_stack_end_exclusive
	ADR_ABS	x2, kernel_init
	mov	x3, x19

	// Keep using the PC-relative physical stack while still running with the MMU off.
	ADR_REL	x4, __boot_core_stack_end_exclusive
	mov	sp, x4

	// Read the CPU's timer counter frequency and store it in ARCH_TIMER_COUNTER_FREQUENCY.
	// Abort if the frequency read back as 0.
	ADR_REL	x4, ARCH_TIMER_COUNTER_FREQUENCY // provided by aarch64/time.rs
	mrs	x5, CNTFRQ_EL0
	cmp	x5, xzr
	b.eq	.L_parking_loop
	str	w5, [x4]

	// Jump to cpp code. x0, x1, x2 and x3 hold the function arguments for _start_cpp().
	b	_start_cpp

	// Park secondary cores until their release flag is set.
.L_secondary_parking_loop:
	cmp	x20, #16
	b.hs	.L_parking_loop
	wfe
	ADR_REL	x3, BOOT_CORE_RELEASE_FLAGS
	ldrb	w4, [x3, x20]
	cbz	w4, .L_secondary_parking_loop

	// Load the base address of the kernel's translation tables.
	ldr	x0, PHYS_KERNEL_TABLES_BASE_ADDR // patched post-link

	// Compute the per-core stack end for the released core.
	mov	x5, x20
	add	x5, x5, #1
	lsl	x5, x5, #16

	// Load the linked virtual stack and entry addresses for EL1.
	ADR_ABS	x1, RELEASED_CORE_BOOT_STACKS
	add	x1, x1, x5
	ADR_ABS	x2, _released_core_main
	mov	x3, x20

	// Keep using the PC-relative physical stack while still running with the MMU off.
	ADR_REL	x4, RELEASED_CORE_BOOT_STACKS
	add	x4, x4, x5
	mov	sp, x4

	b	_start_released_core

	// Infinitely wait for events (aka "park the core").
.L_parking_loop:
	wfe
	b	.L_parking_loop

.size	_start, . - _start
.type	_start, function
.global	_start
