CC = gcc
CFLAGS = -Wall
DEPS = clock.h clock.c descriptor.h descriptor.c queue.h queue.c structs.h userprocess.c oss.h oss.c
OBJ = clock.o descriptor.o queue.o userprocess.o oss.o

all: oss userprocess

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<
	
userprocess: userprocess.o
	$(CC) $(CFLAGS) -o $@ $^
	
oss: oss.o
	$(CC) $(CFLAGS) -o $@ $^
	
clean:
	rm *.h *.c *.o