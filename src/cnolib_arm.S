.text

.global _start
.global syscall
.global foo

_start:
    ldr    %r0, [sp]
    add    r1, sp, #4
    bl      __main
    #mov     %r0, $0     /* status := 0 */
    mov     %r7, $1     /* exit is syscall #1 */
    swi     $0          /* invoke syscall */

syscall:
    mov     ip, sp
    stmfd sp!, {r4, r5, r6, r7}
    mov     %r7, %r0
    mov     %r0, %r1
    mov     %r1, %r2
    mov     %r2, %r3
    mov     %r3, %r4
    ldmfd ip, {r3, r4, r5, r6}
    swi    $0
    ldmfd sp!, {r4, r5, r6, r7}
    bx     lr


