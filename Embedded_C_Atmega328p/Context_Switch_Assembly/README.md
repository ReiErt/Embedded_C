﻿# x-86-Assembly

Goal: 
Create a scheduler by implementing the context switch method.

On bare metal, a function occupies the CPU until it finishes. This is not fair to the program's other functions.

Situation: There are 5 methods that share 1 CPU. Each methods should yield several times before completion to the next method.
App 1 --yield--> App 2 --yield--> App 3 --yield--> App 4 --yield--> App 5 --yield--> App 1 -->


// Step by step:
//
// 1. Save the current register set + SREG to the stack
// of the thread to be suspended (CurrThread).
//
// 2. Switch the stacks from CurrThread to NextThread.
//
// 3. Restore the context of the NextThread.


More in depth:
//push GPR, SR and return adress from task1
job 2:
//load current SP 
job 3:
//get current SP 
job 4:
//store SP of current thread into adress pointed to by Z
job 5:
//manipulate SP to switch context/thread
job 6:
//pop return adress, SR, GPR from task2
