/*
 * scheduler.c
 *
 *  Created on: Sep 27, 2024
 *      Author: jkp717
 */


#include "scheduler.h"

static Scheduler_t Scheduler = {
  .taskQ = NULL,
  .taskCnt = 0,
  .isActive = false
};

/**
  * @brief  Executes task function and removes from taskQ
  * @param  n Task index in taskQ
  * @retval None
  */
static void _executeTask(Task_t *pTask) {

  // execute task function
  pTask->func(Scheduler.taskQ[n].params);

  // free task params space
  if(Scheduler.taskQ[n].params) {
    free(Scheduler.taskQ[n].params);
  }
  // remove task from queue
  if (Scheduler.taskCnt > 1) {
    for (uint16_t i = n; i < Scheduler.taskCnt; i++) {
      Scheduler.taskQ[i] = Scheduler.taskQ[i + 1];
    }
  } else {
    Scheduler.taskQ[0].executeTick = 0;
    Scheduler.taskQ[0].func = NULL;
  }

  Scheduler.taskCnt--;
}

/**
  * @brief  Insert task into a sorted queue based on startTick
  * @param  task Task function pointer
  * @param  params Task function parameters pointer
  * @param  size Size of function parameters
  * @param  runtime When task should run in ms
  * @retval None
  */
 void CORE_ScheduleTask(void (*task)(void *), void *params, size_t size, uint32_t runtime) {

  uint32_t executeTick = HAL_GetTick() + runtime;

  // Only add events when dispatch is active
  if (!Scheduler.isActive)
    return;

  // Prevent overflowing scheduler
  assert(Scheduler.taskCnt < SCHEDULER_TASK_LIMIT);

  if (Scheduler.taskQ != NULL) {
    Task_t *t = Scheduler.taskQ;

    // Find the end of the linked-list
    while (t->next != NULL) {
        t = t->next;
    }

    /* now we can add a new task */
    t->next = (Task_t *) malloc(sizeof(Task_t));
    assert(t->next != NULL);

    t->next->func = task;
    t->next->executeTick = executeTick;

    if (params != NULL) {
      t->next->params = malloc(size);
      memcpy(t->next->params, params, size);

    } else {
      t->next->params = NULL;
    }
    t->next->next = NULL;

  } else {
    // taskQ head was NULL so create new linked-list head
    Scheduler.taskQ = (Task_t *) malloc(sizeof(Task_t));
    assert(Scheduler.taskQ != NULL);

    Scheduler.taskQ->func = task;
    Scheduler.taskQ->executeTick = executeTick;

    if (params != NULL) {
      Scheduler.taskQ->params = malloc(size);
      memcpy(Scheduler.taskQ->params, params, size);

    } else {
      Scheduler.taskQ->params = NULL;
    }
    Scheduler.taskQ->next = NULL;
  }
  Scheduler.taskCnt++;
  return;
}


/**
  * @brief  Removing the first task from queue
  * @retval None
  */
 void CORE_ShiftTaskQ(void) {

  if (Scheduler.taskQ == NULL)
      return;

  Task_t *nextTask = NULL;
  if ((Scheduler.taskQ)->next != NULL)
    nextTask = (Scheduler.taskQ)->next;

  if((Scheduler.taskQ)->params != NULL)
    free((Scheduler.taskQ)->params);

  free(Scheduler.taskQ);
  Scheduler.taskQ = nextTask;

  Scheduler.taskCnt--;
}

/**
  * @brief  Removes task from queue without executing it
  * @param  func Task function pointer
  * @retval None
  */
 void CORE_RemoveScheduledTaskByRef(void (*func)(void *)) {
  if (Scheduler.taskQ == NULL)
    return;
  
  // first task so just use shift function
  if (Scheduler.taskQ->func == func) {
    CORE_ShiftTaskQ();
    return;
  }

  // Get the previous task to the searched task to update it's 'next' pointer
  Task_t *prevNode = Scheduler.taskQ;
  while (prevNode->next != NULL) {
    if ( prevNode->next->func == func )
      break;

    prevNode = prevNode->next;
  }

  // Unable to find node
  if ( prevNode == NULL )
    return;

  // move next of searched event to next of previous event
  prevNode->next = pEvent->next;
  if ( pEvent->data != NULL ) {
    free(pEvent->data);
  }
  free(pEvent);
  eventCnt--;
}


int CORE_GetScheduleTaskCount(void) {
  return Scheduler.taskCnt;
}

void CORE_RunScheduler(void) {

  if (! Scheduler.isActive )
    Scheduler.isActive = true;

  if ( Scheduler.taskCnt == 0 )
    return;

  uint32_t now = HAL_GetTick();
  for (uint16_t i = 0; i < Scheduler.taskCnt; i ++) {
    if (Scheduler.taskQ[i].executeTick <= now) {
      _executeTask(i);
      break;  // _executeTask updates taskQ so continuing to process tasks could lead to weird results.
    } else {
      break;  // tasks are ordered by executeTick so no need to check every task
    }
  }
}

