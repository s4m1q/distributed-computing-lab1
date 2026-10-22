#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "my_rand.h"
#include "timer.h"

#define MAX_ITER 1000

#define X_MIN (-2.0)
#define X_MAX   0.5
#define Y_MIN (-1.25)
#define Y_MAX   1.25

long    thread_count;
long    npoints;
double* xs;
double* ys;

int in_mandelbrot(double cx, double cy) {
    double zx = 0.0, zy = 0.0;

    for (int n = 0; n < MAX_ITER; n++) {

        double new_zx = zx * zx - zy * zy + cx;
        double new_zy = 2.0 * zx * zy + cy;
        zx = new_zx;
        zy = new_zy;

        if (zx * zx + zy * zy > 4.0)
            return 0;
    }
    return 1;
}

void* thread_work(void* rank_arg) {
    long rank = (long) rank_arg;

    long my_first = rank * npoints / thread_count;
    long my_last  = (rank + 1) * npoints / thread_count;

    unsigned seed = (unsigned) rank + 1;

    long i = my_first;
    while (i < my_last) {

        double cx = X_MIN + (X_MAX - X_MIN) * my_drand(&seed);
        double cy = Y_MIN + (Y_MAX - Y_MIN) * my_drand(&seed);

        if (in_mandelbrot(cx, cy)) {
            xs[i] = cx;
            ys[i] = cy;
            i++;
        }
    }
    return NULL;
}

void usage(const char* prog) {
    fprintf(stderr, "usage: %s nthreads npoints [output.csv]\n", prog);
    exit(1);
}

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 4) usage(argv[0]);

    thread_count = strtol(argv[1], NULL, 10);
    npoints      = strtol(argv[2], NULL, 10);
    const char* filename = (argc == 4) ? argv[3] : "mandelbrot.csv";
    if (thread_count <= 0 || npoints <= 0) usage(argv[0]);

    xs = malloc(npoints * sizeof(double));
    ys = malloc(npoints * sizeof(double));
    pthread_t* handles = malloc(thread_count * sizeof(pthread_t));
    if (xs == NULL || ys == NULL || handles == NULL) {
        fprintf(stderr, "Not enough memory\n");
        return 1;
    }

    double start, finish;
    GET_TIME(start);

    for (long t = 0; t < thread_count; t++)
        pthread_create(&handles[t], NULL, thread_work, (void*) t);

    for (long t = 0; t < thread_count; t++)
        pthread_join(handles[t], NULL);

    GET_TIME(finish);

    printf("Threads = %ld, points = %ld\n", thread_count, npoints);
    printf("Elapsed time = %e seconds\n", finish - start);

    FILE* f = fopen(filename, "w");
    if (f == NULL) {
        perror(filename);
        return 1;
    }
    fprintf(f, "x,y\n");
    for (long i = 0; i < npoints; i++)
        fprintf(f, "%.12f,%.12f\n", xs[i], ys[i]);
    fclose(f);

    free(xs);
    free(ys);
    free(handles);
    return 0;
}
