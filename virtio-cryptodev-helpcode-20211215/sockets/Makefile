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
INCLUDE += -I$(PWD)/
SOURCE = $(wildcard $(SRC)*.c)
OBJECTS = $(patsubst $(SRC)%.c, $(OBJ)%.o, $(SOURCE))
# OBJECTS += server.o

# BINS = socket-server socket-client
BINS = fatherServer childServer serverAN client

all: $(BINS)

# server: $(OBJECTS) server.o
# 	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^
fatherServer: $(OBJECTS) fatherServer.o
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^

childServer: $(OBJECTS) childServer.o
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^

serverAN: $(OBJECTS) serverAN.o
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^

client: $(OBJECTS) client.o
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^

clean:
	rm -f *.o *~ $(BINS) $(OBJ)*.o $(TEST)bin/* directorSocket

socket-common.h:

$(OBJ)%.o: $(SRC)%.c	
	$(CC) -c $(CFLAGS) $(INCLUDE) $^ -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $^ $(INCLUDE) $(LIBS)



TEST = tests/
# TESTS = $(wildcard $(TEST)*.c)
TESTS = $(TEST)test2.c
TESTBINS = $(patsubst $(TEST)%.c, $(TEST)bin/%, $(TESTS))

$(TEST)bin/%: $(TEST)%.c 
	$(CC) $(CFLAGS) $(INCLUDE) $^ $(OBJECTS) -o $@ $(LIBS)

test: $(OBJECTS) $(TESTBINS)
	for test in $(TESTBINS) ; do ./$$test ; done
