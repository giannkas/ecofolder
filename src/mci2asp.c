#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"

typedef struct cut_t
{
  int repeat;
  int szcut, szevscut;
  int *cut;
  int *evscut;
} cut_t;

typedef struct clist_t
{
  int   idplace;
  struct clist_t* next;
} clist_t;

typedef struct evprepost
{
  clist_t *preset;
  clist_t *postset;
  struct evprepost *next;
} evprepost;

evprepost **evprps;
int numplaces = 0;

clist_t* clist_add(clist_t** list, int idpl)
{
  clist_t* newco = malloc(sizeof(clist_t));
  newco->idplace = idpl;
  newco->next = *list;
  return *list = newco;
}

void read_mci_file (char *mcifile, int m_repeat, char* ns, char* conf)
{
  #define read_int(x) fread(&(x),sizeof(int),1,mcif)

  FILE *mcif;
  int nqure, nqure_, nquszcut, nquszevscut, szcuts, 
    numco, numev, numpl, numtr, idpl, idtr, sz, i;
  int pre_ev, post_ev, cutoff, harmful, dummy = 0;
  int *co2pl, *ev2tr, *tokens, *queries_co,
   *queries_ev, *cutoffs, *harmfuls, *plids, *trids;
  char **plname, **trname, *c;
  cut_t **cuts;

  if (!(mcif = fopen(mcifile,"rb")))
  {
    fprintf(stderr,"cannot read file %s\n",mcifile);
    exit(1);
  }

  read_int(numco);
  read_int(numev);

  co2pl = malloc((numco+1) * sizeof(int));
  tokens = malloc((numco+1) * sizeof(int));
  queries_co = malloc((numco+1) * sizeof(int));
  queries_ev = malloc((numev+1) * sizeof(int));
  ev2tr = malloc((numev+1) * sizeof(int));
  cutoffs = calloc(numev+1, sizeof(int));
  harmfuls = calloc(numev+1, sizeof(int));
  evprps = calloc(numev+1, sizeof(evprepost*));
  for (i = 0; i <= numev; i++) {
    evprps[i] = malloc(sizeof(evprepost));
    evprps[i]->preset = NULL;
    evprps[i]->postset = NULL;
  }

  read_int(nqure);
  nqure_ = abs(nqure);
  cuts = calloc((szcuts = nqure_+1), sizeof(cut_t*));
  if(nqure_ && m_repeat > 0 && m_repeat <= nqure_) 
    dummy = 1;
  while(nqure_)
  {
    read_int(nquszcut);
    read_int(nquszevscut);
    cuts[nqure_] = malloc(sizeof(cut_t));
    cuts[nqure_]->repeat = nqure;
    cuts[nqure_]->szcut = nquszcut;
    cuts[nqure_]->szevscut = nquszevscut;
    cuts[nqure_]->cut = calloc(nquszcut+1, sizeof(int));
    cuts[nqure_]->evscut = calloc(nquszevscut+1, sizeof(int));
    for (i = 1; i <= nquszcut; i++)
      read_int(cuts[nqure_]->cut[i]);
    for (i = 1; i <= nquszevscut; i++)
      read_int(cuts[nqure_]->evscut[i]);
    read_int(nqure);
    nqure_ = abs(nqure);
  }

  for (i = 1; i <= numev; i++) {
    if (!conf) printf("event((%s,e%d)).\n", ns, i);
    read_int(ev2tr[i]);
    read_int(queries_ev[i]);
  }

  for (i = 1; i <= numco; i++)
  {
    read_int(co2pl[i]);
    read_int(tokens[i]);
    read_int(queries_co[i]);
    read_int(pre_ev);
    if (pre_ev && tokens[i]){
      if (!conf)
        printf("edge((%s,e%d),(%s,c%d)).\n",ns,pre_ev,ns,i);
      else
        clist_add(&evprps[pre_ev]->postset, co2pl[i]);
      //evprps[pre_ev]->preset[i] = co2pl[i];
    }
    else if (conf && tokens[i]){ 
      clist_add(&evprps[0]->postset, co2pl[i]); 
    }
    //evprps[0]->postset[i] = co2pl[i];
    do {
      read_int(post_ev);
      if (post_ev && tokens[i])
      {
        if(!conf)
          printf("edge((%s,c%d),(%s,e%d)).\n",ns,i,ns,post_ev);
        else
          clist_add(&evprps[post_ev]->preset, co2pl[i]);
        //evprps[post_ev]->postset[i] = co2pl[i];
      }
    } while (post_ev);
  }

  for (;;) {
    read_int(harmful);
    if (!harmful) break;
    harmfuls[harmful] = harmful;
  }

  for (;;) {
    read_int(cutoff);
    if (!cutoff) break;
    cutoffs[cutoff] = cutoff;
    if (!conf) printf("cutoff((%s,e%d)).\n", ns, cutoff);
    read_int(dummy);
  }

  do { read_int(dummy); } while(dummy);
  read_int(numpl); numplaces = numpl;
  read_int(numtr);
  read_int(sz);

  plname = malloc((numpl+2) * sizeof(char*));
  trname = malloc((numtr+2) * sizeof(char*));
  plids = calloc(numpl+2, sizeof(int));
  trids = calloc(numtr+2, sizeof(int));

  for (i = 1; i <= numpl+1; i++) plname[i] = malloc(sz+1);
  for (i = 1; i <= numtr+1; i++) trname[i] = malloc(sz+1);

  for (i=1; i <= numpl; i++)
  {
    read_int(idpl);
    plids[i] = idpl;
    c = plname[i];
    do { fread(c,1,1,mcif); } while (*c++);
  }
  fread(c,1,1,mcif);

  for (i=1; i <= numtr; i++)
  {
    read_int(idtr);
    trids[i] = idtr;
    c = trname[i];
    do { fread(c,1,1,mcif); } while (*c++);
  }
  fread(c,1,1,mcif);

  for (i = 1; i <= numco; i++)
    if (!conf && tokens[i]) printf("h((%s,c%d),p%d).\n", ns,i,co2pl[i]);
  for (i = 1; i <= numev; i++)
    if (!conf) printf("h((%s,e%d),t%d).\n", ns,i,ev2tr[i]);

  for (i = 1; i <= numpl; i++)
    if (!conf)  printf("name(p%d,\"%s\").\n",plids[i],plname[i]);
  for (i = 1; i <= numtr; i++)
    if (!conf)  printf("name(t%d,\"%s\").\n",trids[i],trname[i]);

  /* clist_t *j;
  for (i = 0; i <= numev; i++) {
    for (j = evprps[i]->preset; j; j = j->next)
      printf("%d,", j->idplace);
    printf(" e%d ", i);
    for (j = evprps[i]->postset; j; j = j->next)
      printf("%d,", j->idplace);
    printf("\n");
  } */

  fclose(mcif);
}

void get_marking(char* confg)
{
  clist_t *list;
  char* sub;
  char* confg_copy = strdup(confg);
  int subint;

  int *marking = calloc((numplaces*numplaces)+1, sizeof(int));
  char *markingstr = calloc(numplaces*5, sizeof(char));
  int length = snprintf(NULL, 0, "%d", numplaces);
  char* plidstr = malloc(length);
  int plid;

  /* Add initial marking */
  for(list = evprps[0]->postset; list; list = list->next)
  {
    plid = list->idplace;
    marking[plid] = plid;
    //printf("%d,",marking[plid]);
  }
  //printf("\n");

  sub = strtok(confg_copy, ",");
  while (sub != NULL)
  {
    subint = strtoint(sub);
    /* Remove event's preset */
    for(list = evprps[subint]->preset; list; list = list->next)
    {
      plid = list->idplace;
      marking[plid] = 0;
    }
    /* Add event's poset */
    for(list = evprps[subint]->postset; list; list = list->next)
    {
      plid = list->idplace;
      marking[plid] = plid;
    }
    sub = strtok(NULL, ",");
  }

  for(int i = 1; i <= (numplaces*numplaces); i++)
  {
    if (marking[i] > 0)
    {
      sprintf(plidstr, "%d,", marking[i]);
      strcat(markingstr, plidstr);
    }
  }
  printf("%s",markingstr);
}

void usage ()
{
  fprintf(stderr,"usage: mci2asp [options] <mcifile> -p [ns]\n"
    "\t options:\n"
    "\t -cf <confg>:\t used to return the marking led by the configuration <confg>(string type).\n"
    "\t -p <prefixname>:\t used to tag elements in the prefix. \n");
  exit(1);
}

int main (int argc, char **argv)
{
  int i;
  char *configuration = NULL, *prefix_name = NULL;
  char* mcifile = NULL;

  for (i = 1; i < argc; i++)
    if (!strcmp(argv[i], "-cf"))
    {
      if (++i == argc) usage();
      configuration = argv[i];
    }
    else if (!strcmp(argv[i], "-p"))
      prefix_name = ++i == argc ? NULL : argv[i];
    else
      mcifile = argv[i];

  if (!prefix_name) prefix_name = "pi0";
  if (!mcifile) usage ();

  read_mci_file(mcifile, 0, prefix_name, configuration);
  if (configuration)
    get_marking(configuration);
  exit(0);
}
