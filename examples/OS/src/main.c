/** 
* @file  main.c
* @brief 
* @note  Copyright 2019 - Esp. Ing. Matias Alvarez.
*/

/*==================[inclusions]=============================================*/
/* OS Includes */
#include "OS_config.h"
#include "OS.h"
#include "OS_semphr.h"
#include "OS_queue.h"
#include "OS_irq.h"

/* Driver & Board Includes */
#include "board.h"
#include "gpio.h"
#include "uart.h"

/* C Includes */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
/*==================[macros]=================================================*/
/**
* @def MAX_TEC
* @brief Maxima cantidad de teclas usadas
*/
#define MAX_TEC                 2

/**
* @def TEC1_INDEX
* @brief Indice de la tecla 1 dentro del arreglo iterno de datos
*/
#define TEC1_INDEX              0

/**
* @def TEC2_INDEX
* @brief Indice de la tecla 2 dentro del arreglo iterno de datos
*/
#define TEC2_INDEX              1

/**
* @def REBOUND_DELAY
* @brief Delay anti-rebote
*/
#define REBOUND_DELAY           10

/**
* @def LED_STRING_LENGTH
* @brief Largo del string que indica que led debe ser encendido
*/
#define LED_STRING_LENGTH       15

/**
* @def STRING_TO_SEND_LENGTH
* @brief Largo del string de log a ser enviado via UART
*/
#define STRING_TO_SEND_LENGTH   256

/**
* @def QUEUE_LEN
* @brief Largo de las colas
*/
#define QUEUE_LEN               5
/*==================[typedef]================================================*/
/**
* @def edge_t
* @brief Posibles tipos de edge
*/
typedef enum
{
    RISING_EDGE
,   FALLING_EDGE
}edge_t;


/**
* @def tecState_t
* @brief Estados de la maquina de estados de deteccion de pulsos
* @note Casos NO contemplados :
        SOLO TEC1 presionada
        SOLO TEC2 presionada
        Presiono TEC1 - Presiono TEC2 - Libero TEC2 - Presiono TEC2 - Libero TEC2 - Libero TEC1 
        Casos de combinaciones similares al caso anterior
*/ 
typedef enum
{
    WAITING_TEC1_TEC2_FALLING_EDGE_STATE
,   WAITING_TEC1_FALLING_EDGE_STATE
,   WAITING_TEC2_FALLING_EDGE_STATE
,   WAITING_TEC1_TEC2_RISING_EDGE_STATE
,   WAITING_TEC1_RISING_EDGE_STATE
,   WAITING_TEC2_RISING_EDGE_STATE
}tecState_t;

/**
* @struct tecInfo_t
* @brief Estructura con informacion de una tecla
*/
typedef struct
{
    tick_t    edgeTime;
    gpioMap_t   tec;
    edge_t      edge;
}tecInfo_t;

/**
* @struct logInfo_t
* @brief Estructura con informacion para el log via UART
*/
typedef  struct
{
    char        ledString[LED_STRING_LENGTH];   
    tick_t    fallingEdgeTime;
    tick_t    risingEdgeTime;
}logInfo_t;

/**
* @struct ledInfo_t
* @brief Estructura con informacion de un led
*/
typedef struct 
{
    gpioMap_t   led;
    tick_t    totalTime;
}ledInfo_t;
/*==================[internal data declaration]==============================*/
/**
* @var static uint8_t   g_tecQueueBuffer[QUEUE_LEN*sizeof(tecInfo_t)]
* @brief Buffer para almacenar los elementos de la cola de informacion de teclas
*/
static uint8_t   g_tecQueueBuffer[QUEUE_LEN*sizeof(tecInfo_t)];
/**
* @var static queue_t g_tecQueue
* @brief Cola para informacion de teclas
*/
static queue_t g_tecQueue;

/**
* @var static uint8_t   g_ledQueueBuffer[QUEUE_LEN*sizeof(ledInfo_t)];
* @brief Buffer para almacenar los elementos de la cola de informacion de leds
*/
static uint8_t   g_ledQueueBuffer[QUEUE_LEN*sizeof(ledInfo_t)];
/**
* @var static queue_t g_tecQueue
* @brief Cola para informacion de leds
*/
static queue_t g_ledQueue;

/**
* @var static uint8_t   g_logQueueBuffer[QUEUE_LEN*sizeof(logInfo_t)];
* @brief Buffer para almacenar los elementos de la cola de informacion de log
*/
static uint8_t   g_logQueueBuffer[QUEUE_LEN*sizeof(logInfo_t)];
/**
* @var static queue_t g_tecQueue
* @brief Cola para informacion de log
*/
static queue_t g_logQueue;
/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/
uint32_t pulseDetectorTaskStack[OS_MINIMAL_STACK_SIZE];
uint32_t ledTaskStack[OS_MINIMAL_STACK_SIZE];
uint32_t logTaskStack[OS_MINIMAL_STACK_SIZE];
/*==================[external functions definition]==========================*/
void GPIO0IRQHandler(void){
    tecInfo_t tecInfo;

    tecInfo.tec = TEC1;

    /* Si la interrupcion fue a causa de una rising edge */
    if (Chip_PININT_GetRiseStates(LPC_GPIO_PIN_INT) & PININTCH(GPIO_CHANNEL_0)) 
    {

        /* Guardamos el tick actual en edgeTime e indicamos que es rising edge */
        tecInfo.edgeTime = taskGetTickCount();
        tecInfo.edge = RISING_EDGE;
        /* Limpiamos el estado de rising edge */
        Chip_PININT_ClearRiseStates(LPC_GPIO_PIN_INT,PININTCH(GPIO_CHANNEL_0));
    }
    /* Si, en cambio, la interrupcion fue a causa de uns falling edge */
    else 
    {
        /* Guardamos el tick actual en edgeTime e indicamos que es falling edge */
        tecInfo.edgeTime = taskGetTickCount();
        tecInfo.edge = FALLING_EDGE;
        /* Limpiamos el estado de falling edge */
        Chip_PININT_ClearFallStates(LPC_GPIO_PIN_INT,PININTCH(GPIO_CHANNEL_0));
    }

    /* Limpiamos el flag de interrupcion del GPIO_CHANNEL_0 */
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(GPIO_CHANNEL_0));

    /* Hacemos el push dentro de una IRQ */
    queuePushFromISR(&g_tecQueue, (void *)&(tecInfo));
}

void GPIO1IRQHandler(void){
    tecInfo_t tecInfo;

    tecInfo.tec = TEC2;

    /* Si la interrupcion fue a causa de una rising edge */
    if (Chip_PININT_GetRiseStates(LPC_GPIO_PIN_INT) & PININTCH(GPIO_CHANNEL_1)) 
    {

        /* Guardamos el tick actual en edgeTime e indicamos que es rising edge */
        tecInfo.edgeTime = taskGetTickCount();
        tecInfo.edge = RISING_EDGE;
        /* Limpiamos el estado de rising edge */
        Chip_PININT_ClearRiseStates(LPC_GPIO_PIN_INT,PININTCH(GPIO_CHANNEL_1));
    }
    /* Si, en cambio, la interrupcion fue a causa de uns falling edge */
    else 
    {
        /* Guardamos el tick actual en edgeTime e indicamos que es falling edge */
        tecInfo.edgeTime = taskGetTickCount();
        tecInfo.edge = FALLING_EDGE;
        /* Limpiamos el estado de falling edge */
        Chip_PININT_ClearFallStates(LPC_GPIO_PIN_INT,PININTCH(GPIO_CHANNEL_1));
    }

    /* Limpiamos el flag de interrupcion del GPIO_CHANNEL_1 */
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(GPIO_CHANNEL_1));

    /* Hacemos el push dentro de una IRQ */
    queuePushFromISR(&g_tecQueue, (void *)&(tecInfo));
}

void pulseDetectorTask(void * parameters)
{
    tecInfo_t tecInfo;
    ledInfo_t ledInfo;
    logInfo_t logInfo;

    tecState_t tecState = WAITING_TEC1_TEC2_FALLING_EDGE_STATE;

    static uint32_t fallingEdgeTime[MAX_TEC];
    static uint32_t risingEdgeTime[MAX_TEC];
    
    while(TRUE) 
    {

        queuePull(&g_tecQueue, (void*)(&tecInfo), OS_MAX_DELAY);

        switch(tecState)
        {
            case WAITING_TEC1_TEC2_FALLING_EDGE_STATE:
                /* Si es un falling edge de la primera tecla */
                if(TEC1 == tecInfo.tec && FALLING_EDGE == tecInfo.edge)
                {
                    /* Guardamos el tick del sistema de la ocurrencia del falling edge */
                    fallingEdgeTime[TEC1_INDEX] = tecInfo.edgeTime;
                    /* Pasamos a esperar un falling edge de la tecla 2 */
                    tecState = WAITING_TEC2_FALLING_EDGE_STATE;
                }
                /* Si en cambio es un falling edge de la segunda tecla */
                else if(TEC2 == tecInfo.tec && FALLING_EDGE == tecInfo.edge)
                {
                    /* Guardamos el tick del sistema de la ocurrencia del falling edge */
                    fallingEdgeTime[TEC2_INDEX] = tecInfo.edgeTime;
                    /* Pasamos a esperar un falling edge de la tecla 1 */
                    tecState = WAITING_TEC1_FALLING_EDGE_STATE;
                }
            break;

            case WAITING_TEC2_FALLING_EDGE_STATE:
                /* Veniamos de recibir un falling edge de la tecla 1 */
                /* Si es un falling edge de la tecla 2 */
                if(TEC2 == tecInfo.tec && FALLING_EDGE == tecInfo.edge)
                {
                    /* Guardamos el tick del sistema de la ocurrencia del falling edge */
                    fallingEdgeTime[TEC2_INDEX] = tecInfo.edgeTime;
                    /* Guardamos el tiempo entre flancos descendentes para el log via UART */
                    logInfo.fallingEdgeTime = fallingEdgeTime[TEC2_INDEX] - fallingEdgeTime[TEC1_INDEX];
                    /* Pasamos a esperar un rising edge de tecla 1 o tecla 2 */
                    tecState = WAITING_TEC1_TEC2_RISING_EDGE_STATE;
                }
                /* Si en cambio es un flanco ascendente de la tecla 1 que NO esta dentro del delay 
                    de antirebote, es porque se pulso y solto la tecla 1 sin apretar la tecla 2 */
                else if(TEC1 == tecInfo.tec && RISING_EDGE == tecInfo.edge 
                        && REBOUND_DELAY < tecInfo.edgeTime - fallingEdgeTime[TEC1_INDEX])
                {
                    /* Volvemos a comenzar con la deteccion de pulsos superpuestos */
                    tecState = WAITING_TEC1_TEC2_FALLING_EDGE_STATE;
                }
            break;

            case WAITING_TEC1_FALLING_EDGE_STATE:
                /* Veniamos de recibir un falling edge de la tecla 2 */
                /* Si es un falling edge de la tecla 1 */
                if(TEC1 == tecInfo.tec && FALLING_EDGE == tecInfo.edge)
                {
                    /* Guardamos el tick del sistema de la ocurrencia del falling edge */
                    fallingEdgeTime[TEC1_INDEX] = tecInfo.edgeTime;
                    /* Guardamos el tiempo entre flancos descendentes para el log via UART */
                    logInfo.fallingEdgeTime = fallingEdgeTime[TEC1_INDEX] - fallingEdgeTime[TEC2_INDEX];
                    /* Pasamos a esperar un rising edge de tecla 1 o tecla 2 */
                    tecState = WAITING_TEC1_TEC2_RISING_EDGE_STATE;
                }
                /* Si en cambio es un flanco ascendente de la tecla 2 que NO esta dentro del delay 
                    de antirebote, es porque se pulso y solto la tecla 2 sin apretar la tecla 1 */
                else if(TEC2 == tecInfo.tec && RISING_EDGE == tecInfo.edge 
                        && REBOUND_DELAY < tecInfo.edgeTime - fallingEdgeTime[TEC2_INDEX])
                {
                    /* Volvemos a comenzar con la deteccion de pulsos superpuestos */
                    tecState = WAITING_TEC1_TEC2_FALLING_EDGE_STATE;
                }
            break;

            case WAITING_TEC1_TEC2_RISING_EDGE_STATE:
                /* Veniamos de recibir un falling edge de ambas teclas */
                /* Si es un rising edge de la tecla 1 */
                if(TEC1 == tecInfo.tec && RISING_EDGE == tecInfo.edge)
                {
                    /* Guardamos el tick del sistema de la ocurrencia del rising edge */
                    risingEdgeTime[TEC1_INDEX] = tecInfo.edgeTime;
                    /* Pasamos a esperar un rising edge de la tecla 2 */
                    tecState = WAITING_TEC2_RISING_EDGE_STATE;
                }
                else if(TEC2 == tecInfo.tec && RISING_EDGE == tecInfo.edge)
                {
                    /* Guardamos el tick del sistema de la ocurrencia del rising edge */
                    risingEdgeTime[TEC2_INDEX] = tecInfo.edgeTime;
                    /* Pasamos a esperar un rising edge de la tecla 1 */
                    tecState = WAITING_TEC1_RISING_EDGE_STATE;
                }
            break;

            case WAITING_TEC1_RISING_EDGE_STATE:
                /* Pase lo que pase volvemos a comenzar con la deteccion de pulsos superpuestos */
                tecState = WAITING_TEC1_TEC2_FALLING_EDGE_STATE;
                /* Veniamos de recibir un rising edge de la tecla 2 */
                /* Si es un rising edge de la tecla 1 */
                if(TEC1 == tecInfo.tec && RISING_EDGE == tecInfo.edge)
                {
                    /* Guardamos el tick del sistema de la ocurrencia del rising edge */
                    risingEdgeTime[TEC1_INDEX] = tecInfo.edgeTime;
                    /* Guardamos el tiempo entre flancos ascendentes para el log via UART */
                    logInfo.risingEdgeTime = risingEdgeTime[TEC1_INDEX] - risingEdgeTime[TEC2_INDEX];
                    /* Guardamos el tiempo total para el encendido del led */
                    ledInfo.totalTime = logInfo.risingEdgeTime + logInfo.fallingEdgeTime;
                    /* Guardamos el led a pulsar */
                    /* Recordar que si entramos aca el rising edge de la tecla 1 vino despues del de
                       la tecla 2 y por lo tanto solo pueden ocurrir los casos 2 y 4 */
                    /* Si el momento en que se pulso la tecla 2 es mayor que el momento en que
                       se pulso la tecla 1 estamos en el caso 2  == led rojo */
                    if(fallingEdgeTime[TEC2_INDEX] >= fallingEdgeTime[TEC1_INDEX])
                    {
                        ledInfo.led = LEDR;
                        strcpy(logInfo.ledString, "Led Rojo");
                    }
                    /* Si no, es el caso 4 == led azul */
                    else
                    {
                        ledInfo.led = LEDB;
                        strcpy(logInfo.ledString, "Led Azul");
                    }
                    /* Mando a titilar el led */
                    queuePush(&g_ledQueue, (void *)(&ledInfo), OS_MAX_DELAY);
                    /* Envio el log */
                    queuePush(&g_logQueue, (void *)(&logInfo), OS_MAX_DELAY);
                }
            break;

            case WAITING_TEC2_RISING_EDGE_STATE:
                /* Pase lo que pase volvemos a comenzar con la deteccion de pulsos superpuestos */
                tecState = WAITING_TEC1_TEC2_FALLING_EDGE_STATE;
                /* Veniamos de recibir un rising edge de la tecla 1 */
                /* Si es un rising edge de la tecla 2 */
                if(TEC2 == tecInfo.tec && RISING_EDGE == tecInfo.edge)
                {
                    /* Guardamos el tick del sistema de la ocurrencia del rising edge */
                    risingEdgeTime[TEC2_INDEX] = tecInfo.edgeTime;
                    /* Guardamos el tiempo entre flancos ascendentes para el log via UART */
                    logInfo.risingEdgeTime = risingEdgeTime[TEC2_INDEX] - risingEdgeTime[TEC1_INDEX];
                    /* Guardamos el tiempo total para el encendido del led */
                    ledInfo.totalTime = logInfo.risingEdgeTime + logInfo.fallingEdgeTime;
                    /* Guardamos el led a pulsar */
                    /* Recordar que si entramos aca el rising edge de la tecla 2 vino despues del de
                       la tecla 1 y por lo tanto solo pueden ocurrir los casos 1 y 3 */
                    /* Si el momento en que se pulso la tecla 2 es mayor que el momento en que
                       se pulso la tecla 1 estamos en el caso 1  == led verde */
                    if(fallingEdgeTime[TEC2_INDEX] >= fallingEdgeTime[TEC1_INDEX])
                    {
                        ledInfo.led = LEDG;
                        strcpy(logInfo.ledString, "Led Verde");
                    }
                    /* Si no, es el caso 3 == led amarillo */
                    else
                    {
                        ledInfo.led = LED2;
                        strcpy(logInfo.ledString, "Led Amarillo");
                    }
                    /* Mando a titilar el led */
                    queuePush(&g_ledQueue, (void *)(&ledInfo), OS_MAX_DELAY);
                    /* Envio el log */
                    queuePush(&g_logQueue, (void *)(&logInfo), OS_MAX_DELAY);
                }
            break;

        }
    }
}

void logTask(void * parameters)
{
    char stringToSend[STRING_TO_SEND_LENGTH];
    logInfo_t logInfo;

    while(TRUE)
    {
        /* Esperamos hasta que haya que logear info porque hubo 2 flancos superpuestos */
        queuePull(&g_logQueue, (void *)(&logInfo), OS_MAX_DELAY);
        /* Formatemos el string a enviar */
        sprintf(stringToSend, "%s encendido:\n\r"
                "\t Tiempo encendido: %lu ms \n\r"
                "\t Tiempo entre flancos descendentes: %lu ms \n\r"
                "\t Tiempo entre flancos ascendentes: %lu ms \n\r"
                , logInfo.ledString
                , logInfo.risingEdgeTime + logInfo.fallingEdgeTime
                , logInfo.fallingEdgeTime
                , logInfo.risingEdgeTime);
        /* Enviamos el string por UART */
        uartWriteString(UART_USB, stringToSend);

    }
} 


void ledTask(void * parameters)
{
    ledInfo_t ledInfo;
    while(TRUE)
    {
        /* Esperamos hasta que haya que prender un led porque hubo flancos superpuestos */
        queuePull(&g_ledQueue, (void *)(&ledInfo), OS_MAX_DELAY);
        /* Encendemos el led */
        gpioWrite(ledInfo.led, 1);
        taskDelay(ledInfo.totalTime);
        /* Apagamos el led */
        gpioWrite(ledInfo.led, 0);
    }
} 

int main(void)
{
    /* Configuramos placa */
    Board_Init();
    SystemCoreClockUpdate();

    /* Configuramos UART */
    uartConfig(UART_USB, BAUDRATE_115200);

    /* Configuramos GPIO TEC 1*/
    gpioConfig(TEC1, GPIO_INPUT);
    gpioConfigIRQ(TEC1, GPIO_CHANNEL_0, BOTH_EDGE_INT);
    irqAttach(PIN_INT0_IRQn, GPIO0IRQHandler);

    /* Configuramos GPIO TEC 2*/
    gpioConfig(TEC2, GPIO_INPUT);
    gpioConfigIRQ(TEC2, GPIO_CHANNEL_1, BOTH_EDGE_INT);
    irqAttach(PIN_INT1_IRQn, GPIO1IRQHandler);

    /* Configuramos leds */
    gpioConfig(LED2, GPIO_OUTPUT);
    gpioConfig(LEDG, GPIO_OUTPUT);
    gpioConfig(LEDR, GPIO_OUTPUT);
    gpioConfig(LEDB, GPIO_OUTPUT);

    /* Inicializamos las colas */
    queueInit(&g_tecQueue, QUEUE_LEN, g_tecQueueBuffer, sizeof(tecInfo_t));
    queueInit(&g_ledQueue, QUEUE_LEN, g_ledQueueBuffer, sizeof(ledInfo_t));
    queueInit(&g_logQueue, QUEUE_LEN, g_logQueueBuffer, sizeof(logInfo_t));

    /* Creacion de las tareas */
    /* Menor numero mayor prioridad */
    taskCreate(pulseDetectorTask, 1, pulseDetectorTaskStack, OS_MINIMAL_STACK_SIZE, "plsDetTask", (void *)0);
    taskCreate(ledTask, 2, ledTaskStack, OS_MINIMAL_STACK_SIZE, "ledTask", (void *)0);
    taskCreate(logTask, 3, logTaskStack, OS_MINIMAL_STACK_SIZE, "logTask", (void *)0);
      
    /* Start the scheduler */
    taskStartScheduler();

    /* No se deberia arribar aqui nunca */
    return 1;
}

/*==================[end of file]============================================*/
