#!/bin/bash

awk '
{
    $1 == 1 ||
    $1 == 3 ||
    $1 == 4 ||
    $1 == 5 ||
    $1 == 6 ||
    $1 == 8 ||
    $1 == 11 ||
    $1 == 12 ||
    $1 == 14 ||
    $1 == 17 ||
    $1 == 18 ||
    $1 == 19 ||
    $1 == 20 
    {print }
}
' responsetime.out > responsetime_task.out

awk 'BEGIN{i=0}$1==1{print i+1, $2-$3, $2, $3, $4; i++;}' responsetime_task.out > task/task1.out
awk 'BEGIN{i=0}$1==3{print i+1, $2-$3, $2, $3, $4; i++;}' responsetime_task.out > task/task2.out
awk 'BEGIN{i=0}$1==4{print i+1, $2-$3, $2, $3, $4; i++;}' responsetime_task.out > task/task3.out
awk 'BEGIN{i=0}$1==5{print i+1, $2-$3, $2, $3, $4; i++;}' responsetime_task.out > task/task4.out
awk 'BEGIN{i=0}$1==6{print i+1, $2-$3, $2, $3, $4; i++;}' responsetime_task.out > task/task5.out
awk 'BEGIN{i=0}$1==8{print i+1, $2-$3, $2, $3, $4; i++;}' responsetime_task.out > task/task6.out
awk 'BEGIN{i=0}$1==11{print i+1,$2-$3, $2, $3, $4; i++;}' responsetime_task.out > task/task7.out
awk 'BEGIN{i=0}$1==12{print i+1,$2-$3, $2, $3, $4; i++;}' responsetime_task.out > task/task8.out
awk 'BEGIN{i=0}$1==14{print i+1,$2-$3, $2, $3, $4; i++;}' responsetime_task.out > task/task9.out
awk 'BEGIN{i=0}$1==17{print i+1,$2-$3, $2, $3, $4; i++;}' responsetime_task.out > task/task10.out
awk 'BEGIN{i=0}$1==18{print i+1,$2-$3, $2, $3, $4; i++;}' responsetime_task.out > task/task11.out
awk 'BEGIN{i=0}$1==19{print i+1,$2-$3, $2, $3, $4; i++;}' responsetime_task.out > task/task12.out
awk 'BEGIN{i=0}$1==20{print i+1,$2-$3, $2, $3, $4; i++;}' responsetime_task.out > task/task13.out

