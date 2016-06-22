#ifndef APP_CONFIG_H
#define APP_CONFIG_H

extern portTickType xFutureModelTime; 

#define NUMBEROFTHREAD 4
#define NUMBEROFSERVANT 47 
#define NUMBEROFTASK 13
#define NUMBEROFEVENTS 13  // NUMBEROFEVENTS = Sum( Max{Concurrents of Task i} )
#define MAXRELATION NUMBEROFSERVANT 

#define INPUT 3 // the execution time of INPUT
#define OUTPUT 3 // the execution time of OUTPUT

#define MAXOUTDEGREE 2   // network max in degree of every S-servant
#define MAXINDEGREE 2  // network max out degree of every s-servant

#endif
