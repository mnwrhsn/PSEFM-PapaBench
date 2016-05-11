#!/bin/bash

awk '$1==0{print ($4 - $3)}' datafile.out > tmp 
awk '{print NR, $1}' tmp > one.out
rm tmp
