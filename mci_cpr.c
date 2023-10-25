#include <stdio.h>
#include <string.h>

#include "common.h"
#include "netconv.h"
#include "unfold.h"

void write_mci_file_cpr (char *filename)
{
  #define write_int(x) fwrite(&(x),sizeof(int),1,file)

  FILE *file;
  place_t *pl;
  trans_t *tr;
  cond_t *co, *coo;
  event_t *ev, *evv;
  nodelist_t *list1, *list2;
  int ev_num = 0, sz = 0, null = 0, cum, szz = 0, once = 0,
    tmp = 0;

  if (!(file = fopen(filename,"wb")))
    nc_error("cannot write to file %s\n",filename);

  write_int(unf->numco);
  write_int(unf->numev);
<<<<<<< HEAD
  printf("unf->numev: %d\n", unf->numev);
  printf("unf->numco: %d\n", unf->numco);
  
=======
  if(!data)
  {
    printf("unf->numev: %d\n", unf->numev);
    printf("unf->numco: %d\n", unf->numco);
  }
>>>>>>> a2d7029472ef6fd9949d9b3eeab62368d0f101ab
  /* Reverse the lists of places, events etc. This is to maintain
    compatibility with RdlcheckMcM and mcsmodels, which expect events
    to be numbered in accordance with the causality relation. */
  net->places = reverse_list(net->places);
  net->transitions = reverse_list(net->transitions);
  unf->conditions = reverse_list(unf->conditions);
  unf->events = reverse_list(unf->events);

  for (ev = unf->events; ev; ev = ev->next){
    ev->mark = ++ev_num;
    if(ev->queried)
    {
      if(!once)
      {
        printf("chain of events to reach the queried marking:\n  ");
        once = 1;
      }
      printf("%s (e%d)  ", ev->origin->name, ev->mark);
    }
<<<<<<< HEAD
=======

>>>>>>> a2d7029472ef6fd9949d9b3eeab62368d0f101ab
  }
  printf("\n");

  querycell_t *qbuck;
  for(qbuck = *query; qbuck; qbuck = qbuck->next)
  {
    write_int(qbuck->repeat);
    write_int(qbuck->szcut);
    write_int(qbuck->szevscut);
    /* printf("qbuck->repeat: %d\n", qbuck->repeat);
    printf("qbuck->szcut: %d\n", qbuck->szcut);
    printf("qbuck->szevscut: %d\n", qbuck->szevscut); */
    qbuck->cut = nodelist_reverse(qbuck->cut);
    qbuck->evscut = nodelist_reverse(qbuck->evscut);
    for(list1 = qbuck->cut; list1; list1 = list1->next)
    {
      if((co = list1->node))
      {
        tmp = co->num + 1;
        write_int(tmp);
        //printf("tmp: %d\n", tmp);
      }
    }
    for(list1 = qbuck->evscut; list1; list1 = list1->next)
      if((ev = list1->node))
      {
        write_int(ev->mark);
        //printf("ev->mark: %d\n", ev->mark);
      }

  }
  write_int(null);

  for (ev = unf->events; ev; ev = ev->next){
    write_int(ev->origin->num);
    write_int(ev->queried);
  }

<<<<<<< HEAD
=======

>>>>>>> a2d7029472ef6fd9949d9b3eeab62368d0f101ab
  for (co = unf->conditions, cum = 1; co &&
    cum <= unf->numco; co = co->next, cum++)
  {
    int common_transs = 0, one_common_trans = 0;
    if (co->flag) continue;
    for (coo = co->next; coo ; coo = coo->next)
    {
      if (coo->flag) continue;
      if ((!co->pre_ev && !coo->pre_ev) || 
        (co->pre_ev && coo->pre_ev && co->pre_ev->mark == coo->pre_ev->mark) )
      {
        common_transs = 0;
        for (list1 = co->postset; list1; list1 = list1->next)
        {
          one_common_trans = 0;
          for (list2 = coo->postset; list2 && !one_common_trans; list2 = list2->next)
          {
            ev = list1->node;
            evv = list2->node;
            if (!strcmp(ev->origin->name, evv->origin->name) &&
              (ev->mark == evv->mark))
              one_common_trans = 1;
          }
          common_transs = common_transs + one_common_trans;
        }
        sz = nodelist_size(co->postset);
        szz = nodelist_size(coo->postset);
        if ((sz == 0 && szz == 0) || (common_transs > 0 &&
          (common_transs == szz) && (sz == szz)))
        {
          write_int(coo->origin->num);
          write_int(coo->token);
          write_int(coo->queried);
          coo->flag = cum;
          write_int(coo->flag);
        }
      }
    }
    write_int(co->origin->num);
    write_int(co->token);
    write_int(co->queried);
    co->flag = cum;
    write_int(co->flag);
    write_int(null);
    if (co->pre_ev) {
      write_int(co->pre_ev->mark);
    }
    else	{
      write_int(null);}
    for (list1 = co->postset; list1; list1 = list1->next){
      write_int((ev = list1->node)->mark);
    }
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
