    .text
    .global main
main:
    sub $8,%rsp
    call global_0
    call main_fun
    mov $0,%rax
    add $8,%rsp
    ret
//STANDARD FUNCTIONS BLOCK
drawrect_fun:
    pushq %r8
    movq 16(%rsp), %rdi
    movq 24(%rsp), %rsi
    movq 32(%rsp), %rdx
    movq 40(%rsp), %rcx
    call bg_drawrect
    popq %r8
    ret
setcolor_fun:
    push %r8
    movq 16(%rsp), %rdi
    movq 24(%rsp), %rsi
    movq 32(%rsp), %rdx
    call bg_setcolor
    pop %r8
    ret
startpolygon_fun:
    push %r8
    call bg_startpolygon
    pop %r8
    ret
addpoint_fun:
    push %r8
    movq 16(%rsp), %rdi
    movq 24(%rsp), %rsi
    call bg_addpoint
    pop %r8
    ret
endpolygon_fun:
    push %r8
    call bg_endpolygon
    pop %r8
    ret
//END STANDARD FUNCTIONS BLOCK
main_fun:
    push %rbp
    mov %rsp,%rbp
    global_0:
    ret
    .data
output_format:
    .string "%lu\n"
bell_format:
    .string ""
ineedazero:
    .quad 0
windowtitle:
    .string "Potato, the Epic Window"
