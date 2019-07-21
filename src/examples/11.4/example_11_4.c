// compile: gcc % ../../../lib/error.c -I ../../../include -lpthread -o /tmp/%.out
// run: /tmp/%.out
// clean: rm -f /tmp/%.out

#include "apue.h"
#include <pthread.h>

void
cleanup(void *arg)
{
    printf("выход: %s\n", (char *)arg);
}

void *
thr_fn1(void *arg)
{
    printf("запуск потока 1\n");
    pthread_cleanup_push(cleanup, "поток 1, первый обработчик");
    pthread_cleanup_push(cleanup, "поток 1, второй обработчик");
    printf("поток 1, регистрация обработчиков закончена\n");
    if (arg)
        return ((void *)1);
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);
    return((void *)1);
}

void *
thr_fn2(void *arg)
{
    printf("запуск потока 2\n");
    pthread_cleanup_push(cleanup, "поток 2, первый обработчик");
    pthread_cleanup_push(cleanup, "поток 2, второй обработчик");
    printf("поток 2, регистрация обработчиков закончена\n");
    if (arg)
        pthread_exit((void *)2);
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);
    pthread_exit((void *)2);
}

int
main(void)
{
    int err;
    pthread_t tid1, tid2;
    void *tret;

    err = pthread_create(&tid1, NULL, thr_fn1, (void *)1);
    if (err != 0)
        err_exit(err, "невозможно создать поток 1");
    err = pthread_create(&tid2, NULL, thr_fn2, (void *)1);
    if (err != 0)
        err_exit(err, "невозможно создать поток 2");
    err = pthread_join(tid1, &tret);
    if (err != 0)
        err_exit(err, "невозможно присоединить поток 1");
    printf("код выхода потока 1: %ld\n", (long)tret);
    err = pthread_join(tid2, &tret);
    if (err != 0)
        err_exit(err, "невозможно присоединить поток 2");
    printf("код выхода потока 2: %ld\n", (long)tret);
    exit(0);
}
