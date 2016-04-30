#!/bin/bash

# get the execution time of all task under the preemptable scheduler
awk '
BEGIN{
    readytime = 0;
    deadline = 0;
    responsetime = 0;
}
{
    if($1<24){
        id1 = $1;
    }else if ($1 == (id1+10)*3){
        print id1, responsetime, readytime, deadline ;
        readytime = 0;
        deadline = 0;
        responsetime = 0;
    }else if (readytime == 0){
        readytime = $1;
    }else if (deadline == 0){
        deadline = $1;
    }else{
        responsetime = $1;
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
    tmp = $2/1000000.0;
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

