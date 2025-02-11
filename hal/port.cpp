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

extern "C" void PendSV_Handler(void);

// Wrapper function for calling from assembly
extern "C" void rtos_schedule_wrapper() {
    RTOS::schedule();
}

void portInit() {
    // Set PendSV to lowest priority (to ensure it runs only when needed)
    *(volatile uint32_t*)0xE000ED22 = 0xFF;
}

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
