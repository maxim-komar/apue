// compile: gcc % ../../../lib/error.c -I ../../../include -lpthread -Wall -o /tmp/%.out
// run: /tmp/%.out
// clean: rm -f /tmp/%.out

#include "apue.h"
#include <pthread.h>
#include <stdlib.h>
#include <sys/select.h>

#define TASKS_NUMBER 10

struct job {
    struct job *j_next;
    struct job *j_prev;
    pthread_t j_id;
};

struct queue {
    struct job       *q_head;
    struct job       *q_tail;
    pthread_rwlock_t  q_lock;
};

int
queue_init(struct queue *qp)
{
    int err;

    qp->q_head = NULL;
    qp->q_tail = NULL;
    err = pthread_rwlock_init(&qp->q_lock, NULL);
    if (err != 0)
        return(err);

    return(0);
}

void
job_insert(struct queue *qp, struct job *jp)
{
    pthread_rwlock_wrlock(&qp->q_lock);
    jp->j_next = qp->q_head;
    jp->j_prev = NULL;
    if (qp->q_head != NULL) {
        qp->q_head->j_prev = jp;
    } else {
        qp->q_tail = jp;
    }
    qp->q_head = jp;
    pthread_rwlock_unlock(&qp->q_lock);
}

void
job_append(struct queue *qp, struct job *jp)
{
    pthread_rwlock_wrlock(&qp->q_lock);
    jp->j_next = NULL;
    jp->j_prev = qp->q_tail;
    if (qp->q_tail != NULL) {
        qp->q_tail->j_next = jp;
    } else {
        qp->q_head = jp;
    }
    qp->q_tail = jp;
    pthread_rwlock_unlock(&qp->q_lock);
}

void
job_remove(struct queue *qp, struct job *jp)
{
    pthread_rwlock_wrlock(&qp->q_lock);
    if (jp == qp->q_head) {
        qp->q_head = jp->j_next;
        if (qp->q_tail == jp) {
            qp->q_tail = NULL;
        } else {
            jp->j_next->j_prev = jp->j_prev;
        }
    } else if (jp == qp->q_tail) {
        qp->q_tail = jp->j_prev;
        jp->j_prev->j_next = NULL;
    } else {
        jp->j_prev->j_next = jp->j_next;
        jp->j_next->j_prev = jp->j_prev;
    }
    pthread_rwlock_unlock(&qp->q_lock);
}

struct job *
job_find(struct queue *qp, pthread_t id)
{
    struct job *jp;

    if (pthread_rwlock_rdlock(&qp->q_lock) != 0) {
        return(NULL);
    }

    for (jp = qp->q_head; jp != NULL; jp = jp->j_next)
    {
        if (pthread_equal(jp->j_id, id))
            break;
    }

    pthread_rwlock_unlock(&qp->q_lock);
    return(jp);
}

void
queue_wait(struct queue *qp)
{
    int err;
    struct job *jp;
    void *tret;

    for(jp = qp->q_head; jp != NULL; jp = jp->j_next)
    {
        err = pthread_join(jp->j_id, &tret);
        if (err != 0)
            err_exit(err, "невозможно присоединить поток");
    }
}

void
mysleep(unsigned int delay)
{
    struct timeval tv;
    tv.tv_sec = delay / 1000;
    tv.tv_usec = (delay % 1000) * 1000;
    select(0, NULL, NULL, NULL, &tv);
}

void *
thr_fn(void *arg)
{
    mysleep(*((int *) arg) * 100);
    pthread_exit((void *) 0);
}

int
main(void)
{
    int err;

    struct queue *qp;
    if ((qp = malloc(sizeof(struct queue))) == NULL)
        err_quit("невозможно аллоцировать память под очередь");

    err = queue_init(qp);
    if (err != 0)
        err_exit(err, "невозможно создать очередь");

    pthread_t tid;
    pthread_t tids[TASKS_NUMBER];

    struct job *jp;
    for (int i = 0; i < TASKS_NUMBER; i++)
    {
        if ((jp = malloc(sizeof(struct job))) == NULL)
            err_quit("невозможно аллоцировать память под задачу");

        err = pthread_create(&tid, NULL, thr_fn, &i);
        if (err != 0)
            err_exit(err, "невозможно создать поток");

        printf("запустили задачу номер %d с id = %u\n", i, (unsigned int) tid);
        jp->j_id = tid;
        tids[i] = tid;
        job_append(qp, jp);
    }

    // 
    int i = 0;
    jp = job_find(qp, tids[i]);
    if (jp != NULL) {
        printf("найдена задача номер %d с id = %u\n", i, (unsigned int) jp->j_id);
    }

    i = TASKS_NUMBER - 1;
    jp = job_find(qp, tids[i]);
    if (jp != NULL) {
        printf("найдена задача номер %d с id = %u\n", i, (unsigned int) jp->j_id);
    }

    queue_wait(qp);

    exit(0);
}
