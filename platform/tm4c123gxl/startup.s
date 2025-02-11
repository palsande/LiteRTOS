.section .text
.global _start
_stack_top = 0x20001000;

_start:
    ldr r0, =_stack_top
    mov sp, r0
    bl main
    b .
