/* syscalls.h
 *      Nachos system call interface.  These are Nachos kernel operations
 *      that can be invoked from user programs, by trapping to the kernel
 *      via the "syscall" instruction.
 *
 *      This file is included by user programs and by the Nachos kernel.
 *
 * Copyright (c) 1992-1993 The Regents of the University of California.
 * All rights reserved.  See copyright.h for copyright notice and limitation
 * of liability and disclaimer of warranty provisions.
 */

#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "copyright.h"
#include "errno.h"
/* system call codes -- used by the stubs to tell the kernel which system call
 * is being asked for
 */
#define SC_Halt         0
#define SC_Exit         1
#define SC_Exec         2
#define SC_Join         3
#define SC_Create       4
#define SC_Remove       5
//#define SC_Open       6
//#define SC_Read       7
//#define SC_Write      8
#define SC_Seek         9
//#define SC_Close      10
#define SC_ThreadFork   11
#define SC_ThreadYield  12
#define SC_ExecV        13
#define SC_ThreadExit   14
#define SC_ThreadJoin   15
#define SC_PrintInt     16
#define SC_Add          42
#define SC_MSG          100
#ifndef IN_ASM

/* The system call interface.  These are the operations the Nachos
 * kernel needs to support, to be able to run user programs.
 *
 * Each of these is invoked by a user program by simply calling the
 * procedure; an assembly language stub stuffs the system call code
 * into a register, and traps to the kernel.  The kernel procedures
 * are then invoked in the Nachos kernel, after appropriate error checking,
 * from the system call entry point in exception.cc.
 */
