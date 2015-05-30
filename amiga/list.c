#include <stdlib.h>
#include <string.h>
#include "list.h"


void slist_init(SList *list, void (*destroy)(void *data))
{
	list->size = 0;
	list->destroy = destroy;
	list->head = NULL;
	list->tail = NULL;

	return;
}

void slist_destroy(SList *list)
{
	void	*data;

	while(slist_size(list) > 0)
	{
		if(slist_rem_next(list, NULL, (void **)&data) == 0 && list->destroy != NULL)
		{
			list->destroy(data);
		}
	}

	memset(list, 0, sizeof(SList));
	return;
}

int slist_ins_next(SList *list, SListElement *element, const void *data)
{
	SListElement	*new_element;

	if((new_element = (SListElement *)malloc(sizeof(SListElement))) == NULL)
		return -1;

	new_element->data = (void *)data;

	if(element == NULL)
	{
		if (slist_size(list) == 0)
			list->tail = new_element;

		new_element->next = list->head;
		list->head = new_element;
	}
	else
	{
		if (element->next == NULL)
			list->tail = new_element;

		new_element->next = element->next;
		element->next = new_element;
	}

	list->size++;
	return 0;
}



int slist_rem_next(SList *list, SListElement *element, void **data)
{
	SListElement	*old_element;

	if(slist_size(list) == 0)
		return -1;

	if(element == NULL)
	{
		*data = list->head->data;
		old_element = list->head;
		list->head = list->head->next;

		if (slist_size(list) == 1)
			list->tail = NULL;
	}
	else
	{
		if(element->next == NULL)
			return -1;

		*data = element->next->data;
		old_element = element->next;
		element->next = element->next->next;

		if (element->next == NULL)
			list->tail = element;
	}

	free(old_element);

	list->size--;
	return 0;
}

