    .text
    .global main
main:
    push %r13
    push %r14
    push %r15
    .data
x_name:
    .byte 120
    .byte 0
x_value:
    .quad 0
    .text
    push x_value
    mov $0,%rax
    pop %r15
    pop %r14
    pop %r13
    ret
    .data
p4_format:
    .byte 37
    .byte 108
    .byte 108
    .byte 117
    .byte 10
    .byte 0
    .text
