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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "StackMacros.h"
#include "eventlist.h"
#include "servant.h"

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

extern struct context xContexts[NUMBEROFSERVANT];
extern portTickType xPeriodOfTask[NUMBEROFTASK];

portTickType GCDPeriod;  // the GCD of period of all tasks
portTickType xFutureModelTime ;

/* the struct of event item*/
typedef struct eveEventControlBlock
{
    portBASE_TYPE pxSource;         /*< the S-Servant where the event item from >*/
    portBASE_TYPE pxDestination;    /*< the S-Servant where the event item to >*/
    struct tag xTag;    /*< the time stamp used to sort the event item in the event list >*/
    struct eventData xData;
    xListItem  xEventListItem;       /*< connect the eveECB to the xEventList by struct list>*/
}eveECB;


/* Event lists must be initialised before the first time to create an event. */
PRIVILEGED_DATA static xList xEventIdleList;  /*< Idel events are created when system initialising and stored in this list. Any one that needs idle event can get event from this event and return it back with FIFO policy>*/
PRIVILEGED_DATA static xList xEventNonExecutablePool;  /*< Event Pool which store the events from S-Servant in terms of FIFO>*/
PRIVILEGED_DATA static  xList xEventNonExecutableList;                            /*< Event List is used to store the event item in a specific order which sended or received by S-Servant.>*/
PRIVILEGED_DATA static xList xEventExecutablePool;
PRIVILEGED_DATA static xList xEventExecutableList;   /*< store the executable event which satisfies the time requirement >*/
PRIVILEGED_DATA static xList xEventReadyList;
static volatile unsigned portBASE_TYPE xEventSerialNumber  = (portBASE_TYPE)0;       /* used to set the level of timestamp in event */

/* insert new event item into xEventNonExecutableList or xEventExecutableList. */
static void prvEventListGenericInsert( xListItem * pxNewListItem, portBASE_TYPE comp) PRIVILEGED_FUNCTION;

// <unexecutable, TimeStamp, deadline, level>
static portBASE_TYPE xCompareFunction1( const struct tag t1, const struct tag t2 );
// <executable, deadline, timestamp, level>
static portBASE_TYPE xCompareFunction2( const struct tag t1, const struct tag t2 );

// init event item list 
static void vListIntialiseEventItem( xEventHandle pvOwner, xListItem * pxNewEventItem);

static void vEventSetxTag(portTickType xDeadline, portTickType xTimestamp, xEventHandle pxNewEvent );

// if xEventReadyList is not null,
// then return the type of destination servant of the first event
portBASE_TYPE xIsERLNull()
{
    portBASE_TYPE pxDestination;
    struct tag xTag;
    if(listCURRENT_LIST_LENGTH(&xEventReadyList) > 0)
    {
        pxDestination = xEventGetpxDestination((xEventReadyList.xListEnd.pxNext)->pvOwner); 
        xTag = xEventGetxTag(xEventReadyList.xListEnd.pxNext->pvOwner);
        if( xTaskGetTickCount() >= xTag.xTimestamp )
            return xContexts[pxDestination].xType; 
        else
            return 0;
    }
    return -1;
}

portBASE_TYPE xEventGetpxSource( xEventHandle pxEvent )
{
    return ((eveECB *)pxEvent)->pxSource;
}

portBASE_TYPE xEventGetpxDestination( xEventHandle pxEvent)
{
    return ((eveECB *) pxEvent)->pxDestination;
}

struct tag xEventGetxTag( xEventHandle pxEvent)
{
    return ((eveECB *) pxEvent)->xTag;
}

struct eventData xEventGetxData( xEventHandle pxEvent)
{
    return ((eveECB *) pxEvent)->xData;
}

static portBASE_TYPE getGCD(portTickType t1, portTickType t2)
{
    return t1%t2==0 ? t2 : getGCD(t2, t1 % t2);
}

inline static portTickType GCDOfTaskPeriod()
{
    portBASE_TYPE i;
    portTickType result = xPeriodOfTask[0];

    for( i = 1; i < NUMBEROFTASK; ++ i )
    {
        result = getGCD(result , xPeriodOfTask[i]); 
    }
    return result;
}

void vInitialiseEventLists() 
{
    volatile portBASE_TYPE xCPU, i;
    eveECB * pxIdleEvents[NUMBEROFEVENTS];
    eveECB * pxEndFlagEvent;

    // init the GCD of Task period;
    GCDPeriod = GCDOfTaskPeriod();

    vListInitialise( ( xList * ) &xEventIdleList);
    vListInitialise( ( xList * ) &xEventNonExecutablePool );
    vListInitialise( ( xList * ) &xEventNonExecutableList);
    vListInitialise( ( xList * ) &xEventExecutablePool );
    vListInitialise( ( xList * ) &xEventExecutableList );
    vListInitialise( (xList * ) & xEventReadyList);

    for( i = 0; i < NUMBEROFEVENTS; ++i )
    {
        pxIdleEvents[i] = (eveECB *) pvPortMalloc(sizeof(eveECB)); 
        vListIntialiseEventItem( pxIdleEvents[i], (xListItem *) & pxIdleEvents[i]->xEventListItem );
        vListInsertEnd(&xEventIdleList, &pxIdleEvents[i]->xEventListItem); 
    }

    // Creating an End FLag Event and insert into the end of xEventList, which needs sorted events
    pxEndFlagEvent = (eveECB *) pvPortMalloc( sizeof( eveECB ) );
    if( pxEndFlagEvent != NULL )
    {
       // pxEndFlagEvent->pxSource = pxEndFlagEvent->pxDestination = NULL;
        // there may be some problem here because of this assignment way
        pxEndFlagEvent->xTag.xDeadline= portMAX_DELAY;
        pxEndFlagEvent->xTag.xTimestamp = portMAX_DELAY;
        pxEndFlagEvent->xTag.xMicroStep = portMAX_DELAY;
        pxEndFlagEvent->xTag.xLevel = portMAX_DELAY;
        vListIntialiseEventItem( pxEndFlagEvent, (xListItem *) & pxEndFlagEvent->xEventListItem );
        vListInsertEnd(&xEventNonExecutableList, &pxEndFlagEvent->xEventListItem); 
    }

    // Creating an End FLag Event and insert into the end of xEventExecutableList, which needs sorted events
    pxEndFlagEvent = (eveECB *) pvPortMalloc( sizeof( eveECB ) );
    if( pxEndFlagEvent != NULL )
    {
        // there may be some problem here because of this assignment way
        pxEndFlagEvent->xTag.xDeadline= portMAX_DELAY;
        pxEndFlagEvent->xTag.xTimestamp = portMAX_DELAY;
        pxEndFlagEvent->xTag.xMicroStep = portMAX_DELAY;
        pxEndFlagEvent->xTag.xLevel = portMAX_DELAY;
        vListIntialiseEventItem( pxEndFlagEvent, (xListItem *) & pxEndFlagEvent->xEventListItem );
        vListInsertEnd(&xEventExecutableList, &pxEndFlagEvent->xEventListItem); 
    }
}

portBASE_TYPE xIsExecutableEventArrive()
{
    xListItem * temp_pxEventListItem;
    portTickType xCurrentTime;
    struct tag xTag;
    
    if(listCURRENT_LIST_LENGTH(&xEventNonExecutableList) > 1)
    {
        temp_pxEventListItem = (xListItem *)xEventNonExecutableList.xListEnd.pxNext;
        xTag= xEventGetxTag( (xEventHandle) (temp_pxEventListItem)->pvOwner );
        xCurrentTime = xTaskGetTickCount();
        return xTag.xTimestamp <= xCurrentTime? pdTRUE : pdFALSE;
    }
    return 0;
}

/* unexecutable event comparison function is used in xEventList. 
 * The event with earlist timestamp will be proceeded first*/
static portBASE_TYPE xCompareFunction1( const struct tag t1, const struct tag t2 )
{
    if( t1.xTimestamp < t2.xTimestamp)
    {
        return pdTRUE;
    }
    else if( t1.xTimestamp == t2.xTimestamp)
    {
        if( t1.xDeadline < t2.xDeadline )
        {
            return pdTRUE;
        }
        else if( t1.xDeadline == t2.xDeadline )
        {
            if( t1.xLevel < t2.xLevel )
            {
                return pdTRUE;
            }
            else if( t1.xLevel == t2.xLevel && t1.xMicroStep < t2.xMicroStep )
            {
                return pdTRUE;
            }
        }
    }
    return pdFALSE;
}

/* executable event comparison function is used in xEventExecutableList. 
 * The event with earlist deadline will be scheduled to execute first */
static portBASE_TYPE xCompareFunction2( const struct tag t1, const struct tag t2 )
{
    if( t1.xDeadline < t2.xDeadline)
    {
        return pdTRUE;
    }
    else if( t1.xDeadline == t2.xDeadline)
    {
        if( t1.xTimestamp < t2.xTimestamp)
        {
            return pdTRUE;
        }
        else if( t1.xTimestamp == t2.xTimestamp )
        {
            if( t1.xLevel < t2.xLevel )
            {
                return pdTRUE;
            }
            else if( t1.xLevel == t2.xLevel && t1.xMicroStep < t2.xMicroStep )
            {
                return pdTRUE;
            }
        }
    }

    return pdFALSE;
}

static void vEventSetxTag( portTickType xDeadline, portTickType xTimestamp, xEventHandle pxNewEvent )
{
    eveECB * pxEvent =(eveECB *) pxNewEvent;

    /* EDF scheduling algorithm */
    pxEvent->xTag.xDeadline = xDeadline ;
    pxEvent->xTag.xTimestamp = xTimestamp;

    /*the microstep is not used now*/
    pxEvent->xTag.xMicroStep = 0;

    /* set the level of timestamp according to the topology sort result*/
    pxEvent->xTag.xLevel = xEventSerialNumber;

    xEventSerialNumber++;
}

/* insert event to xEventNonExecutableList in terms of comparison function 1 */
static void prvEventListGenericInsert( xListItem *pxNewListItem, portBASE_TYPE comp)
{
    volatile xListItem *pxIterator;
    struct tag xTagOfInsertion;
    xList * pxList; 

    xTagOfInsertion = xEventGetxTag(pxNewListItem->pvOwner);

    if(comp == 1)
    {
        pxList = &xEventNonExecutableList;
        taskENTER_CRITICAL();
        for( pxIterator = ( xListItem * ) &( pxList->xListEnd); 
             xCompareFunction1( xEventGetxTag( pxIterator->pxNext->pvOwner ), xTagOfInsertion ); 
             pxIterator = pxIterator->pxNext ) {}
        taskEXIT_CRITICAL();
    }
    else 
    {
        pxList = &xEventExecutableList;
        taskENTER_CRITICAL();
        for( pxIterator = ( xListItem * ) &( pxList->xListEnd); 
             xCompareFunction2( xEventGetxTag( pxIterator->pxNext->pvOwner ), xTagOfInsertion ); 
             pxIterator = pxIterator->pxNext ) {}
        taskEXIT_CRITICAL();
    }

    // insert the new event before a bigger one.
    pxNewListItem->pxNext = pxIterator->pxNext;
    pxNewListItem->pxNext->pxPrevious = ( volatile xListItem * ) pxNewListItem;
    pxNewListItem->pxPrevious = pxIterator;
    pxIterator->pxNext = ( volatile xListItem * ) pxNewListItem;

    pxNewListItem->pvContainer = ( void * ) pxList;

    ( pxList->uxNumberOfItems )++;
}


static void vListIntialiseEventItem( xEventHandle pvOwner, xListItem * pxNewEventItem)
{
    /* set the pvOwner of the EventItem as a event*/
    listSET_LIST_ITEM_OWNER( pxNewEventItem, pvOwner );
}


xEventHandle pxEventGenericCreate( portBASE_TYPE pxSource, portTickType xDeadline, portTickType xTimestamp, struct eventData pdData)
{
    eveECB * pxNewEvent = NULL;

    /* using the pxCurrentTCB, current task should not be changed */
    taskENTER_CRITICAL();

    pxNewEvent = (eveECB *)xEventIdleList.xListEnd.pxNext->pvOwner;
    vListRemove( (xListItem *)&pxNewEvent->xEventListItem );

    pxNewEvent->pxSource = pxSource;
    vEventSetxTag( xDeadline, xTimestamp, pxNewEvent );
    ((eveECB *) pxNewEvent)->xData = pdData;
    vListIntialiseEventItem( pxNewEvent, (xListItem *) &pxNewEvent->xEventListItem );

    taskEXIT_CRITICAL();

    return pxNewEvent;
}


// An API to transfer all executable Event Items from xEventNonExecutableList to xEventExecutablePool.
// function : transit the nonexecutable event to executable event, and update the inBoolCount of every event 
void vEventListGenericTransit() 
{
    xListItem * temp_pxEventListItem;
    struct tag xTag;
    portBASE_TYPE pxDestination;
    portTickType xCurrentTime;

    // transmit the executable event from xEventNonExecutableList to xEventExecutablePool 
    while( listCURRENT_LIST_LENGTH( &xEventNonExecutableList) > 1 )
    {
        temp_pxEventListItem = (xListItem *)xEventNonExecutableList.xListEnd.pxNext;
        xTag= xEventGetxTag( temp_pxEventListItem->pvOwner );
        xCurrentTime = xTaskGetTickCount();

        // the event is executable
        if( xTag.xTimestamp <= xCurrentTime )
        {
            // update the xInBoolCount of pxDestination
            pxDestination = ((eveECB *) temp_pxEventListItem->pvOwner)->pxDestination;
            xContexts[pxDestination].xInBoolCount ++ ;

            taskENTER_CRITICAL();
            /* remove pxListItem from xEventNonExecutableList */ 
            vListRemove(temp_pxEventListItem);
            /* insert the executable event into the xEventExecutablePool*/
            vListInsertEnd(&xEventExecutablePool, temp_pxEventListItem);
            taskEXIT_CRITICAL();
#ifdef PSEFM_DEBUG
            if( xContexts[pxDestination].xInBoolCount > xContexts[pxDestination].xNumOfIn )
            {
                vPrintString(" Events repeating !!!! errror in function vEventListGenericTransit\n\r");
            }
#endif
        }
        else
        {
          // no executable event
           break; 
        }
    }
}

void vEventGenericMap()
{
    portBASE_TYPE i;
    portBASE_TYPE pxSource, outs;
    eveECB * pxEvent; 
    eveECB * pxCopyEvent;
    xListItem * temp_pxEventListItem;

    // event map
    while(listCURRENT_LIST_LENGTH( &xEventNonExecutablePool ) != 0)
    {
        temp_pxEventListItem = (xListItem *) xEventNonExecutablePool.xListEnd.pxNext;    
        pxEvent = (eveECB *) temp_pxEventListItem->pvOwner;
        pxSource = pxEvent->pxSource;
        outs = xContexts[pxSource].xNumOfOut;

        taskENTER_CRITICAL();
        vListRemove(temp_pxEventListItem);
        // complete the information of the origin event.
        pxEvent->pxDestination = xContexts[pxSource].xOutFlag[0];
        prvEventListGenericInsert(temp_pxEventListItem,1); 

        // copy one event to multiples
        for(i = 1; i < outs; ++ i)
        {
            pxCopyEvent = (eveECB *)pxEventGenericCreate(pxSource, pxEvent->xTag.xDeadline, pxEvent->xTag.xTimestamp, pxEvent->xData);
            // complete the information of the copied event.
            pxCopyEvent->xTag.xMicroStep = i;
            pxCopyEvent->xTag.xLevel = pxEvent->xTag.xLevel;   // copy events have all the same xLevel
            pxCopyEvent->pxDestination = xContexts[pxSource].xOutFlag[i];
            prvEventListGenericInsert( &pxCopyEvent->xEventListItem , 1); 
        }
        taskEXIT_CRITICAL();
    }
}

void vEventGenericReduce()
{
    xListItem * reduce_pxEventListItem, * temp_pxEventListItem;
    portBASE_TYPE i, pxDestination;
    struct tag temp_tag;
    struct eventData temp_data;
    volatile xListItem * pxIterator1, *pxIterator2;
    // event reduce 
    if( listCURRENT_LIST_LENGTH(& xEventExecutablePool) > 1 )
    {
        // find at least all the ready event and move it to ready event list
        // ready event has the smallest timestamp and period.
        // if multi ready event exists, then they all have the same timestamp
        for( pxIterator1 = ( xListItem * ) ( xEventExecutablePool.xListEnd.pxNext ); 
                pxIterator1 != (xListItem *)&xEventExecutablePool.xListEnd; ) 
        {
            reduce_pxEventListItem = (xListItem *) pxIterator1; 
            pxDestination = xEventGetpxDestination(reduce_pxEventListItem->pvOwner); 

            // if the communication between servant is multiple to one,
            // then other events for the same servant need to be found
            if( xContexts[pxDestination].xInBoolCount == xContexts[pxDestination].xNumOfIn )
            {
                taskENTER_CRITICAL();
                if(xContexts[pxDestination].xInBoolCount > 1)
                {
                    i = 1;
                    for( pxIterator2 = pxIterator1->pxNext;  pxIterator2 != (xListItem *)&xEventExecutablePool.xListEnd; ) 
                    {
                        // find another event for the same destinate servant, copy the data to the first one
                        if( xEventGetpxDestination(pxIterator2->pvOwner) == pxDestination ) 
                        {
                            // collecting data into the first event
                            temp_data = xEventGetxData(pxIterator2->pvOwner); 
                            ((eveECB *)reduce_pxEventListItem->pvOwner)->xData.xDataArray[i++] = temp_data.xDataArray[0];
                            // set the timestamp to the biggest one
                            temp_tag = xEventGetxTag(pxIterator2->pvOwner);
                            if(((eveECB *)reduce_pxEventListItem->pvOwner)->xTag.xTimestamp < temp_tag.xTimestamp)
                            {
                                ((eveECB *)reduce_pxEventListItem->pvOwner)->xTag.xTimestamp = temp_tag.xTimestamp;
                            }
                            // delete the useless event, and return it back to event idle list
                            temp_pxEventListItem = (xListItem *) pxIterator2; 
                            pxIterator2 = pxIterator2->pxNext;
                            vEventGenericDelete(temp_pxEventListItem->pvOwner); 
                            if( i == xContexts[pxDestination].xInBoolCount )
                            {
                                break;   // find all event for the same servant 
                            }
                        }
                        else
                        {
                            pxIterator2 = pxIterator2->pxNext;
                        }
                    }
                }
                xContexts[pxDestination].xInBoolCount = 0; 
                pxIterator1 = pxIterator1->pxNext; 
                vListRemove(reduce_pxEventListItem);
                prvEventListGenericInsert(reduce_pxEventListItem, 2);
                taskEXIT_CRITICAL();
            }
            else
            {
                pxIterator1 = pxIterator1->pxNext;
            }

        }
    }
}



static portBASE_TYPE pOverLap( xListItem * pxEventListItem)
{
    eveECB * pxEvent = (eveECB *)pxEventListItem->pvOwner;
    portBASE_TYPE pxDestination = pxEvent->pxDestination; 
    portTickType start = pxEvent->xTag.xTimestamp;
    portTickType end   = start + xContexts[pxDestination].xLet;

    // different execution time overlaped
    if( start % GCDPeriod < INPUT || start % GCDPeriod > (GCDPeriod - OUTPUT) || 
            end % GCDPeriod < INPUT || end % GCDPeriod > (GCDPeriod - OUTPUT))
    {
        return 1;
    }
    return 0;
}

static void xSetTimestamp( portBASE_TYPE count)
{
    portBASE_TYPE i;
    eveECB *pxEvent;
    volatile xListItem * pxIterator;
    xListItem * temp_pxEventListItem;
    portBASE_TYPE pxDestination;
    
    for( i = 0, pxIterator = xEventReadyList.xListEnd.pxNext; 
         i < count; 
         ++ i, pxIterator = pxIterator->pxNext )
    {
        pxEvent = (eveECB *) pxIterator->pvOwner;
        pxEvent->xTag.xTimestamp = xFutureModelTime;
        if( pOverLap( (xListItem *)pxIterator ) == 0 ) // not overlaped
        {
            xFutureModelTime += xContexts[pxEvent->pxDestination].xLet; // update future model time
        }
        else  // overlaped
        {
            // set the future model time to start time of next LET
            xFutureModelTime = (xFutureModelTime/GCDPeriod) * GCDPeriod + GCDPeriod + INPUT;  
            pxEvent->xTag.xTimestamp = xFutureModelTime;
            xFutureModelTime += xContexts[pxEvent->pxDestination].xLet;

            temp_pxEventListItem = (xListItem *)pxIterator;
            pxIterator = pxIterator->pxNext;
            vListRemove( temp_pxEventListItem ); 
            vListInsertEnd(&xEventNonExecutablePool, temp_pxEventListItem);
            break;
        }
    }
    for( ; i < count; ++i)  // update the left simultaneous events and send them to event pool
    {
        pxEvent = (eveECB *) pxIterator->pvOwner;
        pxEvent->xTag.xTimestamp = xFutureModelTime;
        xFutureModelTime += xContexts[pxEvent->pxDestination].xLet;

        temp_pxEventListItem = (xListItem *)pxIterator;
        pxIterator = pxIterator->pxNext;
        vListRemove( temp_pxEventListItem ); 
        vListInsertEnd(&xEventNonExecutablePool, temp_pxEventListItem);
    }
}

inline static portBASE_TYPE pTagEqual(struct tag xTag1, struct tag xTag2)
{
    if(xTag1.xDeadline == xTag2.xDeadline && xTag1.xTimestamp == xTag2.xTimestamp)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// serialise the simultaneous events
portBASE_TYPE xEventGenericSerialize()
{
    xListItem * flag_pxEventListItem, * temp_pxEventListItem;
    portBASE_TYPE count = 0;

    if(listCURRENT_LIST_LENGTH( &xEventExecutableList ) > 1)
    {
        flag_pxEventListItem = (xListItem *) xEventExecutableList.xListEnd.pxNext;
        vListRemove( flag_pxEventListItem );
        vListInsertEnd( &xEventReadyList, flag_pxEventListItem );
        while( listCURRENT_LIST_LENGTH(&xEventExecutableList) > 1 )
        {
            temp_pxEventListItem = (xListItem *) xEventExecutableList.xListEnd.pxNext;
            // transit the simultaneous event to ready list
            if( pTagEqual( xEventGetxTag(flag_pxEventListItem->pvOwner), xEventGetxTag(temp_pxEventListItem->pvOwner)) == 1)
            {
                vListRemove( temp_pxEventListItem );
                vListInsertEnd( &xEventReadyList, temp_pxEventListItem );
                count ++;
            }
            else
            {
                break;
            }
        }
        // update timestamp of ready event in terms of future model time, 
        // which is only work for events that trigger C-Servant
        if( xContexts[xEventGetpxDestination(flag_pxEventListItem->pvOwner)].xType == 2 )
        {
            xSetTimestamp( count );
        }

        if( listCURRENT_LIST_LENGTH( &xEventReadyList ) > 0 )
        {
            return xEventGetpxDestination(flag_pxEventListItem->pvOwner); //  the first one repsenting the type of events
        }
    }
    return -1; // no event avaliable
}

void vEventGenericSend( xEventHandle pxEvent )
{
    vListInsertEnd(&xEventNonExecutablePool, (xListItem *)&((eveECB *)pxEvent)->xEventListItem);
}

xEventHandle pxEventGenericReceive()
{
    // xEventReadyList must not be null, which is ensured by servant

    taskENTER_CRITICAL();
    xListItem * pxFlag = (xListItem *)xEventReadyList.xListEnd.pxNext;
    vListRemove(pxFlag);
    vListInsertEnd(&xEventNonExecutablePool, pxFlag);  // reuse event, which will be update by servant 
    taskEXIT_CRITICAL();

    return (xEventHandle) pxFlag->pvOwner;
}

void vEventGenericUpdate( xEventHandle xEvent, portBASE_TYPE pxSource, portTickType xDeadline, portTickType xTimestamp, struct eventData xData)
{
    eveECB * pxEvent = (eveECB *)xEvent;
    pxEvent->pxSource = pxSource;
    pxEvent->xTag.xDeadline = xDeadline;
    pxEvent->xTag.xTimestamp = xTimestamp; 
    pxEvent->xTag.xLevel = xEventSerialNumber;
    pxEvent->xTag.xMicroStep = 0;
    pxEvent->xData = xData;
    xEventSerialNumber++;
}

void vEventGenericDelete( xEventHandle xEvent)
{
    taskENTER_CRITICAL();

    xListItem * pxEventItem = &((eveECB *)xEvent)->xEventListItem;
    vListRemove (pxEventItem);
    vListInsertEnd( &xEventIdleList, pxEventItem );

    taskEXIT_CRITICAL();
}

