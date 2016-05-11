#!/bin/bash

# get the execution time of all task under the preemptable scheduler
awk '
BEGIN{
    readytime = 0;
    deadline = 0;
    responsetime = 0;
    starttime = 0;
    id1 = -1;
}
{
    if(id1 == -1 && $1<35){
        id1 = $1;
        time1 = $2;
    }else if ($1 == (id1+10)*3){
        print id1, responsetime-starttime, readytime, responsetime, deadline ;
        starttime = 0;
        readytime = 0;
        deadline = 0;
        responsetime = 0;
        id1 = -1;
    }else if (readytime == 0){
        readytime = $1;
    }else if (starttime == 0){
        starttime = $1 ;
    }else if (responsetime == 0){
        responsetime = $1;
    }else{
        deadline = $1;
    }
}
' datafile.data > datafile.out


awk '
BEGIN{ 
    for(i=0;i<34;++i) {
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
    for(i=0;i<34;++i) {
        ave[i]=total[i]/count[i]; 
        print i, count[i], max[i], ave[i], min[i];
    }  
}
' datafile.out > datafile.ave

