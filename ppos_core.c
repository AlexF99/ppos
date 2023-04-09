#include <stdlib.h>
#include <stdio.h>
#include "ppos_data.h"
#include <ucontext.h>

#define STACKSIZE 64 * 1024

task_t *curr, *MainTask;

void ppos_init()
{
    setvbuf(stdout, 0, _IONBF, 0);
}

int task_init(task_t *task, void (*start_routine)(void *), void *arg)
{
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
    curr = task;

    return 0;
}

int task_switch(task_t *task)
{
    task_t *old = curr;
    curr = task;
    swapcontext(&(old->context), &(curr->context));
    return 0;
}

void task_exit(int exit_code)
{
    task_switch(MainTask);
}

int task_id()
{
    return curr->id;
}