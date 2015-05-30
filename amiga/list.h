#ifndef LIST_H
#define LIST_H
#ifdef __cplusplus
extern "C"{
#endif

typedef struct SListElement
{
	void				*data;
	struct SListElement	*next;
} SListElement;

typedef struct SList
{
	int			size;
	void		(*destroy)(void *data);

	SListElement	*head;
	SListElement	*tail;
} SList;

extern void slist_init(SList *list, void (*destroy)(void *data));
extern void slist_destroy(SList *list);
extern int slist_ins_next(SList *list, SListElement *element, const void *data);
extern int slist_rem_next(SList *list, SListElement *element, void **data);

#define slist_size(list) ((list)->size)
#define slist_head(list) ((list)->head)
#define slist_tail(list) ((list)->tail)
#define slist_is_head(list, element) ((element) == (list)->head ? 1 : 0)
#define slist_is_tail(element) ((element)->next == NULL ? 1 : 0)
#define slist_data(element) ((element)->data)
#define slist_next(element) ((element)->next)
#define slist_ins(l, element) (slist_ins_next(l, slist_tail(l), element))


#ifdef __cplusplus
};
#endif
#endif
