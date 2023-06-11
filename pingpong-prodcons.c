#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "ppos.h"
#include "queue.h"

task_t prod1, prod2, prod3, cons1, cons2;
semaphore_t s_item, s_buffer, s_vaga;

int buflen = 0;

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

void producer(void *arg)
{
    for (;;)
    {
        task_sleep(1000);

        int item = rand() % 20;

        filaint_t *elem = malloc(sizeof(filaint_t *));
        elem->next = NULL;
        elem->prev = NULL;
        elem->value = item;

        sem_down(&s_vaga);

        sem_down(&s_buffer);
        queue_append((queue_t **)&buffer, (queue_t *)elem);
        ++buflen;
        printf("%s produziu %d (tem:%d)\n", (char *)arg, item, buflen);
        sem_up(&s_buffer);

        sem_up(&s_item);
    }
}

void consumer(void *arg)
{
    for (;;)
    {
        sem_down(&s_item);

        filaint_t item;

        sem_down(&s_buffer);
        if (buffer != NULL)
        {
            item = *buffer;
            queue_remove((queue_t **)&buffer, (queue_t *)buffer);
            --buflen;
            printf("\t\t\t\t %s consumiu %d (tem:%d)\n", (char *)arg, item.value, buflen);
        }
        sem_up(&s_buffer);

        sem_up(&s_vaga);

        task_sleep(1000);
    }
}

int main()
{
    buffer = NULL;
    printf("main: inicio\n");

    ppos_init();

    // inicia semaforos
    sem_init(&s_vaga, 5);   // 5 (0-4) vagas no buffer
    sem_init(&s_item, 0);   // mutex item
    sem_init(&s_buffer, 1); // mutex buffer

    // inicia produtores
    task_init(&prod1, producer, "producer 1");
    task_init(&prod2, producer, "producer 2");
    task_init(&prod3, producer, "producer 3");

    // inicia consumidores
    task_init(&cons1, consumer, "\tconsumer 1");
    task_init(&cons2, consumer, "\tconsumer 2");

    printf("main: fim\n");
    task_exit(0);

    return 0;
}