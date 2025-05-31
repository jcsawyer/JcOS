.global enter_task
.type enter_task, %function

enter_task:
    msr     elr_el1, x0          // Jump to x0
    mov     x1, #0x345           // EL1h, IQR enabled, others masked
    msr     spsr_el1, x1         // Set PSTATE on exception return
    eret