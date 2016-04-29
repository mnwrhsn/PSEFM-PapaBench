#!/bin/bash

# get the execution time of all task under the preemptable scheduler
awk '
BEGIN{
    for( i=0;i<21;++i ){
        time[i]=0;
    }
}
{
    if($1<24){
        time[$1] = $2;
    }
    else{
        print ($1/3-10), ($2-time[$1/3-10])/1000000.0;
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

