#include "queue.h"

int queue_size (queue_t *queue) {
    if (!queue) return 0;
    int size = 1;
    queue_t *head = queue;
    queue_t *travel = queue->next;
    while (travel != head) {
        travel = travel->next;
        size++;
    }
    return size;
}

void queue_print (char *name, queue_t *queue, void print_elem (void*) ) {}

int queue_append (queue_t **queue, queue_t *elem) {}

int queue_remove (queue_t **queue, queue_t *elem) {}
