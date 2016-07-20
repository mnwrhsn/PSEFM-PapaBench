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

portTickType GCDPeriod;  // the GCD of period of all tasks
portTickType xFutureModelTime ;
extern struct context xContexts[NUMBEROFSERVANT];
extern portTickType xPeriodOfTask[NUMBEROFTASK];

typedef struct eveReduceTrack
{
    xEventHandle pxEvent; // the pointer of event with latest timestamp and all data
    portBASE_TYPE xNumOfEvent; // the number of events which are processed by reduce function at this time
    portBASE_TYPE AllArrive; // bool type. Allarrive == 1 when xNumofEvent equals to the xNumOfIn of target servant
}eveRT;

eveRT xEventReduceTrack[NUMBEROFSERVANT];

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

/* insert new event item into xEventNonExecutableList or xEventExecutableList. */
static void prvEventListGenericInsert( xListItem * pxNewListItem ) PRIVILEGED_FUNCTION;

// <executable, deadline, timestamp, level>
static portBASE_TYPE xCompareFunction( const struct tag * t1, const struct tag * t2 );

static void vEventSetxTag(portTickType xDeadline, portTickType xTimestamp, xEventHandle pxNewEvent );

// if xEventReadyList is not null,
// then return the type of destination servant of the first event
portBASE_TYPE xIsERLNull()
{
    portBASE_TYPE pxDestination;
    struct tag * xTag;
    if(listCURRENT_LIST_LENGTH(&xEventReadyList) > 0)
    {
        pxDestination = xEventGetpxDestination((xEventReadyList.xListEnd.pxNext)->pvOwner); 
        xTag = xEventGetxTag(xEventReadyList.xListEnd.pxNext->pvOwner);
        if( xTaskGetTickCount() >= xTag->xTimestamp )
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

struct tag * xEventGetxTag( xEventHandle pxEvent)
{
    return (struct tag *)&((eveECB *) pxEvent)->xTag;
}

struct eventData * xEventGetxData( xEventHandle pxEvent)
{
    return (struct eventData *)&((eveECB *) pxEvent)->xData;
}

static portBASE_TYPE getGCD(portTickType t1, portTickType t2)
{
    return t1%t2==0 ? t2 : getGCD(t2, t1 % t2);
}

static portTickType GCDOfTaskPeriod()
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
        listSET_LIST_ITEM_OWNER( (xListItem *) & (pxIdleEvents[i]->xEventListItem), pxIdleEvents[i]);
        vListInsertEnd(&xEventIdleList, &pxIdleEvents[i]->xEventListItem); 
    }

    // Creating an End FLag Event and insert into the end of xEventList, which needs sorted events
}

portBASE_TYPE xIsExecutableEventArrive()
{
    xListItem * temp_pxEventListItem;
    volatile xListItem * pxIterator;
    portTickType xCurrentTime;
    portBASE_TYPE xLen, i;
    struct tag * xTag;
    
    if((xLen = listCURRENT_LIST_LENGTH(&xEventNonExecutablePool)) > 0)
    {

        pxIterator = (xListItem *)(xEventNonExecutablePool.xListEnd.pxNext);
        xCurrentTime = xTaskGetTickCount();
        for( i = 0; i < xLen ; i++ ) 
        {
            xTag = xEventGetxTag( pxIterator->pvOwner );
            if(xTag->xTimestamp <= xCurrentTime) return pdTRUE;
            pxIterator = pxIterator->pxNext;
        }
    }
    return pdFALSE;
}

/* executable event comparison function is used in xEventExecutableList. 
 * The event with earlist deadline will be scheduled to execute first */
static portBASE_TYPE xCompareFunction( const struct tag * t1, const struct tag * t2 )
{
    if( t1->xDeadline < t2->xDeadline)
    {
        return pdTRUE;
    }
    else if( t1->xDeadline == t2->xDeadline)
    {
        if( t1->xTimestamp < t2->xTimestamp)
        {
            return pdTRUE;
        }
        else if( t1->xTimestamp == t2->xTimestamp )
        {
            if( t1->xMicroStep < t2->xMicroStep)
            {
                return pdTRUE;
            }
            else if(t1->xMicroStep == t2->xMicroStep && t1->xLevel < t2->xLevel )
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
}

/* insert event to xEventNonExecutableList in terms of comparison function 1 */
static void prvEventListGenericInsert( xListItem *pxNewListItem )
{
    struct tag * xTagOfInsertion;
    xList * pxList = &xEventExecutableList; 
    volatile xListItem *pxIterator;
    portBASE_TYPE xLen, i;

    xTagOfInsertion = xEventGetxTag(pxNewListItem->pvOwner);

    taskENTER_CRITICAL();
    // inserting start from the ending of list
    xLen = listCURRENT_LIST_LENGTH(&xEventExecutableList);
    pxIterator = (xListItem *)&(pxList->xListEnd);
    for( i = 0; i < xLen && xCompareFunction( xTagOfInsertion, xEventGetxTag( pxIterator->pxPrevious->pvOwner )); i ++)
    { 
        pxIterator = pxIterator->pxPrevious;
    } 
    //vPrintString("hello,world\n\r");

    taskEXIT_CRITICAL();

    pxIterator = pxIterator->pxPrevious;
    
    // insert the new event after a smaller one from the back of list
    pxNewListItem->pxNext = pxIterator->pxNext;
    pxNewListItem->pxNext->pxPrevious = ( volatile xListItem * ) pxNewListItem;
    pxNewListItem->pxPrevious = pxIterator;
    pxIterator->pxNext = ( volatile xListItem * ) pxNewListItem;

    pxNewListItem->pvContainer = ( void * ) pxList;

    ( pxList->uxNumberOfItems )++;
}


xEventHandle pxEventGenericCreate( portBASE_TYPE pxSource, portTickType xDeadline, portTickType xTimestamp, struct eventData * pdData)
{
    eveECB * pxNewEvent = NULL;

    /* using the pxCurrentTCB, current task should not be changed */
    taskENTER_CRITICAL();

    pxNewEvent = (eveECB *)xEventIdleList.xListEnd.pxNext->pvOwner;
    vListRemove( (xListItem *)&pxNewEvent->xEventListItem );

    pxNewEvent->pxSource = pxSource;
    vEventSetxTag( xDeadline, xTimestamp, pxNewEvent );
    ((eveECB *) pxNewEvent)->xData = *pdData;
    listSET_LIST_ITEM_OWNER( (xListItem *) &pxNewEvent->xEventListItem, pxNewEvent);

    taskEXIT_CRITICAL();

    return pxNewEvent;
}


// find the executable events from xEventNonExecutablePool.
// scatter the events and transit them into xEventExecutablePool
void vEventGenericScatter()
{
    portBASE_TYPE i, j, xLen;
    portBASE_TYPE pxSource, pxDestination, outs;
    struct tag * xTag;
    eveECB * pxEvent , * pxCopyEvent; 
    xListItem * temp_pxEventListItem;
    portTickType xCurrentTime;
    volatile xListItem * pxIterator = xEventNonExecutablePool.xListEnd.pxNext;

    // event scatter
    if( (xLen = listCURRENT_LIST_LENGTH( &xEventNonExecutablePool )) > 0)
    {
        xCurrentTime = xTaskGetTickCount();
        // search executable event from nonexecutable pool
        for(j = 0; j < xLen; j ++ )
        {
            xTag = xEventGetxTag( pxIterator->pvOwner ); 
            // find one
            if( xTag->xTimestamp <= xCurrentTime )
            {
                pxEvent = (eveECB *) pxIterator->pvOwner;
                outs = xContexts[ pxEvent->pxSource ].xNumOfOut; // used for cloning

                taskENTER_CRITICAL();
                temp_pxEventListItem = (xListItem *) pxIterator;
                pxIterator = pxIterator->pxNext;
                vListRemove(temp_pxEventListItem);
                // complete the information of the origin event.
                pxEvent->pxDestination = xContexts[pxEvent->pxSource].xOutFlag[0];
                xContexts[pxEvent->pxDestination].xInBoolCount++;
                pxEvent->xTag.xLevel = xContexts[pxEvent->pxDestination].xMyFlag;
                vListInsertEnd(&xEventExecutablePool, temp_pxEventListItem);

                // copy one event to multiples
                for(i = 1; i < outs; ++ i)
                {
                    pxCopyEvent = (eveECB *)pxEventGenericCreate(pxSource, pxEvent->xTag.xDeadline, pxEvent->xTag.xTimestamp, &pxEvent->xData);
                    // complete the information of the copied event.
                    pxCopyEvent->xTag.xMicroStep = i;

                    pxCopyEvent->pxDestination = xContexts[pxEvent->pxSource].xOutFlag[i];
                    xContexts[pxCopyEvent->pxDestination].xInBoolCount++;
                    pxCopyEvent->xTag.xLevel = xContexts[pxEvent->pxDestination].xMyFlag; 
                    vListInsertEnd(&xEventExecutablePool, &pxCopyEvent->xEventListItem);
                }
                taskEXIT_CRITICAL();
            }
            else
            {
                pxIterator = pxIterator->pxNext;
            }
        }
    }
}


static void clearEventReduceTrack()
{
    portBASE_TYPE i = NUMBEROFSERVANT;
    while(i--)
    {
        xEventReduceTrack[i].pxEvent = NULL; 
        xEventReduceTrack[i].xNumOfEvent = xEventReduceTrack[i].AllArrive = 0;
    }
}

void vEventGenericReduce()
{
    xListItem * temp_pxEventListItem; 
    portBASE_TYPE i, temp_count, xLen, pxDestination;
    struct tag * temp_tag;
    struct eventData * temp_data;
    volatile xListItem * pxIterator;
    eveRT * temp_RT;

    if( (xLen = listCURRENT_LIST_LENGTH( &xEventExecutablePool )) > 0 )
    {
        clearEventReduceTrack();
        pxIterator = (xListItem *) xEventExecutablePool.xListEnd.pxNext;
        for( i = 0; i < xLen; i++)
        {
            pxDestination = xEventGetpxDestination( pxIterator->pvOwner );
            if(xContexts[pxDestination].xInBoolCount == xContexts[pxDestination].xNumOfIn )
            {
                temp_RT = &xEventReduceTrack[pxDestination];
                if(xContexts[pxDestination].xInBoolCount > 1)
                {
                    if( (temp_count = temp_RT->xNumOfEvent) > 0 )
                    {
                        temp_data = xEventGetxData( pxIterator->pvOwner );  
                        temp_tag = xEventGetxTag( pxIterator->pvOwner );
                        ((eveECB *)temp_RT->pxEvent)->xData.xDataArray[temp_count] = temp_data->xDataArray[0];

                        if( temp_RT->xNumOfEvent++ == xContexts[pxDestination].xInBoolCount - 1)
                        {
                            temp_RT->AllArrive = 1;
                        }
                        if(((eveECB *)temp_RT->pxEvent)->xTag.xTimestamp < temp_tag->xTimestamp)
                        {
                            ((eveECB *)temp_RT->pxEvent)->xTag.xTimestamp = temp_tag->xTimestamp;
                        }

                        temp_pxEventListItem = (xListItem *) pxIterator;
                        pxIterator = pxIterator->pxNext;
                        vEventGenericDelete(temp_pxEventListItem->pvOwner);  // delete the redundant events
                    }
                    else  // the fist one of reduce event
                    {
                        temp_RT->pxEvent = pxIterator->pvOwner; 
                        temp_RT->xNumOfEvent = 1;
                        
                        temp_pxEventListItem = (xListItem *) pxIterator;
                        pxIterator = pxIterator->pxNext;
                        vListRemove(temp_pxEventListItem);
                    }
                }
                else // only one event for the target servant
                {
                    temp_RT->pxEvent = pxIterator->pvOwner; 
                    temp_RT->xNumOfEvent = 1;
                    temp_RT->AllArrive = 1;
                 
                    temp_pxEventListItem = (xListItem *) pxIterator;
                    pxIterator = pxIterator->pxNext;
                    vListRemove(temp_pxEventListItem);
                }

            }
        } // end for
        for(i=0; i<NUMBEROFSERVANT; ++i)
        {
            if(xEventReduceTrack[i].AllArrive)
            {
                prvEventListGenericInsert(&((eveECB *)xEventReduceTrack[i].pxEvent)->xEventListItem);
                xContexts[i].xInBoolCount = 0;  // important recovery !!!!!
            }
        } // end for
    } // end if
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

static void xSetTimestamp( xListItem * pxEventListItem)
{
    portBASE_TYPE i;
    eveECB *pxEvent;  
    
    pxEvent = (eveECB *) pxEventListItem->pvOwner;
    pxEvent->xTag.xTimestamp = xFutureModelTime;

    if( pOverLap(pxEventListItem )== 0 ) // not overlaped
    {
        xFutureModelTime += xContexts[pxEvent->pxDestination].xLet; // update future model time
        vListRemove( pxEventListItem); 
        vListInsertEnd(&xEventReadyList, pxEventListItem);
    }
    else  // overlaped
    {
        // set the future model time to start time of next LET
        xFutureModelTime = (xFutureModelTime/GCDPeriod) * GCDPeriod + GCDPeriod + INPUT;  
        pxEvent->xTag.xTimestamp = xFutureModelTime;
        xFutureModelTime += xContexts[pxEvent->pxDestination].xLet;

        vListRemove( pxEventListItem); 
        vListInsertEnd(&xEventNonExecutablePool, pxEventListItem);
    }
}

static portBASE_TYPE pEqualxDeadline(struct tag * xTag1, struct tag * xTag2)
{
    return (xTag1->xDeadline == xTag2->xDeadline);
}

// update the timestamp of event in xEventExecutableList in terms of xFutureModelTime
portBASE_TYPE xEventGenericSerialize()
{
    xListItem * flag_pxEventListItem;
    struct tag * origin_tag;
    portBASE_TYPE count = 1;

    if( listCURRENT_LIST_LENGTH( &xEventExecutableList ) > 0 )
    {
        origin_tag = xEventGetxTag( (xEventExecutableList.xListEnd.pxNext)->pvOwner );
    }
    
    while( listCURRENT_LIST_LENGTH(&xEventExecutableList) > 0 )
    {
        flag_pxEventListItem = (xListItem *) xEventExecutableList.xListEnd.pxNext;
        // only the timestamp of c_servant need updating
        // only remove events that have same xDeadline with the first event in xEventExecutableList to xEventReadyList

        if( xContexts[xEventGetpxDestination(flag_pxEventListItem->pvOwner)].xType == 2 )
        {
            if( pEqualxDeadline( xEventGetxTag( flag_pxEventListItem->pvOwner ), origin_tag ))
            {
                // update the timestamp of event
                // if overlaped, then remove the event to xEventNonExecutablePool
                // else, remove the event to xEvenReadyList
                xSetTimestamp( flag_pxEventListItem);
            }
            else
            {
                break;
            }
        }
        else
        {
            vListRemove( flag_pxEventListItem );
            vListInsertEnd( &xEventReadyList, flag_pxEventListItem );
        }
    }

    if( listCURRENT_LIST_LENGTH( &xEventReadyList ) > 0 )
    {
        return xEventGetpxDestination((xEventReadyList.xListEnd.pxNext)->pvOwner); //  the first one repsenting the type of events
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
    xListItem * pxEventList;

    taskENTER_CRITICAL();
    pxEventList= (xListItem *)xEventReadyList.xListEnd.pxNext;
    vListRemove(pxEventList);
    vListInsertEnd(&xEventNonExecutablePool, pxEventList);  // reuse event, which will be update by servant 
    taskEXIT_CRITICAL();

    return (xEventHandle) pxEventList->pvOwner;
}

void vEventGenericUpdate( xEventHandle xEvent, portBASE_TYPE pxSource, portTickType xDeadline, portTickType xTimestamp, struct eventData * xData)
{
    eveECB * pxEvent = (eveECB *)xEvent;
    pxEvent->pxSource = pxSource;
    pxEvent->xTag.xDeadline = xDeadline;
    pxEvent->xTag.xTimestamp = xTimestamp; 
    pxEvent->xTag.xMicroStep = 0;
    pxEvent->xData = *xData;
}

void vEventGenericDelete( xEventHandle xEvent)
{
    xListItem * pxEventItem;
    taskENTER_CRITICAL();

    pxEventItem = &((eveECB *)xEvent)->xEventListItem;
    vListRemove (pxEventItem);
    vListInsertEnd( &xEventIdleList, pxEventItem );

    taskEXIT_CRITICAL();
}

