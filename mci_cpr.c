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
  event_t *ev;
  nodelist_t *list1, *list2;
  int ev_num = 0, sz = 0, null = 0, cum;

  if (!(file = fopen(filename,"wb")))
    nc_error("cannot write to file %s\n",filename);

  write_int(unf->numco);
  write_int(unf->numev);
  
  /* Reverse the lists of places, events etc. This is to maintain
    compatibility with RdlcheckMcM and mcsmodels, which expect events
    to be numbered in accordance with the causality relation. */
  net->places = reverse_list(net->places);
  net->transitions = reverse_list(net->transitions);
  unf->conditions = reverse_list(unf->conditions);
  unf->events = reverse_list(unf->events);

  for (ev = unf->events; ev; ev = ev->next)
    ev->mark = ++ev_num;

  for (ev = unf->events; ev; ev = ev->next){
    write_int(ev->origin->num);
  }

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
            if (((event_t*)(list1->node))->mark == ((event_t*)(list2->node))->mark)
              one_common_trans = 1;
          }
          common_transs = common_transs + one_common_trans;
        }        
        if (common_transs == nodelist_size(coo->postset))
        {
          printf("coo->origin->num: %d\n", coo->origin->num);
          printf("coo->token: %d\n", coo->token);
          write_int(coo->origin->num);
          write_int(coo->token);
          coo->flag = cum;
          printf("coo->flag: %d\n", coo->flag);
          write_int(coo->flag);
        }
      }
    }
    
    printf("co->origin->num: %d\n", co->origin->num);
    printf("co->token: %d\n", co->token);
    write_int(co->origin->num);
    write_int(co->token);
    co->flag = cum;
    printf("co->flag: %d\n", co->flag);
    write_int(co->flag);
    printf("after co->token: 0\n");
    write_int(null);
    if (co->pre_ev) {
      printf("co->pre_ev->mark: %d\n", co->pre_ev->mark); 
      write_int(co->pre_ev->mark);
    }
    else	{printf("!co->pre_ev: 0\n"); write_int(null);}
    for (list1 = co->postset; list1; list1 = list1->next){
      printf("(ev = list1->node)->mark: %d\n", (ev = list1->node)->mark);
      write_int((ev = list1->node)->mark);
    }
    printf("end: 0\n");
    write_int(null);
  }

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
