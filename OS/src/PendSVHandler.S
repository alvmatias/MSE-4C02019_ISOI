
    /**
     * Directiva al ensablador que permite indicar que se encarga de buscar
     * la instruccion mas apropiada entre thumb y thumb2
     */
    .syntax unified

    /**
     * .text permite indicar una seccion de codigo.
     */
    .text

    /**
     * .global permite definir un simbolo exportable,
     * es decir que podemos verlo desde otros modulos (equivalente a extern).
     * Definimos la rutina como global para que sea visible desde otros modulos.
     */
    .global PendSV_Handler


    /**
     * Indicamos que la siguiente subrutina debe ser ensamblada en modo thumb,
     * entonces en las direcciones en el ultimo bit tendran el 1 para que se reconozcan como en modo thumb.
     * Siempre hay que ponerla antes de la primer instruccion.
     */
    .thumb_func

PendSV_Handler:

    cpsid i             /* Deshabilitamos interrupciones */

    tst lr, 0x10        /* Comparamos lr(EXC_RETURN) y 0x10 */

    it eq               /* Si el resultado fue 0 */ /* Ejecucion condicional */

    vpusheq {s16-s31}    /* Pusheo s16-s31 al stack, y fuerzo el stacking de s0-s15 y fpscr */

    push {r4-r11,lr}    /* Push de contexto */

    mrs r0, msp         /* Cargo en r0 el Main Stack Pointer */

    bl taskSchedule     /* Salto a la funcion schedule pasando como parametro el MSP(en r0) */

    msr msp, r0         /* Cargo en el MSP el registro r0(valor de retorno de la funcion de scheduling) */

    pop {r4-r11,lr}     /* Pop de contexto */

    tst lr, 0x10        /* Comparamos lr(EXC_RETURN) y 0x10 */
    
    it eq               /* Si el resultado fue 0 */ /* Ejecucion condicional */
    
    vpopeq {s16-s31}    /* Pusheo s16-s31 al stack, y fuerzo el stacking de s0-s15 y fpscr */
 
    cpsie i             /* Habilitamos interrupciones */
    
    bx lr               /* Retorno de interrupcion */