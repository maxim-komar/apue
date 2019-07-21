// compile: gcc % ../../../lib/error.c -I ../../../include -lpthread -o /tmp/%.out
// run: /tmp/%.out
// clean: rm -f /tmp/%.out

#include "apue.h"
#include <pthread.h>

struct foo {
    int a, b, c, d;
};

void
printfoo(const char *s, const struct foo *fp)
{
    printf("%s", s);
    printf("  структура по адресу 0x%lx\n", (unsigned long)fp);
    printf("  foo.a = %d\n", fp->a);
    printf("  foo.b = %d\n", fp->b);
    printf("  foo.c = %d\n", fp->c);
    printf("  foo.d = %d\n", fp->d);
}

void *
thr_fn1(void *arg)
{
    struct foo foo = {1, 2, 3, 4};
    printfoo("поток 1:\n", &foo);
    pthread_exit((void *) &foo);
}

void *
thr_fn2(void *arg)
{
    printf("поток 2: идентификатор - %lu\n", (unsigned long)pthread_self());
    pthread_exit((void *)0);
}

int
main(void)
{
    int err;
    pthread_t tid1, tid2;
    struct foo *fp;

    err = pthread_create(&tid1, NULL, thr_fn1, NULL);
    if (err != 0)
        err_exit(err, "невозможно создать поток 1");
    err = pthread_join(tid1, (void *)&fp);
    if (err != 0)
        err_exit(err, "невозможно присоединить поток 1");
    sleep(1);
    printf("родительский процесс создает второй поток\n");
    err = pthread_create(&tid2, NULL, thr_fn2, NULL);
    if (err != 0)
        err_exit(err, "невозможно содать поток 2");
    sleep(1);
    printfoo("родительский процесс:\n", fp);
    exit(0);
}

/*
поток 1:
  структура по адресу 0x7f19c61c5ed0
  foo.a = 1
  foo.b = 2
  foo.c = 3
  foo.d = 4
родительский процесс создает второй поток
поток 2: идентификатор - 139748674660096
родительский процесс:
  структура по адресу 0x7f19c61c5ed0
  foo.a = 1
  foo.b = 0
  foo.c = -346285568
  foo.d = 1810381408
*/
