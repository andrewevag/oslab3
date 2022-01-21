
def construct_test(mystring, id):
	print(f'commencing {id}')
	print("char buf[] = {", end='')
	for i in range(len(mystring)-1):
		if mystring[i] == '0':
			print(0, end=", ", sep='')
		else:
			s = "'" + str(mystring[i]) + "'"
			print(s, end=", ")

	if mystring[len(mystring)-1] == '0':
		print('0', end="}\n", sep='')
	else: 
		print("'",str( mystring[len(mystring)-1]), "'", end="}\n")	

mystring = ("Q"+"CU" +"andreas0" + "passwd00" + 8*"0" + 8*"0" + 2*"0")
construct_test(mystring, 0)
mystring = ("Q"+"C0" +"nikol000" + "00000000" + "ch000000" + 8*"0" + 2*"0")
construct_test(mystring, 1)
