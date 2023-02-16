#include <string.h>
#include <stdio.h>

#include "common.h"
#include "netconv.h"
#include "unfold.h"

net_t *net;	/* stores the net	*/
unf_t *unf;	/* stores the unfolding */

int exitcode = 0;
int conditions_size, events_size, ce_alloc_step;
int ev_mark;
cond_t  **conditions;			/* condition queue in marking_of   */
event_t **events;			/* event queue in pe_conflict etc. */
trans_t *stoptr = NULL;			/* transition in -T switch         */
int unfold_depth = 0;			/* argument of -d switch	   */
int interactive = 0;			/* interactivem mode (-i)	   */
int compressed = 0;			/* compressed unfolding view (-c)	   */

nodelist_t *cutoff_list, *corr_list;	/* cut-off list, corresponding events */

/*****************************************************************************/
/* The co-relation is kept in the form of an incidence list. Every condition */
/* keeps two lists of conditions such that are concurrent to it. The first list,  */
/* stored in the coarray_common field, stores the conditions that are con-   */
/* current to all conditions with the same event as their singleton preset.  */
/* The second list, coarray_private, contains additional conditions which do */
/* not (necessarily) enjoy this property.				     */

/* Create an empty coarray with 'size' allocated events. */
coa_t alloc_coarray (int size)
{
  coa_t coa;

  coa.size = size;
  coa.inuse = 0;
  coa.conds = MYmalloc((size+1) * sizeof(cond_t*));
  coa.conds[0] = NULL;
  return coa;
}

/* Add a single condition to an array, enlarging the array if necessary. */
void addto_coarray (coa_t *coa, cond_t *co)
{
  if (coa->size == coa->inuse)
  {
    int nsz = coa->size + 1 + coa->size/4;
    coa->conds = MYrealloc(coa->conds,(nsz+1) * sizeof(cond_t*));
    coa->size = nsz;
  }
  coa->conds[coa->inuse++] = co;
  coa->conds[coa->inuse] = NULL;
}

/* Copy an array, truncating it to the necessary size. */
coa_t coarray_copy (coa_t coa)
{
  coa_t ncoa;

  ncoa.size = ncoa.inuse = coa.inuse;
  ncoa.conds = MYmalloc((ncoa.size+1) * sizeof(cond_t*));
  memcpy(ncoa.conds,coa.conds,(ncoa.size+1) * sizeof(cond_t*));
  return ncoa;
}

void print_conditions (cond_t* list)
{
  if (!list) return;
  printf("%s ",list->origin->name);
  print_conditions(list->next);
}

void print_events (event_t* list)
{
  if (!list) return;
  printf("%s ",list->origin->name);
  print_events(list->next);
}

/*****************************************************************************/
/* Insert a condition into the unfolding. The new condition is labelled with */
/* the place pl.							     */

cond_t* insert_condition (place_t *pl, event_t *ev)
{
        cond_t *co = MYmalloc(sizeof(cond_t));
        co->next = unf->conditions;
        unf->conditions = co;

  /* remember relation between co and pl */
  /* the other direction is done in pe() */
  co->origin = pl;
  co->flag = 0;

        co->postset = NULL;
  co->pre_ev = ev;
  co->mark = 0;
  co->num = unf->numco++;
  co->queried = 0;
  if ((ev && nodelist_find(ev->origin->postset, pl)) ||
    (!ev && pl->marked))
    co->token = 1;
  else
    co->token = 0;

  if (interactive)
  {
    printf("Added condition C%d (%s)",co->num,pl->name);
    if (ev) printf(" [event E%d]",ev->id);
    printf(".\n");
  }

  return co;
}

/*****************************************************************************/
/* Insert an event into the unfolding. The new event is derived from	     */
/* qu->trans and the conditions in qu->conds form its preset.		     */

event_t* insert_event (pe_queue_t *qu, char* trans_pool)
{
  
  event_t *ev = MYmalloc(sizeof(event_t));
  int sz = qu->trans->prereset_size;
    cond_t **co_ptr;

  ev->next = unf->events;
  unf->events = ev;
  ev->origin = qu->trans;
  if(!strstr(trans_pool, ev->origin->name))
    strcat(strcat(trans_pool,ev->origin->name), ", ");
  ev->mark = 0;		/* for marking_of */
  ev->foata_level = find_foata_level(qu);
  ev->preset_size = qu->trans->prereset_size;
  ev->postset_size = qu->trans->postreset_size;

  /* add preset (postset comes later) */
        ev->preset = co_ptr = MYmalloc(sz * sizeof(cond_t*));
  memcpy(ev->preset,qu->conds,sz * sizeof(cond_t*));
  
  /* co_ptr2 = ev->preset;
  //printf("event name %s and its preset with size %d: \n", ev->origin->name, sz);
  while(*co_ptr2){	
    printf("EVENT %s precondition name: %s, %d\n", ev->origin->name,(*co_ptr2)->origin->name, (*co_ptr2)->num);
    co_ptr2 = &((*co_ptr2)->next);
  } */
  
  while (sz--) nodelist_push(&((*co_ptr++)->postset),ev);
  
  /* allocate memory for queue in conco_nt if necessary */
  if (++unf->numev >= events_size)
  {
    events_size += ce_alloc_step;
    events = MYrealloc(events,events_size * sizeof(event_t*));
  }
  if (interactive)
  {
    ev->id = qu->id;
    printf("Added event E%d.\n",ev->id);
  }
  
  return ev;
}

/*****************************************************************************/
/* Add the post-conditions of event ev and compute the possible extensions.  */

enum { CUTOFF_NO, CUTOFF_YES };

void add_post_conditions (event_t *ev, char cutoff)
{
  
  cond_t **co_ptr, **cocoptr;
  nodelist_t *list;
  coa_t newarray;
  /* First insert the conditions without putting them in pl->conds;
     that is done by pe() to avoid duplicated new events. */
  ev->postset = co_ptr
    = MYmalloc(ev->postset_size * sizeof(cond_t*));
  
  for (list = nodelist_concatenate(ev->origin->postset, ev->origin->reset); list; list = list->next)
    *co_ptr++ = insert_condition(list->node,ev);
  
  if (cutoff) return;

  /* Having computed the common part of the co-relation for all
     conditions in co_relation(), we create a copy that uses only
     the necessary amount of memory. */
  newarray = coarray_copy(ev->coarray);	
  free(ev->coarray.conds);
  /* Add the reverse half of the concurrency relation. */
  cocoptr = newarray.conds-1;
  while (*++cocoptr)
  {
    co_ptr = ev->postset;
    for (list = nodelist_concatenate(ev->origin->postset, ev->origin->reset); list; list = list->next)
      addto_coarray(&((*cocoptr)->co_private),*co_ptr++);
  }
  
  co_ptr = ev->postset;

  for (list = nodelist_concatenate(ev->origin->postset, ev->origin->reset); list; list = list->next)
  {
    /* record co-relation between new conditions */
    (*co_ptr)->co_common = newarray;
    (*co_ptr)->co_private = alloc_coarray(0);
    cocoptr = ev->postset;
    while (cocoptr != co_ptr)
    {
      addto_coarray(&((*co_ptr)->co_private),*cocoptr);
      addto_coarray(&((*cocoptr)->co_private),*co_ptr);
      cocoptr++;
    }
    
    pe(*co_ptr++);
  }
}

/*****************************************************************************/
/* Function co_relation computes the set of conditions that are concurrent   */
/* to the newly added event ev. That set of conditions is also concurrent to */
/* the postset of ev and forms the common part of their concurrency list.    */
/* The set is essentially computed by taking the intersection of the sets of */
/* conditions concurrent to the input conditions of ev.			     */

typedef struct cqentry_t {
  cond_t *co;
  int index;
  struct cqentry_t *next;
} cqentry_t;

/* This maintains a queue of conditions used for computing the intersection. */
void insert_to_queue (cqentry_t **queue, cqentry_t *newentry,
      cond_t *co, int index)
{
  newentry->co = co;
  newentry->index = index;

  while (*queue && (*queue)->co->num < co->num)
    queue = &((*queue)->next);
  newentry->next = *queue;
  *queue = newentry;
}

void co_relation (event_t *ev)
{
  cond_t  **co_ptr, ***colists;
  int	  evps = ev->preset_size, sz;
  cqentry_t *queue = NULL;
  char	finished = 0, *switched;

  /* Find the maximal potential size of the intersection. */
  int min = 0xfffffff;
  for (sz = evps, co_ptr = ev->preset; sz--; co_ptr++)
  {
    int cosize = (*co_ptr)->co_common.inuse +
        (*co_ptr)->co_private.inuse;
    if (min > cosize) min = cosize;
  }
  ev->coarray = alloc_coarray(min + ev->postset_size);

  /* Add first condition in each list to a "priority queue"
     ordered by condition numbers. */
  colists = MYmalloc(evps * sizeof(cond_t**));
  switched = MYmalloc(evps * sizeof(char));
  for (sz = evps, co_ptr = ev->preset+(evps-1); sz--; co_ptr--)
  {
    colists[sz] = (*co_ptr)->co_common.conds;
    switched[sz] = 0;
    if (!*colists[sz])
    {
      switched[sz] = 1;
      colists[sz] = (*co_ptr)->co_private.conds;
      if (!*colists[sz]) { finished = 1; break; }
    }
    if (*colists[sz])
      insert_to_queue(&queue, MYmalloc(sizeof(cqentry_t)),
          *(colists[sz]++),sz);
  }

  /* Take the next lowest condition out of the queue, check whether
     it occurred in all input conditions, and feed the successors
     into the queue. */
  while (!finished)
  {
    cond_t *minco = queue->co;
    int min = minco->num, count = 0;

    while (queue && queue->co->num == min)
    {
      cqentry_t *tmp = queue;
      int index = tmp->index;
      queue = queue->next;
      count++;

      if (!*colists[index])
      {
        if (!switched[index]++)
          colists[index] = 
            ev->preset[index]->co_private.conds;

        if (!*colists[index])
        {
          free(tmp); finished = 1;
          continue;
        }
      }
      insert_to_queue(&queue,tmp,*(colists[index]++),index);
    }

    if (count == evps) addto_coarray(&(ev->coarray),minco);
  }

  free(colists);
  free(switched);
  while (queue)
  {
    cqentry_t *tmp = queue;
    queue = queue->next;
    free(tmp);
  }
}

void recursive_add (nodelist_t *pre, nodelist_t *current)
{
  if (!pre) return;
  recursive_add(pre->next,current);
  addto_coarray(&(((cond_t*)(pre->node))->co_private),current->node);
  addto_coarray(&(((cond_t*)(current->node))->co_private),pre->node);
}

void recursive_pe (nodelist_t *list)
{
  if (!list) return;
  recursive_pe(list->next);
  recursive_add(list->next,list);
  pe(list->node);
  
}

/*****************************************************************************/

void unfold ()
{
  nodelist_t *list;
  pe_queue_t *qu;
  place_t *pl;
  event_t *ev, *stopev = NULL;
  cond_t  *co;
  int cutoff;
  char trans_pool[(net->maxtrname+2)*(net->numtr)];
  memset( trans_pool, 0, (net->maxtrname+2)*(net->numtr)*sizeof(char) );

  /* create empty unfolding structure */
  unf = nc_create_unfolding();

  /* initializations for backward search algorithms */
  ev_mark = 0;
  events_size = conditions_size = ce_alloc_step
    = (net->maxpre > 2000)? (net->maxpre + 8) & 0xFFFFFFF8 : 2000;
  events = MYmalloc(events_size * sizeof(event_t*));

  /* cut-off events, corresponding events */
  cutoff_list = corr_list = NULL;

  /* init hash table, add initial marking */
  marking_init(); unf->m0 = unf->m0_unmarked = NULL;
  add_marking(list = marking_initial(),NULL);

  if (interactive){
    printf("Initial marking:");
    print_marking_pl(list);
    printf("\n");
  }

  printf("Print initial marking\n");
  print_marking_pl(list);
  printf("\n");
  
  /* initialize PE computation */
  pe_init(list);
  parikh_init();

  for (pl = net->places; pl; pl = pl->next){
    if(!pl->marked && pl->reset != NULL){
      co = insert_condition(pl,NULL);
      co->co_common = alloc_coarray(0);
      co->co_private = alloc_coarray(0);
      nodelist_push(&(unf->m0_unmarked),co);
    }
  }
  /* add initial conditions to unfolding, compute possible extensions */
  for (; list; list = list->next)
  {
    co = insert_condition(pl = list->node,NULL);
    co->co_common = alloc_coarray(0);
    co->co_private = alloc_coarray(0);
    nodelist_push(&(unf->m0),co);
  }
  

  printf("Unfolding initial marking plus resets\n");
  print_marking_co(nodelist_concatenate(unf->m0, unf->m0_unmarked));
  printf("\n");
  recursive_pe(nodelist_concatenate(unf->m0, unf->m0_unmarked));

  /* take the next event from the queue */
  while (pe_qsize)
  {
    int i, e;
    if (interactive) for (;;)
    {
      for (i = 1; i <= pe_qsize; i++)
        if (find_marking(pe_queue[i]->marking, 0)) break;
      if (i <= pe_qsize)
      {
        printf("Event E%d is a cutoff.\n",
          e = pe_queue[i]->id);
        qu = pe_pop(i);
        break;
      }
      printf("\nCurrent event queue:");
      for (i = 1; i <= pe_qsize; i++)
        printf(" E%d",pe_queue[i]->id);
      printf("\nUnfold event E");
      scanf("%d",&e);

      for (i = 1; i <= pe_qsize; i++)
        if (pe_queue[i]->id == e) break;
      if (i <= pe_qsize) { qu = pe_pop(i); break; }
    }
    else
      qu = pe_pop(1);
    
    /* add event to the unfolding */
    ev = insert_event(qu, trans_pool);
    cutoff = add_marking(qu->marking,ev);

    list = format_marking_query();
    for (i = 1; i <= pe_qsize && list; i++)
      if (find_marking(list, 1))
        printf("marking query is present\n");
    
    if (interactive && !cutoff)
    {
      if (!corr_list->node)
        printf("E%d has the initial marking.\n",e);
      else
        printf("E%d has the same marking as E%d.\n",
          e,((event_t*)(corr_list->node))->id);
    }
    pe_free(qu);
    
    /* if we've found the -T transition, stop immediately */
    if (stoptr && ev->origin == stoptr)
    {
      stopev = ev;
      unf->events = unf->events->next;
      break;
    }
    
    
    /* if the marking was already represented in the unfolding,
    we have a cut-off event */
    if (!cutoff) { unf->events = unf->events->next; continue; }
    
    /* compute the co-relation for ev and post-conditions */
    co_relation(ev);

    /* add post-conditions, compute possible extensions */
    add_post_conditions(ev,CUTOFF_NO);
    
  }

  if(strlen(trans_pool) > 2){
    trans_pool[strlen(trans_pool)-2] = 0;
    net->unf_trans = MYstrdup(trans_pool);
  }
  
  /* add post-conditions for cut-off events */
  for (list = cutoff_list; list; list = list->next)
  {
    ev = list->node;
    ev->next = unf->events; unf->events = ev;
    add_post_conditions(ev,CUTOFF_YES);
  }
  
  /* Make sure that stopev is the last event to ensure compatibility
     with Claus Schr�ter's reachability checker (otn). */
  if (stopev)
  {
    exitcode = 2;
    stopev->next = unf->events;
    unf->events = stopev;
  }

  /* release memory that is no longer needed (probably incomplete) */
  pe_finish();
  parikh_finish();
  free(events);
  for (pl = net->places; pl; pl = pl->next)
    nodelist_delete(pl->conds);
    
}
