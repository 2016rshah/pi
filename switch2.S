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
    subq $16,%rsp
    push %r12
    push %r13
    push %r14
    push %r15
	push %rbx
    sub $8,%rsp
    push %r12
    push %r13
    push %r14
    push %r15
	push %rbx
    sub $8,%rsp
    mov $2,%r12
    mov %r12,%r13
    mov %r13,%r14
    mov $3,%r12
    mov %r12,%r13
    mov $4,%r12
    imul %r12,%r13
    add %r13,%r14
    mov $5,%r12
    mov %r12,%r13
    add %r13,%r14
    mov %r14,%r15
    mov %r15,%rbx
    mov %rbx,%rax
    add $8,%rsp
	pop %rbx
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    mov %rax,%r12
    mov %r12,%r13
    mov %r13,%r14
    mov %r14,%r15
    mov %r15,%rbx
    mov %rbx,%rax
    add $8,%rsp
	pop %rbx
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    mov %rax,-8(%rbp)
    push %r12
    push %r13
    push %r14
    push %r15
	push %rbx
    sub $8,%rsp
    push %r12
    push %r13
    push %r14
    push %r15
	push %rbx
    sub $8,%rsp
    mov $2,%r12
    mov %r12,%r13
    mov %r13,%r14
    mov $3,%r12
    mov %r12,%r13
    add %r13,%r14
    mov %r14,%r15
    mov %r15,%rbx
    mov %rbx,%rax
    add $8,%rsp
	pop %rbx
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    mov %rax,%r12
    mov %r12,%r13
    mov $4,%r12
    imul %r12,%r13
    mov %r13,%r14
    mov $5,%r12
    mov %r12,%r13
    add %r13,%r14
    mov %r14,%r15
    mov %r15,%rbx
    mov %rbx,%rax
    add $8,%rsp
	pop %rbx
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    mov %rax,-16(%rbp)
    push %r12
    push %r13
    push %r14
    push %r15
	push %rbx
    sub $8,%rsp
    push %r12
    push %r13
    push %r14
    push %r15
	push %rbx
    sub $8,%rsp
    mov -8(%rbp),%rax
    mov %rax,%r12
    mov %r12,%r13
    mov %r13,%r14
    mov %r14,%r15
    mov %r15,%rbx
    mov %rbx,%rax
    add $8,%rsp
	pop %rbx
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    mov %rax,%r12
    mov %r12,%r13
    mov %r13,%r14
    mov %r14,%r15
    mov %r15,%rbx
    mov %rbx,%rax
    add $8,%rsp
	pop %rbx
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    subq $19, %rax
.data
.SW0:
  .quad    .0SW1
  .quad    .0SW2
  .quad    .0SW3
  .quad    .0SW0
.text
    cmpq $3, %rax
    ja  .0SWDEF
    jmp  *.SW0(,%rax, 8)
.0SW0:
    jmp ESW0
.0SW1:
    push %r12
    push %r13
    push %r14
    push %r15
	push %rbx
    sub $8,%rsp
    mov -8(%rbp),%rax
    mov %rax,%r12
    mov %r12,%r13
    mov %r13,%r14
    mov %r14,%r15
    mov %r15,%rbx
    mov %rbx,%rax
    add $8,%rsp
	pop %rbx
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    mov $output_format,%rdi
    mov %rax,%rsi
    call printf
    push %r12
    push %r13
    push %r14
    push %r15
	push %rbx
    sub $8,%rsp
    push %r12
    push %r13
    push %r14
    push %r15
	push %rbx
    sub $8,%rsp
    mov -16(%rbp),%rax
    mov %rax,%r12
    mov %r12,%r13
    mov %r13,%r14
    mov %r14,%r15
    mov %r15,%rbx
    mov %rbx,%rax
    add $8,%rsp
	pop %rbx
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    mov %rax,%r12
    mov %r12,%r13
    mov %r13,%r14
    mov %r14,%r15
    mov %r15,%rbx
    mov %rbx,%rax
    add $8,%rsp
	pop %rbx
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    subq $24, %rax
    cmpq $51, %rax
    je .1SW2
.data
.SW1:
  .quad    .1SW0
  .quad    .1SW1
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SWDEF
  .quad    .1SW2
.text
    cmpq $51, %rax
    ja  .1SWDEF
    jmp  *.SW1(,%rax, 8)
.1SW0:
    push %r12
    push %r13
    push %r14
    push %r15
	push %rbx
    sub $8,%rsp
    mov $5,%r12
    mov %r12,%r13
    mov %r13,%r14
    mov %r14,%r15
    mov %r15,%rbx
    mov %rbx,%rax
    add $8,%rsp
	pop %rbx
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    mov $output_format,%rdi
    mov %rax,%rsi
    call printf
    jmp ESW1
.1SW1:
    push %r12
    push %r13
    push %r14
    push %r15
	push %rbx
    sub $8,%rsp
    mov -16(%rbp),%rax
    mov %rax,%r12
    mov %r12,%r13
    mov %r13,%r14
    mov %r14,%r15
    mov %r15,%rbx
    mov %rbx,%rax
    add $8,%rsp
	pop %rbx
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    mov $output_format,%rdi
    mov %rax,%rsi
    call printf
    jmp ESW1
.1SW2:
    jmp ESW1
.1SWDEF:
    jmp ESW1
 ESW1:
.0SW2:
    push %r12
    push %r13
    push %r14
    push %r15
	push %rbx
    sub $8,%rsp
    push %r12
    push %r13
    push %r14
    push %r15
	push %rbx
    sub $8,%rsp
    mov -16(%rbp),%rax
    mov %rax,%r12
    mov %r12,%r13
    mov %r13,%r14
    mov %r14,%r15
    mov %r15,%rbx
    mov %rbx,%rax
    add $8,%rsp
	pop %rbx
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    mov %rax,%r12
    mov %r12,%r13
    mov %r13,%r14
    mov %r14,%r15
    mov %r15,%rbx
    mov %rbx,%rax
    add $8,%rsp
	pop %rbx
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    subq $23, %rax
.data
.SW2:
  .quad    .2SW0
  .quad    .2SW1
  .quad    .2SW2
.text
    cmpq $2, %rax
    ja  .2SWDEF
    jmp  *.SW2(,%rax, 8)
.2SW0:
.2SW1:
    jmp ESW2
.2SW2:
    push %r12
    push %r13
    push %r14
    push %r15
	push %rbx
    sub $8,%rsp
    mov -16(%rbp),%rax
    mov %rax,%r12
    mov %r12,%r13
    mov %r13,%r14
    mov %r14,%r15
    mov %r15,%rbx
    mov %rbx,%rax
    add $8,%rsp
	pop %rbx
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    mov $output_format,%rdi
    mov %rax,%rsi
    call printf
.2SWDEF:
    jmp ESW2
 ESW2:
    push %r12
    push %r13
    push %r14
    push %r15
	push %rbx
    sub $8,%rsp
    mov $15,%r12
    mov %r12,%r13
    mov %r13,%r14
    mov %r14,%r15
    mov %r15,%rbx
    mov %rbx,%rax
    add $8,%rsp
	pop %rbx
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    mov $output_format,%rdi
    mov %rax,%rsi
    call printf
    jmp ESW0
.0SW3:
    push %r12
    push %r13
    push %r14
    push %r15
	push %rbx
    sub $8,%rsp
    mov $21,%r12
    mov %r12,%r13
    mov %r13,%r14
    mov %r14,%r15
    mov %r15,%rbx
    mov %rbx,%rax
    add $8,%rsp
	pop %rbx
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    mov $output_format,%rdi
    mov %rax,%rsi
    call printf
.0SWDEF:
    jmp ESW0
 ESW0:
main_end:
    addq $16,%rsp
    pop %rbp
    ret
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
