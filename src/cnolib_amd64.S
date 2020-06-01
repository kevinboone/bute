#=============================================================================
#  shnolib -- a shell without a standard C library 
#  AMD64 part of Kevin's tiny C library
#  Copyright (c)2020 Kevin Boone. Distributed uner the terms of the 
#    GNU PUblic Licence, v3.0
#=============================================================================

   .global _start
   .global syscall

   .text

#=============================================================================
# _start
#  just call main()
#  TODO: argc, argv
#=============================================================================
_start:
    # TODO: do we have to worry about stack alignment here?
    # main() requires argc in %rdi, and argv in %rsi. 
    # The kernel pushes argc, but then pushes the individual argv[n]
    #  pointers. So main() needs to get a reference to the position
    #  in the stack where the argv pointers begin
    mov 0x0(%rsp),%rdi
    lea 0x8(%rsp),%rsi
    call __main
    mov     $60, %rax    # exit
    xor     %rdi, %rdi   # return value, 0
    syscall


#=============================================================================
# syscall
# We need to be really careful here. The arguments from C will be
#  callno, arg0, arg1... following the standard SysV calling convention. So
#  we have on entry callno - rdi, arg0 - rsi, arg1 - rdx, arg2 - rcx, 
#  arg3 - r8, argc4 - r9
# But the syscall interface uses R10 for arg3, instead of RCX. So we
#  need to shift all the supplied arguments down, with the callno ending 
#  up in rax, BUT we need to populate r10 instead of rcx.
#=============================================================================
syscall:
    mov %rdi, %rax
    mov %rsi, %rdi
    mov %rdx, %rsi
    mov %rcx, %rdx
    mov %r8, %r10
    mov %r9, %r8
    syscall
    ret

