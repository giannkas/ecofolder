#include <stdio.h>

#include "common.h"
#include "netconv.h"
#include "unfold.h"

hashcell_t **hash;
int hash_buckets;
int *rep_marking;


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
    val += ((place_t*)(marking->node))->num * ++i;
    marking = marking->next;
  }
  return val % hash_buckets;
}

/*****************************************************************************/
/* Check if a marking is already present in the hash table.		     */
/* Return number of times the marking is repeated otherwise 0. */
/* The given marking is left unchanged.		     */

int find_marking (nodelist_t *marking, int m_query)
{
  hashcell_t **buck = hash + marking_hash(marking);
  int cmp = 2;
  nodelist_t* list = NULL;

  while (*buck && (cmp = nodelist_compare(marking,(*buck)->marking)) > 0)
    buck = &((*buck)->next);

  if (m_query && !cmp){
    for(list = marking; list && !cmp; list = list->next)
      if(!((place_t*)(list->node))->queried)  cmp = 1;
    if(!cmp && (*buck)->repeat != m_repeat)
      cmp = 1;
  }

  return (*buck) && !cmp ? (*buck)->repeat : !cmp;
}

/*****************************************************************************/
/* Inspecting the cone of an event to see if its corresponding marking was */
/* seen before  */
int check_back(cond_t **conds, int size, event_t *ev)
{
  int i, found = 0;
  for(i = 0; i < size && !found; i++)
    if(conds[i]->pre_ev == ev)
      found = 1;
    else if(conds[i]->pre_ev)
      found = check_back(conds[i]->pre_ev->preset, 
                conds[i]->pre_ev->preset_size, ev);
  return found;
}

/*****************************************************************************/
/* Add a marking to the hash table. It is assumed that marking = Mark([ev]). */
/* Return 1 if the marking was not yet present; otherwise, add ev to the     */
/* list of cut-off events and return 0.					     */

int add_marking (nodelist_t *marking, event_t *ev)
{
  hashcell_t *newbuck;
  int key_mk = marking_hash(marking);
  hashcell_t **buck = hash + key_mk;
  char cmp = 2;
  nodelist_t* list;
  int not_present = 0;

  while (*buck && (cmp = nodelist_compare(marking,(*buck)->marking)) > 0)
    buck = &((*buck)->next);
  
  if(!cmp && mcmillan)
  {
    for(list = (*buck)->pre_events; list; list = list->next)
      if (check_back(ev->preset, ev->preset_size, list->node))
      {
        nodelist_push(&cutoff_list,ev);
        nodelist_push(&corr_list, ((event_t*)((*buck)->pre_events->node)));
      }
    (*buck)->repeat++;
    nodelist_push(&((*buck)->pre_events),ev);
  }

  /* printf("hola\n");
  for(list = marking; list; list = list->next)
    printf("place->name: %s\n", ((place_t*)(list->node))->name);
  printf("chao\n"); */

  if (!cmp && !mcmillan)	/* marking is already present */
  {
    (*buck)->repeat++;
    //nodelist_delete(marking);
    nodelist_push(&cutoff_list,ev);
    nodelist_push(&corr_list, ((event_t*)((*buck)->pre_events->node)));
  }
  else if(!!cmp)
  {
    newbuck = MYmalloc(sizeof(hashcell_t));
    newbuck->marking = marking;
    //newbuck->pre_events = ev;
    nodelist_push(&(newbuck->pre_events),ev);
    newbuck->repeat = 1;
    newbuck->next = *buck;
    *buck = newbuck;
    not_present = 1;
  }

  return not_present;
}

/*****************************************************************************/
/* Collect the initial marking.						     */

nodelist_t* marking_initial ()
{
  place_t *pl;
  nodelist_t *list = NULL;

  for (pl = net->places; pl; pl = pl->next)
    if (pl->marked) nodelist_insert(&list,pl);
  
  return list;
}

/*****************************************************************************/
/* Format the marking query.						     */

nodelist_t* format_marking_query ()
{
  place_t *pl;
  nodelist_t *list = NULL;

  for (pl = net->places; pl; pl = pl->next)
    if (pl->queried) nodelist_insert(&list,pl);
  
  return list;
}

/*****************************************************************************/

void print_marking_pl (nodelist_t* list)
{
  if (!list) return;
  printf("%s ",((place_t*)(list->node))->name);
  print_marking_pl(list->next);
}

void print_marking_co (nodelist_t* list)
{
  if (!list) return;
  printf("%s ",((cond_t*)(list->node))->origin->name);
  print_marking_co(list->next);
}
