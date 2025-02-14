#ifndef RTOS_H
#define RTOS_H

#include <stdint.h>

//-----------------------------------------------------------------------------
// RTOS Defines and Kernel Variables
//-----------------------------------------------------------------------------

/// function pointer
typedef void (*_fn)();

/// stack pointer manipulation
void restoreMSP(uint32_t sp);
uint32_t saveMSP();

/// semaphore
#define MAX_QUEUE_SIZE    10
struct semaphore
{
  unsigned int count;
  unsigned int queueSize;
  unsigned int processQueue[MAX_QUEUE_SIZE];
};

extern struct semaphore *s, keyPressed, keyReleased, flashReq, printRTOSModeReq;

/// task
#define MAX_TASKS 10          // maximum number of valid tasks
#define STATE_INVALID    0    // no task
#define STATE_READY      1    // ready to run
#define STATE_BLOCKED    2    // has run, but now blocked by semaphore
#define STATE_DELAYED    3    // has run, but now awaiting timer

extern uint8_t taskCurrent;      // index of last dispatched task
extern uint8_t taskCount;        // total number of valid tasks

/// rtos mode
#define MODE_COOPERATIVE    0
#define MODE_PREEMPTIVE     1

extern int rtosMode;

/// data structure for task control block
struct _tcb
{
  uint8_t state;                 // see STATE_ values above
  void *pid;                     // used to uniquely identify process
  void *sp;                      // location of stack pointer for process
  uint8_t priority;              // 0=highest, 7=lowest
  uint8_t skipCount;             // no of times task can be skipped
  uint8_t currentPriority;       // used for priority inheritance
  uint32_t ticks;                // ticks until sleep complete
};

extern struct _tcb tcb[MAX_TASKS];

/// data structure for stack manipulation 
extern uint32_t stack[MAX_TASKS][256];

/// critical section
#define ENTER_CRITICAL_SECTION   (NVIC_ST_CTRL_R = 0x00)
#define EXIT_CRITICAL_SECTION    (NVIC_ST_CTRL_R = 0x01)

/// Class for RTOS 
class RTOS 
{
public:
    static void bspInit();
    static void rtosInit(int mode, int reload);
    static bool createProcess(_fn fn, int priority);
    static void destroyProcess(_fn fn);
    static int  rtosScheduler();
    static void rtosStart();

    static void initSemaphore(void* p, int count); 
    static void yield();
    static void sleep(uint32_t tick);
    static void waitSemaphore(void* pSemaphore);
    static void postSemaphore(void* pSemaphore);
};

#endif // RTOS_H