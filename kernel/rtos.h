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

#ifndef RTOS_H
#define RTOS_H

#include <stdint.h>

#define MAX_TASKS  2

typedef void (*TaskFunction_t)(void);

struct TaskControlBlock {
    uint32_t *stackPointer;
    TaskFunction_t taskFunction;
    uint32_t stack[128];  // 512 Bytes per task
};

class RTOS {
public:
    static void init();
    static void start();
    static void createTask(TaskFunction_t function, uint32_t taskId);
    static void schedule();
};

// Wrapper for assembly compatibility
extern "C" void rtos_schedule_wrapper();

#endif // RTOS_H
