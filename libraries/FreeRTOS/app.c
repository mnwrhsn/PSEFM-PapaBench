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
#include "app.h"
#include "servant.h"

/* the include file of PapaBench */
#include "ppm.h"
#include "servo.h"
#include "spi_fbw.h"
//#include "inflight_calib.h"
#include "infrared.h"
#include "estimator.h"
#include "pid.h"
#include "link_fbw.h"
#include "gps.h"
#include "autopilot.h"
#include "nav.h"


xSemaphoreHandle xBinarySemaphore[NUMBEROFSERVANT];  // the semaphores which are used to trigger new servant to execute

portTickType xPeriodOfTask[NUMBEROFTASK] =
{
    25 , 
    25 , 
    50 , 
    50 , 
    50 , 
    25 , 
    50 , 
    50 , 
    250, 
    250, 
    250, 
    250, 
    100  
};

portBASE_TYPE xSensorOfTask[NUMBEROFTASK] =
{
    0, 4, 8, 11, 14, 17, 21, 26, 29, 33, 38, 41, 44 
};

portBASE_TYPE xActuatorOfTask[NUMBEROFTASK] =
{
    3, 7, 10, 13, 16, 20, 25, 28, 32, 37, 40, 43, 46
};


portTickType xLetOfServant[NUMBEROFSERVANT] = 
{ 
    1,  // task 0
    1,   //
    1,    //
    1,   //
    1,     // task 1
    1,    //
    1,    //
    1,   //
    1,     // task 2
    1,    //
    1,   //
    1,     // task 3
    1,    //
    1,   //
    1,     // task 4
    1,    //
    1,   //
    1,     // task 5
    1,    //
    1,    //
    1,   //
    1,     // task 6
    1,    //
    1,    //
    1,    //
    1,   //
    1,     // task 7
    1,    //
    1,   //
    1,     // task 8
    1,    //
    1,    //
    1,   //
    1,     // task 9
    1,    //
    1,    //
    1,    //
    1,   //
    1,     // task 10
    1,    //
    1,   //
    1,     // task 11
    1,    //
    1,   //
    1,     // task 12
    1,    //
    1,
};

// mark the task id of every s-servant 100
portBASE_TYPE xTaskOfServant[NUMBEROFSERVANT] =
{
    0, //task 1
    0, 
    0,
    0,
    1, // task 2
    1,
    1,
    1,
    2, // task 3
    2,
    2,
    3, // task 4
    3,
    3,
    4, // task 5
    4,
    4,
    5, // task 6
    5,
    5,
    5,
    6, // task 7
    6,
    6,
    6,
    6, 
    7, // task 8
    7,
    7,
    8, // task 9
    8,
    8, 
    8, 
    9, // task 10
    9,
    9,
    9,
    9,
    10, // task 11
    10,
    10,
    11, // task 12
    11,
    11,
    12,  // task 13
    12, 
    12, 
};

struct xRelationship xRelations = 
{
    47,
    {
        {0, 1, 1}, // task 0
        {1, 2, 1},
        {2, 3, 1},
        {3, 0, 1},
        {4, 5, 1}, // task 1
        {5, 6, 1},
        {6, 7, 1},
        {7, 4, 1},
        {8, 9, 1}, // task 2
        {9, 10,1},  
        {10,8, 1},  
        {11,12,1}, // task 3
        {12,13,1},
        {13,11,1},
        {14,15,1}, // task 4
        {15,16,1},
        {16,14,1},
        {17,18,1}, // task 5
        {18,19,1},
        {19,20,1},
        {20,17,1},
        {21,22,1}, // task 6
        {22,23,1},  
        {23,24,1},  
        {24,25,1},  
        {25,21,1},  
        {26,27,1}, // task 7
        {27,28,1},
        {28,26,1},
        {29,30,1}, // task 8
        {30,31,1},
        {31,32,1},
        {32,29,1}, 
        {33,34,1}, // task 9
        {34,35,1},
        {35,36,1},
        {36,37,1},
        {37,33,1},
        {38,39,1}, // task 10
        {39,40,1},
        {40,38,1},
        {41,42,1}, // task 11
        {42,43,1},
        {43,41,1}, 
        {44,45,1}, // task 12
        {45,46,1},
        {46,44,1},
    }
};

/* explemented in main.c */
extern void to_autopilot_from_last_radio();
extern void check_mega128_values_task();
extern void check_failsafe_task();
//extern void inflight_calib(portBASE_TYPE mode_changed); // main_auto.c , we transfer this function into main.c because of bugs if not 
extern void radio_control_task(); 
extern void send_gps_pos();
extern void send_radIR();
extern void send_takeOff();

extern void  send_boot();
extern void  send_attitude();
extern void  send_adc();
extern void  send_settings();
extern void  send_desired();
extern void  send_bat();
extern void  send_climb();
extern void  send_mode();
extern void  send_debug();
extern void  send_nav_ref();

#define SUNNYBEIKE 1
#ifdef SUNNYBEIKE

/*task0*/
void s_0(struct eventData * pxDataArray) 
{
    // do actuator
    // do sensor
}
void s_1(struct eventData * pxDataArray) 
{
    last_radio_from_ppm(); // ppm.h
}
void s_2(struct eventData * pxDataArray) 
{
    //servo_set();  // servo.h, this function is used by multitask which in terms of multiuse of Servant. And this
                    // is not implemented now. So cancel.
}
void s_3(struct eventData * pxDataArray) 
{
}

/*task1*/
void s_4(struct eventData * pxDataArray) 
{
}
void s_5(struct eventData * pxDataArray) 
{
    to_autopilot_from_last_radio(); // main.c
}
void s_6(struct eventData * pxDataArray) 
{
    spi_reset(); // spi_fbw.h
}
void s_7(struct eventData * pxDataArray) 
{
}

/*task 2*/ 
void s_8(struct eventData * pxDataArray) 
{
}
void s_9(struct eventData * pxDataArray) 
{
    check_mega128_values_task(); // main.c
}
void s_10(struct eventData * pxDataArray) 
{
}

/*task 3, servant 8, 9*/ 
void s_11(struct eventData * pxDataArray) 
{
}
void s_12(struct eventData * pxDataArray) 
{
    servo_transmit(); //servo.h
}
void s_13(struct eventData * pxDataArray) 
{
}

/*task 4, servant 10, 11*/
void s_14(struct eventData * pxDataArray) 
{
}
void s_15(struct eventData * pxDataArray) 
{
    check_failsafe_task(); // main.c
}
void s_16(struct eventData * pxDataArray) 
{
}

/*task 5, servant 12, 13, 14*/
void s_17(struct eventData * pxDataArray) 
{
}
void s_18(struct eventData * pxDataArray) 
{
     //inflight_calib(pdTRUE); // inflight_calib.h
     //inflight_calib(pdTRUE); // main_auto.c , we transfer this function into main.c because of bugs if not 
     radio_control_task();
}
void s_19(struct eventData * pxDataArray) 
{
    ir_gain_calib(); //infrared.h
}
void s_20(struct eventData * pxDataArray) 
{
}

/*task 6, servant 15,16,17,18*/ 
void s_21(struct eventData * pxDataArray) 
{
}
void s_22(struct eventData * pxDataArray) 
{
    ir_update(); // infrared.h
}
void s_23(struct eventData * pxDataArray) 
{
    //servo_transmit();  // servo.h
    estimator_update_state_infrared(); //estimator.h
}
void s_24(struct eventData * pxDataArray) 
{
    roll_pitch_pid_run(); // pid.h
}
void s_25(struct eventData * pxDataArray) 
{
}

/*task 7, servant 19, 20*/
void s_26(struct eventData * pxDataArray) 
{
}
void s_27(struct eventData * pxDataArray) 
{
    link_fbw_send(); //link_fbw.h
}
void s_28(struct eventData * pxDataArray) 
{}

/*task 8, sevrvant 21,22,23*/
void s_29(struct eventData * pxDataArray) 
{
}
void s_30(struct eventData * pxDataArray) 
{
    parse_gps_msg(); //gps.h
}
void s_31(struct eventData * pxDataArray) 
{
    //use_gps_pos(); // autopilot.h  is not implemeted.
    send_gps_pos();
    send_radIR();
    send_takeOff();
}
void s_32(struct eventData * pxDataArray) 
{}

/*task 9, servant 24, 25, 26, 27*/
void s_33(struct eventData * pxDataArray) 
{
}
void s_34(struct eventData * pxDataArray) 
{
    nav_home(); // nav.h
}
void s_35(struct eventData * pxDataArray) 
{
    nav_update(); // nav.h
}
void s_36(struct eventData * pxDataArray) 
{
    course_pid_run(); // pid.h
}
void s_37(struct eventData * pxDataArray) 
{}

/*task 10, servant 28, 29*/
void s_38(struct eventData * pxDataArray) 
{
}
void s_39(struct eventData * pxDataArray) 
{
    altitude_pid_run(); // pid.h
}
void s_40(struct eventData * pxDataArray) 
{}

/*task 11, servant 30, 31*/
void s_41(struct eventData * pxDataArray) 
{
}
void s_42(struct eventData * pxDataArray) 
{
    climb_pid_run(); // pid.h
}
void s_43(struct eventData * pxDataArray) 
{}

/*task 12, servant 32, 33*/
void s_44(struct eventData * pxDataArray) 
{
}
void s_45(struct eventData * pxDataArray) 
{
    // main.c
    send_boot();
    send_attitude();
    send_adc();
    send_settings();
    send_desired();
    send_bat();
    send_climb();
    send_mode();
    send_debug();
    send_nav_ref();
}
void s_46(struct eventData * pxDataArray) 
{}
#endif

// assigned the point of function into specified position of xServantTable.
pvServantFunType xServantTable[NUMBEROFSERVANT] = 
{
    &s_0, 
    &s_1,
    &s_2,
    &s_3,
    &s_4,
    &s_5,
    &s_6,
    &s_7,
    &s_8,
    &s_9,
    &s_10, 
    &s_11,
    &s_12,
    &s_13,
    &s_14,
    &s_15,
    &s_16,
    &s_17,
    &s_18,
    &s_19,
    &s_20,
    &s_21,
    &s_22,
    &s_23,
    &s_24,
    &s_25,
    &s_26,
    &s_27,
    &s_28,
    &s_29,
    &s_30,
    &s_31,
    &s_32,
    &s_33,
    &s_34,
    &s_35,
    &s_36,
    &s_37,
    &s_38,
    &s_39,
    &s_40,
    &s_41,
    &s_42,
    &s_43,
    &s_44,
    &s_45,
    &s_46
};



