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
UTILIBDIR = utillib/
INCLUDE = -I/$(UTILSDIR)  -I/$(UTILIBDIR)
SOURCE = $(wildcard $(UTILSDIR)*.c)
SOURCE += $(wildcard $(UTILIBDIR)*.c)
OBJECTS = $(SOURCE:%.c=%.o)
SOCKET = directorSocket
LIBS = 

# BINS = socket-server socket-client
BINS = serverAN socket-client fatherServer child

all: $(BINS)

socket-server: socket-server.c socket-common.h
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

socket-client: socket-client.c socket-common.h
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f *.o *~ $(BINS) $(UTILSDIR)*.o $(UTILIBDIR)*.o $(SOCKET)


serverAN: serverAN.o $(OBJECTS) socket-common.h
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE)  $(LIBS)


fatherServer: fatherServer.o $(OBJECTS) socket-common.h
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBS)

child: childServer.o $(OBJECTS) socket-common.h
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE)  $(LIBS)

socket-common.h:

$(UTILSDIR)%.o: $(UTILSDIR)%.c
	$(CC) -c $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBS)	

$(UTILIBDIR)%.o: $(UTILIBDIR)%.c
	$(CC) -c $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBS)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBS)

