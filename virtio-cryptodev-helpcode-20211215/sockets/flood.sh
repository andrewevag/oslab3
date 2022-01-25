#!/usr/bin/bash

for i in {1..10000}
do
	./client localhost 35016 < testinput.txt
	echo $i
done