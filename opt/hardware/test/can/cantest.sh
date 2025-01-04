#!/bin/sh

ip link set can0 down
ip link set can0 type can bitrate 1000000
ip link set can0 up

ip link set can1 down
ip link set can1 type can bitrate 1000000
ip link set can1 up

candump can0&
candump can1&

sleep 20

i=1000
while [ $i -gt 0 ]; do
echo $i
cansend can0 5A1#11.2233.44556677.88
i=`expr $i - 1`
done

sleep 10

i=1000
while [ $i -gt 0 ]; do
echo $i
cansend can1 5A1#11.2233.44556677.88
i=`expr $i - 1`
done
