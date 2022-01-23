

for i in {1..1000}
do
	./client localhost 35016 < testinput.txt
	echo $i
done