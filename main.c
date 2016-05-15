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

static portBASE_TYPE IS_INIT[NUMBEROFTASK];
static void setup_hardware( void );

extern struct xParam pvParameters[NUMBEROFSERVANT];

extern xSemaphoreHandle xBinarySemaphore[NUMBEROFSERVANT];  // the network topology
extern xTaskHandle xTaskOfHandle[NUMBEROFSERVANT];         // record the handle of all S-Servant, the last one is for debugging R-Servant

extern portTickType xPeriodOfTask[NUMBEROFTASK];
extern portBASE_TYPE xSensorOfTask[NUMBEROFTASK];

extern xTaskComplete[NUMBEROFTASK];


void vInitInitialise()
{
    portBASE_TYPE i;
    for(i = 0; i < NUMBEROFTASK; ++ i)
    {
        IS_INIT[i] = 0;
    }
}

#define SERVANT_STACK_SIZE 128 
int main(void)
{
    init_led();
    init_rs232();
    enable_rs232_interrupts();
    enable_rs232();

    //vTaskCompleteInitialise();
    vInitInitialise();
    vSemaphoreInitialise();
    vParameterInitialise();
    portBASE_TYPE i,j;
    portBASE_TYPE flag = 0;

    xTaskCreate( vR_Servant, "R-Servant", SERVANT_STACK_SIZE, (void *)&pvParameters[NUMBEROFSERVANT-1],tskIDLE_PRIORITY + 1, &xTaskOfHandle[NUMBEROFSERVANT-1]);

    for( i = 0; i < NUMBEROFSERVANT-1; ++ i )
    {
        for(j = 0; j < NUMBEROFTASK; ++ j)
        {
            if( i == xSensorOfTask[j] )
            {
                flag = 1;  // sensor 
                break;
            }
        }
        // create sensor 
        if(flag == 1)
        {
            flag = 0; 
            xTaskCreate( vSensor, "sensor", SERVANT_STACK_SIZE, (void *)&pvParameters[i], tskIDLE_PRIORITY + 2, &xTaskOfHandle[i] );
        }
        else  // create servant
        {
            xTaskCreate( vServant, "servant", SERVANT_STACK_SIZE, (void *)&pvParameters[i], tskIDLE_PRIORITY + 2, &xTaskOfHandle[i] );
        }
    }
    /* Start running the task. */
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
inline float myTraceGetTick(){
	// 0xE000E014 -> Systick reload value
	// 0xE000E018 -> Systick current value
	return ((float)((*(unsigned long *)0xE000E014)-(*(unsigned long *)0xE000E018)))/(*(unsigned long *)0xE000E014);
}

inline unsigned long myTraceGetTimeMillisecond(){
	return (xTaskGetTickCountFromISR() + myTraceGetTick()) * 1000 / configTICK_RATE_HZ;
}
*/

/* time tick hook which is used to triggered every sensor of corresponding task to execute at
 * specified time according to their period.
 *
 * if there is any task need to be triggered at this time,
 * tick hook function would send semaphore to them.
 * */
void vApplicationTickHook( void )
{
    portTickType xCurrentTime = xTaskGetTickCount();
    portBASE_TYPE i;
    /* init task */
    if(xCurrentTime > 0 && xCurrentTime < 2501)
    {
        for(i = 0; i < NUMBEROFTASK; ++i)
        {
            if( IS_INIT[i] == 0 && xCurrentTime % xPeriodOfTask[i] == 0 )
            {
                IS_INIT[i] = 1;
                xSemaphoreGive( xBinarySemaphore[xSensorOfTask[i]] );
            }
        }

    }
    
    // R-Servant will be suspend when no events need to be proceeded.
    // As a result, we need to send semaphore to R-Servant to triggered it processing events 
    // when time meeting the start time of every task period
    if( xCurrentTime >= xPeriodOfTask[0] * 2 )
    {
        if( xCurrentTime % xPeriodOfTask[0] == 0 || 
            xCurrentTime % xPeriodOfTask[2] == 0 ||
            xCurrentTime % xPeriodOfTask[8] == 0 ||
            xCurrentTime % xPeriodOfTask[12] == 0)
        {
           xSemaphoreGive( xBinarySemaphore[NUMBEROFSERVANT - 1] ); 
        }
    }
}
