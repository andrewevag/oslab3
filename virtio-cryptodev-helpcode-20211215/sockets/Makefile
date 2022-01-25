###################################################
#
# Makefile
# Simple TCP/IP communication using sockets
#
# Vangelis Koukis <vkoukis@cslab.ece.ntua.gr>
#
###################################################

CC = gcc

CFLAGS = -Wall
CFLAGS += -g
# CFLAGS += -O2 -fomit-frame-pointer -finline-functions
INC = inc/
SRC = src/
OBJ = obj/
LIBS = -lpthread
INCLUDE = -I$(PWD)/$(INC)
SOURCE = $(wildcard $(SRC)*.c)
OBJECTS = $(patsubst $(SRC)%.c, $(OBJ)%.o, $(SOURCE))
# OBJECTS += server.o

# BINS = socket-server socket-client
<<<<<<< HEAD
BINS = serverAN clientAN
=======
BINS = fatherServer childServer serverAN client
>>>>>>> 9d03f7f487ca1a0ffdfb192bc1e848bd23f45ad9

all: $(BINS)

# server: $(OBJECTS) server.o
# 	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^
fatherServer: $(OBJECTS) fatherServer.o
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^

<<<<<<< HEAD
socket-client: socket-client.c socket-common.h 
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)
=======
childServer: $(OBJECTS) childServer.o
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^
>>>>>>> 9d03f7f487ca1a0ffdfb192bc1e848bd23f45ad9

serverAN: $(OBJECTS) serverAN.o
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^

<<<<<<< HEAD
clientAN: clientAN.o linkedlist.o message.o user.o channel.o socket-common.h
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBS)

clienAN.o: clienAN.c
	$(CC) -c  $(INCLUDE) $(CFLAGS) $^ $(LIBS)
=======
client: $(OBJECTS) client.o
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^
>>>>>>> 9d03f7f487ca1a0ffdfb192bc1e848bd23f45ad9

clean:
	rm -f *.o *~ $(BINS) $(OBJ)*.o $(TEST)bin/* directorSocket

socket-common.h:

$(OBJ)%.o: $(SRC)%.c	
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $^ $(INCLUDE) $(LIBS)



TEST = tests/
TESTS = $(wildcard $(TEST)*.c)
TESTBINS = $(patsubst $(TEST)%.c, $(TEST)bin/%, $(TESTS))

$(TEST)bin/%: $(TEST)%.c 
	$(CC) $(CFLAGS) $(INCLUDE) $^ $(OBJECTS) -o $@ $(LIBS)

test: $(OBJECTS) $(TESTBINS)
	for test in $(TESTBINS) ; do ./$$test ; done
