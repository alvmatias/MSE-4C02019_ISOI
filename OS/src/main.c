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
/*==================[macros]=================================================*/
#define EXECUTE_EJ1     0
#define EXECUTE_EJ2     0
#define EXECUTE_EJ3     1
#define EXECUTE_EJ6     0

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
                buffer[i] *= 2;
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

#endif
/*==================[end of file]============================================*/
