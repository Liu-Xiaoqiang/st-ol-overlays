#!/bin/bash -e



OUTPUT="PZ6 PZ7 PZ8 PZ9"
INPUT="PZ5 PZ4 PZ3 PZ2"



release() {
	sleep .5 && killall gpioset
}

beep() {
	gpioset -z PJ10=1
	release
	gpioset -z PJ10=0
	release
}


gpioon() {
	for i in $OUTPUT; do
		gpioset -z $i=1
		release
	done
}

gpiooff() {
	for i in $OUTPUT; do
		gpioset -z $i=0
		release
	done
}

gpiocheck() {
	for i in $INPUT; do
		gpioget $i
	done
}

while [ true ]; do
	gpioon
	beep
	echo "-----H-----"
	gpiocheck
	gpiooff
	beep
	echo "-----L-----"
	gpiocheck
done
