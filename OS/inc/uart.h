/** 
* @file uart.h
* @brief Archivo de cabecera del archivo fuente uart.c.
* @brief Manejo del modulo UART del lpc4337.
* @note Copyright 2019 - Esp. Ing. Matias Alvarez.
*/

#ifndef _UART_H_
#define _UART_H_

/*==================[inclusions]=============================================*/
#include "chip.h"
#include "cmsis.h"
#include "stdint.h"
#include "peripheralMap.h"
/*==================[macros]=================================================*/
#define UART_0_IRQ_HANDLER 0
#define UART_2_IRQ_HANDLER 2
#define UART_3_IRQ_HANDLER 3
/*==================[typedef]================================================*/
/**
* @struct uartLpcConfig_t
* @brief Datos de la UART para lpc4337.
*/
typedef struct{
   LPC_USART_T*      uartAddr;  	/**< Numero de UART */
   lpc4337ScuPin_t   txPin;			/**< Pin Tx */
   lpc4337ScuPin_t   rxPin; 		/**< Pin Rx */
   IRQn_Type         uartIrqAddr;   /**< Manejador de interrupciones de la UART */
} uartLpcConfig_t;

/**
* @enum baudRate_t
* @brief Posibles BaudRates para la configuracion de la UART.
*/
typedef enum{
	BAUDRATE_2400=2400,       	/**< BaudRate 2400bps */
	BAUDRATE_4800=4800, 		/**< BaudRate 4800bps */
	BAUDRATE_9600=9600, 		/**< BaudRate 9600bps */
	BAUDRATE_19200=19200,		/**< BaudRate 19200bps */
	BAUDRATE_28800=28800,		/**< BaudRate 28800bps */
	BAUDRATE_38400=38400,		/**< BaudRate 38400bps */
	BAUDRATE_57600=57600,		/**< BaudRate 57600bps */
	BAUDRATE_115200=115200,		/**< BaudRate 115200bps */
	BAUDRATE_230400=230400		/**< BaudRate 230400bps */
} baudRate_t;

/**
* @enum uartMap_t
* @brief Numero de UART.
*/
typedef enum{
   UART_GPIO = 0,   	/**< Hardware UART0 via GPIO1(TX), GPIO2(RX) pins en header P0 */
   UART_RS485  = 1, 	/**< Hardware UART0 via RS_485 borneras A, B y GND */
   //UART_1  = 2, 		// Hardware UART1 not routed
   UART_USB  = 3, 		/**< Hardware UART2 via USB DEBUG port */
   UART_ENET = 4, 		/**< Hardware UART2 via ENET_RXD0(TX), ENET_CRS_DV(RX) pins en header P0 */
   UART_RS232  = 5  	/**< Hardware UART3 via 232_RX and 232_tx pins en header P1 */
} uartMap_t;


/**
* @enum uartError_t
* @brief Indica errores de la UART.
*/
typedef enum{
	UART_NO_DATA_RECEIVED = 0,  /**< Data NO recibida */
	UART_OK = 1,				/**< Data recibida */
	UART_TIMEOUT = 2			/**< Data NO recibida producto de un timeout */
} uartError_t;
/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/
/**
* @fn void uartConfig(uartMap_t uart, baudRate_t baudRate)
* @brief Configuracion de la uart.
* @param uart : UART a configurar.
* @param baudRate : BaudRate a configurar.
* @param config : Configuracion de la UART.
* @return Nada.
*/
void uartConfig(uartMap_t uart, baudRate_t baudRate, uint8_t config);

void uartDeInit(uartMap_t uart);
/**
* @fn void uartWriteByte(uartMap_t uart, uint8_t data)
* @brief Escritura de un byte por la UART.
* @param uart : UART a utilizar en la escritura.
* @param data : byte de datos a enviar.
* @return Nada.
*/
void uartWriteByte(uartMap_t uart, uint8_t data);

/**
* @fn void uartWriteString(uartMap_t uart, char *data)
* @brief Escritura de un string por la UART.
* @param uart : UART a utilizar en la escritura.
* @param data : Puntero al buffer de los datos a escribir.
* @return Nada.
*/
void uartWriteString(uartMap_t uart, char *data);

/**
* @fn uartError_t uartReadByte(uartMap_t uart, uint8_t *data);
* @brief Lectura de un byte de la UART.
* @param uart : UART a utilizar para la lectura.
* @param data : Puntero al espacio de memoria donde guardar el dato recibido.
* @return UART_OK si se recibe el dato correctamente, UART_NO_DATA_RECEIVED caso contrario.
* @note Lectura NO Bloqueante.
*/
uartError_t uartReadByte(uartMap_t uart, uint8_t *data);


/**
* @fn void uartSetRxInterrupt(uartMap_t uart, uint8_t enable)
* @brief Configuracion de la interrupcion de recepcion de la UART.
* @param uart : UART a configurar.
* @param enable : Configuracion a realizar
* @return Nada.
* @note enable puede ser TRUE or FALSE.
*/
void uartSetRxInterrupt(uartMap_t uart, uint8_t enable);

/**
* @fn void uartSetTxInterrupt(uartMap_t uart, uint8_t enable)
* @brief Configuracion de la interrupcion de transmision de la UART.
* @param uart : UART a configurar.
* @param enable : Configuracion a realizar
* @return Nada.
* @note enable puede ser TRUE or FALSE.
*/
void uartSetTxInterrupt(uartMap_t uart, uint8_t enable);
/*==================[end of file]============================================*/
#endif /* #ifndef _UART_H_ */
