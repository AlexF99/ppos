// GRR20205646 Alexandre de Oliveira Plugge Freitas

#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#include "ppos_data.h"
#include "ppos.h"
#include "queue.h"

#include <signal.h>
#include <sys/time.h>

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
int tick_counter = 0;

unsigned int time = 0;

// timer and signal action for time preemption
struct sigaction action;
struct itimerval timer;

unsigned int systime()
{
    return time;
}

void print_task(void *ptr)
{
    task_t *elem = ptr;

    if (!elem)
        return;

    elem->prev ? printf("%d", elem->prev->id) : printf("*");
    printf("<%d prio: %d>", elem->id, elem->priority);
    elem->next ? printf("%d", elem->next->id) : printf("*");
}

void task_setprio(task_t *task, int prio)
{
    if (prio < -20 || prio > 20)
    {
        fprintf(stderr, "Prioridade invalida");
        return;
    }
    task_t *aux = task == NULL ? curr : task;
    aux->static_priority = prio;
    aux->priority = aux->static_priority;
}

int task_getprio(task_t *task)
{
    task_t *aux = task == NULL ? curr : task;
    return aux->static_priority;
}

task_t *scheduler()
{
    int tasks_size = queue_size((queue_t *)user_tasks);
    int tasks_size_cp = tasks_size;
    task_t *aux = user_tasks;
    task_t *next = aux;
    while (tasks_size > 0)
    {
        if (aux->priority < next->priority)
            next = aux;

        aux = aux->next;
        tasks_size--;
    }
    tasks_size = tasks_size_cp;
    while (tasks_size > 0)
    {
        if (aux != next)
            aux->priority--;
        aux = aux->next;
        tasks_size--;
    }
    next->priority = next->static_priority;
    return next;
}

void dispatcher()
{
    task_t *next;
    while (user_tasks != NULL && user_tasks->next != NULL && user_tasks->prev != NULL)
    {
        next = scheduler();
        if (next != NULL)
        {
            switch (next->status)
            {
            case READY:
                /* pronta */
                break;
            case TERMINATED:
                /* terminada */
                queue_remove((queue_t **)&user_tasks, (queue_t *)next);
                if (user_tasks != NULL)
                    next = scheduler();
                break;
            case SUSPENDED:
                /* suspensa */
                break;

            default:
                fprintf(stderr, "erro de status da tarefa");
                break;
            }
            user_tasks = next;
            tick_counter = 20;
            if (next && next->next && next->prev)
                task_switch(next);
        }
    }
    task_exit(0);
}

void tick_handler(int signum)
{
    time++;
    curr->processor_time++;
    if (curr->is_usertask)
    {
        if (tick_counter > 0)
            tick_counter--;
        else
            task_yield();
    }
}

int task_init(task_t *task, void (*start_routine)(void *), void *arg)
{
    task->id = task_index++;
    task->status = READY;
    task->creation_time = systime();
    task->activations = 0;
    task->processor_time = 0;
    task_setprio(task, 0);
    getcontext(&(task->context));

    if (task != &MainTask)
    {
        char *stack = malloc(STACKSIZE);
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
    }

    if (task != &Dispatcher)
    {
        task->is_usertask = 1;
        queue_append((queue_t **)&user_tasks, (queue_t *)task);
    }
    else
        task->is_usertask = 0;

    return 0;
}

void ppos_init()
{
    setvbuf(stdout, 0, _IONBF, 0);
    task_init(&MainTask, (void *)NULL, "");
    curr = &MainTask;
    curr->activations = 1;
    task_init(&Dispatcher, dispatcher, "");

    // registra a ação para o sinal de timer SIGALRM (sinal do timer)
    action.sa_handler = tick_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGALRM, &action, 0) < 0)
    {
        perror("Erro em sigaction: ");
        exit(1);
    }

    // ajusta valores do temporizador
    timer.it_value.tv_usec = 1000;    // primeiro disparo, em micro-segundos
    timer.it_value.tv_sec = 0;        // primeiro disparo, em segundos
    timer.it_interval.tv_usec = 1000; // disparos subsequentes, em micro-segundos
    timer.it_interval.tv_sec = 0;     // disparos subsequentes, em segundos

    // arma o temporizador ITIMER_REAL
    if (setitimer(ITIMER_REAL, &timer, 0) < 0)
    {
        perror("Erro em setitimer: ");
        exit(1);
    }
}

int task_switch(task_t *task)
{
    task->activations++;
    task_t *old = curr;
    curr = task;
    swapcontext(&(old->context), &(curr->context));
    return 0;
}

void task_yield()
{
    task_switch(&Dispatcher);
}

void task_accounting(task_t *task)
{
    printf("task %d exit: ", curr->id);
    printf("execution time %d ms. ", systime() - curr->creation_time);
    printf("processor time %d ms. ", curr->processor_time);
    printf("%d activations\n", curr->activations);
}

void task_exit(int exit_code)
{
    curr->status = exit_code;
    task_accounting(curr);
    if (curr == &Dispatcher)
        task_switch(&MainTask);
    else
        task_yield();
}

int task_id()
{
    return curr->id;
}