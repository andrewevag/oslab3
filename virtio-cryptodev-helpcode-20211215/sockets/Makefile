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
UTILSDIR = ANutils/

INCLUDE = -I/$(UTILSDIR) 
LIBS = 

# BINS = socket-server socket-client
BINS = serverAN

all: $(BINS)

socket-server: socket-server.c socket-common.h
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

socket-client: socket-client.c socket-common.h
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f *.o *~ $(BINS)


serverAN: serverAN.o linkedlist.o message.o user.o channel.o 
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE)  $(LIBS)

serverAN.o: serverAN.c
	$(CC) -c  $(INCLUDE) $(CFLAGS) $^ $(LIBS)

linkedlist.o: $(UTILSDIR)linkedlist.c
	$(CC) -c $(CFLAGS) $^ $(INCLUDE) $(LIBS)

message.o: $(UTILSDIR)message.c
	$(CC) -c $(CFLAGS) $^ $(INCLUDE) $(LIBS)

user.o: $(UTILSDIR)user.c
	$(CC) -c $(CFLAGS) $^ $(INCLUDE) $(LIBS)

channel.o: $(UTILSDIR)channel.c
	$(CC) -c $(CFLAGS) $^ $(INCLUDE) $(LIBS)