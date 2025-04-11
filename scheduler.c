/*
 * scheduler.c
 *
 *  Created on: Sep 27, 2024
 *      Author: jkp717
 */


#include "scheduler.h"


static Scheduler_t _scheduler = {
  .taskQ = NULL,
  .taskCnt = 0,
  .isActive = false
};


/**
  * @brief  Insert task into a sorted queue based on startTick
  * @param  task Task function pointer
  * @param  params Task function parameters pointer
  * @param  size Size of function parameters
  * @param  runtime When task should run in ms
  * @retval None
  */
 void CORE_ScheduleTask(SchedulerTaskFunc_t func, void *params, size_t size, uint32_t runtime) {

  uint32_t executeTick = HAL_GetTick() + runtime;

  // Only add events when dispatch is active
  if (!_scheduler.isActive)
    return;

  // Prevent overflowing scheduler
  assert(_scheduler.taskCnt < SCHEDULER_TASK_LIMIT);

  if (_scheduler.taskQ != NULL) {
    Task_t *t = _scheduler.taskQ;

    // Find the end of the linked-list
    while (t->next != NULL) {
        t = t->next;
    }

    /* now we can add a new task */
    t->next = (Task_t *) malloc(sizeof(Task_t));
    assert(t->next != NULL);

    t->next->func = func;
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
    _scheduler.taskQ = (Task_t *) malloc(sizeof(Task_t));
    assert(_scheduler.taskQ != NULL);

    _scheduler.taskQ->func = func;
    _scheduler.taskQ->executeTick = executeTick;

    if (params != NULL) {
      _scheduler.taskQ->params = malloc(size);
      memcpy(_scheduler.taskQ->params, params, size);

    } else {
      _scheduler.taskQ->params = NULL;
    }
    _scheduler.taskQ->next = NULL;
  }
  _scheduler.taskCnt++;
  return;
}


/**
  * @brief  Removing the first task from queue
  * @retval None
  */
 void CORE_ShiftTaskQ(void) {

  if (_scheduler.taskQ == NULL)
      return;

  Task_t *nextTask = NULL;
  if ((_scheduler.taskQ)->next != NULL)
    nextTask = (_scheduler.taskQ)->next;

  if((_scheduler.taskQ)->params != NULL)
    free((_scheduler.taskQ)->params);

  free(_scheduler.taskQ);
  _scheduler.taskQ = nextTask;

  _scheduler.taskCnt--;
}

/**
  * @brief  Removes task from queue without executing it
  * @param  func Task function pointer
  * @retval Pointer to previous node in linked list or NULL if unable to find
  */
Task_t* CORE_RemoveScheduleTaskByRef(void (*func)(void *)) {
  if (_scheduler.taskQ == NULL)
    return NULL;

  // first task so just use shift function
  if (_scheduler.taskQ->func == func) {
    CORE_ShiftTaskQ();
    return NULL;
  }

  // Get the previous task to the searched task to update it's 'next' pointer
  Task_t *prevNode = _scheduler.taskQ;
  while (prevNode->next != NULL) {
    if ( prevNode->next->func == func )
      break;

    prevNode = prevNode->next;
  }

  // Unable to find node
  if ( prevNode == NULL )
    return NULL;

  // prevNode->next == searched node
  // now we have previous and next nodes, so we can delete the searched node
  Task_t *nextNode = prevNode->next->next;

  // free searched node params
  if ( prevNode->next->params != NULL ) {
    free(prevNode->next->params);
  }

  // free searched node
  free(prevNode->next);

  prevNode->next = nextNode;
  _scheduler.taskCnt--;

  return prevNode;
}



int CORE_GetScheduleTaskCount(void) {
  return _scheduler.taskCnt;
}

void CORE_RunScheduler(void) {

  if (! _scheduler.isActive )
    _scheduler.isActive = true;

  if ( _scheduler.taskCnt == 0 )
    return;

  uint32_t now = HAL_GetTick();

  Task_t *pTask = _scheduler.taskQ;
  while (pTask != NULL) {

    if (pTask->executeTick <= now) {

      pTask->func(pTask->params);

      // Returns previous node to the one searched to continue loop
      pTask = CORE_RemoveScheduleTaskByRef(pTask->func);

    } else {
      pTask = pTask->next;
    }
  }
}
