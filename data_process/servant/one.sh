#!/bin/bash

awk '$1==1{print $2}' datafile.out > tmp 
awk '{print NR, $1}' tmp > one.out
rm tmp
