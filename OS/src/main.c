/** 
* @file  main.c
* @brief 
* @note  Copyright 2019 - Esp. Ing. Matias Alvarez.
*/

/*==================[inclusions]=============================================*/
#include "OS.h"
#include "OS_config.h"
#include "OS_semphr.h"
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

semaphore_t semA;
/*==================[internal functions definition]==========================*/
void taskA(void * parameters)
{
    uint32_t i;

    semphrInit(&semA);
    while(1) 
    {
        if(OS_RESULT_OK == semphrTake(&semA, 500))
        {
            Board_LED_Toggle(3);
        }
        else
        {
            Board_LED_Toggle(2);
        }
        
    }
}

void taskB(void * parameters)
{
    uint32_t i;

    while(1) 
    {
        semphrGive(&semA);
        //Board_LED_Toggle(2);
        taskDelay(2500);
    }
}

void taskC(void * parameters)
{
    uint32_t i;
    uint8_t * prm = (uint8_t*)parameters;
    while(1) 
    {
        if(*prm == 3)
            Board_LED_Toggle(0);
        taskDelay(1000);
    }
}
/*==================[external functions definition]==========================*/
int main(void){

    Board_Init();
    SystemCoreClockUpdate();
    uint8_t taskCParams = 3;
    /* Creacion de las tareas */
    taskCreate(taskA, 3, taskAStack, OS_MINIMAL_STACK_SIZE, "taskA", (void *)1);
    taskCreate(taskB, 2, taskBStack, OS_MINIMAL_STACK_SIZE, "taskB", (void *)2);
    taskCreate(taskC, 2, taskCStack, OS_MINIMAL_STACK_SIZE, "taskC", (void *)(&taskCParams));
    
    /* Start the scheduler */
    taskStartScheduler();

    /* No se deberia arribar aqui nunca */
    return 1;
}
/*==================[end of file]============================================*/

