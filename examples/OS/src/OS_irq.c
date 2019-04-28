/** 
* @file  OS_irq.c
* @brief 
* @note  Copyright 2019 - Esp. Ing. Matias Alvarez.
*/

/*==================[inclusions]=============================================*/
#include "OS.h"
#include "OS_irq.h"
/*==================[macros]=================================================*/
#define OS_MAX_IRQ      53
/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/
/**
* @var static irqCbFunction_t irqCbFunction[OS_MAX_IRQ]
* @brief Arreglo de callbacks para cada irq del sistema
*/
static irqCbFunction_t irqCbFunction[OS_MAX_IRQ];
/*==================[internal functions declaration]=========================*/
/**
* @fn static void irqHandler(IRQn_Type IRQn)
* @brief Manejador de interrupciones generico
* @params IRQn : Numero de interrupcion
* @return Nada
*/
static void irqHandler(IRQn_Type IRQn)
{
    /* Llamamos al callback */
    irqCbFunction[IRQn]();
    /* Limpiamos la interrupcion */
    NVIC_ClearPendingIRQ(IRQn);
}
/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
/**
* @fn osReturn_t irqAttach(IRQn_Type IRQn, irqCbFunction_t irqCbFunction)
* @brief Adjunta un callback a una dada interrupcion
* @params IRQn : Numero de interrupcion
* @params irqCbFunction : Callback a adjuntar
* @return OS_RESULT_ERROR si ya habia un callback adjuntado, OS_RESULT_OK caso contrario
*/
osReturn_t irqAttach(IRQn_Type IRQn, irqCbFunction_t irqCbPointer)
{
    osReturn_t retVal = OS_RESULT_ERROR;

    if(NULL == irqCbFunction[IRQn])
    {
        irqCbFunction[IRQn] = irqCbPointer;
        NVIC_ClearPendingIRQ(IRQn);
        NVIC_EnableIRQ(IRQn);
        retVal = OS_RESULT_OK;
    }

    return retVal;
}

/**
* @fn osReturn_t irqDetach(IRQn_Type IRQn)
* @brief Desadjunta un callback a una dada interrupcion
* @params IRQn : Numero de interrupcion
* @return OS_RESULT_ERROR si NO habia un callback adjuntado, OS_RESULT_OK caso contrario
*/
osReturn_t irqDetach(IRQn_Type IRQn)
{   
    osReturn_t retVal = OS_RESULT_ERROR;

    if(NULL != irqCbFunction[IRQn])
    {
        irqCbFunction[IRQn] = NULL;
        NVIC_ClearPendingIRQ(IRQn);
        NVIC_DisableIRQ(IRQn);
        retVal = OS_RESULT_OK;
    }

    return retVal;
}


void DAC_IRQHandler(void){irqHandler(         DAC_IRQn         );}
void M0APP_IRQHandler(void){irqHandler(       M0APP_IRQn       );}
void DMA_IRQHandler(void){irqHandler(         DMA_IRQn         );}
void FLASH_EEPROM_IRQHandler(void){irqHandler(RESERVED1_IRQn   );}
void ETH_IRQHandler(void){irqHandler(         ETHERNET_IRQn    );}
void SDIO_IRQHandler(void){irqHandler(        SDIO_IRQn        );}
void LCD_IRQHandler(void){irqHandler(         LCD_IRQn         );}
void USB0_IRQHandler(void){irqHandler(        USB0_IRQn        );}
void USB1_IRQHandler(void){irqHandler(        USB1_IRQn        );}
void SCT_IRQHandler(void){irqHandler(         SCT_IRQn         );}
void RIT_IRQHandler(void){irqHandler(         RITIMER_IRQn     );}
void TIMER0_IRQHandler(void){irqHandler(      TIMER0_IRQn      );}
void TIMER1_IRQHandler(void){irqHandler(      TIMER1_IRQn      );}
void TIMER2_IRQHandler(void){irqHandler(      TIMER2_IRQn      );}
void TIMER3_IRQHandler(void){irqHandler(      TIMER3_IRQn      );}
void MCPWM_IRQHandler(void){irqHandler(       MCPWM_IRQn       );}
void ADC0_IRQHandler(void){irqHandler(        ADC0_IRQn        );}
void I2C0_IRQHandler(void){irqHandler(        I2C0_IRQn        );}
void SPI_IRQHandler(void){irqHandler(         I2C1_IRQn        );}
void I2C1_IRQHandler(void){irqHandler(        SPI_INT_IRQn     );}
void ADC1_IRQHandler(void){irqHandler(        ADC1_IRQn        );}
void SSP0_IRQHandler(void){irqHandler(        SSP0_IRQn        );}
void SSP1_IRQHandler(void){irqHandler(        SSP1_IRQn        );}
void UART0_IRQHandler(void){irqHandler(       USART0_IRQn      );}
void UART1_IRQHandler(void){irqHandler(       UART1_IRQn       );}
void UART2_IRQHandler(void){irqHandler(       USART2_IRQn      );}
void UART3_IRQHandler(void){irqHandler(       USART3_IRQn      );}
void I2S0_IRQHandler(void){irqHandler(        I2S0_IRQn        );}
void I2S1_IRQHandler(void){irqHandler(        I2S1_IRQn        );}
void SPIFI_IRQHandler(void){irqHandler(       RESERVED4_IRQn   );}
void SGPIO_IRQHandler(void){irqHandler(       SGPIO_INT_IRQn   );}
void GPIO0_IRQHandler(void){irqHandler(       PIN_INT0_IRQn    );}
void GPIO1_IRQHandler(void){irqHandler(       PIN_INT1_IRQn    );}
void GPIO2_IRQHandler(void){irqHandler(       PIN_INT2_IRQn    );}
void GPIO3_IRQHandler(void){irqHandler(       PIN_INT3_IRQn    );}
void GPIO4_IRQHandler(void){irqHandler(       PIN_INT4_IRQn    );}
void GPIO5_IRQHandler(void){irqHandler(       PIN_INT5_IRQn    );}
void GPIO6_IRQHandler(void){irqHandler(       PIN_INT6_IRQn    );}
void GPIO7_IRQHandler(void){irqHandler(       PIN_INT7_IRQn    );}
void GINT0_IRQHandler(void){irqHandler(       GINT0_IRQn       );}
void GINT1_IRQHandler(void){irqHandler(       GINT1_IRQn       );}
void EVRT_IRQHandler(void){irqHandler(        EVENTROUTER_IRQn );}
void CAN1_IRQHandler(void){irqHandler(        C_CAN1_IRQn      );}
void ADCHS_IRQHandler(void){irqHandler(       ADCHS_IRQn       );}
void ATIMER_IRQHandler(void){irqHandler(      ATIMER_IRQn      );}
void RTC_IRQHandler(void){irqHandler(         RTC_IRQn         );}
void WDT_IRQHandler(void){irqHandler(         WWDT_IRQn        );}
void M0SUB_IRQHandler(void){irqHandler(       M0SUB_IRQn       );}
void CAN0_IRQHandler(void){irqHandler(        C_CAN0_IRQn      );}
void QEI_IRQHandler(void){irqHandler( QEI_IRQn );}
/*==================[end of file]============================================*/
