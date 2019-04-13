/** 
* @file  OS_config.h
* @brief Archivo de configuracion del SO
* @note  Archivo modificable por el usuario
* @note  Copyright 2019 - Esp. Ing. Matias Alvarez.
*/
#ifndef _OS_CONFIG_H_
#define _OS_CONFIG_H_

/*==================[inclusions]=============================================*/

/*==================[macros]=================================================*/
/**
* @def OS_MINIMAL_STACK_SIZE
* @brief Minimo tamaño de stack usado por las tareas
* @note Obligatoria su definicion
*/
#define OS_MINIMAL_STACK_SIZE       128

/**
* @def OS_IDLE_STACK_SIZE
* @brief Tamaño del stack usado por la idle task
* @note Obligatoria su definicion
*/
#define OS_IDLE_STACK_SIZE          128

/**
* @def OS_MAX_TASK
* @brief Maxima cantidad de tareas que soporta el sistema
* @note Obligatoria su definicion
*/
#define OS_MAX_TASK                 3

/**
* @def OS_MAX_TASK_PRIORITY
* @brief Maxima cantidad de prioridades que soport el sistema
* @note Cuanto mayor el numero de prioridad, menor la prioridad real de la tarea
* @note Obligatoria su definicion
*/
#define OS_MAX_TASK_PRIORITY        3 

/**
* @def OS_MAX_TASK_NAME_LEN
* @brief Maxima cantidad de caracteres posible del nombre de la tarea
* @note Obligatoria su definicion
*/
#define OS_MAX_TASK_NAME_LEN        10

/**
* @def OS_TICKS_UNTIL_SCHEDULE
* @var Numero de ticks del sistema hasta el proximo schedule
* @note Obligatoria su definicion
*/
#define OS_TICKS_UNTIL_SCHEDULE     1

/**
* @def OS_USE_TICK_HOOK
* @var Flag que indica si el sistema debe usar la tick hook o no
* @note Obligatoria su definicion
*/
#define OS_USE_TICK_HOOK            0

/**
* @def OS_USE_TASK_DELAY
* @var Flag que indica si el sistema debe incluir la implementacion del delay o no
* @note No es obligatoria su definicion
*/
#define OS_USE_TASK_DELAY           1

/**
* @def OS_USE_ROUND_ROBIN_SCHED
* @var Flag que indica si el sistema usa scheduling preemtive o fifo
* @note POR AHORA SIEMPRE EN 1
* @note Es obligatoria su definicion
*/
#define OS_USE_PRIO_ROUND_ROBIN_SCHED     1  
/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/

/*==================[end of file]============================================*/
#endif /* #ifndef _OS_CONFIG_H_ */
