#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "timer.h"

#define G 6.67430e-11

typedef struct {
    double x, y, z;
} vec3;

int     n;
double* mass;
vec3*   pos;
vec3*   vel;
vec3*   force;
vec3*   loc_force;

int     thread_count;
double  dt;
long    nsteps;
long    out_every;
FILE*   out;

pthread_barrier_t barrier;

void write_state(double t) {
    fprintf(out, "%.10g", t);
    for (int q = 0; q < n; q++)
        fprintf(out, ",%.10e,%.10e", pos[q].x, pos[q].y);
    fprintf(out, "\n");
}

#ifndef BASIC

void compute_forces_local(int rank) {
    vec3* my_f = loc_force + (long) rank * n;

    for (int q = 0; q < n; q++) {
        my_f[q].x = 0.0;
        my_f[q].y = 0.0;
        my_f[q].z = 0.0;
    }

    for (int q = rank; q < n; q += thread_count) {
        for (int k = q + 1; k < n; k++) {

            double dx = pos[k].x - pos[q].x;
            double dy = pos[k].y - pos[q].y;
            double dz = pos[k].z - pos[q].z;
            double dist2 = dx * dx + dy * dy + dz * dz;
            double dist  = sqrt(dist2);

            double coef = G * mass[q] * mass[k] / (dist2 * dist);
            double fx = coef * dx;
            double fy = coef * dy;
            double fz = coef * dz;

            my_f[q].x += fx;  my_f[q].y += fy;  my_f[q].z += fz;
            my_f[k].x -= fx;  my_f[k].y -= fy;  my_f[k].z -= fz;
        }
    }
}

void reduce_forces(int my_first, int my_last) {
    for (int q = my_first; q < my_last; q++) {
        vec3 sum = {0.0, 0.0, 0.0};
        for (int t = 0; t < thread_count; t++) {
            vec3 f = loc_force[(long) t * n + q];
            sum.x += f.x;
            sum.y += f.y;
            sum.z += f.z;
        }
        force[q] = sum;
    }
}

#else

void compute_forces_basic(int my_first, int my_last) {
    for (int q = my_first; q < my_last; q++) {
        vec3 f = {0.0, 0.0, 0.0};
        for (int k = 0; k < n; k++) {
            if (k == q) continue;
            double dx = pos[k].x - pos[q].x;
            double dy = pos[k].y - pos[q].y;
            double dz = pos[k].z - pos[q].z;
            double dist2 = dx * dx + dy * dy + dz * dz;
            double dist  = sqrt(dist2);
            double coef = G * mass[q] * mass[k] / (dist2 * dist);
            f.x += coef * dx;
            f.y += coef * dy;
            f.z += coef * dz;
        }
        force[q] = f;
    }
}
#endif

void update_bodies(int my_first, int my_last) {
    for (int q = my_first; q < my_last; q++) {

        pos[q].x += vel[q].x * dt;
        pos[q].y += vel[q].y * dt;
        pos[q].z += vel[q].z * dt;

        vel[q].x += force[q].x / mass[q] * dt;
        vel[q].y += force[q].y / mass[q] * dt;
        vel[q].z += force[q].z / mass[q] * dt;
    }
}

void* thread_work(void* rank_arg) {
    int rank = (int) (long) rank_arg;

    int my_first = (int) ((long) rank * n / thread_count);
    int my_last  = (int) ((long) (rank + 1) * n / thread_count);

    for (long step = 1; step <= nsteps; step++) {

#ifndef BASIC
        compute_forces_local(rank);
#else
        compute_forces_basic(my_first, my_last);
#endif

        pthread_barrier_wait(&barrier);

#ifndef BASIC
        reduce_forces(my_first, my_last);
#endif
        update_bodies(my_first, my_last);

        pthread_barrier_wait(&barrier);

        if (rank == 0 && out_every > 0 && step % out_every == 0)
            write_state(step * dt);
    }
    return NULL;
}

void usage(const char* prog) {
    fprintf(stderr,
            "usage: %s nthreads tend input.txt dt [out_every] [output.csv]\n",
            prog);
    exit(1);
}

int main(int argc, char* argv[]) {
    if (argc < 5 || argc > 7) usage(argv[0]);

    thread_count       = (int) strtol(argv[1], NULL, 10);
    double tend        = strtod(argv[2], NULL);
    const char* infile = argv[3];
    dt                 = strtod(argv[4], NULL);
    out_every          = (argc >= 6) ? strtol(argv[5], NULL, 10) : 1;
    const char* outfile = (argc >= 7) ? argv[6] : "nbody.csv";
    if (thread_count <= 0 || tend <= 0 || dt <= 0 || out_every < 0)
        usage(argv[0]);

    FILE* in = fopen(infile, "r");
    if (in == NULL) { perror(infile); return 1; }
    if (fscanf(in, "%d", &n) != 1 || n <= 0) {
        fprintf(stderr, "Bad number of bodies in %s\n", infile);
        return 1;
    }
    mass      = malloc(n * sizeof(double));
    pos       = malloc(n * sizeof(vec3));
    vel       = malloc(n * sizeof(vec3));
    force     = malloc(n * sizeof(vec3));
    loc_force = malloc((long) thread_count * n * sizeof(vec3));
    pthread_t* handles = malloc(thread_count * sizeof(pthread_t));
    if (!mass || !pos || !vel || !force || !loc_force || !handles) {
        fprintf(stderr, "Not enough memory\n");
        return 1;
    }
    for (int q = 0; q < n; q++) {
        if (fscanf(in, "%lf %lf %lf %lf %lf %lf %lf", &mass[q],
                   &pos[q].x, &pos[q].y, &pos[q].z,
                   &vel[q].x, &vel[q].y, &vel[q].z) != 7) {
            fprintf(stderr, "Bad data for body %d in %s\n", q + 1, infile);
            return 1;
        }
    }
    fclose(in);

    nsteps = (long) (tend / dt + 0.5);

    if (out_every > 0) {
        out = fopen(outfile, "w");
        if (out == NULL) { perror(outfile); return 1; }
        fprintf(out, "t");
        for (int q = 1; q <= n; q++) fprintf(out, ",x%d,y%d", q, q);
        fprintf(out, "\n");
        write_state(0.0);
    }

    pthread_barrier_init(&barrier, NULL, thread_count);

    double start, finish;
    GET_TIME(start);
    for (long t = 0; t < thread_count; t++)
        pthread_create(&handles[t], NULL, thread_work, (void*) t);
    for (long t = 0; t < thread_count; t++)
        pthread_join(handles[t], NULL);
    GET_TIME(finish);

#ifndef BASIC
    printf("Version: Newton's 3rd law\n");
#else
    printf("Version: basic (all pairs)\n");
#endif
    printf("Threads = %d, bodies = %d, steps = %ld\n", thread_count, n, nsteps);
    printf("Elapsed time = %e seconds\n", finish - start);

    if (out != NULL) fclose(out);
    pthread_barrier_destroy(&barrier);
    free(mass); free(pos); free(vel); free(force); free(loc_force);
    free(handles);
    return 0;
}
