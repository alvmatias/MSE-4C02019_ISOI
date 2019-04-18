/** 
* @file  OS_queue.h
* @brief 
* @note  Copyright 2019 - Esp. Ing. Matias Alvarez.
*/
#ifndef _OS_QUEUE_H_
#define _OS_QUEUE_H_


/*==================[inclusions]=============================================*/
#include "OS_config.h"
#include "OS.h"
#include "OS_semphr.h"
#include <stdbool.h>
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/
#if ( OS_USE_QUEUE == 1 )
/*
* @struct queueData_t
* @brief Estructura de un elemento de la cola
*/
typedef struct
{
	void * 		ptrToData;
	uint32_t 	dataLen;
}queueData_t;

/*
* @struct queue_t
* @brief Estructura de una cola
*/
typedef struct
{
    queueData_t * 	data[OS_QUEUE_LEN];         /**< Cola de punteros a queueData_t */
    uint32_t    	head;                       /**< Ultimo item de la cola */
    uint32_t    	tail;                       /**< Primer item de la cola */
    semaphore_t 	queuePushSem;               /**< Semaforo asociado al agregado de elementos la cola */
    semaphore_t 	queuePullSem;               /**< Semaforo asociado al agregado de elementos la cola */
}queue_t;

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
void queueInit(queue_t * q);
osReturn_t queuePush(queue_t * q, queueData_t * data, uint32_t delay);
osReturn_t queuePull(queue_t * q, queueData_t ** data, uint32_t delay);

#endif 

/*==================[end of file]============================================*/

#endif /* #ifndef _OS_QUEUE_H_ */
