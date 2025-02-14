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

//-----------------------------------------------------------------------------
// Helper Functions
//-----------------------------------------------------------------------------

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

// ------------------------------------------------------------------------------
//  Task Methods
// ------------------------------------------------------------------------------

// one task must be ready at all times or the scheduler will fail
// the idle task is implemented for this purpose

void idle()
{
    while (true)
    {
        BLUE_LED_B = 1;
        waitMicrosecond(1000);
        BLUE_LED_B = 0;
        RTOS::yield();
    }
}

void flash4Hz()
{
    while (true)
    {
        GREEN_LED ^= 1;
        RTOS::sleep(125);
    }
}

void oneshot()
{
    while (true)
    {
        RTOS::waitSemaphore(&flashReq);
        YELLOW_LED = 1;
        RTOS::sleep(1000);
        YELLOW_LED = 0;
    }
}

void partOfLengthyFn()
{
    // represent some lengthy operation
    waitMicrosecond(1000);
    // give another process a chance
    RTOS::yield();
}

void lengthyFn()
{
    uint16_t i;
    while (true)
    {
        for (i = 0; i < 4000; i++)
        {
            partOfLengthyFn();
        }
        RED_LED ^= 1;
    }
}

void readKeys()
{
    uint8_t buttons;
    while (true)
    {
        RTOS::waitSemaphore(&keyReleased);
        buttons = 0;
        while (buttons == 0)
        {
            buttons = readPbs();
            RTOS::yield();
        }
        RTOS::postSemaphore(&keyPressed);
        if ((buttons & 1) != 0)
        {
            YELLOW_LED ^= 1;
            RED_LED = 1;
        }
        if ((buttons & 2) != 0)
        {
            RTOS::postSemaphore(&flashReq);
            RED_LED = 0;
        }
        if ((buttons & 4) != 0)
        {
            RTOS::createProcess(flash4Hz, 0);
        }
        if ((buttons & 8) != 0)
        {
            RTOS::destroyProcess(flash4Hz);
        }

        RTOS::yield();
    }
}

void debounce()
{
    uint8_t count;
    while (true)
    {
        RTOS::waitSemaphore(&keyPressed);
        count = 10;
        while (count != 0)
        {
            RTOS::sleep(10);
            if (readPbs() == 0)
                count--;
            else
                count = 10;
        }
        RTOS::postSemaphore(&keyReleased);
    }
}

void uncooperative()
{
    while (true)
    {
        while (readPbs() == 8)
        {
        }
        RTOS::yield();
    }
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main() {

    bool kernelMode = false;
    bool error = false;
    uint8_t pb = 255;

    RTOS::bspInit(); // init hw

    // blink Red LED
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

    // Add required idle process
    error =  RTOS::createProcess(idle, 7);

    // Add other processes
    error &= RTOS::createProcess(flash4Hz, 0);
    error &= RTOS::createProcess(lengthyFn, 6);
    error &= RTOS::createProcess(oneshot, 3);
    error &= RTOS::createProcess(readKeys, 1);
    error &= RTOS::createProcess(debounce, 3);
    error &= RTOS::createProcess(uncooperative, 5);

    // Start up RTOS
    if (error)
        RTOS::rtosStart(); // never returns
    else
        RED_LED = 1;
        
    return 0;

    // don't delete this unreachable code
    // if a function is only called once in your code, it will be
    // accessed with two goto instructions instead of call-return,
    // so any stack-based code will not function correctly
    RTOS::yield(); RTOS::sleep(0); RTOS::waitSemaphore(0); RTOS::postSemaphore(0);
}
