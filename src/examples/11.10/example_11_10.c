// compile: gcc % ../../../lib/error.c -I ../../../include -lpthread -Wall -o /tmp/%.out
// run: /tmp/%.out
// clean: rm -f /tmp/%.out

#include "apue.h"
#include <pthread.h>
#include <stdlib.h>
#include <sys/select.h>

#define PROCESS_COUNT 100
#define TIMEOUT_BEFORE_PROCESSING 4000

struct msg {
    struct msg *m_next;
    int id;
    pthread_t thread_id;
};

struct msg *workq;

pthread_cond_t qready = PTHREAD_COND_INITIALIZER;

pthread_mutex_t qlock = PTHREAD_MUTEX_INITIALIZER;

void
mysleep(unsigned int delay)
{
    struct timeval tv;
    tv.tv_sec = delay / 1000;
    tv.tv_usec = (delay % 1000) * 1000;
    select(0, NULL, NULL, NULL, &tv);
}

void
process_msg(int limit)
{
    struct msg *mp;
    for (int i = 0; i < limit; i++) {
        pthread_mutex_lock(&qlock);
        while (workq == NULL)
            pthread_cond_wait(&qready, &qlock);
        mp = workq;
        workq = mp->m_next;
        pthread_mutex_unlock(&qlock);

        //
        mysleep(mp->id);
        printf("[0x%x] [%d] processed message ([0x%x] [%d])\n", (unsigned int) pthread_self(), i, (unsigned int) mp->thread_id, mp->id);
    }
}

void
enqueue_msg(struct msg *mp)
{
    pthread_mutex_lock(&qlock);
    mp->m_next = workq;
    workq = mp;
    pthread_mutex_unlock(&qlock);
    pthread_cond_signal(&qready);
}

void *
thr_fn1(void *arg)
{
    struct msg *mp;
    int i;
    for (i = 0; ; i++) {
        int timeout = rand() % 999;
        mysleep(timeout);
        if ((mp = malloc(sizeof(struct msg))) == NULL)
            err_quit("невозможно аллоцировать память");

        mp->id = rand() % 99;
        mp->thread_id = pthread_self();
        mp->m_next = NULL;
        printf("[0x%x] [%d] enqueued message\n", (unsigned int) mp->thread_id, mp->id);

        enqueue_msg(mp);
    }
    pthread_exit((void *) 0);
}

void *
thr_fn2(void *limit)
{
    mysleep(TIMEOUT_BEFORE_PROCESSING);
    process_msg(*((int *) limit));
    pthread_exit((void *) 0);
}

int
main(void)
{
    int err;
    time_t t;
    srand((unsigned) time(&t));

    pthread_t tid1, tid2, tid3, tid4;
    err = pthread_create(&tid1, NULL, thr_fn1, NULL);
    if (err != 0)
        err_exit(err, "невозможно создать поток");

    err = pthread_create(&tid2, NULL, thr_fn1, NULL);
    if (err != 0)
        err_exit(err, "невозможно создать поток");

    err = pthread_create(&tid3, NULL, thr_fn1, NULL);
    if (err != 0)
        err_exit(err, "невозможно создать поток");

    int limit = PROCESS_COUNT;
    err = pthread_create(&tid4, NULL, thr_fn2, &limit);
    if (err != 0)
        err_exit(err, "невозможно создать поток");

    void *tret;
    err = pthread_join(tid4, &tret);
    if (err != 0)
        err_exit(err, "невозможно присоединить поток");

    exit(0);
}
