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

void task1() {
    while (1) {
        // Toggle LED
    }
}

void task2() {
    while (1) {
        // Print debug message
    }
}

int main() {
    RTOS::init();
    RTOS::createTask(task1, 0);
    RTOS::createTask(task2, 1);
    RTOS::start();
    
    while(1); // Should never reach here
}
