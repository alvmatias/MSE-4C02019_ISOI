/** 
* @file  OS_queue.c
* @brief 
* @note  Copyright 2019 - Esp. Ing. Matias Alvarez.
*/

/*==================[inclusions]=============================================*/
#include "OS_queue.h"
#include <string.h>
/*==================[macros]=================================================*/
/**
* @def QUEUE_IS_FULL(q)
* @brief Macro para detectar si la cola esta llena
*/
#define QUEUE_IS_FULL(q) ((q->writePtr + 1) % q->queueLen == q->readPtr)

/**
* @def QUEUE_IS_EMPTY(q)
* @brief Macro para detectar si la cola esta vacia
*/
#define QUEUE_IS_EMPTY(q) (q->readPtr == q->writePtr)

/**
* @def QUEUE_MOVE_PTR(ptr, queueLen)
* @brief Macro para mover alguno de los dos punteros(lectura o escritura)
*/
#define QUEUE_MOVE_PTR(ptr, queueLen) (ptr = (ptr + 1) % queueLen)
/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
#if ( OS_USE_QUEUE == 1 )
/*
* @fn void queueInit(queue_t * q, uint8_t * dataBuffer, uint32_t dataSize)
* @brief Inicializa una cola
* @params q : Puntero a la cola a ser inicializada
* @params queueLen : Cantidad de elementos de la cola
* @params dataBuffer : Puntero a buffer donde almacenar los elementos de la cola
* @params dataSize : Tamaño de los elementos de la cola
* @return Nada
* @danger queueLen * dataSize debe ser igual al largo de dataBuffer
* @danger queueLen >= 2
*/
void queueInit(queue_t * q, uint32_t queueLen, uint8_t * dataBuffer, uint32_t dataSize)
{
    q->writePtr = 0;
    q->readPtr = 0;
    q->queueLen = queueLen;
    q->data = dataBuffer;
    q->dataSize = dataSize;
    semphrInit(&(q->queuePushSem));
    semphrInit(&(q->queuePullSem));
}

/*
* @fn osReturn_t queuePush(queue_t * q, void * data, tick_t delay)
* @brief Agrega un elemento al final de una cola
* @params q     : Puntero a la cola
* @params data  : Puntero al elemento a agregar a la cola
* @params delay : Tiempo a esperar a que se desocupe un lugar en la cola
* @return OS_RESULT_OK si se pudo agregar el elemento,
          OS_RESULT_ERROR si expira el delay antes de poder agregar el elemento
* @danger Desde IRQ o Idle Task con delay 0
*/
osReturn_t queuePush(queue_t * q, void * data, tick_t delay)
{
    osReturn_t retVal = OS_RESULT_ERROR;

    /* Suspendemos cambio de contexto porque estamos manipulando variables globales */
    osSuspendContextSwitching();
    /* Si la cola esta llena */
    if(QUEUE_IS_FULL(q))
    {
        /* Esperamos a que se desocupe un lugar */
        retVal = semphrTake(&(q->queuePushSem), delay);
        
    }
    else
    {
        retVal = OS_RESULT_OK;
    }

    /* Si hay un lugar libre */
    if(OS_RESULT_OK == retVal)
    {
        /* Agregamos el elemento */
        memcpy(&(q->data[q->writePtr * q->dataSize]), data, q->dataSize);
        /* Movemos el puntero un elemento mas */
        QUEUE_MOVE_PTR(q->writePtr, q->queueLen);
        /* Liberamos el semaforo indicando que hay un elemento por si existe
           alguna otra tarea esperando que haya un elemento en la cola */
        semphrGive(&(q->queuePullSem));
    }

    osResumeContextSwitching();

    return retVal;

}

/*
* @fn osReturn_t queuePull(queue_t * q, void * data, tick_t delay)
* @brief Obtiene un elemento del principio de una cola
* @params q     : Puntero a la cola
* @params data  : Porcion de memoria donde se guarda un elemento de la cola
* @params delay : Tiempo a esperar a que haya un elemento en la cola
* @return OS_RESULT_OK si se pudo obtener el elemento,
          OS_RESULT_ERROR si expira el delay antes de poder obtener el elemento
* @danger Desde IRQ o Idle Task con delay 0
*/
osReturn_t queuePull(queue_t * q, void * data, tick_t delay)
{
    osReturn_t retVal = OS_RESULT_ERROR;

    /* Suspendemos cambio de contexto porque estamos manipulando variables globales */
    osSuspendContextSwitching();
    /* Si no hay elementos en la cola */
    if(QUEUE_IS_EMPTY(q))
    {
        /* Esperamos a que haya un elemento */
        retVal = semphrTake(&(q->queuePullSem), delay);
    
    }
    else
    {
        retVal = OS_RESULT_OK;
    }
    
    /* Si hay al menos un elemento en la cola */
    if(OS_RESULT_OK == retVal)
    {
        /* Obtenemos el elemento */
        memcpy(data, &(q->data[q->readPtr * q->dataSize]), q->dataSize);
        /* Movemos el puntero un elementos mas */
        QUEUE_MOVE_PTR(q->readPtr, q->queueLen);
        /* Liberamos el semaforo indicando que hay un lugar en la cola por si existe
           alguna otra tarea esperando que haya espacio en la misma */
        semphrGive(&(q->queuePushSem));
    }

    osResumeContextSwitching();

    return retVal;
}

/*
* @fn osReturn_t queuePush(queue_t * q, void * data, tick_t delay)
* @brief Agrega un elemento al final de una cola
* @params q     : Puntero a la cola
* @params data  : Puntero al elemento a agregar a la cola
* @params delay : Tiempo a esperar a que se desocupe un lugar en la cola
* @return OS_RESULT_OK si se pudo agregar el elemento,
          OS_RESULT_ERROR si expira el delay antes de poder agregar el elemento
* @danger Desde IRQ o Idle Task con delay 0
*/
osReturn_t queuePushFromISR(queue_t * q, void * data)
{
    osReturn_t retVal = OS_RESULT_ERROR;

    /* Suspendemos cambio de contexto porque estamos manipulando variables globales */
    osSuspendContextSwitching();
    /* Si la cola esta llena */
    if(QUEUE_IS_FULL(q))
    {
        /* Esperamos a que se desocupe un lugar */
        retVal = semphrTakeFromISR(&(q->queuePushSem));
        
    }
    else
    {
        retVal = OS_RESULT_OK;
    }

    /* Si hay un lugar libre */
    if(OS_RESULT_OK == retVal)
    {
        /* Agregamos el elemento */
        memcpy(&(q->data[q->writePtr * q->dataSize]), data, q->dataSize);
        /* Movemos el puntero un elemento mas */
        QUEUE_MOVE_PTR(q->writePtr, q->queueLen);
        /* Liberamos el semaforo indicando que hay un elemento por si existe
           alguna otra tarea esperando que haya un elemento en la cola */
        semphrGive(&(q->queuePullSem));
    }

    osResumeContextSwitching();

    return retVal;

}

/*
* @fn osReturn_t queuePull(queue_t * q, void * data, tick_t delay)
* @brief Obtiene un elemento del principio de una cola
* @params q     : Puntero a la cola
* @params data  : Porcion de memoria donde se guarda un elemento de la cola
* @params delay : Tiempo a esperar a que haya un elemento en la cola
* @return OS_RESULT_OK si se pudo obtener el elemento,
          OS_RESULT_ERROR si expira el delay antes de poder obtener el elemento
* @danger Desde IRQ o Idle Task con delay 0
*/
osReturn_t queuePullFromISR(queue_t * q, void * data)
{
    osReturn_t retVal = OS_RESULT_ERROR;

    /* Suspendemos cambio de contexto porque estamos manipulando variables globales */
    osSuspendContextSwitching();
    /* Si no hay elementos en la cola */
    if(QUEUE_IS_EMPTY(q))
    {
        /* Esperamos a que haya un elemento */
        retVal = semphrTakeFromISR(&(q->queuePullSem));
    
    }
    else
    {
        retVal = OS_RESULT_OK;
    }
    
    /* Si hay al menos un elemento en la cola */
    if(OS_RESULT_OK == retVal)
    {
        /* Obtenemos el elemento */
        memcpy(data, &(q->data[q->readPtr * q->dataSize]), q->dataSize);
        /* Movemos el puntero un elementos mas */
        QUEUE_MOVE_PTR(q->readPtr, q->queueLen);
        /* Liberamos el semaforo indicando que hay un lugar en la cola por si existe
           alguna otra tarea esperando que haya espacio en la misma */
        semphrGive(&(q->queuePushSem));
    }

    osResumeContextSwitching();

    return retVal;
}

#endif
/*==================[end of file]============================================*/
