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
    val += ((place_t*)(marking->node))->num * ++i;
    marking = marking->next;
  }
  return val % hash_buckets;
}

/*****************************************************************************/
/* Check if a marking is already present in the hash table.		     */
/* Return 1 if yes. The given marking is left unchanged.		     */

int find_marking (nodelist_t *marking, int m_query)
{
  hashcell_t **buck = hash + marking_hash(marking);
  int cmp = 2;
  nodelist_t* list = NULL;

  while (*buck && (cmp = nodelist_compare(marking,(*buck)->marking)) > 0)
    buck = &((*buck)->next);

  if (m_query && !cmp)
    for(list = marking; list && !cmp; list = list->next){
      if(!((place_t*)(list->node))->queried)  cmp = 1;
      /* printf("place->name: %s\n", ((place_t*)(list->node))->name);
      printf("place->queried: %d\n", ((place_t*)(list->node))->queried); */
    }
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
  nodelist_t* list = NULL;

  //printf("hola\n");
  while (*buck && (cmp = nodelist_compare(marking,(*buck)->marking)) > 0)
    buck = &((*buck)->next);
  
  printf("hola\n");
  for(list = marking; list; list = list->next)
    printf("place->name: %s\n", ((place_t*)(list->node))->name);
  printf("chao\n");

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

  for (pl = net->places; pl; pl = pl->next){
    printf("query name: %s\n", pl->name);
    if (pl->queried) nodelist_insert(&list,pl);
  }
  
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
