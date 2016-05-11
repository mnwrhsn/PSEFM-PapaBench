#!/bin/bash

awk '$3>=4000 && $3<=6000 {print NR, $1, $2, $3, $4, $5}' datafile.out > formate_order.data1
awk '$3>=6000 && $3<=8000 {print NR, $1, $2, $3, $4, $5}' datafile.out > formate_order.data2
