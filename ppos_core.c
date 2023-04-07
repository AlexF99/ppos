#include <stdio.h>
#include "ppos_data.h"
#include <ucontext.h>

task_t curr;

void ppos_init()
{
    setvbuf(stdout, 0, _IONBF, 0);
}

int task_init(task_t *task, void (*start_routine)(void *), void *arg) {}

int task_switch(task_t *task) {}

void task_exit(int exit_code) {}

int task_id()
{
    return curr.id;
}