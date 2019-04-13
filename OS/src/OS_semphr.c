/** 
* @file  main.c
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
/**
* @fn osReturn_t semphrInit(semaphore_t * sem)
* @brief Funcion que inicializa un semaforo
* @param  sem : Puntero a la estructura del semaforo a inicializar
* @return OS_RESULT_ERROR si se llama desde la IDLE TASK, OS_RESULT_OK caso contrario
*/
osReturn_t semphrInit(semaphore_t * sem)
{
    osReturn_t retVal = OS_RESULT_ERROR;
    
    /* Si no se llamo a la funcion desde la idle task */
    if(OS_IDLE_TASK != osGetCurrentTask())
    {   
        /* Inicializamos el semaforo */
        sem->value = 0;
        sem->task = osGetCurrentTask();
        sem->taken = false;
        retVal = OS_RESULT_OK;
    }

    return retVal;
    
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
        if(sem->taken)
        {
            /* Liberamos a la tarea bloqueada : Signal() */
            sem->taken = false;
            taskUnsuspendWithinAPI(sem->task);
            /* Volvemos a permitir el cambio de contexto antes de llamar al scheduler */
            osResumeContextSwitching();
            schedule();
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
* @fn osReturn_t semphrTake(semaphore_t * sem, uint32_t delay)
* @brief Funcion que toma un semaforo
* @param  sem : Puntero a la estructura del semaforo a tomar
* @param  delay: Tiempo maximo de espera hasta la liberacion del semaforo
* @return OS_RESULT_ERROR si se llama a la funcion desde la IDLE TASK,
          OS_RESULT_ERROR si expiro el delay sin que se liberase el semaforo
          OS_RESULT_OK si se pudo tomar el semaforo
*/
osReturn_t semphrTake(semaphore_t * sem, uint32_t delay)
{

    osReturn_t retVal = OS_RESULT_ERROR;

    /* Suspendemos cambio de contexto porque estamos manipulando variables globales */
    osSuspendContextSwitching();

    /* Si no se llamo a esta funcion desde la idle task */
    if(OS_IDLE_TASK != osGetCurrentTask())
    {
        if(NULL != sem)
        {
            /* Si es su semaforo */
            if(osGetCurrentTask() == sem->task)
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
                    sem->taken = true;
                    /* Volvemos a permitir el cambio de contexto antes de llamar al scheduler */
                    osResumeContextSwitching();
                    /* Wait() */
                    taskDelay(delay);
                    /* Suspendemos cambio de contexto porque estamos manipulando variables globales */
                    osSuspendContextSwitching();
                    /* Si al volver del delay el semaforo esta liberado retornamos verdadero */
                    /* Pude haber vuelto del delay porque expiro el tiempo */
                    if(!sem->taken)
                    {
                        retVal = OS_RESULT_OK;
                    }
                    else
                    {
                        sem->taken = false;
                    }
                }
            }

        }
    }

    /* Volvemos a permitir el cambio de contexto */
    osResumeContextSwitching();

    return retVal;

}

/*==================[end of file]============================================*/

