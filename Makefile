EXECS = Project4Client
OBJS = Project4Client.o

CC = gcc
CCFLAGS = -std=gnu11 -Wall -g

all: $(EXECS)

Project1Client: $(OBJS)
	$(CC) $(CCFLAGS) $^ -o $@

%.o: %.c *.h
	$(CC) $(CCFLAGS) -c $<

%.o: %.c
	$(CC) $(CCFLAGS) -c $<

clean:
	/bin/rm -f a.out $(OBJS) $(EXECS)