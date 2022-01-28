#!/bin/bash

for i in {1..1000}
do
	./client localhost 35018 < testinput.txt
	echo $i
done