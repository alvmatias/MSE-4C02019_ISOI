/** 
* @file  OS_config.h
* @brief 
* @note  Copyright 2019 - Esp. Ing. Matias Alvarez.
*/
#ifndef _OS_CONFIG_H_
#define _OS_CONFIG_H_

/*==================[inclusions]=============================================*/

/*==================[macros]=================================================*/
#define OS_MINIMAL_STACK_SIZE	 	128
#define OS_IDLE_STACK_SIZE		 	128
#define OS_MAX_TASK				 	3
#define OS_MAX_TASK_PRIORITY	 	3 

#define OS_TICKS_UNTIL_SCHEDULE	 	1

#define OS_USE_TICK_HOOK		 	0
#define OS_USE_TASK_DELAY		 	1
#define OS_USE_ROUND_ROBIN_SCHED	1  /* Por ahora siempre en 1 */
/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/

/*==================[end of file]============================================*/
#endif /* #ifndef _OS_CONFIG_H_ */
