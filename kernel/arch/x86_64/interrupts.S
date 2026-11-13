/* HYBRID OS X x86-64 exception entry stubs. */
.section .text
.code64
.extern hybrid_exception_dispatch

.macro ISR_NOERR number
.global hybrid_isr_\number
.type hybrid_isr_\number, @function
hybrid_isr_\number:
    pushq $0
    pushq $\number
    jmp hybrid_isr_common
.endm

.macro ISR_ERR number
.global hybrid_isr_\number
.type hybrid_isr_\number, @function
hybrid_isr_\number:
    pushq $\number
    jmp hybrid_isr_common
.endm

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_ERR   21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_ERR   29
ISR_ERR   30
ISR_NOERR 31

.type hybrid_isr_common, @function
hybrid_isr_common:
    /* Preserve all general-purpose registers before entering C code. */
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rbp
    pushq %rsi
    pushq %rdi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    /* Stack layout after 15 pushes: vector at +120, error code at +128. */
    movq 120(%rsp), %rdi
    movq 128(%rsp), %rsi
    movq %rsp, %r12
    andq $-16, %rsp
    call hybrid_exception_dispatch

    /* The C dispatcher is noreturn; retain a safe restore path regardless. */
    movq %r12, %rsp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rdi
    popq %rsi
    popq %rbp
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
    addq $16, %rsp
    iretq

.section .rodata
.align 8
.global hybrid_isr_stub_table
.type hybrid_isr_stub_table, @object
hybrid_isr_stub_table:
    .quad hybrid_isr_0
    .quad hybrid_isr_1
    .quad hybrid_isr_2
    .quad hybrid_isr_3
    .quad hybrid_isr_4
    .quad hybrid_isr_5
    .quad hybrid_isr_6
    .quad hybrid_isr_7
    .quad hybrid_isr_8
    .quad hybrid_isr_9
    .quad hybrid_isr_10
    .quad hybrid_isr_11
    .quad hybrid_isr_12
    .quad hybrid_isr_13
    .quad hybrid_isr_14
    .quad hybrid_isr_15
    .quad hybrid_isr_16
    .quad hybrid_isr_17
    .quad hybrid_isr_18
    .quad hybrid_isr_19
    .quad hybrid_isr_20
    .quad hybrid_isr_21
    .quad hybrid_isr_22
    .quad hybrid_isr_23
    .quad hybrid_isr_24
    .quad hybrid_isr_25
    .quad hybrid_isr_26
    .quad hybrid_isr_27
    .quad hybrid_isr_28
    .quad hybrid_isr_29
    .quad hybrid_isr_30
    .quad hybrid_isr_31
.size hybrid_isr_stub_table, .-hybrid_isr_stub_table

.section .note.GNU-stack,"",@progbits
