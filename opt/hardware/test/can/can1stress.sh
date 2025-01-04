#!/bin/sh

ip link set can1 down
ip link set can1 type can bitrate 1000000
ip link set can1 up

i=10000
while [ $i -gt 0 ]; do
echo $i
cansend can1 5A1#11.2233.44556677.88
i=`expr $i - 1`
done
