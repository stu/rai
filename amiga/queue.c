#include <stdlib.h>

#include "list.h"
#include "queue.h"


int queue_enqueue(Queue *queue, const void *data)
{
	return slist_ins_next(queue, slist_tail(queue), data);
}


int queue_dequeue(Queue *queue, void **data)
{
	return slist_rem_next(queue, NULL, data);
}
