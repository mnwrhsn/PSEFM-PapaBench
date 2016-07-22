/*
* this application is used to try to exam the EventList mechanism.
*
* what this application can do is to print the "hello world! I am from China" in a periodic way.
* The task to print the words is finished by one task which is composed of five S-servant include S-1,
* S-2, S-3, S-4, S-5. They print the words in a specified order and in a collaborative way in a finite 
* time duration.
* */


#define USE_STDPERIPH_DRIVER
#include "stm32f10x.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "eventlist.h"
#include "servant.h"
#include "app.h"
#include "app_config.h"

static void setup_hardware( void );
xTaskHandle xTaskOfHandle[NUMBEROFTHREAD];         

extern xSemaphoreHandle xBinarySemaphore[NUMBEROFSERVANT];  // the network topology
xSemaphoreHandle xInterruptSemaphore;
extern portTickType xPeriodOfTask[NUMBEROFTASK];
extern portBASE_TYPE xSensorOfTask[NUMBEROFTASK];
extern portBASE_TYPE xActuatorOfTask[NUMBEROFTASK];

// init program to start all task when system init
void vStartTask()
{
    portBASE_TYPE i;
    xEventHandle pxEvent;
    struct eventData null_data;

    for( i = 0; i < NUMBEROFTASK; ++i )
    {
        pxEvent = pxEventCreate(xActuatorOfTask[i], xPeriodOfTask[i], xPeriodOfTask[i], &null_data);
        vEventSend( pxEvent );   
    }
}

void vEventInterrupt(void * pvParameters)
{
    vSemaphoreCreateBinary( xInterruptSemaphore );
    xSemaphoreTake(xInterruptSemaphore, portMAX_DELAY);
    while(1)
    {
        //xSemaphoreTake(xInterruptSemaphore, portMAX_DELAY);
        if(xIsExecutableEventArrive())
        {
            xSemaphoreGive( xBinarySemaphore[0] );
        }
        taskYIELD();
    }
}

#define SERVANT_STACK_SIZE 128 
int main(void)
{
    init_led();
    init_rs232();
    enable_rs232_interrupts();
    enable_rs232();
    vSemaphoreInitialise();
    vContextInit();
    vInitialiseEventLists();  
    vStartTask();
    PapabenchInit();
    portBASE_TYPE i,j;
    portBASE_TYPE flag = 0;

    xTaskCreate( vR_Servant, "R-Servant", SERVANT_STACK_SIZE, NULL,tskIDLE_PRIORITY + 2, &xTaskOfHandle[0]);
    xTaskCreate( vSensor, "I-Servant", SERVANT_STACK_SIZE, NULL,tskIDLE_PRIORITY + 2, &xTaskOfHandle[1]);
    xTaskCreate( vServant, "C-Servant", SERVANT_STACK_SIZE, NULL,tskIDLE_PRIORITY + 2, &xTaskOfHandle[2]);
    xTaskCreate( vActuator, "O-Servant", SERVANT_STACK_SIZE, NULL,tskIDLE_PRIORITY + 2, &xTaskOfHandle[3]);
    xTaskCreate( vEventInterrupt, "event", SERVANT_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL );

    xSemaphoreGive(xBinarySemaphore[0]);
    vTaskStartScheduler();

    return 0;
}

void myTraceCreate      (){
}

void myTraceSwitchedIn  (){
}

void myTraceSwitchedOut	(){
}

 /*
 * if there is any task need to be triggered at this time,
 * tick hook function would send semaphore to them.
 * */
void vApplicationTickHook( void )
{
    //xSemaphoreGive( xInterruptSemaphore );
}
