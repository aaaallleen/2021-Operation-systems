/* Start.s
 *      Assembly language assist for user programs running on top of Nachos.
 *
 *      Since we don't want to pull in the entire C library, we define
 *      what we need for a user program here, namely Start and the system
 *      calls.
 */

#define IN_ASM
#include "syscall.h"

        .text
        .align  2

/* -------------------------------------------------------------
 * __start
 *      Initialize running a C program, by calling "main".
 *
 *      NOTE: This has to be first, so that it gets loaded at location 0.
 *      The Nachos kernel always starts a program by jumping to location 0.
 * -------------------------------------------------------------
 */

        .globl __start
        .ent    __start
__start:
        jal     main
        move    $4,$0
        jal     Exit     /* if we return from main, exit(0) */
        .end __start

/* -------------------------------------------------------------
 * System call stubs:
 *      Assembly language assist to make system calls to the Nachos kernel.
 *      There is one stub per system call, that places the code for the
 *      system call into register r2, and leaves the arguments to the
 *      system call alone (in other words, arg1 is in r4, arg2 is
 *      in r5, arg3 is in r6, arg4 is in r7)
 *
 *      The return value is in r2. This follows the standard C calling
 *      convention on the MIPS.
 * -------------------------------------------------------------
 */

        .globl Halt
        .ent    Halt
Halt:
        addiu $2,$0,SC_Halt
        syscall
        j       $31
