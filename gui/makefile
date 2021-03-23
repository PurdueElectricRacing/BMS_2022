# Application name
TARGET = bms

# Source files
SRCS = src/main.c

# Object files
OBJST = main.o

# Compiler
CC = gcc
# Debug
DEBUG = -g
# Optimization
OPT = -O0
# Warnings
WARN = -Wall

PTHREAD = -pthread

CCFLAGS = $(DEBUG) $(OPT) $(WARN) $(PTHREAD) -pipe

GTKLIB = `pkg-config --cflags --libs gtk+-3.0`

# Linker
LD = gcc
LDFLAGS = $(PTHREAD) $(GTKLIB) -export-dynamic

OBJS = $(SRCS:%.c=%.o)

bms: $(OBJS)
	$(CC) $(CCFLAGS) $(OBJST) -o $(TARGET) $(GTKLIB)
	rm -f *.o

.c.o:
	$(CC) -c $(CCFLAGS) $*.c $(GTKLIB)

clean:
	rm -f $(TARGET)

remake: clean bms
