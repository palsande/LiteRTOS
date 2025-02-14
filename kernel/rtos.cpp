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

#include "rtos.h"
#include "port.h"
#include "tm4c123gh6pm.h"

// Define variables
struct semaphore *s, keyPressed, keyReleased, flashReq, printRTOSModeReq;
uint8_t taskCurrent = 0;
uint8_t taskCount = 0;
int rtosMode;
struct _tcb tcb[MAX_TASKS];
uint32_t stack[MAX_TASKS][256];

void restoreMSP(uint32_t sp) {
    __asm volatile ("MSR MSP, %0" : : "r" (sp));
}

uint32_t saveMSP() {
    uint32_t msp;
    __asm volatile ("MRS %0, MSP" : "=r" (msp) );
    return msp;
}

//-----------------------------------------------------------------------------
// RTOS Kernel
//-----------------------------------------------------------------------------

void RTOS::bspInit() {
    hwInit();
}

void RTOS::rtosInit(int mode,int reload) {
    uint8_t i;
    rtosMode = mode;
    // no tasks running
    taskCount = 0;
    // clear out tcb records
    for (i = 0; i < MAX_TASKS; i++)
    {
      tcb[i].state = STATE_INVALID;
      tcb[i].pid = 0;
    }

    // 1ms systick
    if(rtosMode == MODE_PREEMPTIVE)
    {
        NVIC_ST_CTRL_R    = 0;         // disable SysTick during setup
        NVIC_ST_RELOAD_R  = reload-1;  // reload value of system timer
        NVIC_ST_CURRENT_R = 0;         // any write to current clears it
        NVIC_SYS_PRI2_R   = 0;         // SVCALL priority 0
        NVIC_SYS_PRI3_R   = 0;         // SysTick priority 0
        NVIC_ST_CTRL_R   |= NVIC_ST_CTRL_ENABLE| NVIC_ST_CTRL_CLK_SRC | NVIC_ST_CTRL_INTEN;; // enable SysTick with core clock and interrupts
    }
}

bool RTOS::createProcess(_fn fn, int priority) {
    bool ok = false;
    uint8_t i = 0;
    bool found = false;
    // take steps to ensure a task switch cannot occur
    ENTER_CRITICAL_SECTION;
  
    // save starting address if room in task list
    if (taskCount < MAX_TASKS)
    {
      // make sure fn not already in list (prevent reentrancy)
      while (!found && (i < MAX_TASKS))
      {
        found = (tcb[i++].pid ==  (void*)fn);
      }
      if (!found)
      {
        // find first available tcb record
        i = 0;
        while (tcb[i].state != STATE_INVALID) {i++;}
        tcb[i].state = STATE_READY;
        tcb[i].pid = (void*)fn;
        // REQUIRED: preload stack to look like the task had run before
        stack[i][255] = 0x01000000;   // xPSR
        stack[i][254] = (uint32_t)fn; // PC
        stack[i][253] = (uint32_t)fn; // LR
        stack[i][252] = 12;           // R12
        stack[i][251] = 3;            // R3
        stack[i][250] = 2;            // R2
        stack[i][249] = 1;            // R1
        stack[i][248] = 0;            // R0
        stack[i][247] = 4;            // R4
        stack[i][246] = 5;            // R5
        stack[i][245] = 6;            // R6
        stack[i][244] = 7;            // R7
        stack[i][243] = 8;            // R8
        stack[i][242] = 9;            // R9
        stack[i][241] = 10;           // R10
        stack[i][240] = 11;           // R11
        tcb[i].sp = &stack[i][240];   // REQUIRED: + offset as needed for the pre-loaded stack
        tcb[i].priority = priority;
        tcb[i].skipCount = priority;
        tcb[i].currentPriority = priority;
        // increment task count
        taskCount++;
        ok = true;
      }
    }
    // allow tasks switches again
    EXIT_CRITICAL_SECTION;
    return ok;
}

void RTOS::destroyProcess(_fn fn) {
    uint8_t i = 0;
    bool found = false;
    // take steps to ensure a task switch cannot occur
    ENTER_CRITICAL_SECTION;
    // find fn
    while (!found && (i < MAX_TASKS))
    {
      found = (tcb[i++].pid ==  fn);
    }
    if (found)
    {
      // delete task
      tcb[i - 1].state = STATE_INVALID;
      tcb[i - 1].pid = 0;
      tcb[i - 1].sp = 0;
      // decrement task count
      taskCount--;
    }
    // allow tasks switches again
    EXIT_CRITICAL_SECTION;
}

int RTOS::rtosScheduler() {
    // Implement prioritization to 8 levels

    bool ok, priorityReset;
    static uint8_t task = 0xFF;
    ok = false;
    priorityReset = true;

    while (!ok) {
        task++;
        if (task >= MAX_TASKS) {
            task = 0;
            ok = (tcb[task].state == STATE_READY && tcb[task].skipCount <= 7); // Also validating priority of tasks
        }

        if (tcb[task].skipCount <= 7) {
            tcb[task].skipCount++;
        }

        int i;
        for (i = 0; i < MAX_TASKS; i++) {
            if (tcb[i].state != STATE_INVALID) {
                if ((tcb[i].state == STATE_READY) && (tcb[i].skipCount == 8)) {
                    priorityReset = true;
                } else {
                    priorityReset = false;
                }
            }
        }

        if (priorityReset) {
            for (i = 0; i < MAX_TASKS; i++) {
                if (tcb[i].state == STATE_READY) {
                    tcb[i].skipCount = tcb[i].priority;
                }
            }
        }
    }

    return task;
}

void RTOS::rtosStart() {
    // Add code to call the first task to be run, restoring the preloaded context
    ENTER_CRITICAL_SECTION;

    taskCurrent = rtosScheduler();
    // Add code to initialize the MSP with tcb[task_current].sp;
    // Restore the stack to run the first process
    restoreMSP((uint32_t)tcb[taskCurrent].sp);
    __asm(" POP  {R4, R5, R6, R7, R8, R9, R10, R11} ");
    __asm(" POP  {R0, R1, R2, R3, R12} ");
    __asm(" POP  {R14} ");
    __asm(" MOV  PC, R14 ");

    // Call the first task
    _fn fn = (_fn)tcb[taskCurrent].pid;
    fn();

    EXIT_CRITICAL_SECTION;
}

void RTOS::initSemaphore(void* p, int count) {
  s = (struct semaphore*)p;
  s->count = count;
  s->queueSize = 0;
}

void RTOS::yield() {
    // push registers, call scheduler, pop registers, return to new function
    // push registers
    __asm(" PUSH  {R4, R5, R6, R7, R8, R9, R10, R11} ");
    __asm(" PUSH  {R0, R1, R2, R3, R12} ");
    __asm(" PUSH  {R14} ");
    __asm(" PUSH  {R4} ");
    __asm(" PUSH  {R5} ");
    // call scheduler
    taskCurrent = rtosScheduler();  // Find the next task to run
}

void RTOS::sleep(uint32_t tick) {
    // push registers, set state to delayed, store timeout, call scheduler, pop registers,
    // return to new function (separate unrun or ready processing)
        // push registers
        __asm(" PUSH  {R4, R5, R6, R7, R8, R9, R10, R11} ");
        __asm(" PUSH  {R0, R1, R2, R3, R12} ");
        __asm(" PUSH  {R14} ");
        __asm(" PUSH  {R4} ");
        __asm(" PUSH  {R5} ");
    
        // set state to delayed
        tcb[taskCurrent].state = STATE_DELAYED;
        // store timeout
        tcb[taskCurrent].ticks = tick;
    
        // call scheduler
        taskCurrent = rtosScheduler();
    
        // pop registers
        __asm(" POP  {R5} ");
        __asm(" POP  {R4} ");
        __asm(" POP  {R14} ");
        __asm(" POP  {R0, R1, R2, R3, R12} ");
        __asm(" POP  {R4, R5, R6, R7, R8, R9, R10, R11} ");
    
        // return to new function (separate unrun or ready processing)
        __asm(" MOV  PC, LR ");
}

void RTOS::waitSemaphore(void* pSemaphore) {
    struct semaphore* s = (struct semaphore*)pSemaphore;
    ENTER_CRITICAL_SECTION;

    // Check if the semaphore is available
    if (s->count > 0) {
        s->count--;
        EXIT_CRITICAL_SECTION;
        return; // Semaphore is available, return to the calling function
    } else {
        // Semaphore is not available, add the task to the semaphore queue
        s->processQueue[s->queueSize++] = taskCurrent;
        tcb[taskCurrent].state = STATE_BLOCKED;

        // Implement priority inheritance
        for (unsigned int i = 0; i < s->queueSize; i++) {
            if (tcb[s->processQueue[i]].priority < tcb[taskCurrent].priority) {
                tcb[s->processQueue[i]].priority = tcb[taskCurrent].priority;
            }
        }

        // Yield to the scheduler
        EXIT_CRITICAL_SECTION;
        yield();
    }
}

void RTOS::postSemaphore(void* pSemaphore) {
    struct semaphore* s = (struct semaphore*)pSemaphore;
    ENTER_CRITICAL_SECTION;

    // Check if there are any tasks waiting on the semaphore
    if (s->queueSize > 0) {
        // Unblock the first task in the queue
        tcb[s->processQueue[0]].state = STATE_READY;
        s->count++;
        s->queueSize--;

        // Shift the queue to the left
        for (unsigned int i = 0; i < s->queueSize; i++) {
            s->processQueue[i] = s->processQueue[i + 1];
        }
    } else {
        s->count++;
    }

    EXIT_CRITICAL_SECTION;
}   

