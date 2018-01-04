#!/bin/bash
for i in `seq 0 $1`
do
 j=`echo $i%8 | bc` 
 ./test GPU$j &
 #sleep 0.1
done
