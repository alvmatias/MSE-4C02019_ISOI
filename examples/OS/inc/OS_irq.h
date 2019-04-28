/** 
* @file  OS_irq.h
* @brief 
* @note  Copyright 2019 - Esp. Ing. Matias Alvarez.
*/
#ifndef _OS_IRQ_H_
#define _OS_IRQ_H_

/*==================[inclusions]=============================================*/
#include "board.h"
#include "chip.h"
#include "cmsis_43xx.h"
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/
/**
* @def void (*irqCbFunction_t)(void)
* @brief Definicion de prototipo de callback de interrupcion
*/
typedef void (*irqCbFunction_t)(void);
/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
osReturn_t irqAttach(IRQn_Type IRQn, irqCbFunction_t irqCbPointer);
osReturn_t irqDetach(IRQn_Type IRQn);
/*==================[end of file]============================================*/
#endif /* #ifndef _OS_IRQ_H_ */
