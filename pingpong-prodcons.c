#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "ppos.h"
#include "queue.h"

task_t prod, cons;
semaphore_t s_item, s_buffer, s_vaga;

typedef struct filaint_t
{
    struct filaint_t *prev;
    struct filaint_t *next;
    int value;
} filaint_t;

filaint_t *buffer;

// imprime na tela um elemento da fila (chamada pela função queue_print)
void print_elem(void *ptr)
{
    filaint_t *elem = ptr;

    if (!elem)
        return;

    elem->prev ? printf("%d", elem->prev->value) : printf("*");
    printf("<%d>", elem->value);
    elem->next ? printf("%d", elem->next->value) : printf("*");
}

void producer()
{
    task_sleep(1000);
    int item = rand();
    printf("adding item %d to buffer", item);

    sem_down(&s_vaga);

    sem_down(&s_buffer);
    queue_append((queue_t **)&buffer, (queue_t *)&item);
    sem_up(&s_buffer);
    queue_print("buffer", (queue_t *)buffer, print_elem);

    sem_up(&s_item);
}

void consumer()
{
    sem_down(&s_item);

    sem_down(&s_buffer);
    filaint_t item = *buffer;
    queue_remove((queue_t **)&buffer, (queue_t *)buffer);
    sem_up(&s_buffer);

    sem_up(&s_vaga);

    printf("item removido: %d\n", item.value);
    task_sleep(1000);
}

int main()
{
    printf("main: inicio\n");

    ppos_init();

    printf("aqui");

    // inicia semaforos
    sem_init(&s_vaga, 5); // 5 vagas no buffer
    printf("aqui2");

    sem_init(&s_item, 1); // mutex item
    printf("aqui3");

    sem_init(&s_buffer, 1); // mutex buffer
    printf("aqui4");

    // inicia tarefas
    task_init(&prod, producer, "producer");
    printf("aqui5");

    task_init(&cons, consumer, "\tconsumer");
    printf("aqui6\n");

    task_wait(&prod);
    printf("aqui7\n");

    printf("main: fim\n");
    task_exit(0);

    return 0;
}