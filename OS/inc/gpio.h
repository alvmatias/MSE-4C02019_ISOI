/** 
* @file gpio.h
* @brief Archivo de cabecera del archivo fuente gpio.c.
* @brief Manejo de los puertos de Entrada/Salida del lpc4337. 
* @note Copyright 2018 - Ing Matias Alvarez.
*/
#ifndef _GPIO_H_
#define _GPIO_H_

/*==================[inclusions]=============================================*/
#include "peripheralMap.h"
#include "stdint.h"
#include "cmsis.h"
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/
/** 
* @enum gpioMap_t
* @brief Indice del Pin en EDU-CIAA-NXP & CIAA-NXP.  
*/
typedef enum
{
   /* EDU-CIAA-NXP */

   /* P1 Header */
   T_FIL1,    T_COL2,    T_COL0,    T_FIL2,      T_FIL3,  T_FIL0,     T_COL1,
   CAN_TD,    CAN_RD,    RS232_TXD, RS232_RXD,

   /* P2 Header */
   GPIO8,     GPIO7,     GPIO5,     GPIO3,       GPIO1,
   LCD1,      LCD2,      LCD3,      LCDRS,       LCD4,
   SPI_MISO,
   ENET_TXD1, ENET_TXD0, ENET_MDIO, ENET_CRS_DV, ENET_MDC, ENET_TXEN, ENET_RXD1,
   GPIO6,     GPIO4,     GPIO2,     GPIO0,
   LCDEN,
   SPI_MOSI,
   ENET_RXD0,

   /* Switches */
   /* 36   37     38     39 */
   TEC1,  TEC2,  TEC3,  TEC4,

   /* Leds */
   /* 40   41     42     43     44     45 */
   LEDR,  LEDG,  LEDB,  LED1,  LED2,  LED3,

   /* CIAA-NXP */
   /* 46     47     48     49     50     51     52     53 */
   DI0,   DI1,   DI2,   DI3,   DI4,   DI5,   DI6,   DI7,
   /* 54     55     56     57     58     59     60     61 */
   DO0,   DO1,   DO2,   DO3,   DO4,   DO5,   DO6,   DO7,
   
} gpioMap_t;

/** 
* @enum gpioConfig_t
* @brief Pin Modes  
*/
typedef enum{ 
   GPIO_INPUT,                      /**< Input Mode. */ 
   GPIO_OUTPUT,                     /**< Output Mode. */
   GPIO_INPUT_PULLUP,               /**< Pullup Input Mode. */  
   GPIO_INPUT_PULLDOWN,             /**< Pullup Input Mode. */
   GPIO_INPUT_PULLUP_PULLDOWN,      /**< Pullup/Pulldown Input Mode. */
   GPIO_ENABLE                      /**< Enable GPIO Mode. */
} gpioConfig_t;


typedef enum 
{
   GPIO_CHANNEL_0 = 0
,  GPIO_CHANNEL_1
,  GPIO_CHANNEL_2
} gpioChannel_t;
/** 
* @struct gpioConfigLpc4337_t
* @brief Confguracion del GPIO del lpc4337.  
*/
typedef struct{
   int8_t port;   /**< GPIO Port */
   int8_t pin;    /**< GPIO Pin */
} gpioConfigLpc4337_t;


/** 
* @struct pinConfigLpc4337_t
* @brief Configuracion del SCU Pin del lpc4337. 
*/
typedef struct{
	pinConfigLpc4337_t pinName;  /**< Pin Name */
	int8_t func;                 /**< Pin Function */
	gpioConfigLpc4337_t gpio;    /**< GPIO */
} pinConfigGpioLpc4337_t;

/**
* @enum gpioValue_t
* @brief Nivel del GPIO.
*/
typedef enum{ 
   GPIO_OFF = 0,       /**< GPIO High Level */
   GPIO_ON          /**< GPIO Low Level */
} gpioValue_t;

typedef enum 
{
   LOW_EDGE_INT   = 0
,  HIGH_EDGE_INT 
,  BOTH_EDGE_INT   
} edgeInt_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/
/**
 * @fn      void gpioConfig(gpioMap_t pin, gpioConfig_t config)
 * @brief   Configuracion del GPIO.
 * @param   pin    : Indice del GPIO a configurar.
 * @param   config : Configuracion a realizar sobre el GPIO.
 * @return  Nada
 */
void gpioConfig(gpioMap_t pin, gpioConfig_t config);

void gpioConfigIRQ(gpioMap_t pin, uint8_t channel, edgeInt_t edgeInt);
/**
 * @fn      uint8_t gpioRead(gpioMap_t pin)
 * @brief   Lectura del nivel del GPIO.
 * @param   pin    : Indice del GPIO a leer.
 * @return  GPIO_ON si el GPIO esta en ALTO, GPIO_OFF caso contrario.
 */
uint8_t gpioRead(gpioMap_t pin);
/**
 * @fn      void gpioWrite(gpioMap_t pin, gpioValue_t value)
 * @brief   Seteo de nivel del GPIO.
 * @param   pin    : Indice del GPIO a setear el nivel.
 * @param   value  : Nivel a setear.
 * @return  Nada
 */
void gpioWrite(gpioMap_t pin, gpioValue_t value);
/**
 * @fn      void gpioToggle(gpioMap_t pin)
 * @brief   Toggle de GPIO.
 * @param   pin    : Indice del GPIO a togglear.
 * @return  Nada
 */
void gpioToggle(gpioMap_t pin);

/*
void GPIO0_IRQHandler(void);
void GPIO1_IRQHandler(void);
void GPIO2_IRQHandler(void);
*/
/*==================[end of file]============================================*/
#endif /* #ifndef _GPIO_H_ */
