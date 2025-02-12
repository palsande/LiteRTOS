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

TaskControlBlock tcb[MAX_TASKS];
uint32_t currentTask = 0;
uint32_t totalTasks = 0;

void RTOS::createTask(TaskFunction_t function, uint32_t taskId) {
    tcb[taskId].taskFunction = function;
    tcb[taskId].stackPointer = &tcb[taskId].stack[127]; // Stack grows downward
    *(--tcb[taskId].stackPointer) = 0x01000000; // xPSR (Thumb mode)
    *(--tcb[taskId].stackPointer) = (uint32_t)function; // PC (Task entry)
    *(--tcb[taskId].stackPointer) = 0xFFFFFFFD; // LR (Return to thread mode)
    totalTasks++;
}

void RTOS::bspInit() {
    hwInit();
}

void RTOS::start() {
    portStartScheduler();
}

void RTOS::schedule() {
    currentTask = (currentTask + 1) % totalTasks;
    portTriggerContextSwitch();
}
