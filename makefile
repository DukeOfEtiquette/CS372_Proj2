DEFO = ftclient.o
DEFC = ftclient.cpp
OBJS = ${DEFO}
EXES = ftclient
FLAGS = -c
CC = g++

all: $(EXES)

clean:
	rm $(OBJS)

$(EXES): $(OBJS)
	$(CC) $(OBJS) -o ftclient

${DEFO}: ${DEFC}
	$(CC) $(FLAGS) ${DEFC}
