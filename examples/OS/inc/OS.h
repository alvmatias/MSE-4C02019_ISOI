/** 
* @file  OS.h
* @brief Archivo cabecera del SO
* @note  NO deberia ser modificado por el usuario
* @note  Copyright 2019 - Esp. Ing. Matias Alvarez.
*/
#ifndef _OS_H_
#define _OS_H_

/*==================[inclusions]=============================================*/
#include <stdint.h>
#include "OS_config.h"
/*==================[macros]=================================================*/
#ifndef OS_MINIMAL_STACK_SIZE
    #error Missing definition:  OS_MINIMAL_STACK_SIZE must be defined in OS_config.h. 
#endif

#ifndef OS_IDLE_STACK_SIZE
    #error Missing definition:  OS_MINIMAL_STACK_SIZE must be defined in OS_config.h. OS_MINIMAL_STACK_SIZE the size in bytes of the stack allocated to the idle task.
#endif

#ifndef OS_MAX_TASK
    #error Missing definition:  OS_MAX_TASK must be defined in OS_config.h. 
#endif

#if OS_MAX_TASK < 1
    #error OS_MAX_TASK must be defined in OS_config.h to be greater than or equal to 1.
#endif

#ifndef OS_TICKS_UNTIL_SCHEDULE
    #error Missing definition:  OS_TICKS_UNTIL_SCHEDULE must be defined in OS_config.h as either 1 or 0.
#endif

#ifndef OS_MAX_TASK_PRIORITY
    #error Missing definition:  OS_MAX_TASK_PRIORITY must be defined in OS_config.h.
#endif

#if OS_MAX_TASK_PRIORITY < 1
    #error OS_MAX_TASK_PRIORITY must be defined to be greater than or equal to 1.
#endif

#ifndef OS_MAX_TASK_NAME_LEN
    #error Missing definition:  OS_MAX_TASK_NAME_LEN must be defined in OS_config.h.
#endif

#ifndef OS_USE_PRIO_ROUND_ROBIN_SCHED
    #error Missing definition:  OS_USE_PREEMPTIVE_SCHED must be defined in OS_config.h as either 1 or 0.
#endif

#ifndef OS_USE_TICK_HOOK
    #error Missing definition:  OS_USE_TICK_HOOK must be defined in OS_config.h as either 1 or 0.
#endif

#ifndef OS_USE_QUEUE
    #define OS_USE_QUEUE        0
#elif (OS_USE_QUEUE == 1)
    #ifndef OS_USE_SEMPHR
        #define OS_USE_SEMPHR   1
    #elif (OS_USE_SEMPHR != 1)    
        #error OS_USE_SEMPHR must be defined to be equal to 1 when OS_USE_QUEUE == 1.
    #endif
#endif

#ifndef OS_USE_SEMPHR
    #define OS_USE_SEMPHR        0
#elif (OS_USE_SEMPHR == 1)
    #ifndef OS_USE_TASK_DELAY
        #define OS_USE_TASK_DELAY   1
    #elif (OS_USE_TASK_DELAY != 1)    
        #error OS_USE_TASK_DELAY must be defined to be equal to 1 when OS_USE_SEMPHR == 1.
    #endif
#endif

#ifndef OS_USE_TASK_DELAY
    #define OS_USE_TASK_DELAY   0
#endif

#ifndef NULL
    #define NULL    ((void *)0)
#endif
/**
* @def OS_MAX_DELAY
* @brief Tiempo de delay maximo posible
*/
#define OS_MAX_DELAY ( uint32_t ) 0xffffffffUL

/**
* @def OS_IDLE_TASK
* @var ID de la tarea idle
*/
#define OS_IDLE_TASK        OS_MAX_TASK

/**
* @def OS_INVALID_TASK
* @var ID de la tarea invalida
*/
#define OS_INVALID_TASK     0xFF
/*==================[typedef]================================================*/
/**
* @def void (*taskFunction_t)(void *)
* @brief Definicion de prototipo de tarea del SO
* @danger Como indica el prototipo, las tareas NUNCA retornan
*/
typedef void (*taskFunction_t)(void *);

/**
* @def uint32_t tick_t
* @brief Tipo de datos de delay
*/
typedef uint32_t tick_t;

/**
* @enum osReturn_t 
* @brief Posibles valores de retorno de las llamadas al sistema
*/
typedef enum
{
    OS_RESULT_OK = 0x00
,   OS_RESULT_ERROR
}osReturn_t;
/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
osReturn_t taskCreate(taskFunction_t taskFx, uint32_t priority, uint32_t * stack, uint32_t stackSize,
                   char * taskName, void * parameters);

void    taskStartScheduler();

int32_t taskSchedule(int32_t currentContext);

void    taskDelay(tick_t ticksToDelay);

tick_t taskGetTickCount();

void taskUnsuspendWithinAPI(uint8_t taskId);

void osSuspendContextSwitching();

void osResumeContextSwitching();

uint8_t osGetCurrentTask();

uint8_t osIsIdleTask(uint8_t taskId);

void taskYield();
/*==================[end of file]============================================*/
#endif /* #ifndef _OS_H_ */
