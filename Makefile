EXECS = Project4Client Project4Server
OBJS1 = utility.o Project4Client.o
OBJS2 = utility.o Project4Server.o

CC = gcc
CCFLAGS = -std=gnu11 -Wall -g -lcrypto

all: $(EXECS)

Project4Client: $(OBJS1)
	$(CC) $(CCFLAGS) $^ -o $@

Project4Server: $(OBJS2)
	$(CC) $(CCFLAGS) $^ -o $@

%.o: %.c *.h
	$(CC) $(CCFLAGS) -c $<

%.o: %.c
	$(CC) $(CCFLAGS) -c $<

clean:
	/bin/rm -f a.out $(OBJS1) $(OBJS2) $(EXECS)