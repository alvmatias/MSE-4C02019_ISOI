/** 
* @file  OS.c
* @brief Implementacion se un sistema operativo estatico con scheduling preemtivo
* @brief Funcionalidades implementadas:
         1 - Scheduling preemptive con prioridades
         2 - Delay en ms
         3 - Tick Hook
         4 - Idle Hook
         5 - Semaforos
         6 - Colas  
* @note  Copyright 2019 - Esp. Ing. Matias Alvarez.
*/

/*==================[inclusions]=============================================*/
#include "OS.h"
#include "board.h"
#include <string.h>
/*==================[macros]=================================================*/
/**
* @def OS_INVALID_TASK
* @var ID de la tarea invalida
*/
#define OS_INVALID_TASK     0xFF

/**
* @def OS_NULL_PRIORITY
* @var Valor de la prioridad nula
*/
#define OS_NULL_PRIORITY    0

/**
* @def EXC_RETURN
* @var Valor de retorno de interrupcion
*/
#define EXC_RETURN          0xFFFFFFF9

/**
* @def NVIC_PENDSV_PRI
* @var Prioridad de la interrupcion PendSV
*/
#define NVIC_PENDSV_PRI     0xff
/*==================[typedef]================================================*/
/** @enum osState_t
* @brief Posibles estados del SO
*/
typedef enum
{
    OS_STATE_SUSPENDED  = 0x00          /* Estado Suspendido - El OS NO realiza cambio de contexto */
,   OS_STATE_RUNNING                    /* Estado Corriendo  - EL OS ejecuta normalmente */ 
}osState_t;

/**
* @enum taskState_t 
* @brief Posibles estados de una tarea
*/
typedef enum
{
    TASK_STATE_TERMINATED  = 0x00       /**< Estado Terminado - La tarea no pertenece al sistema */
,   TASK_STATE_READY                    /**< Estado Ready     - La tarea esta a la espera de su ejecucion */
,   TASK_STATE_RUNNING                  /**< Estado Corriendo - La tarea esta corriendo */
,   TASK_STATE_BLOCKED                  /**< Estado Bloqueado - La tarea esta a la espera de la ocurrencia de un evento*/
}taskState_t;

/**
* @struct taskControlBlock_t
* @brief Estructura de control de cada tarea del SO
*/
typedef struct 
{
    uint32_t *      stack;                          /**< Puntero al stack de la tarea - Buffer provisto por el usuario del SO */
    uint32_t        stackSize;                      /**< Tamaño del stack de la tarea */
    uint32_t        stackPointer;                   /**< Puntero de pila */   
    taskFunction_t  taskFx;                         /**< Tarea a ejecutar */
    void  *         parameters;                     /**< Puntero a los parametros de la tarea */
    uint32_t        priority;                       /**< Prioridad de la tarea */
    #if ( OS_USE_TASK_DELAY == 1 )
        taskState_t     state;                      /**< Estado de la tarea */
        uint32_t        ticksToWait;                /**< Ticks a esperar en caso de ejecucion de taskDelay() */
    #endif

    uint8_t         taskName[OS_MAX_TASK_NAME_LEN]; /**< Nombre de la tarea - Solo como proposito de debug */ 

}taskControlBlock_t;

/**
* @struct readyTaskInfo_t
* @brief Estructura de informacion de la lista de tareas ready de cada prioridad.
* @note Con el objetivo de evitar hacer corriemientos de los elementos de la lista de
        tareas ready, se implementa un punteroa la primera tarea ready que ira avanzando de
        forma circular. Invertimos memoria por tiempo de ejecucion del scheduler, ya que 
        este debe ejecutarse lo mas rapido posible.
*/
typedef struct 
{
    uint8_t readyTaskCnt;       /**< Cantidad de tareas ready */
    uint8_t firstReadyTask;     /**< Puntero circular a la primera tarea ready */
}readyTaskInfo_t;

/**
* @struct osControl_t
* @brief Estructura de control de  del SO
*/
typedef struct
{
    uint8_t             currentTask;                                       /**< Tarea actual */
    uint8_t             maxTask;                                           /**< Tareas agregadas al SO, puede diferir de OS_MAX_TASK */
    uint32_t            tickCount;                                         /**< Tick del sistema */
    uint32_t            readyTaskList[OS_MAX_TASK_PRIORITY][OS_MAX_TASK];  /**< Lista de tareas ready por cada prioridad */
    readyTaskInfo_t     readyTaskInfo[OS_MAX_TASK_PRIORITY];               /**< Informacion de la lista de tareas ready por cada prioridad */
#if ( OS_USE_TASK_DELAY == 1 )
    /* Si esta estipulado el uso de delay se crea el stack de la idle task */
    uint32_t idleTaskStack[OS_IDLE_STACK_SIZE];                             /**< Task Stack para la Idle Task */
    /* Si esta estipulado el uso de delay se crea un espacio extra para la idle task */
    taskControlBlock_t taskList[OS_MAX_TASK + 1];                           /**< Lista de tareas del sistema */
#else
    taskControlBlock_t taskList[OS_MAX_TASK];                               /**< Lista de tareas del sistema */
#endif
    osState_t           state;

}osControl_t;
/*==================[internal data declaration]==============================*/
/**
* @var static osControl_t g_Os;
* @brief Estructura de control del SO.
* @note Variable privada
*/
static osControl_t g_Os = 
{
    .maxTask        = 0                 /* Inicializacion en 0 */
,   .currentTask    = OS_INVALID_TASK   /* Inicializacion en valor invalido */
,   .tickCount      = 0                 /* Inicializacion en 0 */
};

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

#if ( OS_USE_TICK_HOOK == 1 )
    /**
    * @fn void tickHook(void)
    * @brief Funcion a ejecutarse en cada tick del sistema
    * @param Ninguno
    * @return Nada
    * @note Funcion weak
    */
    __attribute__ ((weak)) void tickHook(void)
    {
        /* DO NOTHING */
    }   
#endif

#if ( OS_USE_TASK_DELAY == 1 )
    /**
    * @fn void idleHook(void * parameters)
    * @brief Tarea Idle - Se ejecuta cuando ninguna tarea esta en estado ready
    * @param parameters : Puntero a los parametros a pasarle a la tarea
    * @return Nada
    * @note Funcion weak
    * @warning No se pueden realizar llamadas de delay en esta funcion - 
               El sistema las descarta.
    */
    __attribute__ ((weak)) void idleHook(void * parameters)
    {
        while(1)
        {
            /* Sleep */
            __WFI();
        }
    }
#endif

/**
* @fn static void returnHook()
* @brief Funcion a ejecutarse en caso de que una tarea retorne
* @param Ninguno
* @return NUNCA RETORNA
*/
static void returnHook()
{
    /* While infinito */
    while(1)
        ;
}

/**
* @fn static osReturn_t osIncrementTick()
* @brief Funcion que incrementa el tick del sistema
* @param Ninguno
* @return Nada
*/
static osReturn_t osIncrementTick()
{
    osReturn_t retVal = OS_RESULT_ERROR;    /**< Valor de retorno de la funcion */

    /* Aumentamos en 1 la cuenta de ticks del SO */
    g_Os.tickCount++;
    /* Si es tick de ejecucion de scheduler */
    if(0 == (g_Os.tickCount % OS_TICKS_UNTIL_SCHEDULE)) 
    {

        #if ( OS_USE_TICK_HOOK == 1 )
        {
            tickHook();
        }
        #endif

        retVal = OS_RESULT_OK;
    }   

    return retVal;

}

#if ( OS_USE_PRIO_ROUND_ROBIN_SCHED == 1)
    /**
    * @fn static void addReadyTask(uint8_t id, uint32_t prio)
    * @brief Funcion que agrega una tarea a la lista de tareas ready
    * @param id    : id de la tarea a agregar
    * @param prio  : Prioridad de la tarea a agregar
    * @return Nada
    */
    static void addReadyTask(uint8_t id, uint32_t prio)
    {
        /* Agregamos la tarea en el primer lugar libre dentro de la prioridad especifica */
        g_Os.readyTaskList[prio]
        [(g_Os.readyTaskInfo[prio].firstReadyTask + g_Os.readyTaskInfo[prio].readyTaskCnt) % g_Os.maxTask] = id;
        /* Aumentamos la cantidad de tareas ready */
        g_Os.readyTaskInfo[prio].readyTaskCnt++;

    }

    /**
    * @fn static void removeReadyTask(uint8_t * id, uint32_t prio)
    * @brief Funcion que remueve la primer tarea ready
    * @param id    : Puntero donde se guarda el id de la tarea removida
    * @param prio  : Prioridad de la tarea a remover 
    * @return Nada
    */
    static void removeReadyTask(uint8_t * id, uint32_t prio)
    {
        /* Obtenemos la primera tarea ready */
        *id = g_Os.readyTaskList[prio][g_Os.readyTaskInfo[prio].firstReadyTask];
        /* Decrementamos la cantidad de tareas ready */
        g_Os.readyTaskInfo[prio].readyTaskCnt--;
        
        /* Si la cantidad de tareas ready es 0 */
        if(0 == g_Os.readyTaskInfo[prio].readyTaskCnt)
        {
            /* Reiniciamos el puntero a la primer tarea ready */
            g_Os.readyTaskInfo[prio].firstReadyTask = 0;
        }
        else
        {
            /* Sino, lo avanzamos en 1 */
            g_Os.readyTaskInfo[prio].firstReadyTask = (g_Os.readyTaskInfo[prio].firstReadyTask + 1) % g_Os.maxTask;
        }
        
    }
#endif  

#if ( OS_USE_TASK_DELAY == 1 )
    /**
    * @fn static void delayUpdate(void)
    * @brief Funcion actualiza los ticks restantes de las tareas bloqueadas
    * @param  Ninguno
    * @return Nada
    */
    static void delayUpdate(void)
    {
        uint8_t i;  /**< Indice del for */
        /* Por cada tarea */
        for (i = 0; i < g_Os.maxTask; i++) 
        {
            /* Si la tarea esta bloqueada y la cantidad de ticks a esperar es mayor a 0 
                y menor que el maximo delay(delay forever) */
            if (TASK_STATE_BLOCKED == g_Os.taskList[i].state 
                && 0 < g_Os.taskList[i].ticksToWait && OS_MAX_DELAY > g_Os.taskList[i].ticksToWait) 
            {
                /* Decrementamos los ticks a esperar */
                g_Os.taskList[i].ticksToWait--;
                /* Si luego del decremento los ticks llegaron a 0 */
                if (g_Os.taskList[i].ticksToWait == 0) {
                    /* Ponemos la tarea en ready y la agregamos a la lista de tareas ready */
                    g_Os.taskList[i].state = TASK_STATE_READY;
                    addReadyTask(i, g_Os.taskList[i].priority - 1);
                }
            }
        }
    }
#endif

/**
* @fn static void initStack(uint32_t * stack, 
                      uint32_t stackSize, 
                      uint32_t priority,    
                      taskFunction_t taskFx,
                      void * parameters)
* @brief Funcion inicializa el stack de una tarea
* @param stack      : Puntero al stack de la tarea
* @param stackSize  : Tamaño del stack de la tarea
* @param priority   : Prioridad de la tarea
* @param taskFx     : Prototipo de la tarea
* @param taskName   : Nombre de la tarea
* @parameters       : Puntero a los parametros a pasarle a la tarea
* @return Nada
*/
static void initStack(uint32_t * stack, 
                      uint32_t stackSize, 
                      uint32_t priority,    
                      taskFunction_t taskFx,
                      char * taskName,
                      void * parameters)
{

    /* Inicializo el frame en cero */
    bzero(stack, stackSize);

    /* Ultimo elemento del contexto inicial: xPSR
     * Necesita el bit 24 (T, modo Thumb) en 1
     */
    stack[stackSize/4 - 1]  = 1 << 24;

    /* Anteultimo elemento: PC (entry point) */
    stack[stackSize/4 - 2]  = (uint32_t)taskFx;

    /* Penultimo elemento: LR (return hook) */
    stack[stackSize/4 - 3]  = (uint32_t)returnHook;

    /* Elemento -8: R0 (parámetro) */
    stack[stackSize/4 - 8]  = (uint32_t)parameters;

    stack[stackSize/4 - 9]  = EXC_RETURN;

    /* Inicializo stack pointer inicial considerando lo otros 8 registros pusheados */
    g_Os.taskList[g_Os.maxTask].stackPointer  = (uint32_t)&(stack[stackSize/4 - 17]);

    /* Inicialiazamos el TCB */
    g_Os.taskList[g_Os.maxTask].taskFx        = taskFx;
    g_Os.taskList[g_Os.maxTask].stack         = stack;
    g_Os.taskList[g_Os.maxTask].stackSize     = stackSize;
    g_Os.taskList[g_Os.maxTask].parameters    = parameters;
    g_Os.taskList[g_Os.maxTask].priority      = priority;
    if(OS_MAX_TASK_NAME_LEN > strlen(taskName))
    {
        strcpy(g_Os.taskList[g_Os.maxTask].taskName, taskName);    
    }
    else
    {
        strcpy(g_Os.taskList[g_Os.maxTask].taskName, "noName");    
    }
    
    g_Os.taskList[g_Os.maxTask].state         = TASK_STATE_READY;

    #if ( OS_USE_TASK_DELAY == 1 )
        /* Inicializamos los ticks de delay en 0 */
        g_Os.taskList[g_Os.maxTask].ticksToWait   = 0;
    #endif

}
/*==================[external functions definition]==========================*/
/**
* @fn void schedule()
* @brief Funcion que setea la interrupcion del PendSV para que se ejecute el scheduler
* @param Ninguno
* @return Nada
* @warning NO DEBERIA SER LLAMADA POR EL USUARIO DEL OS
*/
void schedule()
{
    /* Instruction Synchronization Barrier: aseguramos que se
     * ejecuten todas las instrucciones en  el pipeline
     */
    __ISB();
    /* Data Synchronization Barrier: aseguramos que se
     * completen todos los accesos a memoria
     */
    __DSB();

    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
    
}

/**
* @fn osReturn_t taskCreate(taskFunction_t taskFx, uint32_t priority, uint32_t * stack, uint32_t stackSize,
                   char * taskName, void * parameters)
* @brief Funcion que crea una tarea dentro del SO
* @param stack      : Puntero al stack de la tarea
* @param stackSize  : Tamaño del stack de la tarea
* @param priority   : Prioridad de la tarea
* @param taskFx     : Prototipo de la tarea
* @param taskName   : Nombre de la tarea
* @parameters       : Puntero a los parametros a pasarle a la tarea
* @return osReturn_t OS_RESULT_ERROR si la tarea no se puede crear, OS_RESULT_OK caso contrario
*/
osReturn_t taskCreate(taskFunction_t taskFx, uint32_t priority, uint32_t * stack, uint32_t stackSize,
                   char * taskName, void * parameters)
{
    osReturn_t retVal = OS_RESULT_ERROR;

    /* Si hay lugar para crear una nueva tarea, su stack size es mayor que el menor permitido,
    su prioridad es menor que la maxima prioridad y distinta de 0 */
    if(OS_MAX_TASK > g_Os.maxTask && OS_MINIMAL_STACK_SIZE <= stackSize && 
        OS_MAX_TASK_PRIORITY >= priority && OS_NULL_PRIORITY != priority)
    {
        /* Inicializamos el stack y aumentamos en 1 la cantidad de tareas actual del SO */
        initStack(stack, stackSize, priority, taskFx, taskName, parameters);
        g_Os.maxTask++;

        retVal = OS_RESULT_OK;
        
    }

    return retVal;
}

/**
* @fn void taskStartScheduler()
* @brief Funcion que inicializa el scheduler del SO
* @param  Ninguno
* @return Nada
* @note No deberia retornar nunca
*/
void taskStartScheduler()
{
    
    uint8_t p; /** Variable para recorrer la lista de prioridades y la lista de tareas */

    g_Os.state = OS_STATE_RUNNING;

    /* So se usa el delay, se inicializa el stack de la idle task */
    #if ( OS_USE_TASK_DELAY == 1 )
        /* Creacion idle task */
        initStack(g_Os.idleTaskStack, OS_IDLE_STACK_SIZE, OS_NULL_PRIORITY, idleHook, "IdleTask", (void*)0);
    #endif

    /* Se agregan todas las tareas */
    for(p = 0; p < g_Os.maxTask; p++)
    {
        addReadyTask(p, g_Os.taskList[p].priority - 1);
    }
    /* Se inicializan los punteros a primera tarea ready */
    for(p = 0; p < OS_MAX_TASK_PRIORITY; p++)
    {
        g_Os.readyTaskInfo[p].firstReadyTask = 0;
    }

    /* Systick y pendSV con menor prioridad posible */
    NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS) - 1);

    SysTick_Config(SystemCoreClock / 1000);

    /* Se setea la interrupcion de pendSV */
    schedule();

    /* No deberia arribar aca NUNCA */
    while(1)
    {
        __WFI();
    }
}

/**
* @fn void taskStartScheduler()
* @brief Funcion que retorna la siguiente tarea a ejecutarse
* @param  currentContext : Contexto de la tarea actual
* @return Contexto de la siguiente tarea a ejecutarse
*/
int32_t taskSchedule(int32_t currentContext)
{
    uint8_t p;  /** Variable para recorrer la lista de prioridades */
    
    if(OS_IDLE_TASK == g_Os.currentTask)
    {
        /* Aca se entra si volvemos de la idle task */
        #if ( OS_USE_TASK_DELAY == 1 )
            /* Guardamos el contexto de la idle task */
            g_Os.taskList[g_Os.maxTask].stackPointer  = currentContext; 
            /* Si bien no es necesario cambiarle el estado a la idle task
             porque solo puede tener dos estados posibles lo hacemos para 
             mantener coherencia con el resto de las tareas */
            g_Os.taskList[g_Os.maxTask].state         = TASK_STATE_READY;
            
        #endif  
    }
    else
    {
        /* Aca se entra si volvemos de cualquiera de las otras tareas del sistema */
        /* Guardamos el contexto de la tarea actual */
        g_Os.taskList[g_Os.currentTask].stackPointer  = currentContext;
        /* Podemos haber entrado al schedule por culpa de un taskDelay */
        if(OS_STATE_RUNNING == g_Os.state && TASK_STATE_RUNNING == g_Os.taskList[g_Os.currentTask].state)
        {
            /* Solo seteamos la tarea en ready si estaba corriendo */
            g_Os.taskList[g_Os.currentTask].state = TASK_STATE_READY;
            /* La agregamos a la lista de tareas ready */
            addReadyTask(g_Os.currentTask, g_Os.taskList[g_Os.currentTask].priority - 1);
        }
        
    }
    /* Si el SO esta corriendo, hacemos el cambio de contexto */
    if(OS_STATE_RUNNING == g_Os.state)
    {
        /* Por cada prioridad - IMPORTANTE: Mayor prioridad == Menor numero */
        for(p = 0; p < OS_MAX_TASK_PRIORITY; p++)
        {
            /* Si hay tareas ready en la prioridad */
            if(g_Os.readyTaskInfo[p].readyTaskCnt > 0)
            {
                /* Removemos siempre la primera tarea ready */
                removeReadyTask(&g_Os.currentTask, p);
                /* Frenamos la busqueda de tareas ready */
                break; 
            }
        }

        #if ( OS_USE_TASK_DELAY == 1 )
            /* Si salimos del for sin encontrar una tarea ready */
            if(p >= OS_MAX_TASK_PRIORITY)
            {
                /* Seteamos como tarea actual a la idle task */
                /* Recordar que guardamos su informacion en el ultimo elemento de la lista de tareas */
                g_Os.currentTask = g_Os.maxTask;
            }
        #endif
        
        /* Seteamos el estado de la tarea a ejecutarse como corriendo */
        g_Os.taskList[g_Os.currentTask].state  = TASK_STATE_RUNNING;
        
    }
    /* Retornamos su contexto */
    return g_Os.taskList[g_Os.currentTask].stackPointer;

}

#if ( OS_USE_TASK_DELAY == 1 )
    /**
    * @fn void taskDelay(uint32_t ticksToDelay)
    * @brief Funcion que bloquea una tarea por ticksToDelay ms
    * @param  ticksToDelay : Ticks que debe esperar bloqueada la tarea
    * @return Nada
    */
    void taskDelay(uint32_t ticksToDelay)
    {
        /* Si el delay es mayor a 0 y la tarea que llama a taskDelay NO es la idle task */
        if(0 < ticksToDelay && OS_IDLE_TASK != g_Os.currentTask)
        {
            g_Os.taskList[g_Os.currentTask].state = TASK_STATE_BLOCKED;
            g_Os.taskList[g_Os.currentTask].ticksToWait = ticksToDelay; 
            schedule();
        }
        
    }
#endif

/**
* @fn uint32_t taskGetTickCount()
* @brief Funcion que bloquea una tarea por ticksToDelay ms
* @param  Ninguno
* @return Tick actual del sistema
*/
uint32_t taskGetTickCount()
{
    return g_Os.tickCount;
}

/**
* @fn uint32_t taskYield()
* @brief Funcion que cede el procesador a otra tarea
* @param  Ninguno
* @return osReturn_t OS_RESULT_OK si pudo ceder el procesador, OS_RESULT_ERROR caso contrario
*/
osReturn_t taskYield()
{
    
    osReturn_t retVal = OS_RESULT_OK;  /** Valor de retorno */
    /* Si la tarea se encuentra corriendo */
    if(TASK_STATE_RUNNING == g_Os.taskList[g_Os.currentTask].state)
    {
        /* Pasa a ready */
        g_Os.taskList[g_Os.currentTask].state = TASK_STATE_READY;
        addReadyTask(g_Os.currentTask, g_Os.taskList[g_Os.currentTask].priority - 1);
        /* Llamamos al scheduler */
        schedule();
    }
    else
    {
        /* Sino, devolvemos error */
        retVal = OS_RESULT_ERROR;
    }

    return retVal;

}

/**
* @fn void taskUnsuspendWithinAPI(uint8_t taskId)
* @brief Funcion que desbloquea una tarea
* @param  taskId : id de la tarea a desbloquear
* @return Nada
* @warning NO DEBE SER USADA POR EL USUARIO
*/
void taskUnsuspendWithinAPI(uint8_t taskId)
{
    g_Os.taskList[taskId].ticksToWait = 0;
    g_Os.taskList[taskId].state = TASK_STATE_READY;
    addReadyTask(taskId, g_Os.taskList[taskId].priority - 1);
}

/**
* @fn void osSuspendContextSwitching()
* @brief Funcion que suspende el context switch
* @param  Ninguno
* @return Nada
*/
void osSuspendContextSwitching()
{
    g_Os.state = OS_STATE_SUSPENDED;
}

/**
* @fn void osResumeContextSwitching()
* @brief Funcion que resume el context switch
* @param  Ninguno
* @return Nada
*/
void osResumeContextSwitching()
{
    g_Os.state = OS_STATE_RUNNING;
}

/**
* @fn void osGetCurrentTask()
* @brief Funcion que devuelve la tarea en ejecucion
* @param  Ninguno
* @return Id de la tarea en ejecucion
*/
uint8_t osGetCurrentTask()
{
    return g_Os.currentTask;
}
/*==================[IRQ Handlers]======================================*/
/**
* @fn void SysTick_Handler( void )
* @brief  Manejador de interrupcion del Systick
* @param  Ninguno
* @return Nada 
*/
void SysTick_Handler( void )
{
    
    /* Incrementa el tick del SO */
    if(OS_RESULT_OK == osIncrementTick())
    {
        /* Si estamos usando delay, actualizamos los ticks de cada tarea bloqueada */
        #if ( OS_USE_TASK_DELAY == 1 )
            delayUpdate();
        #endif
        /* Llamamos al scheduler */
        schedule();
    }
    
}
/*==================[end of file]============================================*/

