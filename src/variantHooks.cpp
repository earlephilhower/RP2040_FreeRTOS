/*
 * Copyright (C) 2021 Phillip Stevens  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * This file is NOT part of the FreeRTOS distribution.
 *
 */
#include <stdlib.h>

/* Arduino Core includes */
#include <Arduino.h>
#include <RP2040USB.h>

/* Raspberry PI Pico includes */
#include <pico.h>
#include <pico/time.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/*-----------------------------------------------------------*/

void initFreeRTOS(void)
{
}

void startFreeRTOS(void)
{
    // As the Task stacks are on heap before Task allocated heap variables,
    // --- NOT NEEDED WHEN USING heap_3.c ---
    // the library default __malloc_heap_end = 0 doesn't work.
    //__malloc_heap_end = (char *)(RAMEND - __malloc_margin);

	// Initialise and run the freeRTOS scheduler. Execution should never return here.
    vTaskStartScheduler();
}


/*-----------------------------------------------------------*/

void prvDisableInterrupts()
{
    portDISABLE_INTERRUPTS();
}

void prvEnableInterrupts()
{
    portENABLE_INTERRUPTS();
}

/*-----------------------------------------------------------*/
#if ( configUSE_IDLE_HOOK == 1 )
/*
 * Call the user defined loop() function from within the idle task.
 * This allows the application designer to add background functionality
 * without the overhead of a separate task.
 *
 * NOTE: vApplicationIdleHook() MUST NOT, UNDER ANY CIRCUMSTANCES, CALL A FUNCTION THAT MIGHT BLOCK.
 *
 */
void loop( void ) __attribute__((weak));
void loop() {} //Empty loop function

extern "C"
void vApplicationIdleHook( void ) __attribute__((weak));

// Idle functions (USB, events, ...) from the core
extern void __loop();

void vApplicationIdleHook( void )
{
	// the normal Arduino loop() function is run here.
	loop();

	// run idle functions from the core
	__loop();
}

#endif /* configUSE_IDLE_HOOK == 1 */
/*-----------------------------------------------------------*/

#if ( configUSE_MINIMAL_IDLE_HOOK == 1 )
/*
 * Call the user defined minimalIdle() function from within the idle task.
 * This allows the application designer to add background functionality
 * without the overhead of a separate task.
 *
 * NOTE: vApplicationMinimalIdleHook() MUST NOT, UNDER ANY CIRCUMSTANCES, CALL A FUNCTION THAT MIGHT BLOCK.
 *
 */
void minimalIdle( void ) __attribute__((weak));
void minimalIdle() {} //Empty minimalIdle function

extern "C"
void vApplicationMinimalIdleHook( void ) __attribute__((weak));

void vApplicationMinimalIdleHook( void )
{
	minimalIdle();
}

#endif /* configUSE_MINIMAL_IDLE_HOOK == 1 */
/*-----------------------------------------------------------*/

#if ( configUSE_TICK_HOOK == 1 )
/*
 * Call the user defined minimalIdle() function from within the idle task.
 * This allows the application designer to add background functionality
 * without the overhead of a separate task.
 *
 * NOTE: vApplicationMinimalIdleHook() MUST NOT, UNDER ANY CIRCUMSTANCES, CALL A FUNCTION THAT MIGHT BLOCK.
 *
 */
void tick( void ) __attribute__((weak));
void tick() {} //Empty minimalIdle function

extern "C"
void vApplicationTickHook( void ) __attribute__((weak));

void vApplicationTickHook( void )
{
	tick();
}

#endif /* configUSE_TICK_HOOK == 1 */
/*-----------------------------------------------------------*/

#if ( configUSE_MALLOC_FAILED_HOOK == 1 || configCHECK_FOR_STACK_OVERFLOW >= 1 || configDEFAULT_ASSERT == 1 )

/**
 * Private function to enable board led to use it in application hooks
 */
void prvSetMainLedOn( void ) 
{
	gpio_init(LED_BUILTIN);
	gpio_set_dir(LED_BUILTIN, true);
	gpio_put(LED_BUILTIN, true);
}

/**
 * Private function to blink board led to use it in application hooks
 */
void prvBlinkMainLed( void ) 
{
	gpio_put(LED_BUILTIN, !gpio_get(LED_BUILTIN));
}

#endif

/*---------------------------------------------------------------------------*\
Usage:
	called on fatal error (interrupts disabled already)
\*---------------------------------------------------------------------------*/
extern "C"
void rtosFatalError(void)
{
    prvSetMainLedOn(); // Main LED on.
    
    for(;;)
    {
    	// Main LED slow flash
        sleep_ms(100);
        prvBlinkMainLed();
        sleep_ms(2000);
        prvBlinkMainLed();
    }
}

#if ( configUSE_MALLOC_FAILED_HOOK == 1 )
/*---------------------------------------------------------------------------*\
Usage:
    called by task system when a malloc failure is noticed
Description:
    Malloc failure handler -- Shut down all interrupts, send serious complaint
    to command port. FAST Blink on main LED.
Arguments:
    pxTask - pointer to task handle
    pcTaskName - pointer to task name
Results:
    <none>
Notes:
    This routine will never return.
    This routine is referenced in the task.c file of FreeRTOS as an extern.
\*---------------------------------------------------------------------------*/
extern "C"
void vApplicationMallocFailedHook( void ) __attribute__((weak));

void vApplicationMallocFailedHook( void )
{
    prvSetMainLedOn(); // Main LED on.
    
    for(;;)
    {
        sleep_ms(50);
        prvBlinkMainLed(); // Main LED fast blink.
    }
}

#endif /* configUSE_MALLOC_FAILED_HOOK == 1 */
/*-----------------------------------------------------------*/


#if ( configCHECK_FOR_STACK_OVERFLOW >= 1 )

extern "C"
void vApplicationStackOverflowHook( TaskHandle_t xTask,
                                    char * pcTaskName ) __attribute__((weak));

void vApplicationStackOverflowHook( TaskHandle_t xTask __attribute__((unused)),
                                    char * pcTaskName __attribute__((unused)) )
{
    prvSetMainLedOn(); // Main LED on.

    for(;;)
    {
        sleep_ms(2000);
        prvBlinkMainLed();  // Main LED slow blink.
    }
}

#endif /* configCHECK_FOR_STACK_OVERFLOW >= 1 */
/*-----------------------------------------------------------*/

#if ( configSUPPORT_STATIC_ALLOCATION >= 1 )

extern "C"
void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    configSTACK_DEPTH_TYPE * pulIdleTaskStackSize ) __attribute__((weak));

void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    configSTACK_DEPTH_TYPE * pulIdleTaskStackSize )
{
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

#if ( configUSE_TIMERS >= 1 )

extern "C"
void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     configSTACK_DEPTH_TYPE * pulTimerTaskStackSize ) __attribute__((weak));

void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     configSTACK_DEPTH_TYPE * pulTimerTaskStackSize )
{
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

#endif /* configUSE_TIMERS >= 1 */

#endif /* configSUPPORT_STATIC_ALLOCATION >= 1 */

/**
 * configASSERT default implementation
 */
#if configDEFAULT_ASSERT == 1

extern "C"
void vApplicationAssertHook() {

    taskDISABLE_INTERRUPTS(); // Disable task interrupts

    prvSetMainLedOn(); // Main LED on.
    for(;;)
    {
        sleep_ms(100);
        prvBlinkMainLed(); // Led off.

        sleep_ms(2000);
        prvBlinkMainLed(); // Led on.

        sleep_ms(100);
        prvBlinkMainLed(); // Led off

        sleep_ms(100);
        prvBlinkMainLed(); // Led on.
    }
}
#endif
