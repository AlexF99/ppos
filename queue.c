#include "queue.h"
#include <stdio.h>

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
    queue_t *head = queue;
    print_elem(head);
    queue_t *travel = queue->next;
    while (travel && travel != head)
    {
        print_elem(travel);
        travel = travel->next;
    }
}

int queue_append(queue_t **queue, queue_t *elem)
{
    queue_t **head = queue;
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
        return 1;

    if (*queue == elem) {
        printf("oi\n");
        *queue = (*queue)->next;
        (*queue)->prev = (*queue)->prev->prev;
    }
    

    return 0;
}
