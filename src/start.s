.section .text.boot
.globl _start
_start:
    mrs x0,mpidr_el1
    mov x1,#0xC1000000
    bic x0,x0,x1
    cbz x0,master
    b hang
master:
    mov sp,#0x08000000
        // clear bss
    ldr     x5, =__bss_start
    ldr     w6, =__bss_size
1:  cbz     w6, master
    str     xzr, [x5], #8
    sub     w6, w6, #1
    cbnz    w6, 1b
    bl main
hang:
    wfe
    b hang