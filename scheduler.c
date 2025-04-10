/*
 * scheduler.c
 *
 *  Created on: Sep 27, 2024
 *      Author: jkp717
 */


#include "scheduler.h"


static Scheduler_t Scheduler = { 0 };


/**
  * @brief  Executes task function and removes from taskQ
  * @param  n Task index in taskQ
  * @retval None
  */
static void _executeTask(uint16_t n) {

  // execute task function
  Scheduler.taskQ[n].func(Scheduler.taskQ[n].params);

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
  * @retval int16_t Returns position added or -1 if unable to add
  */
int CORE_ScheduleTask(void (*task)(void *), void *params, size_t size, uint32_t runtime) {

  int pos = -1;
  uint32_t executeTick = HAL_GetTick() + runtime;

  // Get task array position for new task
  if (Scheduler.taskCnt == 0) {
    pos = 0;

  } else {
    for (int p = 0; p < Scheduler.taskCnt; p++) {
      if (executeTick < Scheduler.taskQ[p].executeTick) {
        pos = p;
        break;
      }
    }

    // executeTick is > all others in taskQ so add it to the end
    if (pos < 0) {
      assert(Scheduler.taskCnt < SCHEDULER_TASK_LIMIT);

      pos = (int)Scheduler.taskCnt;  // add to end
    }

    for (uint16_t i = Scheduler.taskCnt; i > pos; i--) {
      Scheduler.taskQ[i] = Scheduler.taskQ[i - 1];
    }
  }

  void *pParams = NULL;

  // create space for parameters if provided
  if (params) {
    pParams = (void *) malloc(size);
    if (!pParams) {
      debug_msg("[file:%s:lineno:%d] ***ERROR*** Unable to allocate space for task params",
          __FILE__, __LINE__);
      return -1;
    }
    memcpy(pParams, params, size);
  }

  Scheduler.taskQ[pos].func = task;
  Scheduler.taskQ[pos].params = pParams;
  Scheduler.taskQ[pos].executeTick = executeTick;

  Scheduler.taskCnt++;

  return pos;
}


/**
  * @brief  Removes task from queue without executing it
  * @param  task Task function pointer
  * @retval int16_t Returns position removed or -1 if unable to find
  */
int CORE_RemoveScheduleTask(void (*task)(void *)) {

  if (Scheduler.taskCnt == 0)
    return 0;

  uint16_t taskIdx = 0;
  for (uint16_t i = 0; i < Scheduler.taskCnt; i++) {
    if (task == Scheduler.taskQ[i].func) {
      taskIdx = i;
      break;
    } else if (i == (Scheduler.taskCnt - 1)) {
      // reaches the end without finding task
      return -1;
    }
  }
  // free task params space
  if(Scheduler.taskQ[taskIdx].params) {
    free(Scheduler.taskQ[taskIdx].params);
  }
  // remove task from queue
  if (Scheduler.taskCnt > 1) {
    for (uint16_t n = taskIdx; n < Scheduler.taskCnt; n++) {
      Scheduler.taskQ[n] = Scheduler.taskQ[n + 1];
    }
  } else {
    Scheduler.taskQ[0].executeTick = 0;
    Scheduler.taskQ[0].func = NULL;
  }
  Scheduler.taskCnt--;
  return taskIdx;
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

