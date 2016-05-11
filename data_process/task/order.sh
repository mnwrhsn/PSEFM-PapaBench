#!/bin/bash

awk '$2>=4000 && $2<6000 {print NR, $1, $2, $3, $4, $5}' datafile.out > order.data1 
awk '$2>=6000 && $2<8000 {print NR, $1, $2, $3, $4, $5}' datafile.out > order.data2 
