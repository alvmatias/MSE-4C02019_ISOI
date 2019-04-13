/** 
* @file  OS_semphr.h
* @brief 
* @note  Copyright 2019 - Esp. Ing. Matias Alvarez.
*/
#ifndef _OS_SEMPHR_H_
#define _OS_SEMPHR_H_

/*==================[inclusions]=============================================*/
#include "OS_config.h"
#include "OS.h"
#include <stdbool.h>
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/
typedef struct
{
    uint8_t value; 		/**< Valor del semaforo - Como es binario es 0(tomado) y 1(libre) */
    uint8_t task;		/**< Tarea que inicializa el semaforo */
    bool  taken;		/**< Indica si una tarea esta a la espera de que liberen el semaforo */
}semaphore_t;
/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
osReturn_t semphrInit(semaphore_t * sem);
void semphrGive(semaphore_t * sem);
osReturn_t semphrTake(semaphore_t * sem, uint32_t delay);
/*==================[end of file]============================================*/
#endif /* #ifndef _OS_SEMPHR_H_ */
