#include <stdio.h>
#include <string.h>

#include "common.h"
#include "netconv.h"
#include "unfold.h"

/*****************************************************************************/

parikh_t *parikh;
parikh_t *pa_last;

int parikh_count;	/* counts the total number of elements in the vector */
int parikh_size;	/* counts the number of different transitions        */

/*****************************************************************************/

void parikh_init ()
{
	if (net->numtr > 0xFFFE)
	{
		fprintf(stderr,"Parikh vector has too few entries.\n");
		exit(1);
	}
	parikh = MYmalloc((net->numtr + 2) * sizeof(parikh_t));
}

void parikh_finish ()
{
	free(parikh);
}

/*****************************************************************************/
/* Set the Parikh vector to empty.					     */

void parikh_reset ()
{
	/* The entries 0 and 1 serve as boundary elements. */
	parikh[0].tr_num = 0;
	parikh[1].tr_num = 0;

	parikh_count = 0;
	parikh_size = 0;

	/* Assuming that causally related events are due to transitions
	   whose numbers are near to each other, we start searching for
	   additional transitions near the last insertion.		*/
	pa_last = parikh + 1;
}

/*****************************************************************************/
/* Add an appearance of transition number tr_num.			     */

void parikh_add (int tr_num)
{
	parikh_count++;
	if (tr_num < pa_last->tr_num)
	{	/* smaller than last insertion - search downwards */
		if (tr_num < parikh[1].tr_num)
			pa_last = parikh + 1;
		else	
		{
			while (tr_num < (--pa_last)->tr_num);
			if (tr_num == pa_last->tr_num) goto parikh_add;
			else pa_last++;
		}
	}
	else if (tr_num > pa_last->tr_num)
	{	/* larger than last insertion - search upwards */
		if (tr_num > parikh[parikh_size].tr_num)
			pa_last = parikh + parikh_size + 1;
		else	
		{
			while (tr_num > (++pa_last)->tr_num);
			if (tr_num == pa_last->tr_num) goto parikh_add;
		}
	}
	else
		goto parikh_add;

	/* insert one appearance of tr_num at pa_last */
	memmove(pa_last+1, pa_last, sizeof(parikh_t)
		* (2 + parikh_size - (pa_last - parikh)));
	pa_last->tr_num = tr_num;
	pa_last->appearances = 1;
	parikh_size++;
	return;

	parikh_add: pa_last->appearances++;
}

/*****************************************************************************/
/* Finish vector and return a copy.					     */

parikh_t* parikh_save ()
{
	parikh_t *vector = MYmalloc(++parikh_size * sizeof(parikh_t));
	parikh[parikh_size].tr_num = 0; /* mark the last array element */
	memcpy(vector, parikh+1, parikh_size * sizeof(parikh_t));
	return vector;
}

/*****************************************************************************/
/* Compares two Parikh vectors pe1 and pe2.				     */
/* Returns -1 if pe1 < pe2, and 1 if pe1 > pe2, 0 if pe1 == pe2.	     */ 

int parikh_compare (parikh_t *pv1, parikh_t *pv2)
{
	while (pv1->tr_num && pv2->tr_num && (pv1->tr_num == pv2->tr_num)
			&& (pv1->appearances == pv2->appearances))
		pv1++, pv2++;

	if (!pv1->tr_num && !pv2->tr_num) return 0;
	if (pv1->tr_num == pv2->tr_num)
		return pv2->appearances - pv1->appearances;
	else
		return pv1->tr_num - pv2->tr_num;
}

/*****************************************************************************/
/* Computes ordering information for the possible extension e=(tr,pe_conds)  */
/* and returns a queue entry with that information. The ordering information */
/* consists of the size and Parikh vector of [e]; the Foata normal form is   */
/* computed only when necessary.					     */

// SEE THIS!!!!!!

pe_queue_t* create_queue_entry (trans_t *tr)
{
	pe_queue_t *qu_new;
	event_t *ev, **queue;
	cond_t  *co, **co_ptr;
	nodelist_t *list = NULL, *resconf;
	nodelist_t *tr_prev;
	int sz;
	static int queuecount = 0;
	
	ev_mark++;
	*(queue = events) = NULL;
	parikh_reset();
	parikh_add(tr->num);
	/* add the input events of the pre-conditions into the queue */
	//for (sz = tr->preset_size, co_ptr = pe_conds; sz--; )
	for (sz = tr->prereset_size, co_ptr = pe_conds; sz--; ) 			//*** NEW ***//
	{
		(co = *co_ptr++)->mark = ev_mark;
		if ((ev = co->pre_ev) && ev->mark != ev_mark)
			(*++queue = ev)->mark = ev_mark;
	}
	

	while ((ev = *queue))
	{
		queue--;
		parikh_add(ev->origin->num);

		/* add the immediate predecessor events of ev to the queue */
		//for (sz = ev->origin->preset_size, co_ptr = ev->preset; sz--; )
		for (sz = ev->origin->prereset_size, co_ptr = ev->preset; sz--; )	//*** NEW ***//
		{
			(co = *co_ptr++)->mark = ev_mark;
			if ((ev = co->pre_ev) && ev->mark != ev_mark)
				(*++queue = ev)->mark = ev_mark;
		}
	}

	/* create the queue element */
        qu_new = MYmalloc(sizeof(pe_queue_t));
        qu_new->trans = tr;

	if (interactive)
	{
		qu_new->id = ++queuecount;
		int i = tr->prereset_size;
		printf("Discovered new extension E%d (%s) [condition%s",
			qu_new->id, tr->name, i == 1? "" : "s");	//*** NEW ***//
		while (i) { printf(" C%d",pe_conds[--i]->num); }
		printf("].\n");
	}

        /* copy the pre-conditions */
        qu_new->conds = MYmalloc((tr->prereset_size) * sizeof(cond_t*));		//*** NEW ***//
        memcpy(qu_new->conds,pe_conds,(tr->prereset_size)*sizeof(cond_t*));

	/* copy Parikh vector */
	qu_new->p_vector = parikh_save();
	qu_new->lc_size = parikh_count;

	/* now compute the marking */
	ev_mark++;
	*(queue = events) = NULL;
	qu_new->marking = NULL;

	//for (sz = tr->preset_size, co_ptr = pe_conds; sz--; )
	for (sz = tr->prereset_size, co_ptr = pe_conds; sz--; )	//*** NEW ***//
		if ((ev = (*co_ptr++)->pre_ev) && ev->mark != ev_mark)
			(*++queue = ev)->mark = ev_mark;

	while ((ev = *queue))
	{
		queue--;

		
		/* check off the postset conditions */
		//for (sz = ev->origin->postset_size, co_ptr = ev->postset; sz--;)
		for (sz = ev->origin->postreset_size,
			 co_ptr = ev->postset; sz--;){		//*** NEW ***//
			/* printf("event name: %s\n", ev->origin->name);
			printf("condition name: %s\n", (*co_ptr)->origin->name);
			printf("transition name: %s\n", tr->name);
			printf("ev->postset_size: %d\n", ev->postset_size);
			printf("postreset_size: %d\n", ev->origin->postreset_size);
			printf("co_ptr size: %lu\n", (&co_ptr)[1] - co_ptr);
			printf("ev->postset size: %lu\n", sizeof(ev->postset) / sizeof(ev->postset[0])); */
			if ((co = *co_ptr++)->mark != ev_mark-1){
				//printf("ev_mark-1: %d\n", ev_mark-1);
				nodelist_insert(&(qu_new->marking),co->origin);
			}
			//printf("(co = *co_ptr++)->mark: %d\n", (co = *co_ptr++)->mark);
		}
		//printf("hola1\n");
		/* add the immediate predecessor events of ev to the queue */
		// for (sz = ev->origin->preset_size, co_ptr = ev->preset; sz--; )
		for (sz = ev->origin->prereset_size, co_ptr = ev->preset; sz--; )		//*** NEW ***//
			if ((ev = (*co_ptr++)->pre_ev) && ev->mark != ev_mark)
				(*++queue = ev)->mark = ev_mark;
	}
	
	/* add the post-places of tr */
	//for (list = tr->postset; list; list = list->next)
	for (resconf = tr->preset; resconf; resconf = resconf->next){
		tr_prev = ((place_t*)(resconf->node))->preset;
		if (((place_t*)(resconf->node))->reset &&
			(!tr_prev ||
			(tr_prev &&
			!nodelist_find(((trans_t*)(tr_prev->node))->postset, resconf->node)))){
			list = nodelist_concatenate(list, resconf);
			//printf("place name with reset set: %s", ((place_t*)(resconf))->name);
		}
		/* if (((place_t*)(resconf->node))->reset &&
			tr_prev &&
			!nodelist_find(((trans_t*)(tr_prev->node))->postset, resconf)){
			print_marking(((trans_t*)(tr_prev->node))->postset);
			printf("\nresconf: %s\n", ((place_t*)(resconf->node))->name);			
			printf("!nodelist_find result: %d\n", !nodelist_find(((trans_t*)(tr_prev->node))->postset, resconf));
		} */
	}
	list = nodelist_concatenate(list, tr->postset);
	for (list = nodelist_concatenate(list,tr->reset); list; list = list->next)	//*** NEW ***//
		nodelist_insert(&(qu_new->marking), list->node);
	/* add the places of unconsumed minimal conditions */
	for (list = unf->m0; list; list = list->next)
		if ((co = list->node)->mark != ev_mark-1)
			nodelist_insert(&(qu_new->marking), co->origin);

	if (interactive)
	{
		printf("  Corresponding marking:");
		print_marking(qu_new->marking);
		printf("\n");
	}

	return qu_new;
}

/*****************************************************************************/
/* Determine the Foata level of a possible extension.			     */

int find_foata_level (pe_queue_t *pe)
{
	int level = 1;
	int sz = pe->trans->prereset_size;	//*** NEW ***//
        cond_t **co_ptr = pe->conds;
	event_t *ev;

	while (sz--)
	{
		if ((ev = (*co_ptr++)->pre_ev) && ev->foata_level >= level)
			level = ev->foata_level + 1;
	}

	return level;
}

/*****************************************************************************/
/* Identify the "slices" of the Foata normal form in the local configuration */
/* of the possible extension pe.					     */

nodelist_t** create_foata_lists (pe_queue_t *pe)
{
	int sz, tr_level = find_foata_level(pe);
	nodelist_t **fo = MYcalloc((tr_level + 2) * sizeof(nodelist_t*));
	event_t *ev, **queue;
	cond_t **co_ptr;

	ev_mark++;
	*(queue = events) = NULL;

	//for (sz = pe->trans->preset_size, co_ptr = pe->conds; sz--; )
	for (sz = pe->trans->prereset_size, co_ptr = pe->conds; sz--; )	//*** NEW ***//
		if ((ev = (*co_ptr++)->pre_ev) && ev->mark != ev_mark)
			(*++queue = ev)->mark = ev_mark;

	nodelist_push(fo + tr_level,pe->trans);

	while ((ev = *queue))
	{
		--queue;
		nodelist_push(fo + ev->foata_level, ev->origin);

		//for (sz = ev->origin->preset_size, co_ptr = ev->preset; sz--; )
		for (sz = ev->origin->prereset_size, co_ptr = ev->preset; sz--; )	//*** NEW ***//
			if ((ev = (*co_ptr++)->pre_ev) && ev->mark != ev_mark)
				(*++queue = ev)->mark = ev_mark;
	}

	return fo;
}
			
/*****************************************************************************/
/* Compare the Foata normal form of two local configurations.		     */

int foata_compare (pe_queue_t *pe1, pe_queue_t *pe2)
{
	/* create the Foata "slices" for both configurations */
	nodelist_t **fo1 = create_foata_lists(pe1);
	nodelist_t **fo2 = create_foata_lists(pe2);
	nodelist_t **f1 = fo1+1, **f2 = fo2+1, *list;
	parikh_t *pv1;
	int res = 0, pc1;

	while (*f1 && *f2)	/* compare Parikh vectors, level by level */
	{
		parikh_reset();
		for (list = *f1; list; list = list->next)
			parikh_add(((trans_t*)(list->node))->num);
		pc1 = parikh_count; pv1 = parikh_save();

		parikh_reset();
		for (list = *f2; list; list = list->next)
			parikh_add(((trans_t*)(list->node))->num);
		parikh[++parikh_size].tr_num = 0;

		res = pc1 - parikh_count;
		if (!res) res = parikh_compare(pv1,parikh+1);
		free(pv1);
		if (res) break;

		f1++, f2++;
	}

	for (f1 = fo1+1; *f1; f1++) nodelist_delete(*f1);
	for (f2 = fo2+1; *f2; f2++) nodelist_delete(*f2);
	free(fo1);
	free(fo2);

	return res;	/* should never return 0 */
}

/*****************************************************************************/
/* Compares two possible extensions according to the <_E order from [ERV02]. */
/* Returns -1 if pe1 < pe2, and 1 if pe1 > pe2, 0 otherwise (can't happen?). */

int pe_compare (pe_queue_t *pe1, pe_queue_t *pe2)
{
	int res;

	/* Try to decide by local configuration size first. */
	if (pe1->lc_size < pe2->lc_size) return -1;
	if (pe1->lc_size > pe2->lc_size) return 1;

	/* Then decide by comparing the Parikh vectors. */
	if ((res = parikh_compare(pe1->p_vector,pe2->p_vector)))
		return res;

	/* Finally, decide by Foata normal form. */
	return foata_compare(pe1,pe2);
}
