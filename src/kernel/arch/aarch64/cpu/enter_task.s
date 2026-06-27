.global enter_task
.type enter_task, %function

enter_task:
    mov     sp, x1
    mov     x29, xzr
    mov     x30, xzr
    br      x0
