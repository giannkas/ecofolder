#include <string.h>
#include <stdio.h>
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
  int   idcond;
  struct clist_t* next;
} clist_t;

typedef struct evprepost
{
  clist_t *preset;
  clist_t *postset;
  struct evprepost *next;
} evprepost;

clist_t* clist_add(clist_t** list, int idpl)
{
  clist_t* newco = malloc(sizeof(clist_t));
  newco->idcond = idpl;
  newco->next = *list;
  return *list = newco;
}

int read_mci_file (char *mcifile, char *evev_cofile, int m_repeat, char* evname, int cutout, char* conf)
{
  #define read_int(x) fread(&(x),sizeof(int),1,mcif)

  FILE *mcif, *ev_cof;
  int nqure, nqure_, nquszcut, nquszevscut, szcuts, 
    numco, numev, numpl, idpl, idtr, numtr, sz, i, j, value;
  int pre_ev, post_ev, cutoff, harmful, dummy = 0, tmp = 0;
  int *co2pl, *ev2tr, *tokens, *cut0, *queries_co,
   *queries_ev, *cutoffs, *harmfuls, *queries_coset;
  char **plname, **trname, *c;
  char valuech[12] = "", valuechtmp[12] = "";
  cut_t **cuts;
  evprepost **evprps;


  if (!(mcif = fopen(mcifile,"rb")))
  {
    fprintf(stderr,"cannot read file %s\n",mcifile);
    exit(1);
  }

  if (evev_cofile)
  {
    if (!(ev_cof = fopen(evev_cofile, "r")))
    {
      fprintf(stderr,"cannot read file %s\n",evev_cofile);
      exit(1);
    }
  }

  if(!evname && !conf) 
    printf("digraph test {\n");


  read_int(numco);
  read_int(numev);

  co2pl = malloc((numco+1) * sizeof(int));
  tokens = malloc((numco+1) * sizeof(int));
  cut0 = calloc(numco+1, sizeof(int));
  queries_co = malloc((numco+1) * sizeof(int));
  queries_ev = malloc((numev+1) * sizeof(int));
  queries_coset = malloc((numco+1) * sizeof(int));
  ev2tr = malloc((numev+1) * sizeof(int));
  cutoffs = calloc(numev+1, sizeof(int));
  harmfuls = calloc(numev+1, sizeof(int));
  evprps = calloc(numev+1, sizeof(evprepost*));
  for (i = 0; i <= numev; i++) {
    evprps[i] = malloc(sizeof(evprepost));
    evprps[i]->preset = NULL;
    evprps[i]->postset = NULL;
  }
  int (*ev_predc)[numev+1] = calloc(numev+1, sizeof *ev_predc); // matrix to record events' predecesors.


  read_int(nqure);
  nqure_ = abs(nqure);
  cuts = calloc((szcuts = nqure_+1), sizeof(cut_t*));
  if(nqure_ && !m_repeat)
    dummy = nqure_;
  else if(nqure_ && (m_repeat > szcuts-1 || m_repeat < 0))
    m_repeat = -1;
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

  if(evev_cofile)
  {  
    dummy = 0;
    if (strstr(evev_cofile, ".evco"))
      while (fscanf(ev_cof," %d",&value) != EOF)
      {
        if (value != 0 && !dummy)
          queries_ev[value] = 1;
        else
        {
          queries_co[value] = 1; 
          dummy = 1;
        }
      }
    else
    {
      while (fscanf(ev_cof," %s", valuech) != EOF)
        if (strstr(valuech, "+")) // in case of reading redundant events
          strcpy(valuechtmp, valuech);
        else
        {
          value = strtoint(valuech);
          if (tmp && strcmp(valuechtmp, ""))
          {
            ev_predc[value][tmp] = tmp;
            strcpy(valuechtmp, "");
          }
          tmp = value;
          if (value > 0 && !queries_ev[value])
            queries_ev[value] = 1;
        }
    }
  }
  else if (m_repeat > 0 && cuts[m_repeat] && cuts[m_repeat]->repeat < 0)
  {
    for (i = 1; i <= cuts[m_repeat]->szcut; i++)
      queries_co[cuts[m_repeat]->cut[i]] = 1;
    for (i = 1; i <= cuts[m_repeat]->szevscut; i++)
      queries_ev[cuts[m_repeat]->evscut[i]] = 1;
  }
  else if (!m_repeat)
  {
    for (j = 1; j <= dummy; j++)
    {
      for (i = 1; i <= cuts[j]->szcut; i++)
        queries_co[cuts[j]->cut[i]] = 1;
      for (i = 1; i <= cuts[j]->szevscut; i++)
        queries_ev[cuts[j]->evscut[i]] = 1;
    }
  }

  for (i = 1; i <= numev; i++){
    read_int(ev2tr[i]);
    read_int(dummy);
    if (!evev_cofile && m_repeat > 0 && cuts[m_repeat] && cuts[m_repeat]->repeat > 0) 
      queries_ev[i] = dummy;
  }

  for (i = 1; i <= numco; i++)
  {
    read_int(co2pl[i]);
    read_int(tokens[i]);
    read_int(dummy);
    if (!evev_cofile && m_repeat > 0 && cuts[m_repeat] && cuts[m_repeat]->repeat > 0) 
      queries_co[i] = dummy;
    read_int(pre_ev);
    if (cutout && pre_ev && !evname && (queries_ev[pre_ev] || queries_co[i]) && !conf)
    {
      printf("  e%d -> c%d;\n",pre_ev,i);
      queries_coset[i] = 1;
    }
    else if (cutout && !pre_ev && !evname && queries_co[i] && !conf)
      queries_coset[i] = 1;
    else if (!cutout && pre_ev && !evname) 
    {
      if (!conf) printf("  e%d -> c%d;\n",pre_ev,i);
      if(tokens[i] && conf) clist_add(&evprps[pre_ev]->postset, i);
    }
    if (!pre_ev)
    { 
      if(tokens[i] && conf) clist_add(&evprps[0]->postset, i);
      cut0[i] = 1;
    }
    do {
      read_int(post_ev);
      if (cutout && post_ev && !evname && (queries_ev[post_ev] || queries_co[i]) && !conf)
      {
        if (queries_ev[post_ev]) printf("  c%d -> e%d;\n",i,post_ev);
        queries_coset[i] = 1;
      }
      else if (!cutout && post_ev && !evname)
      { 
        if (!conf) printf("  c%d -> e%d;\n",i,post_ev);
        if(tokens[i] && conf) clist_add(&evprps[post_ev]->preset, i);
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
#ifdef CUTOFF
    printf("  e%d [style=filled];\n",cutoff);
#endif
    read_int(dummy);
#ifdef CUTOFF
    printf("  e%d [style=dashed];\n",dummy);
#endif
  }

  do { read_int(dummy); } while(dummy);
  read_int(numpl);
  read_int(numtr);
  read_int(sz);

  plname = malloc((numpl+2) * sizeof(char*));
  trname = malloc((numtr+2) * sizeof(char*));

  for (i = 1; i <= numpl+1; i++) plname[i] = malloc(sz+1);
  for (i = 1; i <= numtr+1; i++) trname[i] = malloc(sz+1);

  for (i=1; i <= numpl; i++)
  {
    read_int(idpl);
    c = plname[idpl];
    do { fread(c,1,1,mcif); } while (*c++);
  }
  fread(c,1,1,mcif);

  for (i=1; i <= numtr; i++)
  {
    read_int(idtr);
    c = trname[idtr];
    do { fread(c,1,1,mcif); } while (*c++);
  }
  fread(c,1,1,mcif);

  int *cut = calloc(numpl+1, sizeof(int));
  int *frsq = calloc(numev+1, sizeof(int));

  if(conf)
  {  
    clist_t *list;
    char* sub;
    int coid;
    int subint;
    char* conf_copy = strdup(conf);
    char *markingstr = calloc(numpl*sz, sizeof(char));
    /* Add initial cut */
    for(list = evprps[0]->postset; list; list = list->next)
    {
      coid = list->idcond;
      cut[co2pl[coid]] = co2pl[coid];
    }

    sub = strtok(conf_copy, ",");
    while (sub != NULL)
    {
      subint = strtoint(sub);
      frsq[subint] = subint;
      /* Remove event's preset */
      for(list = evprps[subint]->preset; list; list = list->next)
      {
        coid = list->idcond;
        cut[co2pl[coid]] = 0;
      }
      /* Add event's poset */
      for(list = evprps[subint]->postset; list; list = list->next)
      {
        coid = list->idcond;
        cut[co2pl[coid]] = co2pl[coid];
      }
      sub = strtok(NULL, ",");
    }

    for(int i = 1; i <= numpl; i++)
    {
      if (cut[i] > 0)
      {
        if (strlen(markingstr) > 0) strcat(markingstr,",");
        strcat(markingstr, plname[i]);
      }
    }

    printf("%s\n",markingstr);
  }

  char color0[] = "transparent";
  char color1[] = "#ccccff"; // or "lightblue";
  char color2[] = "gold";
  char color3[] = "orange";
  char color4[] = "#cce6cc"; // or "palegreen";
  char color5[] = "cornflowerblue";
  char color6[] = "black";
  char color7[] = "firebrick2";
  char color8[] = "#4040ff";
  char color9[] = "#409f40";
  
  int found = 0;
  if(!evname && !conf)
  {
    for (i = 1; i <= numco; i++)
      if (cutout && queries_coset[i])
        printf("  c%d [color=\"%s\" fillcolor=\"%s\" label= <%s<FONT COLOR=\"red\"><SUP>%d</SUP></FONT>&nbsp;(c%d)> shape=circle style=filled];\n",
            i,color8,queries_co[i] ? color2 : cut0[i] ? color0 : color1,plname[co2pl[i]],tokens[i],i);
      else if (!cutout)
        printf("  c%d [color=\"%s\" fillcolor=\"%s\" label= <%s<FONT COLOR=\"red\"><SUP>%d</SUP></FONT>&nbsp;(c%d)> shape=circle style=filled];\n",
          i,color8,queries_co[i] || cut[i] ? color2 : cut0[i] ? color0 : color1,plname[co2pl[i]],tokens[i],i);
    for (i = 1; i <= numev; i++)
      if (cutout && queries_ev[i])
      {
        if ( i == harmfuls[i])
          printf("  e%d [color=%s fillcolor=\"%s:%s\" label=\"%s (e%d)\" shape=box style=filled];\n",
              i,color6,color7,queries_ev[i] ? color3 : color7,trname[ev2tr[i]],i);
        else if (i == cutoffs[i])
          printf("  e%d [color=%s fillcolor=\"%s:%s\" label=\"%s (e%d)\" shape=box style=filled];\n",
              i,color6,color5,queries_ev[i] ? color3 : color5,trname[ev2tr[i]],i);
        else
          printf("  e%d [color=\"%s\" fillcolor=\"%s\" label=\"%s (e%d)\" shape=box style=filled];\n",
              i,queries_ev[i] ? color6 : color9,queries_ev[i] ? color3 : color4,trname[ev2tr[i]],i);
      }
      else if (!cutout)
      {
        if ( i == harmfuls[i])
          printf("  e%d [color=%s fillcolor=\"%s:%s\" label=\"%s (e%d)\" shape=box style=filled];\n",
              i,color6,color7,queries_ev[i] || frsq[i] ? color3 : color7,trname[ev2tr[i]],i);
        else if (i == cutoffs[i])
          printf("  e%d [color=%s fillcolor=\"%s:%s\" label=\"%s (e%d)\" shape=box style=filled];\n",
              i,color6,color5,queries_ev[i] || frsq[i] ? color3 : color5,trname[ev2tr[i]],i);
        else
          printf("  e%d [color=\"%s\" fillcolor=\"%s\" label=\"%s (e%d)\" shape=box style=filled];\n",
              i,queries_ev[i] || frsq[i] ? color6 : color9,queries_ev[i] || frsq[i] ? color3 : color4,trname[ev2tr[i]],i);
      }
    printf("}\n");
  }
  else{
    for (i = 1; i <= numev && !found && evname; i++)
      if (!strcmp(trname[ev2tr[i]], evname))
        found = 1;
  }

  fclose(mcif);
  if (evev_cofile) fclose(ev_cof);
  return found;
}

void usage ()
{
  fprintf(stderr,"usage: mci2dot [options] <mcifile> <evev_cofile>\n\n"

    "     Options:\n"
    "      -c --cutout  if a marking is queried or \n                  part of a reachability check then\n                  it will show a cutout of\n                  the whole unfolding\n"
    "      -r <instance>  highlight <instance> of a repeated marking\n"
    "      -cf <confg>:   used to return the marking led \n by the configuration <confg>(string type).\n You cannot enable cutouts and this \n flag at the same time.\n"
    "      -reach <evname>  if used, it will look for\n                  an event related to an attractor\n                  to return whether such event was found\n\n"

    "<evev_cofile> is an optional file which can be formatted as .evco file or .evev file (important to keep the extension)\n. The former's first line contains\n"
    "the IDs of a firing sequence of events and the second line\n"
    "represents IDs of conditions in the cut.\n While the latter is similar \n but mainly used in <mci2dot_ev>"
    "-r and -reach are mutually exclusive.\n\n");
    exit(1);
}

int main (int argc, char **argv)
{
  int i, m_repeat = -1, cutout = 0;
  char *mcifile = NULL, *evev_cofile = NULL, *evname = NULL;
  char *configuration = NULL;

  for (i = 1; i < argc; i++)
    if (!strcmp(argv[i],"-r"))
    {
      if (++i == argc) usage();
      m_repeat = atoi(argv[i]);
    }
    else if (!strcmp(argv[i], "-cf"))
    {
      if (++i == argc) usage();
      configuration = argv[i];
    }
    else if (!strcmp(argv[i],"-reach"))
    {
      if (++i == argc) usage();
      evname = argv[i];
    }
    else if (!strcmp(argv[i],"-c") || !strcmp(argv[i],"--cutout"))
    {
      cutout = 1;
    }
    else if(!mcifile)
      mcifile = argv[i];
    else
      evev_cofile = argv[i];

  if (!mcifile || (evname && m_repeat >= 0) || (cutout && configuration)) 
    usage();

  exit(!read_mci_file(mcifile, evev_cofile, m_repeat, evname, cutout, configuration));
}
