.syntax unified
.cpu cortex-m4
.fpu softvfp
.thumb

// USART Registers
.equ USART1, 0x40013800
.equ USART2, 0x40004400
.equ USART3, 0x40004800
.equ BRR,    0x0C

/*
 * @funcname: change_baud()
 *
 * @brief: Changes the current baud rate for USART
 */
.global change_baud
change_baud:
    // Note: This function is configured for 16x oversampling only!
    push {r4-r5, lr}    // Push r4-r5 and lr to stack
    ldr  r2, =USART1    // Load address of USART1 into r2
    ldr  r3, [r2, #BRR] // Load value of USART1_BRR into r3
    ldr  r4, =#0xffff   // r4 = 0xffff
    ldr  r5, =#1000000  // r5 = 1,000,000
    muls r1, r5         // r1 = 1 * 1,000,000
    bics r3, r4         // r3 = r3 & ~(0xffff)
    udiv r1, r0         // r1 = Fck / Baud
    orrs r3, r1         // r3 = USART_BRR | (Fck / Baud)
    str  r3, [r2, #BRR] // Store new USART_BRR value
    pop  {r4-r5, pc}    // Pop r4-r5 and lr to pc

// Note: Fck is the clock frequency driving the USART peripheral. This can be either
//       LSE, HSI, PCLK, or SYS. For now we're using PCLK2 at a rate of 80 MHz.