
def construct_test(mystring, id):
	print(f'void parse')
	print(f"char buf{id}[] = " + "{", end='')
	for i in range(len(mystring)-1):
		if mystring[i] == '0':
			print(0, end=", ", sep='')
		else:
			s = "'" + str(mystring[i]) + "'"
			print(s, end=", ")

	if mystring[len(mystring)-1] == '0':
		print('0', end="}\n", sep='')
	else: 
		print("'",str( mystring[len(mystring)-1]), "'", end="};\n")	
	print(f'insist_write(fd, buf{id}, sizeof(buf{id}));')

# mystring = ("Q"+"CU" +"andreas0" + "passwd00" + 8*"0" + 8*"0" + 2*"0")
# construct_test(mystring, 0)
# mystring = ("Q"+"C0" +"nikol000" + "00000000" + "ch000000" + 8*"0" + 2*"0")
# construct_test(mystring, 1)
# mystring = ("A"+"F0" +"00000000" + "00000000" + "00000000" + 8*"0" + 2*"0")
# construct_test(mystring, 2)

# mystring = ("Q"+"C0" +"nikol000" + "00000000" + "ch000000" + 8*"0" + 2*"0")
# construct_test(mystring, 3)
# mystring = ("Q"+"CU" +"andreas0" + "passwd00" + 8*"0" + 8*"0" + 2*"0")
# construct_test(mystring, 4)
# mystring = ("Q"+"R0" +"andreas0" + "passwd00" + "channel0" + 8*"0" + 2*"0")
# construct_test(mystring, 5)
# mystring = ("Q"+"S0" +"nik00000" + "kwdikos0" + "channel0" + 8*"0" + 2*"0" + "hell123456789!!")
# construct_test(mystring, 6)
# mystring = ("A"+"F0" +"00000000" + "00000000" + "00000000" + 8*"0" + 2*"0" + "newmsg")
# construct_test(mystring, 7)
# mystring = ("A"+"S0" +"00000000" + "00000000" + "00000000" + 8*"0" + 2*"0" + "newmsg")
# construct_test(mystring, 8)

# mystring = ("Q"+"CU" +"00000000" + "00000000" + "00000000" + 8*"0" + 2*"0")
# construct_test(mystring, 9)

# mystring = ("Q"+"CU" +"OKONOMA0" + "OKPASSWD" + "00000000" + 8*"0" + 2*"0")
# construct_test(mystring, 10)

# mystring = ("Q"+"CU" +"0KONOMA0" + "00000000" + "00000000" + 8*"0" + 2*"0")
# construct_test(mystring, 11)


# mystring = ("Q"+"CU" +"OKONOMA0" + "00000000" + "00000000" + 8*"0" + 2*"0")
# construct_test(mystring, 10)
# mystring = ("Q"+"CU" +"OKONOMA0" + "OKPASSWD" + "00000000" + 8*"0" + 2*"0")
# construct_test(mystring, 10)
# mystring = ("Q"+"CU" +"OKONOMA0" + "000wrong" + "00000000" + 8*"0" + 2*"0")
# construct_test(mystring, 10)


# mystring = ("Q"+"C0" +"OKONOMA0" + "notwrong" + "kanali00" + 8*"0" + 2*"0")
# construct_test(mystring, 16)
# mystring = ("Q"+"C0" +"OKONOMA0" + "00000000" + "kanali00" + 8*"0" + 2*"0")
# construct_test(mystring, 17)

# mystring = ("Q"+"C0" +"OKONOMA0" + "notwrong" + "00000000" + 8*"0" + 2*"0")
# construct_test(mystring, 18)
# mystring = ("Q"+"C0" +"OKONOMA0" + "00000000" + "00kanali" + 8*"0" + 2*"0")
# construct_test(mystring, 19)


# mystring = ("Q"+"R0" +"OKONOMA0" + "notwrong" + "kanali00" + "32000000" + 2*"0")
# construct_test(mystring, 20)
# mystring = ("Q"+"R0" +"OKONOMA0" + "passwd00" + "kanali00" + "00000000" + 2*"0")
# construct_test(mystring, 21)

# mystring = ("Q"+"R0" +"OKONOMA0" + "notwrong" + "kanali00" + "00000320" + 2*"0")
# construct_test(mystring, 22)
# mystring = ("Q"+"R0" +"OKONOMA0" + "passwd00" + "kanali00" + "12300000" + 2*"0")
# construct_test(mystring, 23)


mystring = ("Q" + "S0" + "OKONOMA0" + "passwd00" + "kanali00" + 8*"0" + str(len("hello people here we go")) + "hello people here we go")
construct_test(mystring, 24)

mystring = ("Q" + "S0" + "OKONOMA0" + "passwd00" + "kanali00" + 8*"0" + str(len("nextmsg")) + "next msg")
construct_test(mystring, 25)

mystring = ("Q" + "S0" + "OKONOMA0" + "passwd00" + "kanali00" + 8*"0" + str(len("GEIAAAAA")) + "GEIAAAAA")
construct_test(mystring, 26)

mystring = ("Q" + "S0" + "OKONOMA0" + "passwd00" + "kanali00" + 8*"0" + 2*"0" + "whaaat")
construct_test(mystring, 27)