#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#include "ppos_data.h"
#include "ppos.h"
#include "queue.h"

#define STACKSIZE 64 * 1024

enum status
{
    TERMINATED = 0,
    READY = 1,
    SUSPENDED = 2,
    RUNNING = 3
};

task_t *curr;
task_t MainTask, Dispatcher;
task_t *user_tasks;
int task_index = 0;

task_t *scheduler()
{
    task_t *aux = user_tasks;
    user_tasks = user_tasks->next;
    return aux;
}

void dispatcher()
{
    task_t *next;
    while (user_tasks != NULL)
    {
        next = scheduler();
        if (next != NULL)
        {
            task_switch(next);
            switch (next->status)
            {
            case READY:
                /* pronta */
                break;
            case TERMINATED:
                queue_remove((queue_t**)&user_tasks, (queue_t*)next);
                /* terminada */
                break;
            case SUSPENDED:
                /* suspensa */
                break;

            default:
                fprintf(stderr, "erro de status da tarefa");
                break;
            }
        }
    }
    task_exit(0);
}

void print_task(void *ptr)
{
    task_t *elem = ptr;

    if (!elem)
        return;

    elem->prev ? printf("%d", elem->prev->id) : printf("*");
    printf("<%d>", elem->id);
    elem->next ? printf("%d", elem->next->id) : printf("*");
}

int task_init(task_t *task, void (*start_routine)(void *), void *arg)
{
    task->id = task_index++;
    task->status = READY;

    char *stack = malloc(STACKSIZE);

    getcontext(&(task->context));

    if (stack)
    {
        (task->context).uc_stack.ss_sp = stack;
        (task->context).uc_stack.ss_size = STACKSIZE;
        (task->context).uc_stack.ss_flags = 0;
        (task->context).uc_link = 0;
    }
    else
    {
        perror("Erro na criação da pilha: ");
        exit(1);
    }

    makecontext(&(task->context), (void *)(*start_routine), 1, (char *)arg);

    if (task != &Dispatcher)
        queue_append((queue_t **)&user_tasks, (queue_t *)task);

    return 0;
}

void ppos_init()
{
    setvbuf(stdout, 0, _IONBF, 0);
    task_init(&MainTask, (void *)NULL, "");
    curr = &MainTask;
    task_init(&Dispatcher, dispatcher, "");
}

int task_switch(task_t *task)
{
    task_t *old = curr;
    curr = task;
    swapcontext(&(old->context), &(curr->context));
    return 0;
}

void task_yield()
{
    task_switch(&Dispatcher);
}

void task_exit(int exit_code)
{
    curr->status = exit_code;
    if (curr == &Dispatcher)
        task_switch(&MainTask);
    else
        task_yield();
}

int task_id()
{
    return curr->id;
}