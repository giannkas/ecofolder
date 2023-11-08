/****************************************************************************/
/* netconv.c                                                                */
/*                                                                          */
/* Functions for creating and modifying Petri net elements		    */
/****************************************************************************/

#include <string.h>
#include <stdio.h>

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
  net->numpl = net->numtr = net->numrt = 0;
  net->ign_trans = net->rt_trans = net->unf_trans = "";
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

place_t* nc_create_place (net_t *net, int idpl)
{
  place_t *pl = MYmalloc(sizeof(place_t));
  pl->next = net->places;
  net->places = pl;
  pl->preset = pl->postset = pl->conds =
    pl->reset = pl->ctxset = NULL;
  pl->num = ++net->numpl;
  if (idpl && useids) pl->id = idpl;
  else
    pl->id = net->numpl;
  return pl;
}

trans_t* nc_create_transition (net_t *net, int idtr)
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
  if (idtr && useids) tr->id = idtr;
  else
    tr->id = net->numtr;
  return tr;
}

restr_t* nc_create_restriction (net_t *net)
{
  restr_t *rt = MYmalloc(sizeof(restr_t));
  rt->next = net->restrictions;
  net->restrictions = rt;
  rt->num = ++net->numrt;
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
  place_t *pl;
  int k, sz = 0;

  net->maxpre = net->maxpost = net->maxres = net->maxctx = 0;
  for (tr = net->transitions; tr; tr = tr->next)
  {
    nodelist_t *list;

    if (strlen(tr->name) > sz) sz = strlen(tr->name);

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
  net->maxtrname = sz;
  sz = 0;
  for (pl = net->places; pl; pl = pl->next)
    if (strlen(pl->name) > sz) sz = strlen(pl->name);
  net->maxplname = sz;
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

/*****************************************************************************/
void nc_create_trans_pool (net_t* net)
{
  restr_t *rt;
  trans_t *tr;
  int const_check;
  char *token_rt, *token_pl;
  
  char trans_pool[(net->maxtrname+2)*(net->numtr)];
  memset( trans_pool, 0, (net->maxtrname+2)*(net->numtr)*sizeof(char) );

  for (tr = net->transitions; tr; tr = tr->next){
    const_check = 0;
    for (rt = net->restrictions; rt && !const_check; rt = rt->next){
      nodelist_t *ptr = tr->postset;
      for (; ptr && !const_check; ptr = ptr->next){
        if(strstr(((place_t*)(ptr->node))->name,"+") && strstr(rt->name,"-")){
          token_pl = ftokstr(((place_t*)(ptr->node))->name, 0, '+');
          token_rt = ftokstr(rt->name, 0, '-');
          if(!strcmp(token_pl,token_rt)){ const_check = 1;
          strcat(strcat(trans_pool,tr->name), ", ");}
        } else if (strstr(((place_t*)(ptr->node))->name,"-") && strstr(rt->name,"+")){
          token_pl = ftokstr(((place_t*)(ptr->node))->name, 0, '-');
          token_rt = ftokstr(rt->name, 0, '+');
          if(!strcmp(token_pl,token_rt)){ const_check = 1;
          strcat(strcat(trans_pool,tr->name), ", ");}
        }
      }
    }
  }
  if(strlen(trans_pool) > 2){
    trans_pool[strlen(trans_pool)-2] = 0;
    net->rt_trans = MYstrdup(trans_pool);
  }
}

/*****************************************************************************/

void nc_create_ignored_trans (net_t* net)
{
  trans_t *tr;

  char trans_pool[(net->maxtrname+2)*(net->numtr)];
  memset( trans_pool, 0, (net->maxtrname+2)*(net->numtr)*sizeof(char) );
  
  for (tr = net->transitions; tr; tr = tr->next){
    if (!strstr(net->rt_trans, tr->name) && 
      !strstr(net->unf_trans, tr->name))
      strcat(trans_pool,strcat(tr->name, ", "));
  }
  
  if(strlen(trans_pool) > 2){
    trans_pool[strlen(trans_pool)-2] = 0;
    net->ign_trans = MYstrdup(trans_pool);
  }
}