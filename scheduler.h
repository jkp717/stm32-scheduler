/*
 * scheduler.h
 *
 *  Created on: Sep 27, 2024
 *      Author: joshk
 */

#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

#define SCHEDULER_TASK_LIMIT 50  // Limit the number of scheduled tasks that can be in queue 

typedef void (*ScheduleFunc_t)(void*);

typedef struct _task {
  uint32_t          executeTick;    // When to start task (based on sysTick)
  void             *params;
  void (*func)( void* );
  struct _task     *next;        // Next in linked list
} Task_t;

typedef struct {
  Task_t                *taskQ;
  uint16_t               taskCnt;
  bool                   isActive;          // Boolean if the RunScheduler has been called
} Scheduler_t;

typedef void (*VoidFunc_t)(void*);

int CORE_ScheduleTask(void (*task)(void *), void *params, size_t size, uint32_t runtime);
int CORE_RemoveScheduleTask(void (*task)(void *));
void CORE_RunScheduler(void);
int CORE_GetScheduleTaskCount(void);

void CORE_TestScheduler(void);

#endif /* INC_SCHEDULER_H_ */
