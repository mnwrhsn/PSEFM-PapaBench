#!/bin/bash

# get the execution time of all task under the preemptable scheduler
awk '
BEGIN{
    for( i=0;i<21;++i ){
        time[i]=0;
    }
    max_execution[0] = 13; 
    max_execution[1] = 2; 
    max_execution[2] = 4; 
    max_execution[3] = 3; 
    max_execution[4] = 3; 
    max_execution[5] = 6; 
    max_execution[6] = 7; 
    max_execution[7] = 7; 
    max_execution[8] = 5; 
    max_execution[9] = 3; 
    max_execution[10] = 4; 
    max_execution[11] = 6; 
    max_execution[12] = 2; 
    max_execution[13] = 2; 
    max_execution[14] = 4; 
    max_execution[15] = 3; 
    max_execution[16] = 5; 
    max_execution[17] = 4; 
    max_execution[18] = 3; 
    max_execution[19] = 6; 
    max_execution[20] = 6; 
}
{
    if($1<24){
        time[$1] = $2;
    }
    else{
        execution_time = ($2-time[$1/3-10])/1000000.0;
        if(execution_time <= max_execution[$1/3-10]+0.5){
            print ($1/3-10), execution_time; 
        }
    }
}
' datafile.data > datafile.out


awk '
BEGIN{ 
    for(i=0;i<21;++i) {
        count[i]=0; 
        ave[i]=0.0; 
        total[i]=0.0; 
        max[i]=0.0; 
        min[i]=1000.0
    } 
}
{
    tmp = $2;
    total[$1] += tmp;
    count[$1] ++;
    if(tmp > max[$1]){
        max[$1] = tmp;

    }

    if( tmp < min[$1] ){
        min[$1] = tmp;
    }
}
END{ 
    print "ID", "Cycles", "Max", "Average", "Min"; 
    for(i=0;i<21;++i) {
        ave[i]=total[i]/count[i]; 
        print i, count[i], max[i], ave[i], min[i];
    }  
}
' datafile.out > datafile.ave

