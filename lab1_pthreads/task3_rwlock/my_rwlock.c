#include "my_rwlock.h"

int my_rwlock_init(my_rwlock_t* rw) {
    pthread_mutex_init(&rw->mutex, NULL);
    pthread_cond_init(&rw->readers_cv, NULL);
    pthread_cond_init(&rw->writers_cv, NULL);
    rw->readers_active  = 0;
    rw->readers_waiting = 0;
    rw->writers_waiting = 0;
    rw->writer_active   = 0;
    return 0;
}

int my_rwlock_destroy(my_rwlock_t* rw) {
    pthread_cond_destroy(&rw->readers_cv);
    pthread_cond_destroy(&rw->writers_cv);
    pthread_mutex_destroy(&rw->mutex);
    return 0;
}

int my_rwlock_rdlock(my_rwlock_t* rw) {
    pthread_mutex_lock(&rw->mutex);

    rw->readers_waiting++;

    while (rw->writer_active || rw->writers_waiting > 0)
        pthread_cond_wait(&rw->readers_cv, &rw->mutex);
    rw->readers_waiting--;

    rw->readers_active++;

    pthread_mutex_unlock(&rw->mutex);
    return 0;
}

int my_rwlock_wrlock(my_rwlock_t* rw) {
    pthread_mutex_lock(&rw->mutex);

    rw->writers_waiting++;
    while (rw->writer_active || rw->readers_active > 0)
        pthread_cond_wait(&rw->writers_cv, &rw->mutex);
    rw->writers_waiting--;

    rw->writer_active = 1;

    pthread_mutex_unlock(&rw->mutex);
    return 0;
}

int my_rwlock_unlock(my_rwlock_t* rw) {
    pthread_mutex_lock(&rw->mutex);

    if (rw->writer_active)
        rw->writer_active = 0;
    else
        rw->readers_active--;

    if (rw->readers_active == 0) {
        if (rw->writers_waiting > 0)

            pthread_cond_signal(&rw->writers_cv);
        else if (rw->readers_waiting > 0)

            pthread_cond_broadcast(&rw->readers_cv);
    }

    pthread_mutex_unlock(&rw->mutex);
    return 0;
}
