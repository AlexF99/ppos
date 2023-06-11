#include <stdlib.h>
#include <stdio.h>
#include "ppos.h"

int sem_id = 0;
int sum = 0;
int lock = 0;

void wakeup_sem_queue(task_t **queue)
{
    if (queue == NULL || *queue == NULL)
        return;
    task_t *travel = (*queue)->next;
    while (travel != *queue)
    {
        task_t *task = travel;
        travel = travel->next;
        task_resume(task, queue);
    }
    task_resume(travel, queue);
}

void enter_cs(int *lock)
{
    while (__sync_fetch_and_or(lock, 1))
        ;
}

void leave_cs(int *lock)
{
    (*lock) = 0;
}

int sem_init(semaphore_t *s, int value)
{
    if (s == NULL)
        return -1;
    s->value = value;
    s->queue = NULL;
    s->id = ++sem_id;
    return 0;
}

int sem_down(semaphore_t *s)
{
    if (s == NULL)
        return -1;

    enter_cs(&lock);
    s->value--;
    leave_cs(&lock);
    if (s->value < 0)
    {
        enter_cs(&lock);
        task_suspend(&(s->queue));
        leave_cs(&lock);
        task_yield();
    }
    return 0;
}

int sem_up(semaphore_t *s)
{
    if (s == NULL)
        return -1;

    enter_cs(&lock);
    s->value++;
    leave_cs(&lock);
    if (s->queue != NULL)
    {
        enter_cs(&lock);
        task_t *task = s->queue;
        s->queue = s->queue->next;
        task_resume(task, &(s->queue));
        leave_cs(&lock);
    }
    task_yield();
    return 0;
}

int sem_destroy(semaphore_t *s)
{
    if (s == NULL)
        return -1;

    wakeup_sem_queue(&(s->queue));
    s = NULL;
    return 0;
}