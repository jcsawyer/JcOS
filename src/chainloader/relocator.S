.section .text.chainloader_relocator, "ax"

.global chainloader_relocator_start
.global chainloader_relocator
.global chainloader_relocator_end

chainloader_relocator_start:
chainloader_relocator:
	cbz	x2, 2f
1:
	ldrb	w5, [x1], #1
	strb	w5, [x0], #1
	subs	x2, x2, #1
	b.ne	1b
2:
	mov	x0, x4
	br	x3

chainloader_relocator_end:
