CC     = gcc
CFLAGS = -O2 -Wall -Wextra -pthread

mandelbrot: mandelbrot.c my_rand.c my_rand.h timer.h
	$(CC) $(CFLAGS) -o mandelbrot mandelbrot.c my_rand.c

clean:
	rm -f mandelbrot mandelbrot.csv mandelbrot.png
