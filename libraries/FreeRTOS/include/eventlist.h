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
	
	http://www.FreeRTOS.org/plus - Selection of FreeRTOS ecosystem products,
	including FreeRTOS+Trace - an indispensable productivity tool.

	Real Time Engineers ltd license FreeRTOS to High Integrity Systems, who sell
	the code with commercial support, indemnification, and middleware, under
	the OpenRTOS brand:  http://www.OpenRTOS.com.  High Integrity Systems also
	provide a safety engineered and independently SIL3 certified version under
	the	SafeRTOS brand: http://www.SafeRTOS.com.
*/

#ifndef EVENTLIST_H
#define EVENTLIST_H

#include "portable.h"
#include "list.h"
#include "FreeRTOSConfig.h"
#include "app_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------
 * MACROS AND DEFINITIONS
 *----------------------------------------------------------*/

#define tskKERNEL_VERSION_NUMBER "V7.1.1"

 /**************all the following structures and functions are implemented in eventlist.c to operating the event item*************/

/*
 * time stamp is used to mark the time to process corresponding event item. This can be used to reassigned the scheduling order of S-Servant
 * */
struct tag 
{
    portTickType xDeadline;     /*< RM. The smaller the period the task is , the higher priority the event is. >*/ 
    portTickType xTimestamp;             /*< the time to be proecessed >*/
    portBASE_TYPE xLevel;                /*< the depth of current servant in a task >*/ 
    portBASE_TYPE xMicroStep;            /*< the topology order >*/ 
};

struct eventData
{
    /* data type can be changed here. Data type including portCHAR, portFLOAT, portLONG, portSHORT, portBASE_TYPE*/
    portBASE_TYPE xDataArray[MAXINDEGREE];
};

typedef void * xEventHandle;

void vInitialiseEventLists();

portBASE_TYPE xIsERLNull();

portBASE_TYPE xIsExecutableEventArrive();

portTickType xEventGetxDeadline( xEventHandle pxEvent );

portBASE_TYPE xEventGetpxSource( xEventHandle pxEvent );

portBASE_TYPE  xEventGetpxDestination( xEventHandle pxEvent );
/*
 * get the time stamp of specified pxEvent
 *
 * @param pxEvent is the target event item.
 * @return the timestamp of this event item as struct timeStamp.
 * */

struct tag * xEventGetxTag( xEventHandle pxEvent );

/*
 * get the data of specified event.
 *
 * @param pxEvent is the target event item
 * @return the data which is struct eventData
 * */
struct eventData * xEventGetxData( xEventHandle pxEvent );

/*
 * create a new event item.
 *
 * @param pxDestination is the destination where this event will be sent to./
 * @param pvData is the data that the event should bring along.
 * */
xEventHandle pxEventGenericCreate( portBASE_TYPE pxSource, portTickType xDeadline, portTickType xTimestamp, struct eventData *pvData);  


/*
 * servant receive event whose source is the pxSource and destination is current servant from specified xEventReadyList.
 * 
 * @param pxEvent will record the point of the event item that will be received.
 * @param pxSource is used to choose the right event item in xEventReadyList.
 * @param pxList is the target xEventReadyList.
 * */
xEventHandle pxEventGenericReceive(); 



/* 
 * after events are received, they will be destroyed by deleting.
 *
 * @param xEvent is the event item which will be destroyed and freed.
 * */
void vEventGenericDelete ( xEventHandle pxEvent);

void vEventGenericSend(xEventHandle pxEvent); //  insert event into nonexecutable pool

void vEventGenericScatter(); // once a event, map one event to multiple events

void vEventGenericReduce();  // reduce the mutiple event for the same destination to one event, transit them from
                             // executable event pool to executable event list

portBASE_TYPE xEventGenericSerialize(); // serialize the timestamp of simultaneous events
                                        // and update other events who are affected by serialisation

void vEventGenericUpdate( xEventHandle xEvent, portBASE_TYPE pxSource , portTickType xDeadline, portTickType xTimestamp, struct eventData * xData );


#define pxEventCreate( pxSource, xDeadline, xTimestamp, pvData)  pxEventGenericCreate(pxSource, xDeadline, xTimestamp, pvData)


#define pxEventReceive()  pxEventGenericReceive()

#define vEventSend(pxEvent) vEventGenericSend(pxEvent)

#define vEventScatter() vEventGenericScatter()

#define vEventReduce() vEventGenericReduce()

#define xEventSerialize() xEventGenericSerialize()

#define vEventDelete( pxEvent )     vEventGenericDelete( pxEvent )

#define vEventUpdate( xEvent, pxSource, xDeadline, xTimestamp, xData ) vEventGenericUpdate( xEvent, pxSource, xDeadline, xTimestamp, xData );


#ifdef __cplusplus
}
#endif

#endif /* EVENTLIST_H*/
