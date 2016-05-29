#!/bin/bash

make clean
nice -n -20 make qemu 2>&1 > ./data_process/no_let_$1.data
