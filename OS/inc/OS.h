/** 
* @file  OS.h
* @brief 
* @note  Copyright 2019 - Esp. Ing. Matias Alvarez.
*/
#ifndef _OS_H_
#define _OS_H_

/*==================[inclusions]=============================================*/
#include <stdint.h>
/*==================[macros]=================================================*/
#define OS_MAX_DELAY ( uint32_t ) 0xffffffffUL
/*==================[typedef]================================================*/
/**
* @def void (*taskFunction_t)(void *)
* @brief Definicion de prototipo de tarea del SO
*/
typedef void (*taskFunction_t)(void *);
/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
uint8_t taskCreate(taskFunction_t pxTaskCode, uint32_t * stack, uint32_t stackSize,
				   void * paramenter);

void 	taskStartScheduler();

int32_t taskSchedule(int32_t actualContext);

void 	taskDelay(uint32_t ticksToDelay);
/*==================[end of file]============================================*/
#endif /* #ifndef _OS_H_ */
