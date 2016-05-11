#!/bin/bash

awk '$2>=4000 && $2<5000 {print NR, $1, $2, $3, $4, $5}' datafile.out > formate_order.data1
awk '$2>=5000 && $2<6000 {print NR, $1, $2, $3, $4, $5}' datafile.out > formate_order.data2
awk '$2>=6000 && $2<7000 {print NR, $1, $2, $3, $4, $5}' datafile.out > formate_order.data3
awk '$2>=7000 && $2<8000 {print NR, $1, $2, $3, $4, $5}' datafile.out > formate_order.data4
