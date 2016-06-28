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

#ifndef APP_H
#define APP_H


#include "eventlist.h"
#include "app_config.h"


struct sparseRelation
{
    portBASE_TYPE xInFlag;  // source servant
    portBASE_TYPE xOutFlag; // destination servant
    portBASE_TYPE xFlag;  // 1 means relation exist but without event, 2 means that there is a event
};

struct xRelationship
{
    portBASE_TYPE xNumOfRelation;   // the real number of relations
    struct sparseRelation xRelation[MAXRELATION];  // the number of effective relations among servant
};

void PapabenchInit();
void s_0 (struct eventData *xLoopData) ;
void s_1 (struct eventData *xLoopData) ;
void s_2 (struct eventData *xLoopData) ;
void s_3 (struct eventData *xLoopData) ;
void s_4 (struct eventData *xLoopData) ;
void s_5 (struct eventData *xLoopData) ;
void s_6 (struct eventData *xLoopData) ;
void s_7 (struct eventData *xLoopData) ;
void s_8 (struct eventData *xLoopData) ;  
void s_9 (struct eventData *xLoopData) ;
void s_10(struct eventData *xLoopData) ;
void s_11(struct eventData *xLoopData) ;
void s_12(struct eventData *xLoopData) ;
void s_13(struct eventData *xLoopData) ;
void s_15(struct eventData *xLoopData) ;
void s_16(struct eventData *xLoopData) ;
void s_17(struct eventData *xLoopData) ;
void s_18(struct eventData *xLoopData) ;
void s_19(struct eventData *xLoopData) ;
void s_20(struct eventData *xLoopData) ;
void s_21(struct eventData *xLoopData) ;
void s_22(struct eventData *xLoopData) ;
void s_23(struct eventData *xLoopData) ;
void s_24(struct eventData *xLoopData) ;
void s_25(struct eventData *xLoopData) ;
void s_26(struct eventData *xLoopData) ;
void s_27(struct eventData *xLoopData) ;
void s_28(struct eventData *xLoopData) ;
void s_29(struct eventData *xLoopData) ;
void s_30(struct eventData *xLoopData) ;
void s_31(struct eventData *xLoopData) ;
void s_32(struct eventData *xLoopData) ;
void s_33(struct eventData *xLoopData) ;
void s_34(struct eventData *xLoopData) ;
void s_35(struct eventData *xLoopData) ;
void s_36(struct eventData *xLoopData) ;
void s_37(struct eventData *xLoopData) ;
void s_38(struct eventData *xLoopData) ;
void s_39(struct eventData *xLoopData) ;
void s_40(struct eventData *xLoopData) ;
void s_41(struct eventData *xLoopData) ;
void s_42(struct eventData *xLoopData) ;
void s_43(struct eventData *xLoopData) ;
void s_44(struct eventData *xLoopData) ;
void s_45(struct eventData *xLoopData) ;
void s_46(struct eventData *xLoopData) ;


#endif
