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
* @struct queue_t
* @brief Estructura de una cola
*/
typedef struct
{
    uint8_t 		* data;         			/**< Buffer donde copiar los elementos a enviar */
    uint32_t 		dataSize;					/**< Tamaño en bytes de los datos */
    uint32_t 		queueLen;					/**< Cantidad maxima de elementos de la cola */
    uint32_t    	readPtr;                    /**< Ultimo item de la cola */
    uint32_t    	writePtr;                   /**< Primer item de la cola */
    semaphore_t 	queuePushSem;               /**< Semaforo asociado al agregado de elementos la cola */
    semaphore_t 	queuePullSem;               /**< Semaforo asociado al agregado de elementos la cola */
}queue_t;

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
void queueInit(queue_t * q, uint32_t queueLen, uint8_t * dataBuffer, uint32_t dataSize);
osReturn_t queuePush(queue_t * q, void * data, tick_t delay);
osReturn_t queuePull(queue_t * q, void * data, tick_t delay);
osReturn_t queuePushFromISR(queue_t * q, void * data);
osReturn_t queuePullFromISR(queue_t * q, void * data);

#endif 
/*==================[end of file]============================================*/
#endif /* #ifndef _OS_QUEUE_H_ */
