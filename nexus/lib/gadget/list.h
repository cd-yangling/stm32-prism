/* SPDX-License-Identifier: GPL-2.0*/
/* 本文件参考 Linux list.h 裁剪实现*/
#ifndef __NEXUS_LIB_GADGET_LIST_HEADER__
#define __NEXUS_LIB_GADGET_LIST_HEADER__

#include <stdint.h>

struct list_head
{
	struct list_head *next;
	struct list_head *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

static inline void __list_add(struct list_head *n,
                              struct list_head *prev,
                              struct list_head *next)
{
	next->prev = n;
	n->next = next;
	n->prev = prev;
	prev->next = n;
}

static inline void list_add(struct list_head *n, struct list_head *head)
{
	__list_add(n, head, head->next);
}

static inline void list_add_tail(struct list_head *n, struct list_head *head)
{
	__list_add(n, head->prev, head);
}

static inline void __list_del(struct list_head *prev, struct list_head *next)
{
	next->prev = prev;
	prev->next = next;
}
static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = (struct list_head *)0;
	entry->prev = (struct list_head *)0;
}

static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}

#define container_of(ptr, type, member) \
	((type *)((char *)(ptr) - (uintptr_t)(&((type *)0)->member)))

#define list_first_entry(ptr, type, member) \
	container_of((ptr)->next, type, member)

#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; \
		!list_is_head(pos, (head)); \
		pos = n, n = pos->next)

#endif	//	__NEXUS_LIB_GADGET_LIST_HEADER__
