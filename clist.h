#if ! defined(CLIST_H_INCLUDED)
#define CLIST_H_INCLUDED

#include "clnode.h"

struct clist {
	struct clnode hdr;
};

static struct clist *clist_init(void *p)
{
	struct clist *l = (struct clist *)p;
	clnode_init(&l->hdr);
	return l;
}

static struct clnode *clist_end(struct clist *list)
{
	return &list->hdr;
}

static _Bool clist_is_empty(struct clist *list)
{
	return clnode_is_single(&list->hdr);
}

static struct clnode *clist_first(struct clist *list)
{
	return clnode_next(&list->hdr);
}

static struct clnode *clist_last(struct clist *list)
{
	return clnode_prev(&list->hdr);
}

static struct clist *clist_push(struct clist *list, struct clnode *n)
{
	clnode_swapprev(n, clist_first(list));
	return list;
}

static struct clist *clist_queue(struct clist *list, struct clnode *n)
{
	clnode_swapprev(n, clist_end(list));
	return list;
}

#endif
