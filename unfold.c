#include <string.h>
#include <stdio.h>

#include "common.h"
#include "netconv.h"
#include "unfold.h"

net_t *net; /* stores the net */
unf_t *unf; /* stores the unfolding */

#define CO_ALLOC_STEP 1024

int exitcode = 0;
int conditions_size, events_size, ce_alloc_step;
int ev_mark;
cond_t  **conditions;     /* condition queue in marking_of   */
event_t **events;     /* event queue in pe_conflict etc. */
trans_t *stoptr = NULL;     /* transition in -T switch         */
int unfold_depth = 0;     /* argument of -d switch     */
int confmax = 0;      /* display maximal configurations, 
                      it will automatically set interactive mode (-i)    */
int interactive = 0;      /* interactive mode (-i)    */
int compressed = 0;     /* compressed unfolding view (-c)    */
int mcmillan = 0;      /* mcmillan criteria flag (-mcmillan) */
int m_repeat = 0;     /* marking repeat to highlight (-r)    */
int attractors = 0;     /* enabling Ecofolder to extract attractors (-att)    */
int conflsteps = 0;   /* allocating blocks of CO_ALLOC_STEP Bytes */
int** confl_evs = NULL;  /* matrix of events X conditions whether they are 
                          in direct conflict*/

nodelist_t *cutoff_list, *corr_list;  /* cut-off list, corresponding 
  events */
nodelist_t *harmful_list; /* list of bad/harmful events */

/*********************************************************************/
/* 
  The co-relation is kept in the form of an incidence list. Every 
  condition keeps two lists of conditions such that are concurrent to 
  it. The first list, stored in the coarray_common field, stores the 
  conditions that are concurrent to all conditions with the same event 
  as their singleton preset. The second list, coarray_private, contains
  additional conditions which do not (necessarily) enjoy this property.
*/

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

/* 
  Add a single condition to an array, enlarging the array if necessary.
*/
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

/**************************************************************/
/* Insert a condition into the unfolding. The new condition is 
labelled with the place pl. */

cond_t* insert_condition (place_t *pl, event_t *ev, int queried,
  int queryable)
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
  co->queried = queried ? 1 : 0;
  if(ev && co->queried) co->pre_ev->queried = 1;
  if ((ev && nodelist_find(ev->origin->postset, pl)) ||
    (!ev && pl->marked))
    co->token = 1;
  else
    co->token = 0;

  if (interactive)
  {
    printf("Added condition C%d (%s)",(co->num)+1,pl->name);
    if (ev) printf(" [event E%d]",ev->id);
    printf(".\n");
  }
  if(queryable)
  {
    if(ev) nodelist_insert(&((*query)->evscut),ev);
    nodelist_push(&((*query)->cut),co);
    (*query)->szcut++;
  }
  return co;
}

/*******************************************************************/
/* Insert an event into the unfolding. The new event is derived from
 qu->trans and the conditions in qu->conds form its preset.*/

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
  ev->mark = 0;   /* for marking_of */
  ev->queried = 0;
  ev->foata_level = find_foata_level(qu);
  ev->preset_size = qu->trans->prereset_size;
  ev->postset_size = qu->trans->postreset_size;

  /* add preset (postset comes later) */
        ev->preset = co_ptr = MYmalloc(sz * sizeof(cond_t*));
  memcpy(ev->preset,qu->conds,sz * sizeof(cond_t*));
  
  /*if (ev) printf("ev name: %s\n", ev->origin->name);
  co_ptr2 = ev->preset;
  for(i = 0; i < ev->preset_size; i++)
  {
    printf("i: %d, co name: %s and num: %d\n", i, co_ptr2[i]->origin->name, co_ptr2[i]->num + 1);
  } */

  while (sz--)
  { 
    /* if (confmax && nodelist_size((*co_ptr)->postset) < 1)
      nodelist_push(&((*co_ptr++)->postset),ev);
    else if (!confmax) */
    nodelist_push(&((*co_ptr++)->postset),ev);
  }
  
  /* allocate memory for queue in conco_nt if necessary */
  if (++unf->numev >= events_size)
  {
    events_size += ce_alloc_step;
    events = MYrealloc(events,events_size * sizeof(event_t*));
  }
  ev->id = qu->id;
  if (interactive)
  {
    printf("Added event E%d.\n",ev->id);
  }
  
  return ev;
}

/*******************************************************/
/* Add the post-conditions of event ev and compute the 
  possible extensions.  */

enum { CUTOFF_NO, CUTOFF_YES, HARMFUL_YES };

void add_post_conditions (event_t *ev, char cutoff, int queried, 
  int queryable)
{
  
  cond_t **co_ptr, **cocoptr;
  nodelist_t *list;
  coa_t newarray;
  /* First insert the conditions without putting them in pl->conds;
     that is done by pe() to avoid duplicated new events. */
  ev->postset = co_ptr
    = MYmalloc(ev->postset_size * sizeof(cond_t*));
  for (list = nodelist_concatenate(ev->origin->postset, 
    ev->origin->reset); list; list = list->next)
  {
    int tmp_find = nodelist_find(ev->origin->postset, list->node);
    *co_ptr++ = insert_condition(list->node,ev, 
      tmp_find ? queried : 0, tmp_find ?
      queryable : 0);
  }
  
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
    for (list = nodelist_concatenate(ev->origin->postset, 
          ev->origin->reset); list; list = list->next)
      addto_coarray(&((*cocoptr)->co_private),*co_ptr++);
  }
  
  co_ptr = ev->postset;

  for (list = nodelist_concatenate(ev->origin->postset, 
      ev->origin->reset); list; list = list->next)
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

/******************************************************************/
/* 
  Function co_relation computes the set of conditions that are 
  concurrent to the newly added event ev. That set of conditions 
  is also concurrent to the postset of ev and forms the common 
  part of their concurrency list. The set is essentially computed 
  by taking the intersection of the sets of conditions concurrent 
  to the input conditions of ev. 
*/

typedef struct cqentry_t {
  cond_t *co;
  int index;
  struct cqentry_t *next;
} cqentry_t;

/* 
  This maintains a queue of conditions used for computing the 
  intersection. 
*/
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

void co_relation (event_t *ev, pe_queue_t *qu, int check, 
  int queryable)
{
  cond_t  **co_ptr, ***colists;
  int   evps = ev->preset_size, sz;
  cqentry_t *queue = NULL;
  char  finished = 0, *switched;
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

    if (count == evps){
      if(queryable){
        nodelist_t *list, *list2;
        for(list = qu->marking; list; list = list->next)
          if(!nodelist_find(ev->origin->postset, list->node))
            for(list2 = (((place_t*)(list->node))->conds); list2;
               list2 = list2->next)
              if (((cond_t*)(list2->node))->num == minco->num)
                if (minco->origin->queried)
                {
                  if(check)
                  {
                    if(minco->pre_ev)
                      minco->pre_ev->queried = 1;
                    minco->queried = 1;
                  }
                  nodelist_insert(&((*query)->evscut),minco->pre_ev);
                  nodelist_push(&((*query)->cut),minco);
                  (*query)->szcut++;
                }
      }
      addto_coarray(&(ev->coarray),minco);
    }
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

void recursive_queried(querycell_t *qbuck, cond_t **co_ptr, int sz)
{
  for(int i = 0; i < sz; i++)
  {
    if(co_ptr[i]->pre_ev)
    {
      nodelist_insert(&(qbuck->evscut),co_ptr[i]->pre_ev);
      if (qbuck->repeat > 0 && !co_ptr[i]->pre_ev->queried)
        co_ptr[i]->pre_ev->queried = 1;
      recursive_queried(qbuck, co_ptr[i]->pre_ev->preset, 
        co_ptr[i]->pre_ev->preset_size);
    }
    else return;
  }
}


void pred_conds(cond_t **conds_ev1, trans_t *tr1, cond_t **conds_ev2, trans_t* tr2, int id_ev1, int id_ev2)
{
  int conum, i;
  cond_t **co1 = conds_ev1, **co2 = conds_ev2;
  
  for(i = 0; i < tr1->prereset_size; i++)
  {
    conum = co1[i]->num + 1;

    if (confl_evs[id_ev1][conum] == 0)
      confl_evs[id_ev1][conum] = conum;
  }

  for(i = 0; i < tr2->prereset_size; i++)
  {
    conum = co2[i]->num + 1;
    if (confl_evs[id_ev2][conum] == 0)
      confl_evs[id_ev2][conum] = conum;
  }
  
}

int check_conflict(int id_ev1, int id_ev2, int size)
{
  int num1, num2, conflyes = 0;

  for (int i = 1; i < size && !conflyes; i++)
  {
    num1 = confl_evs[id_ev1][i];
    num2 = confl_evs[id_ev2][i];
    if (num1 > 0 && num1 <= unf->numco && num1 == num2)
      conflyes = 1;
  }
  
  return conflyes;
}

/*******************************************************************/

void unfold ()
{
  nodelist_t *list, *mark_qr = NULL, *harmful_marking = NULL;
  pe_queue_t *qu;
  place_t *pl;
  event_t *ev, *stopev = NULL;
  cond_t  *co;
  querycell_t *qbuck;
  int i, cutoff, repeat = 0, check_query, harmful_check;
  int conflsteps = CO_ALLOC_STEP;
  confl_evs = MYmalloc(CO_ALLOC_STEP * sizeof(int*));
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
  mark_qr = retrieve_list("queried");
  harmful_marking = retrieve_list("harmful");
  add_marking(list = retrieve_list("marked"),NULL);
  check_query = nodelist_compare(list, mark_qr);
  if(!check_query)
  {
    qbuck = MYmalloc(sizeof(querycell_t));
    qbuck->repeat = 1;
    qbuck->szcut = 0;
    qbuck->szevscut = 0;
    qbuck->evscut = NULL;
    qbuck->cut = NULL;
    qbuck->next = *query;
    *query = qbuck;
  }

  if (interactive){
    printf("Initial marking:");
    print_marking_pl(list);
    printf("\n");
  }

  if(!attractors && !interactive)
  {  
    printf("Print initial marking\n");
    print_marking_pl(list);
    printf("\n"); 
  }

  /* initialize PE computation */
  pe_init(list);
  parikh_init();

  for (pl = net->places; pl; pl = pl->next){
    if(!pl->marked && pl->reset != NULL){
      co = insert_condition(pl,NULL,0,0);
      co->co_common = alloc_coarray(0);
      co->co_private = alloc_coarray(0);
      nodelist_push(&(unf->m0_unmarked),co);
    }
  }
  /* add initial conditions to unfolding, compute possible extensions */
  for (; list; list = list->next)
  {
    co = insert_condition(pl = list->node,NULL, !check_query ? 
      repeat : 0, !check_query);
    co->co_common = alloc_coarray(0);
    co->co_private = alloc_coarray(0);
    nodelist_push(&(unf->m0),co);
  }
  
  if(!attractors)
  {
    printf("Unfolding initial marking plus resets\n");
    print_marking_co(nodelist_concatenate(unf->m0, unf->m0_unmarked));
    printf("\n");
  }

  recursive_pe(nodelist_concatenate(unf->m0, unf->m0_unmarked));

  if(confmax)
    for (i = 0; i < conflsteps; i++)
      confl_evs[i] = MYcalloc(conflsteps * sizeof(confl_evs));
  /* take the next event from the queue */
  while (pe_qsize)
  {
    int e, ev_choice;
    check_query = 1; harmful_check = 1;
    if(confmax && pe_qsize > unf->numev + unf->numco)
    {
      conflsteps += CO_ALLOC_STEP;
      confl_evs = MYrealloc(confl_evs, conflsteps * sizeof(int*));
      for (i = conflsteps-CO_ALLOC_STEP; i < conflsteps; i++) 
        confl_evs[i] = MYcalloc(conflsteps * sizeof(confl_evs));
    }

    if (interactive) for (;;)
    {
      ev_choice = 1;
      for (i = pe_qsize; i > 0; i--)
        if (find_marking(pe_queue[i]->marking, 0))
          printf("Event E%d (%s) is a cutoff.\n", pe_queue[i]->id, pe_queue[i]->trans->name);

      printf("\nCurrent event queue:");
      for (i = 1; i <= pe_qsize; i++)
        printf(" E%d (%s)",pe_queue[i]->id, pe_queue[i]->trans->name);
      printf("\nUnfold event E");
      scanf("%d",&e);

      if (confmax && pe_qsize > 1)
      {
        for (; ev_choice <= pe_qsize && pe_queue[ev_choice]->id != e; ev_choice++);
        if (ev_choice > pe_qsize) exit(1);
        for (i = 1; i <= pe_qsize; i++)
        {
          if (i != ev_choice)
          {
            pred_conds(pe_queue[ev_choice]->conds, pe_queue[ev_choice]->trans, pe_queue[i]->conds, pe_queue[i]->trans, pe_queue[ev_choice]->id, pe_queue[i]->id);
            if(check_conflict(pe_queue[ev_choice]->id, pe_queue[i]->id, conflsteps))
            {
              qu = pe_pop(i); 
              i = 0; 
              for (ev_choice = 1; ev_choice <= pe_qsize && pe_queue[ev_choice]->id != e; ev_choice++);
            }
          }
        }
      }

      for (i = 1; i <= pe_qsize; i++)
        if (pe_queue[i]->id == e) break;
      if (i <= pe_qsize) { qu = pe_pop(i); break; }
      else exit(1);
    }
    else
      qu = pe_pop(1);

    /* add event to the unfolding */
    ev = insert_event(qu, trans_pool);
    cutoff = add_marking(qu->marking,ev);
    //printf("ev name: %s\n", ev->origin->name);
    
    check_query = nodelist_compare(qu->marking, mark_qr);
    // harmful_check = nodelist_compare(qu->marking, harmful_marking);
    for(list = harmful_marking; list && harmful_check;
      list = list->next)
      if(!nodelist_find(qu->marking, list->node)){
        harmful_check = 0;
      }

    if(!check_query)
    {
      repeat = find_marking(mark_qr, 1);
      if(repeat)
      {
        qbuck = MYmalloc(sizeof(querycell_t));
        qbuck->repeat = repeat;
        qbuck->szcut = 0;
        qbuck->szevscut = 0;
        qbuck->evscut = NULL;
        qbuck->cut = NULL;
        qbuck->next = *query;
        *query = qbuck;
      }
    }

    if (interactive && !cutoff)
    {
      if (!corr_list->node)
        printf("E%d has the initial marking.\n",e);
      else
        printf("E%d has the same marking as E%d.\n",
          e,((event_t*)(corr_list->node))->id);
    }

    /* if we've found the -T transition, stop immediately */
    if (stoptr && ev->origin == stoptr)
    {
      stopev = ev;
      unf->events = unf->events->next;
      break;
    }

    repeat = repeat > 0 && !check_query ? repeat : 0;

    /* compute the co-relation for ev and post-conditions */
    co_relation(ev, qu, repeat, !check_query);

    /* if the marking was already represented in the unfolding,
    we have a cut-off event */
    /* add post-conditions for cut-off events */
    if (!cutoff)
    { 
      unf->events = unf->events->next; 
      add_post_conditions(ev,CUTOFF_YES, repeat, !check_query);
      continue;
    }
    else if(harmful_marking && harmful_check)
    {
      unf->events = unf->events->next; 
      nodelist_push(&harmful_list,ev);
      add_post_conditions(ev,HARMFUL_YES, repeat, !check_query);
      continue;
    }
    else
      add_post_conditions(ev,CUTOFF_NO, repeat, !check_query);
    //printf("cutoff: %d\n", cutoff);

    /* add post-conditions, compute possible extensions */
    pe_free(qu);
  }

  if(strlen(trans_pool) > 2){
    trans_pool[strlen(trans_pool)-2] = 0;
    net->unf_trans = MYstrdup(trans_pool);
  }

  for (list = harmful_list; list; list = list->next)
  {
    ev = list->node;
    ev->next = unf->events; unf->events = ev;
  }

  for (list = cutoff_list; list; list = list->next)
  {
    ev = list->node;
    ev->next = unf->events; unf->events = ev;
  }

  for(qbuck = *query; qbuck; qbuck = qbuck->next)
  {
    for (list = qbuck->cut; list; list = list->next)
      if((co = list->node) && co->pre_ev)
        recursive_queried(qbuck, co->pre_ev->preset,
          co->pre_ev->preset_size);
    qbuck->szevscut = nodelist_size(qbuck->evscut);
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
