#!/bin/bash

# get the execution time of all task under the preemptable scheduler
awk '
BEGIN{
    time=0;
    maxtime=1.2;
    mintime=0.6;
}
{
    if($1 == 21){
        time = $2;
    }else{
        execution_time = ($2-time)/1000000.0;
        if(execution_time <= maxtime && execution_time >= mintime){
            print NR/2, execution_time; 
        }
    }
}
' datafile.data > datafile.out


awk '
BEGIN{ 
        count=0; 
        ave=0.0; 
        total=0.0; 
        max=0.0; 
        min=1000.0
}
{
    tmp = $2;
    total += tmp;
    count ++;
    if(tmp > max){
        max = tmp;

    }

    if( tmp < min ){
        min = tmp;
    }
}
END{ 
    print "ID", "Cycles", "Max", "Average", "Min"; 
    ave=total/count; 
    print 21, count, max, ave, min;
}
' datafile.out > datafile.ave

