/** 
* @file  OS_queue.c
* @brief 
* @note  Copyright 2019 - Esp. Ing. Matias Alvarez.
*/

/*==================[inclusions]=============================================*/
#include "OS_queue.h"
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
#if ( OS_USE_QUEUE == 1 )
/*
* @fn void queueInit(queue_t * q)
* @brief Inicializa una cola
* @params q : Puntero a la cola a ser inicializada
* @return Nada
*/
void queueInit(queue_t * q)
{
    q->tail = 0;
    q->head = 0;
    semphrInit(&(q->queuePushSem));
    semphrInit(&(q->queuePullSem));
}

/*
* @fn osReturn_t queuePush(queue_t * q, queueData_t * data, uint32_t delay)
* @brief Agrega un elemento al final de una cola
* @params q     : Puntero a la cola
* @params data  : Puntero al elemento a agregar a la cola
* @params delay : Tiempo a esperar a que se desocupe un lugar en la cola
* @return OS_RESULT_OK si se pudo agregar el elemento,
          OS_RESULT_ERROR si expira el delay antes de poder agregar el elemento
*/
osReturn_t queuePush(queue_t * q, queueData_t * data, uint32_t delay)
{
    osReturn_t retVal = OS_RESULT_ERROR;

    /* Suspendemos cambio de contexto porque estamos manipulando variables globales */
    osSuspendContextSwitching();
    /* Si la cola esta llena */
    if((q->tail + 1) % OS_QUEUE_LEN == q->head)
    {
        /* Esperamos a que se desocupe un lugar */
        if(OS_RESULT_OK == semphrTake(&(q->queuePushSem), delay))
        {
            retVal = OS_RESULT_OK;
        }
    }
    else
    {
        retVal = OS_RESULT_OK;
    }

    /* Si hay un lugar libre */
    if(OS_RESULT_OK == retVal)
    {
        /* Agregamos el elemento */
        q->data[q->tail] = data;
        /* Movemos el puntero al ultimo elemento */
        q->tail = (q->tail + 1) % OS_QUEUE_LEN;
        /* Liberamos el semaforo indicando que hay un elemento por si existe
           alguna otra tarea esperando que haya un elemento en la cola */
        semphrGive(&(q->queuePullSem));
    }

    osResumeContextSwitching();

    return retVal;

}

/*
* @fn osReturn_t queuePush(queue_t * q, queueData_t * data, uint32_t delay)
* @brief Obtiene un elemento del principio de una cola
* @params q     : Puntero a la cola
* @params data  : Porcion de memoria donde se guarda un elemento de la cola
* @params delay : Tiempo a esperar a que haya un elemento en la cola
* @return OS_RESULT_OK si se pudo obtener el elemento,
          OS_RESULT_ERROR si expira el delay antes de poder obtener el elemento
*/
osReturn_t queuePull(queue_t * q, queueData_t ** data, uint32_t delay)
{
    osReturn_t retVal = OS_RESULT_ERROR;

    /* Suspendemos cambio de contexto porque estamos manipulando variables globales */
    osSuspendContextSwitching();
    /* Si no hay elementos en la cola */
    if(q->head == q->tail)
    {
        /* Esperamos a que haya un elemento */
        if(OS_RESULT_OK == semphrTake(&(q->queuePullSem), delay))
        {
            retVal = OS_RESULT_OK;     
        }
    }
    else
    {
        retVal = OS_RESULT_OK;
    }
    
    /* Si hay al menos un elemento en la cola */
    if(OS_RESULT_OK == retVal)
    {
        /* Obtenemos el elemento */
        *data = q->data[q->head];
        /* Movemos el puntero al primer elemento de la cola */
        q->head = (q->head + 1) % OS_QUEUE_LEN;
        /* Liberamos el semaforo indicando que hay un lugar en la cola por si existe
           alguna otra tarea esperando que haya espacio en la misma */
        semphrGive(&(q->queuePushSem));
    }

    osResumeContextSwitching();

    return retVal;
}

#endif
/*==================[end of file]============================================*/
