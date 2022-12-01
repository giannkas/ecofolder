/****************************************************************************/
/* netconv.c                                                                */
/*                                                                          */
/* Functions for creating and modifying Petri net elements		    */
/****************************************************************************/

#include <string.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "netconv.h"
#include "unfold.h"

/****************************************************************************/
/* nc_create_net							    */
/* Creates a new net without places or transitions.			    */

net_t* nc_create_net()
{
	net_t *net = MYmalloc(sizeof(net_t));
	net->places = NULL;
	net->transitions = NULL;
  net->restrictions = NULL;
	net->numpl = net->numtr = net->numct = 0;
	return net;
}

unf_t* nc_create_unfolding()
{
	unf_t *unf = MYmalloc(sizeof(unf_t));
	unf->conditions = NULL;
	unf->events = NULL;
	unf->numco = unf->numev = 0;
	return unf;
}

/****************************************************************************/
/* nc_create_{place,transition}						    */
/* Creates a new place or transition in the given net. The new node has no  */
/* incoming or outgoing arcs and is unmarked.				    */

place_t* nc_create_place (net_t *net)
{
	place_t *pl = MYmalloc(sizeof(place_t));
	pl->next = net->places;
	net->places = pl;
	pl->preset = pl->postset = pl->conds =
		pl->reset = pl->ctxset = NULL;
	pl->num = ++net->numpl;
	return pl;
}

trans_t* nc_create_transition (net_t *net)
{
	trans_t *tr = MYmalloc(sizeof(trans_t));
	tr->next = net->transitions;
	net->transitions = tr;
	tr->preset = tr->postset = tr->reset = tr->ctxset = NULL;
	tr->preset_size = tr->reset_size = tr->ctxset_size =
		tr->prereset_size = tr->postreset_size = 0;
		// prereset_size is the sum of preset_size + reset_size
		// dropping out those in common. Same for postreset_size.
	tr->num = ++net->numtr;
	return tr;
}

restr_t* nc_create_restriction (net_t *net)
{
  restr_t *rt = MYmalloc(sizeof(restr_t));
  rt->next = net->restrictions;
  net->restrictions = rt;
  rt->num = ++net->numct;
  return rt;
}

/****************************************************************************/
/* nc_create_arc							    */
/* Create an arc between two nodes (place->transition or transition->place) */

void nc_create_arc (nodelist_t **fromlist, nodelist_t **tolist,
		   void *from, void *to)
{
	nodelist_t *list;

	for (list = *fromlist; list; list = list->next)
		if (list->node == to) return;

	nodelist_push(fromlist,to);
	nodelist_push(tolist,from);
}

/****************************************************************************/
/* nc_compute_sizes							    */
/* compute (maximal) sizes of transition presets/postsets/resets		    */

void nc_compute_sizes (net_t *net)
{
	trans_t *tr;	
	int k;

	net->maxpre = net->maxpost = net->maxres = net->maxctx = 0;
	for (tr = net->transitions; tr; tr = tr->next)
	{
		nodelist_t *list;

		for (k = 0, list = tr->preset; list; k++, list = list->next);
		tr->preset_size = k;
		if (net->maxpre < k) net->maxpre = k;

		for (k = 0, list = tr->postset; list; k++, list = list->next);
		tr->postset_size = k;
		if (net->maxpost < k) net->maxpost = k;

		for (k = 0, list = tr->reset; list; k++, list = list->next);
		tr->reset_size = k;
		if (net->maxres < k) net->maxres = k;

		for (k = 0, list = tr->ctxset; list; k++, list = list->next);
		tr->ctxset_size = k;
		if (net->maxctx < k) net->maxctx = k;

		for (k = 0, list = nodelist_concatenate(tr->preset, tr->reset); list; k++, list = list->next);
		tr->prereset_size = k;

		for (k = 0, list = nodelist_concatenate(tr->postset, tr->reset); list; k++, list = list->next);
		tr->postreset_size = k;
	}
}

/*****************************************************************************/

void nc_static_checks (net_t* net, char *stoptr_name)
{
	place_t *pl;
	trans_t *tr;

	for (tr = net->transitions; tr; tr = tr->next)
	{
		if (!tr->preset)
			nc_warning("%s is not restricted",tr->name);
		if (stoptr_name && tr->name && !strcmp(tr->name,stoptr_name))
			stoptr = tr;
	}

	if (stoptr_name && !stoptr)
		nc_error("Transition %s not found",stoptr_name);

	for (pl = net->places; pl; pl = pl->next)
		if (pl->marked) break;
	if (!pl) nc_error("no initial marking");
}
