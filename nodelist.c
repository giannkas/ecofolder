#include <stdlib.h>

#include "common.h"
#include "netconv.h"

/*****************************************************************************/
/* Allocate a nodelist element. To reduce memory fragmentation, nodelist_t   */
/* elements are allocated in large contingents, and freed cells are kept in  */
/* li_freelist. New allocation requests are served from li_freelist first.   */

#define LI_ALLOC_STEP 1024 /* how many nodelist elements to grab at a time  */

typedef struct contingent_t
{
	nodelist_t *nodes;
	struct contingent_t *next;
} contingent_t;

nodelist_t *li_freelist = NULL;	  /* pointer to freed nodelist elements */
contingent_t *li_contlist = NULL; /* pointer to allocated contingents   */
int li_counter = 0;    /* num of free elements in the latest contingent */

nodelist_t* nodelist_alloc ()
{
	nodelist_t *tmp_nl;
	contingent_t *tmp_co;

	if ((tmp_nl = li_freelist))
	{
		li_freelist = tmp_nl->next;
		return tmp_nl;
	}

	if (li_counter--) return li_contlist->nodes + li_counter;

	tmp_co = MYmalloc(sizeof(contingent_t));
	tmp_co->nodes = MYmalloc(LI_ALLOC_STEP * sizeof(nodelist_t));
	tmp_co->next = li_contlist;
	li_contlist = tmp_co;
	li_counter = LI_ALLOC_STEP-1;

	return li_contlist->nodes + li_counter;
}

/*****************************************************************************/
/* add a node to the front of a list					     */

nodelist_t* nodelist_push (nodelist_t **list, void *node)
{
	nodelist_t *newlist = nodelist_alloc();
	newlist->node = node;
	newlist->next = *list;
	return *list = newlist;
}

/*****************************************************************************/
/* add a place to a list; sort by pointer in ascending order		     */

nodelist_t* nodelist_insert (nodelist_t **list, void *node)
{
	while (*list && node < (*list)->node)
		list = &((*list)->next);

	if (*list && (*list)->node == node) return *list;

	return nodelist_push(list,node);
}

/*****************************************************************************/
/* release a nodelist; move its cells to the "free" list		     */

void nodelist_delete (nodelist_t *list)
{
	nodelist_t *tmp;

	while (list)
	{
		tmp = list;
		list = list->next;
		tmp->next = li_freelist;
		li_freelist = tmp;
	}
}

/*****************************************************************************/
/* compare two lists; return 0 if equal, -1 if list1 < list2, 1 otherwise    */

char nodelist_compare (nodelist_t *list1, nodelist_t *list2)
{
	while (list1 && list2 && list1->node == list2->node)
		list1 = list1->next, list2 = list2->next;

	if (!list1 && !list2) return 0;
	if (!list1 || (list2 && list1->node < list2->node)) return -1;
	return 1;
}

/*****************************************************************************/ 	//*** NEW FUNCTION ***//
/* concatenate two lists and return the join    */

nodelist_t* nodelist_concatenate(nodelist_t *list1, nodelist_t *list2)
{	
	nodelist_t *tmp;
	// printf("nodelist_concatenate");
	tmp = list1;
	while (tmp && tmp->next != NULL)
		tmp = tmp->next;
	tmp->next = list2;

	return list1;
}
