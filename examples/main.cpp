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

#define RED_LED_B          (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 1*4)))
#define BLUE_LED_B         (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 2*4)))
#define ORANGE_LED         (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 4*4)))
#define GREEN_LED          (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 5*4)))
#define RED_LED            (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 7*4)))
#define YELLOW_LED         (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 6*4)))

#define PB_0               (*((volatile uint32_t *)(0x42000000 + (0x400063FC-0x40000000)*32 + 4*4)))
#define PB_1               (*((volatile uint32_t *)(0x42000000 + (0x400063FC-0x40000000)*32 + 5*4)))
#define PB_2               (*((volatile uint32_t *)(0x42000000 + (0x400063FC-0x40000000)*32 + 6*4)))
#define PB_3               (*((volatile uint32_t *)(0x42000000 + (0x400063FC-0x40000000)*32 + 7*4)))

#define MODE_COOPERATIVE   0
#define MODE_PREEMPTIVE    1

// Approximate busy waiting (in units of microseconds), given a 40 MHz system clock
void waitMicrosecond(uint32_t us)
{
                                                // Approx clocks per us
    __asm("WMS_LOOP0:   MOV  R1, #6");          // 1
    __asm("WMS_LOOP1:   SUB  R1, #1");          // 6
    __asm("             CBZ  R1, WMS_DONE1");   // 5+1*3
    __asm("             NOP");                  // 5
    __asm("             B    WMS_LOOP1");       // 5*3
    __asm("WMS_DONE1:   SUB  R0, #1");          // 1
    __asm("             CBZ  R0, WMS_DONE0");   // 1
    __asm("             B    WMS_LOOP0");       // 1*3
    __asm("WMS_DONE0:");                        // ---
                                                // 40 clocks/us + error
}

uint8_t readPbs()
{
    uint8_t pb_status=0;

    if(PB_0 == 0)
        pb_status = 1;
    else if(PB_1 == 0)
        pb_status = 2;
    else if(PB_2 == 0)//MODE_COOPERATIVE
        pb_status = 4;
    else if(PB_3 == 0)//MODE_PREEMPTIVE
        pb_status = 8;

    return pb_status;
}

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

    bool kernelMode = false;
    uint8_t pb = 255;

    RTOS::bspInit(); // init hw

    // flash Red LED
    RED_LED_B = 1;
    waitMicrosecond(250000);
    RED_LED_B = 0;
    waitMicrosecond(250000);

    // Initialize selected kernel mode
    while (!kernelMode)
    {
      pb = readPbs();
      if (pb == 4)
      {
        kernelMode = true;
        waitMicrosecond(1000000);
        //rtosInit(MODE_COOPERATIVE,40000);
      }
      if (pb == 8)
      {
        kernelMode = true;
        waitMicrosecond(1000000);
        //rtosInit(MODE_PREEMPTIVE,40000);
      }
    }

    RTOS::createTask(task1, 0);
    RTOS::createTask(task2, 1);
    RTOS::start();
    
    while(1); // Should never reach here
}
