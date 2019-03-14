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
/*==================[typedef]================================================*/
/**
* @struct taskControlBlock_t
* @brief Estructura de control de cada tarea del SO
*/
typedef struct 
{
	uint32_t * 		stack; 			/**< Puntero al stack de la tarea - Buffer provisto por el usuario del SO */
	uint32_t    	stackSize;		/**< Tamaño del stack de la tarea */
	uint32_t *		stackPointer;	/**< Puntero de pila */   
	taskFunction_t 	taskFx;     	/**< Tarea a ejecutar */
	void  * 		parameter;		/**< Puntero a los parametros de la tarea */
}taskControlBlock_t;
/*==================[internal data declaration]==============================*/
static taskControlBlock_t taskList[OS_MAX_TASK];
/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
static void returnHook()
{
	while(1)
		;
}
/*==================[external functions definition]==========================*/
uint8_t taskCreate(taskFunction_t taskFx, uint32_t * stack, uint32_t stackSize,
				   uint32_t * stackPointer, void * parameter)
{
	static uint8_t currentTask = 0;
	uint8_t retVal = false;

	if(OS_MAX_TASK > currentTask)
	{

		if(OS_MINIMAL_STACK_SIZE <= stackSize)
		{
			/* Inicializo el frame en cero */
			bzero(stack, stackSize);

			/* Ultimo elemento del contexto inicial: xPSR
			 * Necesita el bit 24 (T, modo Thumb) en 1
			 */
			stack[stackSize/4 - 1] 	= 1<<24;

			/* Anteultimo elemento: PC (entry point) */
			stack[stackSize/4 - 2] 	= (uint32_t)taskFx;

			/* Penultimo elemento: LR (return hook) */
			stack[stackSize/4 - 3] 	= (uint32_t)returnHook;

			/* Elemento -8: R0 (parámetro) */
			stack[stackSize/4 - 8] 	= (uint32_t)parameter;

			stack[stackSize/4 - 9] 	= 0xFFFFFFF9;

			/* Inicializo stack pointer inicial considerando lo otros 8 registros pusheados */
			*stackPointer 			= (uint32_t)&(stack[stackSize/4 - 17]);

			/* Inicialiazo el TCB */
			taskList[currentTask].taskFx 		= taskFx;
			taskList[currentTask].stack 		= stack;
			taskList[currentTask].stackSize		= stackSize;
			taskList[currentTask].stackPointer	= stackPointer;
			taskList[currentTask].parameter		= parameter;

			currentTask++;

			retVal = true;
		}
		
	}

	return retVal;
}

void taskStartScheduler()
{
	SysTick_Config(SystemCoreClock / 1000);
	while(1)
	{
		__WFI();
	}
}



int32_t taskSchedule(int32_t currentContext)
{
	static uint8_t currentTask = OS_INVALID_TASK;

	if(OS_INVALID_TASK == currentTask)
	{
		currentTask = 0;
	}
	else
	{
		currentTask = (currentTask + 1) % OS_MAX_TASK; /* Aumentamos 1 de manera circular */
		/* Si no dimos la vuelta, es decir que no estamos en la primer tarea */
		if(currentTask)
			*(taskList[currentTask - 1].stackPointer) = currentContext;
		else /* Si es la primer tarea */
			*(taskList[OS_MAX_TASK - 1].stackPointer) = currentContext;
	}

	return *(taskList[currentTask].stackPointer);
	
}
/*==================[end of file]============================================*/

