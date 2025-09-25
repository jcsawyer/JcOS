    .text
    .global syscall0
    .global syscall1
    .global syscall2
    .global syscall3
    .global syscall4
    .global syscall5
    .global syscall6

    .type syscall0, %function
syscall0:
    // x0 = syscall_num
    mov    x8, x0
    svc    #0
    ret

    .type syscall1, %function
syscall1:
    // x0 = syscall_num, x1 = arg0
    mov    x8, x0
    mov    x0, x1
    svc    #0
    ret

    .type syscall2, %function
syscall2:
    // x0 = syscall_num, x1 = arg0, x2 = arg1
    mov    x8, x0
    mov    x0, x1
    mov    x1, x2
    svc    #0
    ret

    .type syscall3, %function
syscall3:
    // x0 = syscall_num, x1 = arg0, x2 = arg1, x3 = arg2
    mov    x8, x0
    mov    x0, x1
    mov    x1, x2
    mov    x2, x3
    svc    #0
    ret

    .type syscall4, %function
syscall4:
    // x0 = syscall_num, x1..x4 = args
    mov    x8, x0
    mov    x0, x1
    mov    x1, x2
    mov    x2, x3
    mov    x3, x4
    svc    #0
    ret

    .type syscall5, %function
syscall5:
    // x0 = syscall_num, x1..x5 = args
    mov    x8, x0
    mov    x0, x1
    mov    x1, x2
    mov    x2, x3
    mov    x3, x4
    mov    x4, x5
    svc    #0
    ret

    .type syscall6, %function
syscall6:
    // x0 = syscall_num, x1..x6 = args
    mov    x8, x0
    mov    x0, x1
    mov    x1, x2
    mov    x2, x3
    mov    x3, x4
    mov    x4, x5
    mov    x5, x6
    svc    #0
    ret
