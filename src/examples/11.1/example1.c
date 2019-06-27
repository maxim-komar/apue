// compile: gcc % ../../../lib/error.c -I ../../../include -lpthread -o /tmp/%.out
// run: /tmp/%.out
// clean: rm -f /tmp/%.out

#include "apue.h"
#include <pthread.h>

pthread_t ntid;

typedef unsigned int uint;

void
printids(const char *s)
{
    pid_t     pid;
    pthread_t tid;

    pid = getpid();
    // мы не можем здесь использовать id потока, который содержится в ntid,
    // так как, если новый поток получит управление первым, то ntid не будет
    // проинициализирован
    tid = pthread_self();
    printf("%s pid %u tid %u (0x%x)\n", s, (uint) pid, (uint) tid, (uint) tid);
}

void *
thr_fn(void *arg)
{
    printids("новый поток: ");
    return ((void *)0);
}

int
main(void)
{
    int err;

    err = pthread_create(&ntid, NULL, thr_fn, NULL);
    if (err != 0)
        err_exit(err, "невозможно создать поток");
    printids("главный поток:");
    sleep(1);
    exit(0);
}
