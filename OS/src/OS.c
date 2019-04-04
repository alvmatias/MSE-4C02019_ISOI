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
#define OS_INVALID_TASK		0xFF
#define OS_IDLE_TASK 		OS_MAX_TASK
#define OS_NULL_PRIORITY	0

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
	uint32_t * 		stack; 				/**< Puntero al stack de la tarea - Buffer provisto por el usuario del SO */
	uint32_t    	stackSize;			/**< Tamaño del stack de la tarea */
	uint32_t 		stackPointer;		/**< Puntero de pila */   
	taskFunction_t 	taskFx;     		/**< Tarea a ejecutar */
	void  * 		parameters;			/**< Puntero a los parametros de la tarea */
	uint32_t  		priority;			/**< Prioridad de la tarea */
	#if ( OS_USE_TASK_DELAY == 1 )
		taskState_t 	state;			/**< Estado de la tarea */
		uint32_t        ticksToWait;	/**< Ticks a esperar en caso de ejecucion de taskDelay() */
	#endif
}taskControlBlock_t;

/* Invertimos memoria por tiempo de ejecucion del scheduler */
typedef struct 
{
	uint8_t readyTaskCnt;		/**< Cantidad de tareas ready */
	uint8_t firstReadyTask; 	/**< Puntero circular a la primera tarea ready */
}readyTaskInfo_t;
/*==================[internal data declaration]==============================*/
#if ( OS_USE_TASK_DELAY == 1 )
	/* Si esta estipulado el uso de delay se crea un espacio para la idle task */
	static uint32_t idleTaskStack[OS_IDLE_STACK_SIZE];

	static taskControlBlock_t taskList[OS_MAX_TASK + 1];
#else
	static taskControlBlock_t taskList[OS_MAX_TASK];
#endif

/* Para un mejor uso de memoria solo guardaremos la posicion de la tarea en el arreglo de TCB's */
static uint32_t 		readyTaskList[OS_MAX_TASK_PRIORITY][OS_MAX_TASK];
/* A su vez llevamos informacion de las tareas ready en cada prioridad */
static readyTaskInfo_t 	readyTaskInfo[OS_MAX_TASK_PRIORITY];
/* Cantidad de tareas agregadas al SO */
static uint8_t 			maxTask 	= 0;
/* Tarea actual */
static uint8_t 			currentTask = OS_INVALID_TASK;	
/* Tick del sistema */
static uint32_t 		tickCount 	= 0;
/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

#if ( OS_USE_TICK_HOOK == 1 )
	/**
	*
	*
	*/
	__attribute__ ((weak)) void tickHook(void)
	{

	}
#endif

#if ( OS_USE_TASK_DELAY == 1 )
	/**
	*
	*
	*/
	__attribute__ ((weak)) void idleHook(void * parameters)
	{
		while(1)
		{
			__WFI();
		}
	}
#endif

/**
*
*
*/
static void returnHook()
{
	while(1)
		;
}

/**
*
*
*/
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

/**
*
*
*/
static bool taskIncrementTick()
{
	bool retVal = false;

	tickCount++;
	if(0 == (tickCount % OS_TICKS_UNTIL_SCHEDULE)) 
	{

		#if ( OS_USE_TICK_HOOK == 1 )
		{
			tickHook();
		}
		#endif

		retVal = true;
	}	

	return retVal;

}

/* TO DO: Implementar FIFO SCHED */
#if ( OS_USE_ROUND_ROBIN_SCHED == 1)
	/**
	*
	*
	*/
	static void addReadyTask(uint8_t id, uint32_t priority)
	{

		readyTaskList[priority]
		[(readyTaskInfo[priority].firstReadyTask + readyTaskInfo[priority].readyTaskCnt) % maxTask] = id;
		readyTaskInfo[priority].readyTaskCnt++;

	}

	static void removeReadyTask(uint8_t * id, uint32_t priority)
	{
		*id = readyTaskList[priority][readyTaskInfo[priority].firstReadyTask];
		readyTaskInfo[priority].readyTaskCnt--;
		if(0 == readyTaskInfo[priority].readyTaskCnt)
		{
			readyTaskInfo[priority].firstReadyTask = 0;
		}
		else
		{
			readyTaskInfo[priority].firstReadyTask = (readyTaskInfo[priority].firstReadyTask + 1) % maxTask;
		}
		
	}
#endif	

#if ( OS_USE_TASK_DELAY == 1 )
	/**
	*
	*
	*/
	static void delayUpdate(void)
	{
		uint8_t i;
		for (i = 0; i < maxTask; i++) 
		{
			if (TASK_STATE_BLOCKED == taskList[i].state && 0 < taskList[i].ticksToWait) 
			{
				taskList[i].ticksToWait--;
				if (taskList[i].ticksToWait == 0) {
					taskList[i].state = TASK_STATE_READY;
					addReadyTask(i, taskList[i].priority - 1);
				}
			}
		}
	}
#endif

/**
*
*
*/
static void initStack(uint32_t * stack, 
					  uint32_t stackSize, 
					  uint32_t priority,	
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
	taskList[maxTask].priority		= priority;
	taskList[maxTask].state 		= TASK_STATE_READY;

	#if ( OS_USE_TASK_DELAY == 1 )
		taskList[maxTask].ticksToWait   = 0;
	#endif

}
/*==================[external functions definition]==========================*/
/**
*
*
*/
uint8_t taskCreate(taskFunction_t taskFx, uint32_t priority, uint32_t * stack, uint32_t stackSize,
				   void * parameters)
{
	uint8_t retVal = false;

	if(OS_MAX_TASK > maxTask && OS_MINIMAL_STACK_SIZE <= stackSize && 
		OS_MAX_TASK_PRIORITY >= priority && OS_NULL_PRIORITY != priority)
	{

		initStack(stack, stackSize, priority, taskFx, parameters);
		maxTask++;

		retVal = true;
		
	}

	return retVal;
}

/**
*
*
*/
void taskStartScheduler()
{
	uint8_t p;

	#if ( OS_USE_TASK_DELAY == 1 )
		/* Creacion idle task */
		initStack(idleTaskStack, OS_IDLE_STACK_SIZE, OS_NULL_PRIORITY, idleHook, (void*)0);
	#endif

	for(p = 0; p < maxTask; p++)
	{
		addReadyTask(p, taskList[p].priority - 1);
	}
	for(p = 0; p < OS_MAX_TASK_PRIORITY; p++)
	{
		readyTaskInfo[p].firstReadyTask = 0;
	}

	/* Systick y pendSV con menor prioridad posible */
	NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS) - 1);

	SysTick_Config(SystemCoreClock / 1000);

	schedule();

	/* No deberia arribar aca */
	while(1)
	{
		__WFI();
	}
}

/**
*
*
*/
int32_t taskSchedule(int32_t currentContext)
{
	uint8_t p;

	if(OS_IDLE_TASK == currentTask)
	{
		/* Aca se entra cuando arranca el SO y cuando volvemos de la idle task */
		#if ( OS_USE_TASK_DELAY == 1 )

			taskList[maxTask].stackPointer 	= currentContext; /* Guardamos el contexto de la idle task */
			taskList[maxTask].state 		= TASK_STATE_READY;
			
		#endif	
	}
	else
	{
		taskList[currentTask].stackPointer 	= currentContext;
		/* Podemos haber entrado al schedule por culpa de un taskDelay */
		if(TASK_STATE_RUNNING == taskList[currentTask].state )
		{
			taskList[currentTask].state = TASK_STATE_READY;
			addReadyTask(currentTask, taskList[currentTask].priority - 1);
		}
		
	}

	for(p = 0; p < OS_MAX_TASK_PRIORITY; p++)
	{
		if(readyTaskInfo[p].readyTaskCnt > 0)
		{
			/* Removemos siempre la primera */
			removeReadyTask(&currentTask, p);
			break;
		}
	}
	if(p >= OS_MAX_TASK_PRIORITY)
	{
		currentTask = maxTask;
	}
	
	taskList[currentTask].state  = TASK_STATE_RUNNING;
	return taskList[currentTask].stackPointer;

}

#if ( OS_USE_TASK_DELAY == 1 )
	/**
	*
	*
	*/
	void taskDelay(uint32_t ticksToDelay)
	{
		if(0 < ticksToDelay && OS_IDLE_TASK != currentTask)
		{
			taskList[currentTask].state = TASK_STATE_BLOCKED;
			taskList[currentTask].ticksToWait = ticksToDelay; 
			schedule();
		}
	}
#endif

/**
*
*
*/
uint32_t taskGetTickCount()
{
	return tickCount;
}

/*==================[IRQ Handlers]======================================*/
/**
*
*
*/
void SysTick_Handler( void )
{
	
	/* Increment the RTOS tick. */
	if(taskIncrementTick())
	{
		#if ( OS_USE_TASK_DELAY == 1 )
			delayUpdate();
    	#endif

		schedule();
	}
	
	

}
/*==================[end of file]============================================*/

