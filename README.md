# stm32-scheduler
A simple C scheduler for STM32 MCUs

# Setup
To use, add scheduler.c and scheduler.h to application.

# Usage
Add the CORE_RunScheduler() in main while loop. This function will call a scheduled task when the task runtime <= systick. To add a new task to the scheduler, use CORE_ScheduleTask. Parameter(s) can also be passed to the task function by pointer.  
