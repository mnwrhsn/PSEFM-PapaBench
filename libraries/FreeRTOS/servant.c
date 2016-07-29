/*
    FreeRTOS V7.1.1 - Copyright (C) 2012 Real Time Engineers Ltd.
	

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    >>>NOTE<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.  FreeRTOS is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public
    License and the FreeRTOS license exception along with FreeRTOS; if not it
    can be viewed here: http://www.freertos.org/a00114.html and also obtained
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?                                      *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************


    http://www.FreeRTOS.org - Documentation, training, latest information,
    license and contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool.

    Real Time Engineers ltd license FreeRTOS to High Integrity Systems, who sell
    the code with commercial support, indemnification, and middleware, under
    the OpenRTOS brand: http://www.OpenRTOS.com.  High Integrity Systems also
    provide a safety engineered and independently SIL3 certified version under
    the SafeRTOS brand: http://www.SafeRTOS.com.
*/


#define USE_STDPERIPH_DRIVER
#include "stm32f10x.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "list.h"
#include "queue.h"
#include "semphr.h"
#include "eventlist.h"
#include "servant.h"
#include "app.h"

struct context xContexts[NUMBEROFSERVANT];

extern xSemaphoreHandle xBinarySemaphore[NUMBEROFTHREAD];  // the semaphores which are used to trigger new servant to execute

// record the relationship among servants excluding R-Servant
//extern portBASE_TYPE xRelation[NUMBEROFSERVANT][NUMBEROFSERVANT] ;
// the new xRelation table which is implemeted with sparse matrix
extern struct xRelationship xRelations; 

// the LET of all S-Servant
extern portTickType xLetOfServant[NUMBEROFSERVANT] ;
// In app.c, this is used to sepcify the function of Servant
extern pvServantFunType xServantTable[NUMBEROFSERVANT];
// record the task id where the servant is in
extern portBASE_TYPE xTaskOfServant[NUMBEROFSERVANT];
// record the period of Task
extern portTickType xPeriodOfTask[NUMBEROFTASK];

extern portBASE_TYPE xSensorOfTask[NUMBEROFTASK];

extern portBASE_TYPE xActuatorOfTask[NUMBEROFTASK];

/* create all semaphores which are used to triggered s-servant */
void vSemaphoreInitialise()
{
    portBASE_TYPE i;

    for( i = 0; i < NUMBEROFTHREAD; ++ i )
    {
        vSemaphoreCreateBinary(xBinarySemaphore[i]);
        /* when created, it is initialised to 1. So, we take it away.*/
        xSemaphoreTake(xBinarySemaphore[i], portMAX_DELAY);
    }
}

/*
 * Set all the parameters of S-Servants.
 *
 * */
void vContextInit()
{
    portBASE_TYPE i, j;
    portBASE_TYPE xSource, xDest;
       
    // initialise the member of pvParameters
    for( i = 0; i < NUMBEROFSERVANT; ++ i )
    {
        xContexts[i].xMyFlag = i;
        xContexts[i].xType = 2;
        xContexts[i].xCount = 1; // the first period of task is used to init system
        xContexts[i].xNumOfIn = 0;
        xContexts[i].xNumOfOut = 0;
        xContexts[i].xLet = xLetOfServant[i];
        xContexts[i].xFp = xServantTable[i];
        xContexts[i].xTaskId = xTaskOfServant[i];
        xContexts[i].xPeriod = xPeriodOfTask[xTaskOfServant[i]];
        xContexts[i].xInBoolCount = 0;
    }

    // setup the type of servants who are initialised to 2
    for( i = 0; i < NUMBEROFTASK; ++ i )
    {
        xContexts[xSensorOfTask[i]].xType = 1;
        xContexts[xActuatorOfTask[i]].xType = 3;
    }

    // new edition with sparse matrix relation table
    for( i = 0; i < xRelations.xNumOfRelation; ++ i )
    {
        xSource = xRelations.xRelation[i].xInFlag;
        xDest   = xRelations.xRelation[i].xOutFlag;

        xContexts[xSource].xOutFlag[xContexts[xSource].xNumOfOut] = xDest;
        xContexts[xSource].xNumOfOut ++;

        xContexts[xDest].xInFlag[xContexts[xDest].xNumOfIn] = xSource;
        xContexts[xDest].xNumOfIn ++;
    }
}

void vSensor( void * pvParameter )
{
    xEventHandle pxEvent;
    portBASE_TYPE xMyFlag;
    portTickType xDeadline;
    portTickType xTimestamp;
    struct eventData * xMyData;
    struct tag * xMyTag;
    portBASE_TYPE boolFlag;

    while(1)
    {
        xSemaphoreTake( xBinarySemaphore[1], portMAX_DELAY );

        while((boolFlag = xIsERLNull()) != -1)
        {
            if(boolFlag == 0)
                continue;
            pxEvent = pxEventReceive();   // receive event from ready list and send to event pool straightly
            xMyFlag = xEventGetpxDestination( pxEvent );
            xMyData = xEventGetxData( pxEvent );
            xMyTag = xEventGetxTag( pxEvent );
            xContexts[xMyFlag].xCount ++;
            xDeadline= xEventGetxDeadline( pxEvent ); 
            xTimestamp = xMyTag->xTimestamp + INPUT;   
            xFutureModelTime = xTimestamp;  // init the future model time to the start of LET execution duration.
            vPrintNumber(xMyFlag);
            vPrintNumber(xTaskGetTickCount());
        
            xContexts[xMyFlag].xFp( xMyData );  // get the loop data and sensor data
            vEventUpdate( pxEvent, xMyFlag, xDeadline, xTimestamp, xMyData );  // reuse event
        }
        xSemaphoreGive( xBinarySemaphore[0] );
    }
}

void vServant( void * pvParameter )
{
    xEventHandle pxEvent;
    portBASE_TYPE xMyFlag;
    portBASE_TYPE pxDestination;
    portTickType xDeadline;
    portTickType xTimestamp;
    struct eventData * xMyData;
    struct tag * xMyTag;
    portBASE_TYPE boolFlag;
    while(1)
    {
        xSemaphoreTake( xBinarySemaphore[2], portMAX_DELAY );

        while((boolFlag = xIsERLNull()) != -1)
        {
            if(boolFlag == 0)
                continue;
            pxEvent = pxEventReceive();   // receive event from ready list and send to event pool straightly
            xMyFlag = xEventGetpxDestination( pxEvent );
            xMyData = xEventGetxData( pxEvent );
            xMyTag = xEventGetxTag( pxEvent );
            xContexts[xMyFlag].xCount ++;
            xDeadline = xEventGetxDeadline( pxEvent ); 
            // set the timestamp of event in terms of destination servant
            // if destination is actuator, then set as the EndOfLET
            // else set as the sum of input event's timestamp and let
            pxDestination = xContexts[xMyFlag].xOutFlag[0];
            switch(xContexts[pxDestination].xType)
            {
                case 2:
                    xTimestamp = xMyTag->xTimestamp + xContexts[xMyFlag].xLet;
                    break;
                case 3:
                    // the output execution time start from 3ms before end of the task period
                    xTimestamp = xDeadline - OUTPUT; 
                    break;
                default:
                    break;
                    // wrong events type
            }
            vPrintNumber(xMyFlag);
            vPrintNumber(xTaskGetTickCount());
        
            xContexts[xMyFlag].xFp( xMyData );  // get the loop data and sensor data
            vEventUpdate( pxEvent, xMyFlag, xDeadline, xTimestamp, xMyData );
        }
        xSemaphoreGive( xBinarySemaphore[0] );
    }

}

void vActuator( void * pvParameter )
{
    xEventHandle pxEvent;
    portBASE_TYPE xMyFlag;
    portTickType xDeadline;
    portTickType xTimestamp;
    struct eventData * xMyData;
    struct tag * xMyTag;
    portBASE_TYPE boolFlag;

    while(1)
    {
        xSemaphoreTake( xBinarySemaphore[3], portMAX_DELAY );

        while((boolFlag = xIsERLNull()) != -1)
        {
            if(boolFlag == 0)
                continue;

            pxEvent = pxEventReceive();   // receive event from ready list and send to event pool straightly
            xMyFlag = xEventGetpxDestination( pxEvent );
            xMyData = xEventGetxData( pxEvent );
            xMyTag = xEventGetxTag( pxEvent );
            xContexts[xMyFlag].xCount ++;
            xDeadline = xEventGetxDeadline( pxEvent ) + xContexts[xMyFlag].xPeriod; 
            xTimestamp = xMyTag->xTimestamp + OUTPUT;  // all sensor are scheduled to execute start from 0 to 4 ms of every period
            vPrintNumber(xMyFlag);
            vPrintNumber(xTaskGetTickCount());
        
            xContexts[xMyFlag].xFp( xMyData );  // get the loop data and sensor data
            vEventUpdate( pxEvent, xMyFlag, xDeadline, xTimestamp, xMyData ); // update the information of output event 
        }
        xSemaphoreGive( xBinarySemaphore[0] );
    }
}


void vR_Servant( void * pvParameter)
{
    portBASE_TYPE pxDestination;
    while(1)
    {
        // waiting for events created by tick hook or S-Servant
        xSemaphoreTake( xBinarySemaphore[0], portMAX_DELAY );
        vPrintString("R-Servant\n\r");

        // transit the events from events pool to nonexecutable event list
        // Copy one event to multiple when communication mode is 1 to N
        vEventScatter();

        // reduce multiple event for destination to one when communication mode is N to 1,
        // and transit the event from executable event pool to executable event list
        vEventReduce(); 

        // serialize the timestamp of simulataneous events,
        // and transit the ready events to ready list
        // return the pxDestination of the first ready event in ready list
        pxDestination = xEventSerialize(); 
        if(pxDestination != -1)
        {
            switch(xContexts[pxDestination].xType)
            {
                case 1:
                    xSemaphoreGive( xBinarySemaphore[1] );
                    break;
                case 2:
                    xSemaphoreGive( xBinarySemaphore[2] );
                    break;
                case 3:
                    xSemaphoreGive( xBinarySemaphore[3] );
                    break;
                default:
                    break;
                    // no event available 
            }
        }
    }
}
