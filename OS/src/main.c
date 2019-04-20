/** 
* @file  main.c
* @brief 
* @note  Copyright 2019 - Esp. Ing. Matias Alvarez.
*/

/*==================[inclusions]=============================================*/
#include "OS_config.h"
#include "OS.h"
#include "OS_semphr.h"
#include "OS_queue.h"
#include "board.h"
#include <stdint.h>
#include "gpio.h"
#include "uart.h"
/*==================[macros]=================================================*/
#define EXECUTE_EJ1     0
#define EXECUTE_EJ2     0
#define EXECUTE_EJ3     0
#define EXECUTE_EJ6     0
#define EXECUTE_EJ7     1

#if ( EXECUTE_EJ1 == 1 )

#define MEF_DELAY       40
/*==================[typedef]================================================*/
typedef enum{
    BUTTON_UP
,   BUTTON_FALLING
,   BUTTON_DOWN
,   BUTTON_RAISING
} fsmButtonState_t;
/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/
/* Buffers a las pilas de cada tarea */
uint32_t buttonTaskStack[OS_MINIMAL_STACK_SIZE];
uint32_t ledTaskStack[OS_MINIMAL_STACK_SIZE];

semaphore_t sem;

uint32_t time;
/*==================[internal functions definition]==========================*/
void buttonTask(void * parameters)
{

    fsmButtonState_t buttonState = BUTTON_UP;
    uint32_t pressedButtonTime1, pressedButtonTime2;

    while(TRUE) {
        switch(buttonState)
        {
            case BUTTON_UP:
                if (!gpioRead(TEC1))
                {
                    buttonState = BUTTON_FALLING;
                }
            break;
            case BUTTON_FALLING:
                if (!gpioRead(TEC1))
                {
                buttonState = BUTTON_DOWN;
                /* Cuando se aprieta la tecla, tomamos el valor actual del tick */
                pressedButtonTime1 = taskGetTickCount();
                } 
                else
                {
                buttonState = BUTTON_UP;
                }
            break;
            case BUTTON_DOWN:
                if (gpioRead(TEC1))
                {
                    buttonState = BUTTON_RAISING;
                }
            break;
            case BUTTON_RAISING:
                if (!gpioRead(TEC1))
                {
                    buttonState = BUTTON_DOWN;
                } 
                else
                { /* Si se suelta tomamos el valor actual del tick */
                    pressedButtonTime2 = taskGetTickCount();

                    if(pressedButtonTime2 > pressedButtonTime1)
                        time = pressedButtonTime2 - pressedButtonTime1;
                    else
                        time = pressedButtonTime1 - pressedButtonTime2;

                    semphrGive(&sem);
                    buttonState = BUTTON_UP;
                }
            break;

        }
        taskDelay(MEF_DELAY);          
    }
}

void ledTask(void * parameters){
    uint8_t ledState = 0;

    while(TRUE){
        /* Si se puede tomar el semaforo significa que el usuario apreto y solto la tecla */
        if(OS_RESULT_OK == semphrTake(&sem, OS_MAX_DELAY))
        {
            /* Muestro en el led azul el tiempo que el usuario apreto la tecla */    
            gpioWrite(LEDB, 1);
            taskDelay(time);         
            gpioWrite(LEDB, 0);
        }

    }
} 

/*==================[external functions definition]==========================*/
int main(void){

    Board_Init();
    SystemCoreClockUpdate();
    /* Inicializamos los semaforos */
    semphrInit(&sem);
    /* Creacion de las tareas */
    /* Menor numero mayor prioridad */
    taskCreate(ledTask, 2, ledTaskStack, OS_MINIMAL_STACK_SIZE, "ledTask", (void *)0);
    taskCreate(buttonTask, 3, buttonTaskStack, OS_MINIMAL_STACK_SIZE, "buttonTask", (void *)0);
      
    /* Start the scheduler */
    taskStartScheduler();

    /* No se deberia arribar aqui nunca */
    return 1;
}
#elif (EXECUTE_EJ2 == 1)
/*==================[typedef]================================================*/
#define EXCHANGE_BUFFER_DELAY   6
#define BUFFER_SAMPLES          240
/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/
/* Buffers a las pilas de cada tarea */
uint32_t exchangeBufferTaskStack[OS_MINIMAL_STACK_SIZE];
uint32_t fillBufferTaskStack[OS_MINIMAL_STACK_SIZE];
uint32_t processTaskStack[OS_MINIMAL_STACK_SIZE];


semaphore_t processTaskSem;
semaphore_t fillBufferTaskSem;

uint16_t bufferA[BUFFER_SAMPLES];
uint16_t bufferB[BUFFER_SAMPLES];
uint16_t resultBuffer[BUFFER_SAMPLES];

void processTask(void * parameters)
{
   uint16_t * buffer = bufferA;
   uint32_t i;
   while(TRUE) {

        if(OS_RESULT_OK == semphrTake(&processTaskSem, OS_MAX_DELAY))
        {
            /* Toggleamos un led para indicar que esta ejecutando */
            Board_LED_Toggle(2);
            for(i = 0; i < BUFFER_SAMPLES; i++)
            {
                resultBuffer[i] = 2 * buffer[i];
            }

            buffer = (buffer == bufferA) ? bufferB : bufferA;
        }
   }
}

void fillBufferTask(void * parameters){
    uint16_t * buffer = bufferA;
    uint32_t i;
    while(TRUE){
    
        if(OS_RESULT_OK == semphrTake(&fillBufferTaskSem, OS_MAX_DELAY))
        {
            /* Toggleamos un led para indicar que esta ejecutando */
            Board_LED_Toggle(3);
            for(i = 0; i < BUFFER_SAMPLES; i++)
            {
                buffer[i] = i + 1;
            }
            buffer = (buffer == bufferA) ? bufferB : bufferA;
        }

    }
} 

void exchangeBufferTask(void * parameters){
    uint8_t firstTime = 1;

    while(TRUE){
        /* La primera vez ambos buffers estan vacios, entonces solo deberia ejecutarse
            la tarea que los llena */
        if(!firstTime)
        {
            semphrGive(&processTaskSem);
        }
        firstTime = 0;
        semphrGive(&fillBufferTaskSem);
        
        taskDelay(EXCHANGE_BUFFER_DELAY);

    }
} 
/*==================[external functions definition]==========================*/
int main(void){

    Board_Init();
    SystemCoreClockUpdate();

    /* Inicializamos los semaforos */
    semphrInit(&processTaskSem);
    semphrInit(&fillBufferTaskSem);

    /* Creacion de las tareas */
    /* Menor numero mayor prioridad */
    taskCreate(exchangeBufferTask, 1, exchangeBufferTaskStack, OS_MINIMAL_STACK_SIZE, "exchBuff", (void *)0);
    taskCreate(fillBufferTask, 2, fillBufferTaskStack, OS_MINIMAL_STACK_SIZE, "fillBuff", (void *)0);
    taskCreate(processTask, 2, processTaskStack, OS_MINIMAL_STACK_SIZE, "procBuff", (void *)0);
      
    /* Start the scheduler */
    taskStartScheduler();

    /* No se deberia arribar aqui nunca */
    return 1;
}
#elif (EXECUTE_EJ3 == 1)

#define MEF_DELAY       40
/*==================[typedef]================================================*/
typedef enum{
    BUTTON_UP
,   BUTTON_FALLING
,   BUTTON_DOWN
,   BUTTON_RAISING
} fsmButtonState_t;
/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/
/* Buffers a las pilas de cada tarea */
uint32_t buttonTaskStack[OS_MINIMAL_STACK_SIZE];
uint32_t ledTaskStack[OS_MINIMAL_STACK_SIZE];

queue_t q;
/*==================[internal functions definition]==========================*/
void buttonTask(void * parameters)
{

    fsmButtonState_t buttonState = BUTTON_UP;
    uint32_t pressedButtonTime1, pressedButtonTime2;
    queueData_t qData =  {
                        .ptrToData = (void *)&pressedButtonTime2,
                        .dataLen   = sizeof(uint32_t)
                   };
    while(TRUE) {
        switch(buttonState)
        {
            case BUTTON_UP:
                if (!gpioRead(TEC1))
                {
                    buttonState = BUTTON_FALLING;
                }
            break;
            case BUTTON_FALLING:
                if (!gpioRead(TEC1))
                {
                buttonState = BUTTON_DOWN;
                /* Cuando se aprieta la tecla, tomamos el valor actual del tick */
                pressedButtonTime1 = taskGetTickCount();
                } 
                else
                {
                buttonState = BUTTON_UP;
                }
            break;
            case BUTTON_DOWN:
                if (gpioRead(TEC1))
                {
                    buttonState = BUTTON_RAISING;
                }
            break;
            case BUTTON_RAISING:
                if (!gpioRead(TEC1))
                {
                    buttonState = BUTTON_DOWN;
                } 
                else
                { /* Si se suelta tomamos el valor actual del tick */
                    pressedButtonTime2 = taskGetTickCount();

                    if(pressedButtonTime2 > pressedButtonTime1)
                        pressedButtonTime2 = pressedButtonTime2 - pressedButtonTime1;
                    else
                        pressedButtonTime2 = pressedButtonTime1 - pressedButtonTime2;

                    buttonState = BUTTON_UP;
                    queuePush(&q, &qData, 0);
                }
            break;

        }
        taskDelay(MEF_DELAY);          
    }
}

void ledTask(void * parameters){
    uint8_t ledState = 0;
    queueData_t *qData;
    while(TRUE){
        /* Si se puede tomar el semaforo significa que el usuario apreto y solto la tecla */
        if(OS_RESULT_OK == queuePull(&q, &qData, OS_MAX_DELAY))
        {
            uint32_t time = *((uint32_t *)(*qData).ptrToData);

            /* Muestro en el led azul el tiempo que el usuario apreto la tecla */    
            gpioWrite(LEDB, 1);
            taskDelay(time);         
            gpioWrite(LEDB, 0);
        }

    }
} 

/*==================[external functions definition]==========================*/
int main(void){

    Board_Init();
    SystemCoreClockUpdate();
    /* Inicializamos los semaforos */
    queueInit(&q);
    /* Creacion de las tareas */
    /* Menor numero mayor prioridad */
    taskCreate(ledTask, 2, ledTaskStack, OS_MINIMAL_STACK_SIZE, "ledTask", (void *)0);
    taskCreate(buttonTask, 1, buttonTaskStack, OS_MINIMAL_STACK_SIZE, "buttonTask", (void *)0);
      
    /* Start the scheduler */
    taskStartScheduler();

    /* No se deberia arribar aqui nunca */
    return 1;
}

#elif (EXECUTE_EJ6 == 1)
#define REBOUND_DELAY 40
/*==================[typedef]================================================*/
typedef enum
{
    RISING_EDGE
,   FALLING_EDGE
}edge_t;

typedef struct
{
   uint32_t currentTickTime;
   edge_t edge;
}tecInfo_t;
/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/
/* Buffers a las pilas de cada tarea */
uint32_t buttonTaskStack[OS_MINIMAL_STACK_SIZE];
uint32_t ledTaskStack[OS_MINIMAL_STACK_SIZE];

semaphore_t buttonTaskSem;
semaphore_t ledTaskSem;
tecInfo_t tecInfo;
uint32_t time;

void buttonTask(void * parameters)
{
   uint32_t pressedButtonTime;
   
   while(TRUE) {
        if(OS_RESULT_OK == semphrTake(&buttonTaskSem, OS_MAX_DELAY))
        {

            if(FALLING_EDGE == tecInfo.edge)
            {
                pressedButtonTime = tecInfo.currentTickTime;
            }
            else if(tecInfo.currentTickTime - pressedButtonTime > REBOUND_DELAY)
            {
                time = tecInfo.currentTickTime - pressedButtonTime;
                semphrGive(&ledTaskSem);
            }

        }
   }
}

void ledTask(void * parameters){
    uint8_t ledState = 0;

    while(TRUE){
        /* Si se puede tomar el semaforo significa que el usuario apreto y solto la tecla */
        if(OS_RESULT_OK == semphrTake(&ledTaskSem, OS_MAX_DELAY))
        {
            /* Muestro en el led azul el tiempo que el usuario apreto la tecla */    
            gpioWrite(LEDB, 1);
            taskDelay(time);         
            gpioWrite(LEDB, 0);
        }

    }
} 
/*==================[external functions definition]==========================*/
int main(void){

    Board_Init();
    SystemCoreClockUpdate();
    /* Configuramos GPIO */
    gpioConfig(TEC1, GPIO_INPUT);
    gpioConfigIRQ(TEC1, GPIO_CHANNEL_0, BOTH_EDGE_INT);

    /* Inicializamos los semaforos */
    semphrInit(&ledTaskSem);
    semphrInit(&buttonTaskSem);

    /* Creacion de las tareas */
    /* Menor numero mayor prioridad */
    taskCreate(ledTask, 1, ledTaskStack, OS_MINIMAL_STACK_SIZE, "ledTask", (void *)0);
    taskCreate(buttonTask, 2, buttonTaskStack, OS_MINIMAL_STACK_SIZE, "buttonTask", (void *)0);
      
    /* Start the scheduler */
    taskStartScheduler();

    /* No se deberia arribar aqui nunca */
    return 1;
}

void GPIO0_IRQHandler(void){

    /* If interrupt was because a rising edge */
    if ( Chip_PININT_GetRiseStates(LPC_GPIO_PIN_INT) & PININTCH(GPIO_CHANNEL_0) ) 
    {

        /* Save actual tick count in currentTickTime */
        tecInfo.currentTickTime = taskGetTickCount();
        tecInfo.edge = RISING_EDGE;
        /* Clear rise edge irq */
        Chip_PININT_ClearRiseStates(LPC_GPIO_PIN_INT,PININTCH(GPIO_CHANNEL_0));
    }
    /* If not, interrupt was because a falling edge */
    else 
    {
        /* Save actual tick count in currentTickTime */
        tecInfo.currentTickTime = taskGetTickCount();
        tecInfo.edge = FALLING_EDGE;
        /* Clear falling edge irq */
        Chip_PININT_ClearFallStates(LPC_GPIO_PIN_INT,PININTCH(GPIO_CHANNEL_0));
    }

    /* Clear interrupt flag for GPIO_CHANNEL_0 */
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(GPIO_CHANNEL_0));

    semphrGive(&buttonTaskSem);
}

#elif ( EXECUTE_EJ7 == 1 )
#include <stdlib.h>

#define MAX_TEMP_SENSOR 4   
#define MAX_HUM_SENSOR  3

#define MS_TO_SEC(ms) ((ms) / 1000)
#define SEC_TO_MIN(sec) ((sec) / 60)
#define SEC_TO_HOUR(min) ((min) / 3600)

#define HOUR_TO_SEC(hour)   ((hour)*3600)

#define MIN_TO_SEC(min)    ((min)*60)

#define MAX_BUFFER_SIZE 64
/* Buffers a las pilas de cada tarea */
uint32_t tempSensorTaskStack[MAX_TEMP_SENSOR][OS_MINIMAL_STACK_SIZE];
uint32_t humSensorTaskStack[MAX_HUM_SENSOR][OS_MINIMAL_STACK_SIZE];

uint8_t tempSensorId[MAX_TEMP_SENSOR];
uint8_t humSensorId[MAX_HUM_SENSOR];

semaphore_t uartSem;

void tempSensorTask(void * parameters)
{
    uint8_t * id;
    uint8_t buffer[MAX_BUFFER_SIZE];
    id = (uint8_t *)parameters;

    while(1)
    {
        uint32_t ticks = taskGetTickCount();
        
        while(OS_RESULT_OK != semphrTake(&uartSem, OS_MAX_DELAY))
            ;

        uint8_t sec, min, hour;
        sec = MS_TO_SEC(ticks);
        hour = SEC_TO_HOUR(sec);
        sec -= HOUR_TO_SEC(hour);
        min = SEC_TO_MIN(sec);
        sec -= MIN_TO_SEC(min); 

        sprintf(buffer, "[%02d:%02d:%02d][INVERNADERO:001][TEMP:%d][%d.%dC]\r\n", hour, min, sec, *id, rand() % 50, rand() % 10);

        uartWriteString(UART_USB, buffer);
        semphrGive(&uartSem);
    

        taskDelay((rand() % 500) + 5);
    }
    
}

void humSensorTask(void * parameters){
    uint8_t * id;
    uint8_t buffer[MAX_BUFFER_SIZE];
    id = (uint8_t *)parameters;

    while(1)
    {
        uint32_t ticks = taskGetTickCount();
        uint8_t sec, min, hour;

        while(OS_RESULT_OK != semphrTake(&uartSem, OS_MAX_DELAY))
            ;

        sec = MS_TO_SEC(ticks);
        hour = SEC_TO_HOUR(sec);
        sec -= HOUR_TO_SEC(hour);
        min = SEC_TO_MIN(sec);
        sec -= MIN_TO_SEC(min); 

        sprintf(buffer, "[%02d:%02d:%02d][INVERNADERO:001][HUM:%d][%d%%]\r\n", hour, min, sec, *id, rand() % 101);

        uartWriteString(UART_USB, buffer);
        semphrGive(&uartSem);

        taskDelay((rand() % 500) + 5);
    }

} 

int main(void){
    uint8_t i;

    Board_Init();
    SystemCoreClockUpdate();
    /* Configuramos GPIO */
    uartConfig(UART_USB, BAUDRATE_115200);

    /* Inicializamos los semaforos */
    semphrInit(&uartSem);

    semphrGive(&uartSem);

    /* Creacion de las tareas */
    /* Menor numero mayor prioridad */
    for(i = 0; i < MAX_TEMP_SENSOR; i++){
        tempSensorId[i] = i + 1;
        taskCreate(tempSensorTask, 1, tempSensorTaskStack[i], OS_MINIMAL_STACK_SIZE, "tempSensor", (void *)&tempSensorId[i]);
    }

    for(i = 0; i < MAX_HUM_SENSOR; i++){
        humSensorId[i] = i + 1;
        taskCreate(humSensorTask, 1, humSensorTaskStack[i], OS_MINIMAL_STACK_SIZE, "humSensor", (void *)&humSensorId[i]);
    }

    /* Start the scheduler */
    taskStartScheduler();

    /* No se deberia arribar aqui nunca */
    return 1;
}

#endif
/*==================[end of file]============================================*/
