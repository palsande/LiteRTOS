/*-----------------------------------------------------------------------------
 * This file is part of the RTOS-Framework Project.
 * 
 * RTOS-Framework is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, either version 3 of the License, or 
 * (at your option) any later version.
 * 
 * RTOS-Framework is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 * 
 * Copyright (c) 2025 Sandeep K. Pal
 *-----------------------------------------------------------------------------
 */

#include "port.h"
#include <stdint.h>
#include "rtos.h"
#include "tm4c123gh6pm.h"

#if 0
extern "C" void PendSV_Handler(void);

// Wrapper function for calling from assembly
extern "C" void rtos_schedule_wrapper() {
    RTOS::schedule();
}
#endif

void hwInit() {
    // Set PendSV to lowest priority (to ensure it runs only when needed)
    *(volatile uint32_t*)0xE000ED22 = 0xFF;

    // 4 pushbuttons, and uart
    // Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
    SYSCTL_RCC_R = SYSCTL_RCC_USESYSDIV|SYSCTL_RCC_XTAL_16MHZ|SYSCTL_RCC_OSCSRC_MAIN|(4 << SYSCTL_RCC_SYSDIV_S);
    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    // Note UART on port A must use APB
    SYSCTL_GPIOHBCTL_R  = 0;//use of APB bus
    SYSCTL_RCGC2_R      = SYSCTL_RCGC2_GPIOF | SYSCTL_RCGC2_GPIOA| SYSCTL_RCGC2_GPIOB| SYSCTL_RCGC2_GPIOC; //Enable port A and port F
    //port F Configuration
    GPIO_PORTF_DIR_R    = 0x06;//port F pins 1,2 are outputs and pins 0,4 are input
    GPIO_PORTF_DR2R_R   = 0x06;//pins 1,2 has 2mA current drive strength
    GPIO_PORTF_PUR_R    = 0x11;// enable internal pull-up for push button on pins 0,4
    GPIO_PORTF_DEN_R    = 0x17;//enable(or make LED's and push buttons digital) LEDs and pushbuttons
    //port F Configuration
    GPIO_PORTA_DIR_R    = 0x0C;//port A pins 3,2 are outputs and pins 5,4 are input
    GPIO_PORTA_DR2R_R   = 0x0C;//pins 3,2 has 2mA current drive strength
    GPIO_PORTA_PUR_R    = 0x30;// enable internal pull-up for push button on pins 5,4
    GPIO_PORTA_DEN_R    = 0x3C;//enable(or make LED's and push buttons digital) LEDs and pushbuttons

    // Configure External LED and pushbutton pins
    GPIO_PORTB_DIR_R  = 0xF0;                           // bits 1, 2, and 3 are outputs, other pins are inputs
    GPIO_PORTB_DR2R_R = 0xF0;                           // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTB_DEN_R  = 0xF0;                           // enable LEDs and pushbuttons

    GPIO_PORTC_DIR_R = 0x00;                            // Bits PC7, PC6, PC5, and PC4 are Inputs for Push Buttons
    GPIO_PORTC_DEN_R = 0xF0;                            // Enable digital functions for the Push Buttons
    GPIO_PORTC_PUR_R = 0xF0;                            // Enable internal pull-up for push buttons

    //-----------------------------------Init of SysTick Timer--------------------------
    NVIC_ST_CTRL_R = 0;           // disable SysTick during setup
    NVIC_ST_RELOAD_R = 40000-1;   // reload value of system timer
    NVIC_ST_CURRENT_R = 0;        // any write to current clears it
    NVIC_ST_CTRL_R |= 0x07;       // enable SysTick with down counting with interrupt enabled
    //-----------------------------------------------------------------------------------

    //---------------------------Init Uart0 Module---------------------------------------
    // Configure UART0 pins
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R0;         // turn-on UART0, leave other uarts in same status
    GPIO_PORTA_DEN_R |= 3;                           // default, added for clarity
    GPIO_PORTA_AFSEL_R |= 3;                         // default, added for clarity
    GPIO_PORTA_PCTL_R |= GPIO_PCTL_PA1_U0TX | GPIO_PCTL_PA0_U0RX;

    // Configure UART0 to 115200 baud, 8N1 format (must be 3 clocks from clock enable and config writes)
    UART0_CTL_R  = 0;                                 // turn-off UART0 to allow safe programming
    UART0_CC_R   = UART_CC_CS_SYSCLK;                  // use system clock (40 MHz)
    UART0_IBRD_R = 21;                               // r = 40 MHz / (Nx115.2kHz), set floor(r)=21, where N=16
    UART0_FBRD_R = 45;                               // round(fract(r)*64)=45
    UART0_LCRH_R = UART_LCRH_WLEN_8;// | UART_LCRH_FEN; // configure for 8N1 w/ 16-level FIFO
    UART0_IM_R   = UART_IM_RXIM;                       // turn-on RX interrupt
    NVIC_EN0_R   = 1<<5;                               // turn-on interrupt 21 (UART0)
    //NVIC_PRI1_R  |= 0x000040000                         //priority 2
    UART0_CTL_R = UART_CTL_TXE | UART_CTL_RXE | UART_CTL_UARTEN; // enable TX, RX, and module
}

#if 0
void portStartScheduler() {
    // Start first task
    __asm volatile (" SVC #0 "); // Supervisor Call to switch to first task
}

void portTriggerContextSwitch() {
    *(volatile uint32_t*)0xE000ED04 = (1 << 28); // Set PendSV to trigger context switch
}

// Context Switch Handler
__attribute__((naked)) void PendSV_Handler() {
    __asm volatile (
        "   MRS     R0, PSP         \n"  // Load Process Stack Pointer
        "   STMDB   R0!, {R4-R11}    \n" // Save registers R4-R11
        "   LDR     R1, =currentTask \n"
        "   LDR     R1, [R1]         \n"
        "   LDR     R2, =tcb         \n"
        "   LDR     R1, [R2, R1, LSL #2] \n"
        "   STR     R0, [R1]         \n"
        
        "   BL      rtos_schedule_wrapper   \n"
        
        "   LDR     R1, =currentTask \n"
        "   LDR     R1, [R1]         \n"
        "   LDR     R2, =tcb         \n"
        "   LDR     R1, [R2, R1, LSL #2] \n"
        "   LDR     R0, [R1]         \n"
        "   LDMIA   R0!, {R4-R11}    \n"
        "   MSR     PSP, R0         \n"
        "   BX      LR              \n"
    );
}
#endif