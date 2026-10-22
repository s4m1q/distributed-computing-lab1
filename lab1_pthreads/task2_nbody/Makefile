CC     = gcc
CFLAGS = -O2 -Wall -Wextra -pthread

all: nbody nbody_basic

nbody: nbody.c timer.h
	$(CC) $(CFLAGS) -o nbody nbody.c -lm

nbody_basic: nbody.c timer.h
	$(CC) $(CFLAGS) -DBASIC -o nbody_basic nbody.c -lm

clean:
	rm -f nbody nbody_basic nbody.csv nbody.png bodies_*.txt
