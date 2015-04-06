#if ! defined(CLNODE_H_INCLUDED)
#define CLNODE_H_INCLUDED

struct clnode {
	struct clnode *next, *prev;
};

static struct clnode *clnode_init(void *p)
{
	struct clnode *n = (struct clnode *) p;
	return n ? n->next = n->prev = n : n;
}

static struct clnode *clnode_next(struct clnode *n)
{
	return n->next;
}

static struct clnode *clnode_prev(struct clnode *n)
{
	return n->prev;
}

static void clnode_swapprev(struct clnode *n, struct clnode *m)
{
	struct clnode *n_prev = n->prev;
	struct clnode *m_prev = m->prev;
	n->prev = m_prev;
	m->prev = n_prev;
	n_prev->next = m;
	m_prev->next = n;
}

static void clnode_swapnext(struct clnode *n, struct clnode *m)
{
	struct clnode *n_next = n->next;
	struct clnode *m_next = m->next;
	n->next = m_next;
	m->next = n_next;
	n_next->prev = m;
	m_next->prev = n;
}

static struct clnode *clnode_remove(struct clnode *n)
{
	n->prev->next = n->next;
	n->next->prev = n->prev;
	n->prev = n->next = n;
	return n;
}

static _Bool clnode_is_single(struct clnode *n)
{
	return n->next == n;
}

#endif
