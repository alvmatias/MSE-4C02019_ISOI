/** 
* @file  main.c
* @brief 
* @note  Copyright 2019 - Esp. Ing. Matias Alvarez.
*/

/*==================[inclusions]=============================================*/
#include "OS.h"
#include "OS_config.h"
#include "board.h"
#include <stdint.h>
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/
/* Buffers a las pilas de cada tarea */
uint32_t taskAStack[OS_MINIMAL_STACK_SIZE];
uint32_t taskBStack[OS_MINIMAL_STACK_SIZE];
uint32_t taskCStack[OS_MINIMAL_STACK_SIZE];

uint32_t taskAStackPointer;
uint32_t taskBStackPointer;
uint32_t taskCStackPointer;
/*==================[internal functions definition]==========================*/
void taskA(void * parameters)
{
	uint32_t i;

	while(1) 
	{
		Board_LED_Toggle(3);
		for (i=0; i<0x3FFFFF; i++)
			;
	}
}

void taskB(void * parameters)
{
	uint32_t j;

	while(1) 
	{
		Board_LED_Toggle(2);
		for (j=0; j<0xFFFFF; j++)
			;
	}
}

void taskC(void * parameters)
{
	uint32_t j;
	uint8_t * prm = (uint8_t*)parameters;
	while(1) 
	{
		if(*prm == 3)
			Board_LED_Toggle(0);
		for (j=0; j<0xFFFFF; j++)
			;
	}
}
/*==================[external functions definition]==========================*/
int main(void){

	Board_Init();
	SystemCoreClockUpdate();
	uint8_t taskCParams = 3;
	/* Creacion de las tareas */
	taskCreate(taskA, taskAStack, OS_MINIMAL_STACK_SIZE, &taskAStackPointer, (void *)1);
	taskCreate(taskB, taskBStack, OS_MINIMAL_STACK_SIZE, &taskBStackPointer, (void *)2);
	taskCreate(taskC, taskCStack, OS_MINIMAL_STACK_SIZE, &taskCStackPointer, (void *)(&taskCParams));
	
	/* Start the scheduler */
	taskStartScheduler();

	/* No se deberia arribar aqui nunca */
	return 1;
}
/*==================[end of file]============================================*/

