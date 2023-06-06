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
#define QUANTUM 20

enum status
{
    TERMINATED = 0,
    READY = 1,
    SUSPENDED = 2,
    RUNNING = 3
};

task_t *curr;
task_t MainTask, Dispatcher;
task_t *ready_queue;
task_t *sleeping_queue;

int alive_tasks = 0;
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
    printf("<%d>", elem->id);
    elem->next ? printf("%d", elem->next->id) : printf("*");
}

void task_setprio(task_t *task, int prio)
{
    if (prio < QUANTUM * -1 || prio > QUANTUM)
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
    if (ready_queue != NULL)
    {
        int tasks_size = queue_size((queue_t *)ready_queue);
        int tasks_size_cp = tasks_size;
        task_t *aux = ready_queue;
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
    return NULL;
}

void wakeup_all(task_t **queue)
{
    task_t *head = *queue;
    while (head != NULL)
    {
        task_t *task = head;
        head = task->next;
        task_resume(task, &head);
    }
}

// wake up tasks that are asleep and can be awakened
void sleeping_awake()
{
    if (sleeping_queue != NULL)
    {
        if (sleeping_queue == sleeping_queue->next && systime() >= sleeping_queue->wakeup_time)
        {
            task_resume(sleeping_queue, &sleeping_queue);
            return;
        }
        if (systime() >= sleeping_queue->wakeup_time)
        {
            task_t *aux = sleeping_queue;
            sleeping_queue = sleeping_queue->next;
            task_resume(aux, &sleeping_queue);
        }
        if (sleeping_queue == NULL)
            return;
        task_t *travel = sleeping_queue->next;
        while (travel != sleeping_queue)
        {
            if (systime() >= travel->wakeup_time)
            {
                task_resume(travel, &sleeping_queue);
                travel = sleeping_queue->next;
            }
            else
            {
                travel = travel->next;
            }
        }
    }
}

void dispatcher()
{
    task_t *next;
    while (alive_tasks > 0)
    {
        sleeping_awake();
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
                if (ready_queue != NULL)
                    next = scheduler();
                break;
            case SUSPENDED:
                fprintf(stderr, "ERRO: tarefa suspensa escalonada\n");
                break;

            default:
                fprintf(stderr, "ERRO: erro de status da tarefa\n");
                break;
            }
            ready_queue = next;
            tick_counter = QUANTUM;
            if (alive_tasks > 0)
                task_switch(next);
        }
    }
    task_exit(0);
}

void tick_handler(int signum)
{
    if (curr->is_usertask)
    {
        if (tick_counter > 0)
            tick_counter--;
        else
            task_yield();
    }
    time++;
    curr->processor_time++;
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
        queue_append((queue_t **)&ready_queue, (queue_t *)task);
        alive_tasks++;
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
    curr->status = TERMINATED;
    curr->exit_code = exit_code;
    if (ready_queue != NULL && curr != NULL)
    {
        queue_remove((queue_t **)&ready_queue, (queue_t *)curr);
        alive_tasks--;
    }
    task_accounting(curr);
    wakeup_all(&(curr->suspended_queue));
    if (curr == &Dispatcher)
        task_switch(&MainTask);
    else
        task_yield();
}

int task_wait(task_t *task)
{
    if (task != NULL && task->status != TERMINATED)
    {
        task_suspend(&(task->suspended_queue));
        task_yield();
        return task->exit_code;
    }
    return -1;
}

void task_suspend(task_t **queue)
{
    curr->status = SUSPENDED;
    if (ready_queue != NULL)
        queue_remove((queue_t **)&ready_queue, (queue_t *)curr);
    queue_append((queue_t **)queue, (queue_t *)curr);
}

void task_resume(task_t *task, task_t **queue)
{
    task->status = READY;
    if (queue != NULL)
        queue_remove((queue_t **)queue, (queue_t *)task);
    queue_append((queue_t **)&ready_queue, (queue_t *)task);
}

void task_sleep(int t)
{
    curr->wakeup_time = systime() + t;
    task_suspend(&sleeping_queue);
    task_yield();
}

int task_id()
{
    return curr->id;
}

int sem_id = 0;
int sum = 0;
int lock = 0;

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

    wakeup_all(&(s->queue));
    s = NULL;
    return 0;
}