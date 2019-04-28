/** 
* @file peripheralMap.h
* @brief Archivo de cabecera.
* @brief Manejo de los puertos SCU del lpc4337.
* @note Copyright 2018 - Ing Matias Alvarez.
*/

#ifndef _PERIPHERAL_MAP_H_
#define _PERIPHERAL_MAP_H_

/*==================[inclusions]=============================================*/
#include "chip.h"
#include "cmsis.h"
#include "stdint.h"
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/
/**
* @struct pinConfigLpc4337_t
* @brief Definicion GPIO Port y Pin
*/
typedef struct{
   int8_t port;  	/**< GPIO Port */
   int8_t pin;		/**< GPIO Pin */
} pinConfigLpc4337_t;

/**
* @struct lpc4337ScuPin_t
* @brief Definicion SCU Port, Pin y Funcion
*/
typedef struct{
   uint8_t lpcScuPort;  /**< SCU Port */
   uint8_t lpcScuPin;   /**< SCU Pin */
   uint8_t lpcScuFunc;  /**< SCU Funcion */
}lpc4337ScuPin_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/*==================[end of file]============================================*/
#endif /* #ifndef _PERIPHERAL_MAP_H_ */
