/** 
* @file  uart.c
* @brief 
* @note  Copyright 2019 - Esp. Ing. Matias Alvarez.
*/
/*==================[inclusions]=============================================*/
#include "uart.h"
#include "stdint.h"
/*==================[macros and definitions]=================================*/
#define UART_IRQ_PRIO   6
/*==================[internal data declaration]==============================*/
/**
* @var lpcUarts
* @brief Configuracion de cada UART disponible en lpc4337.
*/
static const uartLpcConfig_t lpcUarts[] = 
{
// { uartAddr, { txPort, txpin, txfunc }, { rxPort, rxpin, rxfunc }, uartIrqAddr  },
    // UART_GPIO (GPIO1 = U0_TXD, GPIO2 = U0_RXD)
    { LPC_USART0, { 6, 4, FUNC2 }, { 6, 5, FUNC2 }, USART0_IRQn } // 0
    // UART_485 (RS485/Profibus)
,   { LPC_USART0, { 9, 5, FUNC7 }, { 9, 6, FUNC7 }, USART0_IRQn } // 1
    // UART not routed
,   {  LPC_UART1, { 0, 0, 0     }, { 0, 0, 0     }, UART1_IRQn  } // 2
    // UART_USB
,   { LPC_USART2, { 7, 1, FUNC6 }, { 7, 2, FUNC6 }, USART2_IRQn } // 3
    // UART_ENET
,   { LPC_USART2, { 1,15, FUNC1 }, { 1,16, FUNC1 }, USART2_IRQn } // 4
    // UART_232
,   { LPC_USART3, { 2, 3, FUNC2 }, { 2, 4, FUNC2 }, USART3_IRQn }  // 5   
};

/**
* @var lpcUart485DirPin
* @brief Configuracion especial para la UART 485.
*/
static const lpc4337ScuPin_t lpcUart485DirPin = 
{
   6, 2, FUNC2
};
/*==================[internal functions declaration]=========================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/

void uartConfig(uartMap_t uart, baudRate_t baudRate)
{
    /* Init Uart */
    Chip_UART_Init(lpcUarts[uart].uartAddr);
    /* Set Baud rate */
    Chip_UART_SetBaud(lpcUarts[uart].uartAddr, baudRate);  
    /* Modify FCR (FIFO Control Register)*/
    Chip_UART_SetupFIFOS(lpcUarts[uart].uartAddr, UART_FCR_FIFO_EN |  
                                                  UART_FCR_TX_RS   | 
                                                  UART_FCR_RX_RS   | 
                                                  UART_FCR_TRG_LEV0); 
    
    /* Enable UART lpcUarts[uart].uartAddr */
    Chip_UART_TXEnable(lpcUarts[uart].uartAddr); 
    /* Set pin functions */ 
    Chip_SCU_PinMux(lpcUarts[uart].txPin.lpcScuPort, 
                    lpcUarts[uart].txPin.lpcScuPin, 
                    MD_PDN,
                    lpcUarts[uart].txPin.lpcScuFunc );             
    /* Set direction pin function */
    Chip_SCU_PinMux(lpcUarts[uart].rxPin.lpcScuPort, 
                    lpcUarts[uart].rxPin.lpcScuPin, 
                    MD_PLN | MD_EZI | MD_ZI,
                    lpcUarts[uart].rxPin.lpcScuFunc); 

    if(UART_RS485 == uart)
    {
        /* Set RS-485 flags*/ /* Auto Direction Control Enabled */
        Chip_UART_SetRS485Flags(lpcUarts[uart].uartAddr, UART_RS485CTRL_DCTRL_EN | UART_RS485CTRL_OINV_1);
        /* Set direction pin function */
        Chip_SCU_PinMux(lpcUart485DirPin.lpcScuPort, 
                        lpcUart485DirPin.lpcScuPin, 
                        MD_PDN, 
                        lpcUart485DirPin.lpcScuFunc );             
    }

    NVIC_SetPriority(lpcUarts[uart].uartIrqAddr, UART_IRQ_PRIO);
    NVIC_EnableIRQ(lpcUarts[uart].uartIrqAddr);

    Chip_UART_ReadIntIDReg(lpcUarts[uart].uartAddr);
 
}

void uartDeInit(uartMap_t uart)
{
    Chip_UART_DeInit(lpcUarts[uart].uartAddr);
}

void uartSetRxInterrupt(uartMap_t uart, uint8_t enable)
{
    if(enable)
    {  
        // Enable UART Receiver Buffer Register Interrupt
        Chip_UART_IntEnable( lpcUarts[uart].uartAddr, UART_IER_RBRINT );
        // Enable Interrupt for UART channel
        NVIC_EnableIRQ( lpcUarts[uart].uartIrqAddr );
    } 
    else
    {
        // Disable UART Receiver Buffer Register Interrupt
        Chip_UART_IntDisable( lpcUarts[uart].uartAddr, UART_IER_RBRINT );
        // Disable Interrupt for UART channel
        NVIC_DisableIRQ( lpcUarts[uart].uartIrqAddr );
    }
}

void uartSetTxInterrupt(uartMap_t uart, uint8_t enable)
{
    if(enable){  
        // Enable THRE irq (TX)
        Chip_UART_IntEnable( lpcUarts[uart].uartAddr, UART_IER_THREINT );
        //NVIC_EnableIRQ( lpcUarts[uart].uartIrqAddr );
    }
    else
    {
        // Disable THRE irq (TX)
        Chip_UART_IntDisable( lpcUarts[uart].uartAddr, UART_IER_THREINT );
        //NVIC_DisableIRQ( lpcUarts[uart].uartIrqAddr );
    }
}

uartError_t uartReadByte(uartMap_t uart, uint8_t *data)
{ 
    
    if (Chip_UART_ReadLineStatus(lpcUarts[uart].uartAddr) & UART_LSR_RDR) 
    {         
        *data = Chip_UART_ReadByte(lpcUarts[uart].uartAddr);
        return UART_OK;
    }
    else return UART_NO_DATA_RECEIVED;
    
}

void uartWriteByte(uartMap_t uart, uint8_t data)
{
   /* Wait for space in FIFO */
    while ((Chip_UART_ReadLineStatus(lpcUarts[uart].uartAddr) & UART_LSR_THRE) == FALSE)
        ; 
    Chip_UART_SendByte(lpcUarts[uart].uartAddr, data);
}

void uartWriteString(uartMap_t uart, char* data)
{
    while(*data)
    {
       uartWriteByte(uart, (uint8_t)*data);
       data++;
   }
}
/*==================[end of file]============================================*/