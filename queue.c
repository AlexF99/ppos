// GRR20205646 Alexandre de Oliveira Plugge Freitas

#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int queue_size(queue_t *queue)
{
    if (queue == NULL)
        return 0;
    int size = 1;
    queue_t *head = queue;
    queue_t *travel = queue->next;
    while (travel && travel != head)
    {
        travel = travel->next;
        size++;
    }
    return size;
}

void queue_print(char *name, queue_t *queue, void print_elem(void *))
{
    printf("%s: [", name);
    if (queue == NULL)
    {
        printf("]\n");
        return;
    }
    queue_t *head = queue;
    print_elem(head);
    printf(" ");

    queue_t *travel = queue->next;
    while (travel && travel != head)
    {
        print_elem((queue_t *)travel);
        travel = travel->next;
        if (travel != head)
            printf(" ");
    }
    printf("]\n");
}

int queue_append(queue_t **queue, queue_t *elem)
{
    queue_t **head = queue;

    if (elem->next != NULL || elem->prev != NULL)
    {
        fprintf(stderr, "elemento a ser inserido pertence a outra fila\n");
        exit(-1);

        return 1;
    }

    if ((*head) == NULL)
    {
        // fila vazia
        *head = elem;
        (*head)->next = (*head);
        (*head)->prev = (*head);
    }
    else
    {
        // fila com ao menos 1 elemento
        queue_t *h = *head;
        queue_t *last = h->prev;
        h->prev = elem;
        last->next = elem;
        elem->next = h;
        elem->prev = last;
    }
    return 0;
}

int queue_remove(queue_t **queue, queue_t *elem)
{
    if (elem == NULL || *queue == NULL)
    {
        fprintf(stderr, "elemento a ser deletado nulo ou fila nula\n");
        return 1;
    }

    queue_t *travel = *queue;

    if (travel == elem && travel->next == travel) // se fila tiver so 1 elemento
        *queue = NULL;

    while (travel != elem)
    {
        travel = travel->next;
        if (travel == *queue) // elem nao esta em queue
        {
            fprintf(stderr, "elemento a ser deletado nao pertence a fila\n");
            return 1;
        }
    }

    travel->next->prev = travel->prev;
    travel->prev->next = travel->next;

    if (*queue == travel)
        *queue = travel->next;

    elem->next = NULL;
    elem->prev = NULL;

    return 0;
}
