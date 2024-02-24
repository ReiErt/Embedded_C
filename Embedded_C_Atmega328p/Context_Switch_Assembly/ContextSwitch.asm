
/**
 * Context Switch in Assembly for bare metal Atmega328p
 * Reilly - 2022
 */

 .section .text
 .global ContextSwitch

#define SREG         0x3f
#define SPH          0x3e
#define SPL          0x3d
#define R25 25
#define R24 24


;
; This assembler routine is called as a function like
; void ContextSwitch(uint8_t * NextThread_sp, uint8_t ** CurrThread_sp)
; and performs the context switch between the passed two threads.
; *CurrThread_sp = NextThread_sp;

ContextSwitch:

// Passed parameters:
//
// r22 CurrThread_sp, lo-part Adress
// r23 CurrThread_sp, hi-part
//
// r24 NextThread_sp, lo-part Value
// r25 NextThread_sp, hi-part

// Aim:
//
// 1. Save the current register set + SREG to the stack
// of the thread to be suspended (CurrThread).
//
// 2. Switch the stacks from CurrThread to NextThread.
//
// 3. Restore the context of the NextThread.


// - Data between registers can be moved. To transfer data between memory and
//   registers, the in and out instructions must be used
//   Examples:
// mov r31, r23 ; moves the value of r23 to r31
//   in r22, SPL ; moves the content of the memory location 0x3E (which
//                      ; is defined as SPL, see register file) to r22
// std Z+1, r23 ; 'store indirect with displacement': Stores the word
    // st Z,   r22 ; contained in the registers r23 (hi part) and r22 (lo part)
    // ; to the memory location pointed to by the Z register
 
//push GPR, SR and return adress from task1
push r0
push r1
push r2
push r3
push r4
push r5
push r6
push r7
push r8
push r9
push r10
push r11
push r12
push r13
push r14
push r15
push r16
push r17
push r18
push r19
push r20
push r21
push r22
push r23
push r24
push r25
push r26
push r27
push r28
push r29
push r30
push r31

in r8, SREG
push r8

//load current SP 
mov r31, r23
mov r30, r22

//get current SP 
in r10, SPL
in r11, SPH

//store SP of current thread into adress pointed to by Z
std Z+1, r11
st Z, r10

//manipulate SP to switch context/thread
out SPL, r24
out SPH, r25

//pop return adress, SR, GPR from task2
pop r1
out SREG, r1

pop r31
pop r30    
pop r29
pop r28
pop r27
pop r26
pop r25
pop r24
pop r23
pop r22
pop r21
pop r20
pop r19
pop r18
pop r17
pop r16
pop r15
pop r14
pop r13
pop r12
pop r11
pop r10
pop r9
pop r8
pop r7
pop r6
pop r5
pop r4
pop r3
pop r2
pop r1
pop r0

//TODO - Simple switch to the next process. 2 lines. One-way, one shot only.
//This means that after the MainThread is run, 
//application 1, application 2, application 3, application 4, application 5 should run. 
//Undefined what happens, next. (What happens next? :-)

ret
