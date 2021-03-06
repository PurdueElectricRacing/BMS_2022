GCC = gcc -g -std=c99 -Wall -Wshadow -Wvla -pedantic
VAL = valgrind --log-file=memcheck.txt --leak-check=full --verbose

TESTFLAGS = #-DDEBUG

SRCS = main.c queue.c
OBJS = $(SRCS:%.c=%.o)

queue: $(OBJS)
	$(GCC) $(TESTFLAGS) $(OBJS) -o queue
	rm -f *.o

.c.o:
	$(GCC) $(TESTFLAGS) -c $*.c

testmemory: queue
	$(VAL) ./queue

testall: remake clean 

clean:
	rm -f queue *.o

remake: clean queue