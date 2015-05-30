#ifndef QUEUE_H
#define QUEUE_H

#include "list.h"

typedef SList Queue;

#define queue_init slist_init
#define queue_destroy slist_destroy

int queue_enqueue(Queue *queue, const void *data);
int queue_dequeue(Queue *queue, void **data);

#define queue_peek(queue) ((queue)->head == NULL ? NULL : (queue)->head->data)
#define queue_peek_tail(queue) ((queue)->tail == NULL ? NULL : (queue)->tail->data)
#define queue_size slist_size

#endif
