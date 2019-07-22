// compile: gcc % ../../../lib/error.c -I ../../../include -lpthread -Wall -o /tmp/%.out
// run: /tmp/%.out
// clean: rm -f /tmp/%.out

#include "apue.h"
#include <stdlib.h>
#include <pthread.h>

#define ATTEMPTS 10
#define NHASH 29
#define HASH(id) (((unsigned long)id)%NHASH)

struct foo *fh[NHASH];

pthread_mutex_t hashlock = PTHREAD_MUTEX_INITIALIZER;

struct foo {
    int             f_count;
    pthread_mutex_t f_lock;
    int             f_id;
    struct foo      *f_next;
};

struct foo *
foo_alloc(int id)
{
    struct foo *fp;
    int idx;

    if ((fp = malloc(sizeof(struct foo))) != NULL) {
        fp->f_count = 1;
        fp->f_id = id;
        if (pthread_mutex_init(&fp->f_lock, NULL) != 0) {
            free(fp);
            return(NULL);
        }
        idx = HASH(id);
        pthread_mutex_lock(&hashlock);
        fp->f_next = fh[idx];
        fh[idx] = fp;
        pthread_mutex_lock(&fp->f_lock);
        pthread_mutex_unlock(&hashlock);
        pthread_mutex_unlock(&fp->f_lock);
    }
    return(fp);
}

void
foo_hold(struct foo *fp)
{
    pthread_mutex_lock(&fp->f_lock);
    fp->f_count++;
    pthread_mutex_unlock(&fp->f_lock);
}

struct foo *
foo_find(int id)
{
    struct foo *fp;

    pthread_mutex_lock(&hashlock);
    for (fp = fh[HASH(id)]; fp != NULL; fp = fp->f_next) {
        if (fp->f_id == id) {
            foo_hold(fp);
            break;
        }
    }
    pthread_mutex_unlock(&hashlock);
    return(fp);
}

void
foo_rele(struct foo *fp)
{
    struct foo *tfp;
    int idx;

    pthread_mutex_lock(&fp->f_lock);
    if (fp->f_count == 1) {
        pthread_mutex_unlock(&fp->f_lock);
        pthread_mutex_lock(&hashlock);
        pthread_mutex_lock(&fp->f_lock);
        if (fp->f_count != 1) {
            fp->f_count--;
            pthread_mutex_unlock(&fp->f_lock);
            pthread_mutex_unlock(&hashlock);
            return;
        }
        idx = HASH(fp->f_id);
        tfp = fh[idx];
        if (tfp == fp) {
            fh[idx] = fp->f_next;
        } else {
            while (tfp->f_next != fp)
                tfp = tfp->f_next;
            tfp->f_next = fp->f_next;
        }
        pthread_mutex_unlock(&hashlock);
        pthread_mutex_unlock(&fp->f_lock);
        pthread_mutex_destroy(&fp->f_lock);
        free(fp);
    } else {
        fp->f_count--;
        pthread_mutex_unlock(&fp->f_lock);
    }
}

void *
thr_fn1(void *arg)
{
    unsigned int seed = 1;

    struct foo * f;

    for (int i = 0; i < ATTEMPTS; i++) {
        int idx = rand_r(&seed) % NHASH;
        f = foo_alloc(idx);
        printf("allocated object with id = %d, address = %p\n", idx, f);
    }
    pthread_exit((void *)0);
}

void *
thr_fn2(void *arg)
{
    unsigned int seed = 3;

    struct foo * f;
    for (int i = 0; i < ATTEMPTS * 3; i++) {
        int idx = rand_r(&seed) % NHASH;
        f = foo_find(idx);
        if (f) {
            printf("found object with id = %d, address = %p\n", idx, f);
        } else {
            printf("object with id = %d was not found\n", idx);
        }
    }
    pthread_exit((void *)0);
}

int
main(void)
{
    srand(time(NULL));
    int err;
    pthread_t tid1, tid2;
    void *tret;

    printf("начали инициализировать структуру данных\n");
    err = pthread_create(&tid1, NULL, thr_fn1, NULL);
    if (err != 0)
        err_exit(err, "невозможно создать поток 1");

    err = pthread_create(&tid2, NULL, thr_fn2, NULL);
    if (err != 0)
        err_exit(err, "невозможно создать поток 2");
    printf("начали читать структуру данных в 1 раз\n");

    err = pthread_join(tid1, &tret);
    if (err != 0)
        err_exit(err, "невозможно присоединить поток 1");
    printf("закончили инициализировать структуру данных\n");

    err = pthread_join(tid2, &tret);
    if (err != 0)
        err_exit(err, "невозможно присоединить поток 2");
    printf("закончили читать структуру данных в 1 раз\n");
    printf("\n\n\n");

    err = pthread_create(&tid2, NULL, thr_fn2, NULL);
    if (err != 0)
        err_exit(err, "невозможно создать поток 2");
    printf("начали читать структуру данных во 2 раз\n");

    err = pthread_join(tid2, &tret);
    if (err != 0)
        err_exit(err, "невозможно присоединить поток 2");
    printf("закончили читать структуру данных во 2 раз\n");

    exit(0);
}
