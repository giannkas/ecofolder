#include <stdio.h>

#include "common.h"
#include "netconv.h"
#include "unfold.h"

typedef struct hashcell_t
{
	nodelist_t *marking;
	event_t    *event;
	struct hashcell_t *next;
} hashcell_t;

hashcell_t **hash;
int hash_buckets;

/*****************************************************************************/

void marking_init ()
{
	hash_buckets = net->numpl*4 + 1;
	hash = MYcalloc(hash_buckets * sizeof(hashcell_t*));
}

/*****************************************************************************/
/* Compute the hash value of a marking.					     */
/* TODO: Is this a good hash function?? If not, replace by something better. */
/* Also, this might be the only place that really needs pl->num.	     */

int marking_hash (nodelist_t *marking)
{
	unsigned int val = 0, i = 0;
	while (marking)
	{
		//printf("marking place: %s\n", ((place_t*)(marking->node))->name);
		val += ((place_t*)(marking->node))->num * ++i;
		marking = marking->next;
	}
	return val % hash_buckets;
}

/*****************************************************************************/
/* Check if a marking is already present in the hash table.		     */
/* Return 1 if yes. The given marking is left unchanged.		     */

int find_marking (nodelist_t *marking)
{
	hashcell_t **buck = hash + marking_hash(marking);
	char cmp = 2;

	while (*buck && (cmp = nodelist_compare(marking,(*buck)->marking)) > 0)
		buck = &((*buck)->next);

	return !cmp;
}

/*****************************************************************************/
/* Add a marking to the hash table. It is assumed that marking = Mark([ev]). */
/* Return 1 if the marking was not yet present; otherwise, add ev to the     */
/* list of cut-off events and return 0.					     */

int add_marking (nodelist_t *marking, event_t *ev)
{
	hashcell_t *newbuck;
	hashcell_t **buck = hash + marking_hash(marking);
	char cmp = 2;

	while (*buck && (cmp = nodelist_compare(marking,(*buck)->marking)) > 0)
		buck = &((*buck)->next);

	if (!cmp)	/* marking is already present */
	{
		nodelist_delete(marking);
		nodelist_push(&cutoff_list,ev);
		nodelist_push(&corr_list,(*buck)->event);
		return 0;
	}

	newbuck = MYmalloc(sizeof(hashcell_t));
	newbuck->marking = marking;
	newbuck->event = ev;
	newbuck->next = *buck;
	*buck = newbuck;

	return 1;
}

/*****************************************************************************/
/* Collect the initial marking.						     */

nodelist_t* marking_initial ()
{
	place_t *pl;
	trans_t *tr;
	nodelist_t *list = NULL, *respl = NULL;

	for (pl = net->places; pl; pl = pl->next)
		if (pl->marked) nodelist_insert(&list,pl);
	
	for (tr = net->transitions; tr; tr = tr->next){
		for (respl = tr->reset; respl; respl = respl->next){
			//if (((place_t*)(respl->node))->preset == NULL)
				nodelist_insert(&list,((place_t*)(respl->node)));
		}
	}
	
	return list;
}

/*****************************************************************************/

void print_marking (nodelist_t* list)
{
	if (!list) return;
	printf("%s ",((place_t*)(list->node))->name);
	print_marking(list->next);
}
