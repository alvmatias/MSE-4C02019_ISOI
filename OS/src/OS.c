/** 
* @file  OS.c
* @brief 
* @note  Copyright 2019 - Esp. Ing. Matias Alvarez.
*/

/*==================[inclusions]=============================================*/
#include "OS_config.h"
#include "OS.h"
#include "board.h"
#include <string.h>
/*==================[macros]=================================================*/
#define OS_INVALID_TASK 	0xFF
#define EXC_RETURN			0xFFFFFFF9

#define NVIC_PENDSV_PRI		0xff
/*==================[typedef]================================================*/
typedef enum
{
	TASK_STATE_ERROR
,	TASK_STATE_READY
,	TASK_STATE_RUNNING
,	TASK_STATE_BLOCKED
,	TASK_STATE_TERMINATED  /* TO DO */
}taskState_t;
/**
* @struct taskControlBlock_t
* @brief Estructura de control de cada tarea del SO
*/
typedef struct 
{
	uint32_t * 		stack; 			/**< Puntero al stack de la tarea - Buffer provisto por el usuario del SO */
	uint32_t    	stackSize;		/**< Tamaño del stack de la tarea */
	uint32_t 		stackPointer;	/**< Puntero de pila */   
	taskFunction_t 	taskFx;     	/**< Tarea a ejecutar */
	void  * 		parameters;		/**< Puntero a los parametros de la tarea */
	
	#if ( OS_USE_TASK_DELAY == 1 )
		taskState_t 	state;
		uint32_t        ticksToWait;
	#endif
}taskControlBlock_t;



/*==================[internal data declaration]==============================*/
#if ( OS_USE_TASK_DELAY == 1 )
	static uint32_t idleTaskStack[OS_MINIMAL_STACK_SIZE];

	static taskControlBlock_t taskList[OS_MAX_TASK + 1];
#else
	static taskControlBlock_t taskList[OS_MAX_TASK];
#endif

static uint8_t maxTask = 0;
static uint8_t currentTask = OS_INVALID_TASK;

static uint32_t tickCount = 0;
/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
__attribute__ ((weak)) void tickHook(void)
{

}

__attribute__ ((weak)) void idleHook(void * parameters)
{
	while(1)
	{
		__WFI();
	}
}

static void returnHook()
{
	while(1)
		;
}

static void initStack(uint32_t * stack, 
					  uint32_t stackSize, 	
					  taskFunction_t taskFx,
					  void * parameters)
{

	/* Inicializo el frame en cero */
	bzero(stack, stackSize);

	/* Ultimo elemento del contexto inicial: xPSR
	 * Necesita el bit 24 (T, modo Thumb) en 1
	 */
	stack[stackSize/4 - 1] 	= 1 << 24;

	/* Anteultimo elemento: PC (entry point) */
	stack[stackSize/4 - 2] 	= (uint32_t)taskFx;

	/* Penultimo elemento: LR (return hook) */
	stack[stackSize/4 - 3] 	= (uint32_t)returnHook;

	/* Elemento -8: R0 (parámetro) */
	stack[stackSize/4 - 8] 	= (uint32_t)parameters;

	stack[stackSize/4 - 9] 	= EXC_RETURN;

	/* Inicializo stack pointer inicial considerando lo otros 8 registros pusheados */
	taskList[maxTask].stackPointer	= (uint32_t)&(stack[stackSize/4 - 17]);

	/* Inicialiazo el TCB */
	taskList[maxTask].taskFx 		= taskFx;
	taskList[maxTask].stack 		= stack;
	taskList[maxTask].stackSize		= stackSize;
	taskList[maxTask].parameters	= parameters;

	#if ( OS_USE_TASK_DELAY == 1 )
		taskList[maxTask].state 		= TASK_STATE_READY;
		taskList[maxTask].ticksToWait   = 0;
	#endif

}

static void schedule()
{
	/* Instruction Synchronization Barrier: aseguramos que se
	 * ejecuten todas las instrucciones en  el pipeline
	 */
	__ISB();
	/* Data Synchronization Barrier: aseguramos que se
	 * completen todos los accesos a memoria
	 */
	__DSB();

	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
	
}
#if ( OS_USE_TASK_DELAY == 1 )
	static void delayUpdate(void)
	{
		uint8_t i;
		for (i = 0; i < maxTask; i++) {
			if (TASK_STATE_BLOCKED == taskList[i].state && 0 < taskList[i].ticksToWait) {
				if (--taskList[i].ticksToWait == 0) {
					taskList[i].state = TASK_STATE_READY;
				}
			}
		}
	}
#endif
/*==================[external functions definition]==========================*/
uint8_t taskCreate(taskFunction_t taskFx, uint32_t * stack, uint32_t stackSize,
				   void * parameters)
{
	uint8_t retVal = false;

	if(OS_MAX_TASK > maxTask)
	{

		if(OS_MINIMAL_STACK_SIZE <= stackSize)
		{
			initStack(stack, stackSize, taskFx, parameters);
			maxTask++;

			retVal = true;
		}
		
	}

	return retVal;
}

void taskStartScheduler()
{
	#if ( OS_USE_TASK_DELAY == 1 )
		/* Creacion idle task */
		initStack(idleTaskStack, OS_MINIMAL_STACK_SIZE, idleHook, (void*)0);

		currentTask = maxTask;
	#endif

	/* Systick y pendSV con menor prioridad posible */
	NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS) - 1);

	SysTick_Config(SystemCoreClock / 1000);

	schedule();

	while(1)
	{
		__WFI();
	}
}



int32_t taskSchedule(int32_t currentContext)
{
	#if ( OS_USE_TASK_DELAY == 1 )
		uint8_t previousTask = currentTask;
	#endif

	if(OS_INVALID_TASK == currentTask)
	{
		/* Aca se entra cuando arranca el SO y cuando volvemos de la idle task */
		#if ( OS_USE_TASK_DELAY == 1 )
			taskList[maxTask].stackPointer = currentContext; /* Guardamos el contexto de la idle task */
		#endif
		currentTask = 0;
	}
	else
	{
		taskList[currentTask].stackPointer = currentContext;
		//taskList[previousTask].state = TASK_STATE_READY;
		currentTask = (currentTask + 1) % maxTask; /* Aumentamos 1 de manera circular */
	}
	/*
	 * Existen 4 posibilidades de cambio de contexto
     * 1 - Estamos en una tarea RUNNING y pasamos a una tarea READY
     * 2 - Estamos en una tarea RUNNING, NO hay tareas READY y volvemos a la tarea RUNNING
     * 3 - Estamos en una tarea que llamo a taskDelay y entro en estado BLOCKED y pasamos a una tarea READY
     * 4 - Estamos en una tarea que llamo a taskDelay y entro en estado BLOCKED, NO hay tarea READY y pasamos
     * a la tarea idle 
	*/
	#if ( OS_USE_TASK_DELAY == 1 )
		/* Salgo del while cuando encuentro una tarea ready
		 * o cuando doy toda la vuelta -> es decir que previousTask == currentTask */
		/* Si salgo del while al encontrar una tarea ready ataco los casos 1 y 3 */
		/* Si salgo del while al volver a la tarea actual, tengo que diferenciar los casos 2 y 4 */
		while(TASK_STATE_READY != taskList[currentTask].state && previousTask != currentTask)
		{
			currentTask = (currentTask + 1) % maxTask; /* Aumentamos 1 de manera circular */
		}
		/* Si sali del while es porque volvi a la misma tarea */
		if(previousTask == currentTask)
		{
			/* Si esta pausada == Caso 4 */
			if(TASK_STATE_BLOCKED == taskList[currentTask].state)
			{
				/* En maxTask almacenamos el estado de la idleTask al empezar el SO */
				taskList[maxTask].state = TASK_STATE_RUNNING;
				currentTask = OS_INVALID_TASK;
				return taskList[maxTask].stackPointer; /* Le doy el procesador a la idle task */
			}
			/* else Caso 2, no hago nada porque sigo en el mismo contexto */		
		}
		else /* Casos 1 y 3 */
		{
			/* Caso 1 */
			if(TASK_STATE_RUNNING == taskList[previousTask].state)
			{
				taskList[previousTask].state = TASK_STATE_READY;
				
			}
			/* else Caso 3 ataco el problema en la siguiente linea */

			taskList[currentTask].state  = TASK_STATE_RUNNING;

		}
	#endif

	return taskList[currentTask].stackPointer;
	
}

#if ( OS_USE_TASK_DELAY == 1 )
void taskDelay(uint32_t ticksToDelay)
{
	if(0 < ticksToDelay && OS_INVALID_TASK != currentTask)
	{
		taskList[currentTask].state = TASK_STATE_BLOCKED;
		taskList[currentTask].ticksToWait = ticksToDelay; 
		schedule();
	}
}
#endif

bool taskIncrementTick()
{
	bool retVal = false;

	if(!((++tickCount) % OS_TICKS_UNTIL_SCHEDULE))
	{
		retVal = true;
	}	

	#if ( OS_USE_TICK_HOOK == 1 )
	{
		tickHook();
	}
	#endif

	return retVal;

}

uint32_t taskGetTickCount()
{
	return tickCount;
}

/*==================[IRQ Handlers]======================================*/

void SysTick_Handler( void )
{
	
	/* Increment the RTOS tick. */
	if(taskIncrementTick())
	{
		schedule();
	}
	
	#if ( OS_USE_TASK_DELAY == 1 )
		delayUpdate();
    #endif

}
/*==================[end of file]============================================*/

