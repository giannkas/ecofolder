#include <stdio.h>
#include <string.h>

#include "common.h"
#include "netconv.h"
#include "unfold.h"

void* reverse_list (void *ptr)
{
  place_t *newlist = NULL, *list = ptr;

  while (list)
  {
    place_t *tmp = list->next;
    list->next = newlist;
    newlist = list;
    list = tmp;
  }

  return newlist;
}

void write_mci_file (char *filename)
{
  #define write_int(x) fwrite(&(x),sizeof(int),1,file)

  FILE *file;
  place_t *pl;
  trans_t *tr;
  cond_t *co;
  event_t *ev;
  nodelist_t *list1, *list2;
  int ev_num = 0, sz = 0, null = 0, once = 0, tmp = 0;

  if (!(file = fopen(filename,"wb")))
    nc_error("cannot write to file %s\n",filename);


  write_int(unf->numco);
  write_int(unf->numev);
  if(!data)
  {  
    printf("unf->numev: %d\n", unf->numev);
    printf("unf->numco: %d\n", unf->numco);
  }
  
  /* Reverse the lists of places, events etc. This is to maintain
    compatibility with RdlcheckMcM and mcsmodels, which expect events
    to be numbered in accordance with the causality relation. */
  net->places = reverse_list(net->places);
  net->transitions = reverse_list(net->transitions);
  unf->conditions = reverse_list(unf->conditions);
  unf->events = reverse_list(unf->events);


  for (ev = unf->events; ev; ev = ev->next){
    ev->mark = ++ev_num;
    if(ev->queried && !data)
    {
      if(!once)
      {
        printf("chain of events to reach the queried marking:\n  ");
        once = 1;
      }
      printf("%s (e%d)  ", ev->origin->name, ev->mark);
    }
  }
  //if(!data) printf("\n");


  querycell_t *qbuck;
  for(qbuck = *query; qbuck; qbuck = qbuck->next)
  {
    write_int(qbuck->repeat);
    write_int(qbuck->szcut);
    write_int(qbuck->szevscut);
    qbuck->cut = nodelist_reverse(qbuck->cut);
    qbuck->evscut = nodelist_reverse(qbuck->evscut);
    for(list1 = qbuck->cut; list1; list1 = list1->next)
    {
      if((co = list1->node))
      {
        tmp = co->num + 1;
        write_int(tmp);
      }
    }
    for(list1 = qbuck->evscut; list1; list1 = list1->next)
      if((ev = list1->node))
        write_int(ev->mark);
  }
  write_int(null);

  
  for (ev = unf->events; ev; ev = ev->next){
    //printf("ev name: %s\n", ev->origin->name);
    write_int(ev->origin->num);
    write_int(ev->queried);
  }

  for (co = unf->conditions; co ; co = co->next)
  {
    write_int(co->origin->num);
    write_int(co->token);
    write_int(co->queried);

    if (co->pre_ev) 
      write_int(co->pre_ev->mark);
      else	write_int(null);
    for (list1 = co->postset; list1; list1 = list1->next)
      write_int((ev = list1->node)->mark);
    write_int(null);
  }
  
  for (list1 = harmful_list; list1; list1 = list1->next)
    write_int((ev = list1->node)->mark);
  write_int(null);

  for (list1 = cutoff_list, list2 = corr_list; list1;
    list1 = list1->next, list2 = list2->next)
  {
    write_int((ev = list1->node)->mark);
    if ((ev = list2->node)) write_int(ev->mark);
      else		write_int(null);
  }
  write_int(null);
  write_int(null);
  write_int(net->numpl);
  write_int(net->numtr);
  /* printf("places: %d\n", net->numpl);
  printf("transitions: %d\n", net->numtr); */

  sz = net->maxplname >= net->maxtrname ? 
    net->maxplname : net->maxtrname;
  write_int(sz);

  for (pl = net->places; pl; pl = pl->next)
    fwrite(pl->name,strlen(pl->name)+1,1,file);
  fwrite("",1,1,file);
  for (tr = net->transitions; tr; tr = tr->next)
    fwrite(tr->name,strlen(tr->name)+1,1,file);
  fwrite("",1,1,file);

  fclose(file);
}
