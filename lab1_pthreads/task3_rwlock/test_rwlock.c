#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "my_rwlock.h"
#include "my_rand.h"

#define K 256

my_rwlock_t rw;
int  data[K];
long writes_done = 0;

int  thread_count;
long ops_per_thread;

pthread_mutex_t stat_mutex = PTHREAD_MUTEX_INITIALIZER;
int  readers_inside = 0, writers_inside = 0, max_readers_inside = 0;
long errors = 0;
long total_writes = 0;

void enter(int is_writer) {
    pthread_mutex_lock(&stat_mutex);
    if (is_writer) {
        if (readers_inside > 0 || writers_inside > 0) errors++;
        writers_inside++;
    } else {
        if (writers_inside > 0) errors++;
        readers_inside++;
        if (readers_inside > max_readers_inside)
            max_readers_inside = readers_inside;
    }
    pthread_mutex_unlock(&stat_mutex);
}

void leave(int is_writer) {
    pthread_mutex_lock(&stat_mutex);
    if (is_writer) writers_inside--;
    else           readers_inside--;
    pthread_mutex_unlock(&stat_mutex);
}

void* thread_work(void* rank_arg) {
    long rank = (long) rank_arg;
    unsigned seed = (unsigned) rank + 1;
    long my_errors = 0, my_writes = 0;

    for (long i = 0; i < ops_per_thread; i++) {
        if (my_drand(&seed) < 0.8) {
            my_rwlock_rdlock(&rw);
            enter(0);
            for (int j = 1; j < K; j++)
                if (data[j] != data[0]) { my_errors++; break; }
            leave(0);
            my_rwlock_unlock(&rw);
        } else {
            my_rwlock_wrlock(&rw);
            enter(1);
            for (int j = 0; j < K; j++)
                data[j]++;
            writes_done++;
            leave(1);
            my_rwlock_unlock(&rw);
            my_writes++;
        }
    }

    pthread_mutex_lock(&stat_mutex);
    errors       += my_errors;
    total_writes += my_writes;
    pthread_mutex_unlock(&stat_mutex);
    return NULL;
}

int main(int argc, char* argv[]) {
    thread_count   = (argc > 1) ? atoi(argv[1]) : 4;
    ops_per_thread = (argc > 2) ? atol(argv[2]) : 200000;
    if (thread_count <= 0 || ops_per_thread <= 0) {
        fprintf(stderr, "usage: %s [nthreads] [ops_per_thread]\n", argv[0]);
        return 1;
    }

    my_rwlock_init(&rw);
    pthread_t* h = malloc(thread_count * sizeof(pthread_t));
    for (long t = 0; t < thread_count; t++)
        pthread_create(&h[t], NULL, thread_work, (void*) t);
    for (long t = 0; t < thread_count; t++)
        pthread_join(h[t], NULL);
    my_rwlock_destroy(&rw);
    free(h);

    int ok = (errors == 0 && writes_done == total_writes
              && data[0] == writes_done);
    printf("threads = %d, ops per thread = %ld\n", thread_count, ops_per_thread);
    printf("writes: counted by threads = %ld, done under lock = %ld, data[0] = %d\n",
           total_writes, writes_done, data[0]);
    printf("rule violations = %ld\n", errors);
    printf("max readers inside at once = %d\n", max_readers_inside);
    printf("%s\n", ok ? "OK: lock works correctly" : "FAIL: lock is broken!");
    return ok ? 0 : 1;
}
