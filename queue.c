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
        printf("entrou aqui\n");
        // fila vazia
        *head = elem;
        (*head)->next = (*head);
        (*head)->prev = (*head);
    }
    else
    {
        printf("entrou nessoutro\n");

        queue_t *travel = *head;
        if (travel->next == *head)
        {
            travel->next = elem;
            travel->prev = elem;
            travel->next->next = *head;
            travel->next->prev = *head;
        }
        else
        {
            queue_t *last = travel->prev;
            travel->prev = elem;
            travel->prev->prev = last;
            last->next = elem;
        }
    }
    return 0;
}

int queue_remove(queue_t **queue, queue_t *elem)
{
    return 0;
}
