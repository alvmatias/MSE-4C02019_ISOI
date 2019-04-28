/** 
* @file  OS_semphr.c
* @brief 
* @note  Copyright 2019 - Esp. Ing. Matias Alvarez.
*/

/*==================[inclusions]=============================================*/
#include "OS_semphr.h"
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
#if ( OS_USE_SEMPHR == 1 )
/**
* @fn void semphrInit(semaphore_t * sem)
* @brief Funcion que inicializa un semaforo
* @param  sem : Puntero a la estructura del semaforo a inicializar
* @return Nada
*/
void semphrInit(semaphore_t * sem)
{
    
    /* Inicializamos el semaforo */
    sem->value = 0;
    sem->task = OS_INVALID_TASK;
    sem->taskWaiting = false;

}

/**
* @fn void semphrGive(semaphore_t * sem)
* @brief Funcion que libera un semaforo
* @param  sem : Puntero a la estructura del semaforo a liberar
* @return Nada
*/
void semphrGive(semaphore_t * sem)
{
    /* Suspendemos cambio de contexto porque estamos manipulando variables globales */
    osSuspendContextSwitching();

    /* NOTE: La idle task SI puede liberar semaforos */
    if (NULL != sem)
    {   
        /* Si el semaforo esta tomado */
        if(sem->taskWaiting)
        {
            /* Liberamos a la tarea bloqueada : Signal() */
            sem->taskWaiting = false;
            taskUnsuspendWithinAPI(sem->task);
            /* Volvemos a permitir el cambio de contexto antes de llamar al scheduler */
            osResumeContextSwitching();
            /* Llamamos al scheduler aun dentro de una IRQ porque la pendSV tiene la menor prioridad del
               sistema, lo que nos permite salir de la IRQ actual antes de su ejecucion */
            taskYield();
        } 
        else
        {
            /* Si no, liberamos el semaforo */
            sem->value = 1;
        }  

        /* Volvemos a permitir el cambio de contexto
        en caso que no hayamos llegado a liberarlo antes de llamar al scheduler*/
        osResumeContextSwitching();
            
    }
 
}

/**
* @fn osReturn_t semphrTake(semaphore_t * sem, tick_t delay)
* @brief Funcion que toma un semaforo
* @param  sem : Puntero a la estructura del semaforo a tomar
* @param  delay: Tiempo maximo de espera hasta la liberacion del semaforo
* @return OS_RESULT_ERROR si se llama a la funcion desde la IDLE TASK,
          OS_RESULT_ERROR si expiro el delay sin que se liberase el semaforo
          OS_RESULT_OK si se pudo tomar el semaforo
* @danger Desde IRQ o Idle Task con delay 0
*/
osReturn_t semphrTake(semaphore_t * sem, tick_t delay)
{

    osReturn_t retVal = OS_RESULT_ERROR;

    /* Suspendemos cambio de contexto porque estamos manipulando variables globales */
    osSuspendContextSwitching();

    if(NULL != sem)
    {
        /* Si no hay otra tarea bloqueada en el semaforo */
        if(OS_INVALID_TASK == sem->task)
        {
            /* Si esta liberado lo tomo */
            if(0 < sem->value)
            {
                sem->value = 0;

                retVal = OS_RESULT_OK;
            }
            /* Si no espero : Wait() */
            else
            {
                sem->taskWaiting = true;
                sem->task = osGetCurrentTask();
                /* Volvemos a permitir el cambio de contexto antes de llamar al scheduler */
                osResumeContextSwitching();
                /* Wait() */
                taskDelay(delay);
                /* Suspendemos cambio de contexto porque estamos manipulando variables globales */
                osSuspendContextSwitching();
                /* Si al volver del delay el semaforo esta liberado retornamos verdadero */
                /* Pude haber vuelto del delay porque expiro el tiempo */
                if(!sem->taskWaiting)
                {
                    retVal = OS_RESULT_OK;
                }
                /* Ahora no hay tareas bloqueadas a la espera de la liberacion del semaforo */
                sem->taskWaiting = false;
                sem->task = OS_INVALID_TASK;
            }
        }

    }

    /* Volvemos a permitir el cambio de contexto */
    osResumeContextSwitching();

    return retVal;

}


osReturn_t semphrTakeFromISR(semaphore_t * sem)
{

    osReturn_t retVal = OS_RESULT_ERROR;

    /* Suspendemos cambio de contexto porque estamos manipulando variables globales */
    osSuspendContextSwitching();

    if(NULL != sem)
    {
        /* Si no hay otra tarea bloqueada en el semaforo */
        if(OS_INVALID_TASK == sem->task)
        {
            /* Si esta liberado lo tomo */
            if(0 < sem->value)
            {
                sem->value = 0;

                retVal = OS_RESULT_OK;
            }
        }
    }

    /* Volvemos a permitir el cambio de contexto */
    osResumeContextSwitching();

    return retVal;

}
/*==================[end of file]============================================*/
#endif
