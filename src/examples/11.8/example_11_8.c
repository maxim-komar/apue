// compile: gcc % ../../../lib/error.c -I ../../../include -lpthread -Wall -o /tmp/%.out
// run: /tmp/%.out
// clean: rm -f /tmp/%.out

#include "apue.h"
#include <pthread.h>

int
main(void)
{
    int err;
    struct timespec tout;
    struct tm *tmp;
    char buf[64];

    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&lock);
    printf("мьютекс заперт\n");
    clock_gettime(CLOCK_REALTIME, &tout);
    tmp = localtime(&tout.tv_sec);
    strftime(buf, sizeof(buf), "%r", tmp);
    printf("текущее время: %s\n", buf);
    tout.tv_sec += 10;
    err = pthread_mutex_timedlock(&lock, &tout);
    clock_gettime(CLOCK_REALTIME, &tout);
    tmp = localtime(&tout.tv_sec);
    strftime(buf, sizeof(buf), "%r", tmp);
    printf("текущее время: %s\n", buf);
    if (err == 0)
        printf("мьютекс снова заперт\n");
    else
        printf("не получилось повторно запереть мьютекс: %s", strerror(err));
    exit(0);
}
