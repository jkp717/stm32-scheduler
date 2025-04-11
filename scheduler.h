/*
 * scheduler.h
 *
 *  Created on: Sep 27, 2024
 *      Author: joshk
 */

#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_


typedef void (*SchedulerTaskFunc_t)(void*);

typedef struct _task {
  uint32_t              executeTick;    // When to start task (based on sysTick)
  void                 *params;
  SchedulerTaskFunc_t   func;
  struct _task         *next;        // Next in linked list
} Task_t;

typedef struct {
  Task_t                *taskQ;
  uint16_t               taskCnt;
  bool                   isActive;          // Boolean if the RunScheduler has been called
} Scheduler_t;

void CORE_ScheduleTask(SchedulerTaskFunc_t func, void *params, size_t size, uint32_t runtime);
Task_t* CORE_RemoveScheduleTaskByRef(void (*func)(void *));
void CORE_RunScheduler(void);
int CORE_GetScheduleTaskCount(void);

#endif /* INC_SCHEDULER_H_ */
