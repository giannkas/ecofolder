/* 
 * Copyright (C) 2010, 2011  Cesar Rodriguez <cesar.rodriguez@lsv.ens-cachan.fr>
 * Copyright (C) 2023-2024  Giann Karlo Aguirre-Samboní <giann.aguirre_samboni@ens-paris-saclay.fr>
 */

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <err.h>
#include <fcntl.h>
#include <stdarg.h>
#include <assert.h>

#include "common.h"

#define P(args...) fprintf (fout, ##args)

void error (const char * fmt, ...)
{
  va_list	args;

  va_start (args, fmt);
  verrx (EXIT_FAILURE, fmt, args);
}

char *bpos;

static unsigned int read_int ()
{
  int i = *(int*)bpos;
  bpos += sizeof(int);
  return i;
}

static void read_str ()
{
  while (*bpos++);
}

typedef struct cut_t
{
  int repeat;
  int szcut, szevscut;
  int *cut;
  int *evscut;
} cut_t;

typedef struct list_t {
  int alloc;
  int size;
  int contents;
} list_t;

typedef struct clist_t {
  int alloc;
  int size;
  char *contents;
} clist_t;

void l_init (list_t **l)
{
  *l = malloc(6*sizeof(int));
  (*l)->alloc = 4; (*l)->size = 0;
}

void l_reset (list_t **l)
{
  (*l)->size = 0;
}

void l_add (list_t **l, int e)
{
  if ((*l)->alloc == (*l)->size)
  {
    int newsize = 2 * (*l)->alloc;
    (*l) = realloc(*l,(2+newsize)*sizeof(int));
    (*l)->alloc = newsize;
  }
  int *c = &((*l)->contents);
  c[(*l)->size++] = e;
}

void c_init (clist_t **l)
{
  *l = malloc(6*sizeof(char*));
  (*l)->alloc = 4; (*l)->size = 0;
}

void c_add (clist_t **l, char *e)
{
  if ((*l)->alloc == (*l)->size)
  {
    int newsize = 2 * (*l)->alloc;
    (*l) = realloc(*l,(2+newsize)*sizeof(char*));
    (*l)->alloc = newsize;
  }
  char **c = &((*l)->contents);
  c[(*l)->size++] = e;
  //printf("e: %s\n", c[3]);
}

void print_clist(clist_t *l)
{
  char **jptr;

  jptr = &(l->contents);
  for (int j = 0; j < l->size; j++)
    printf("content: %s\n", jptr[j]);
}

FILE *fout;

int numpl, numtr, idpl, events, conds;
char *cutoff;		// indicates which events are cutoffs (=1)
int *evtrl;		// maps non-cutoff events to variable indices
int *cotrl;		// maps conditions to variable indices
list_t **preset;	// preset for all events (incl cutoffs)
list_t **plcond;	// for each place, non-cutoff conditions labelled by it
char **plname;		// names of places
int vars = 0;		// variables used so far in the formula
int clauses = 0;	// number of clauses
int nosense = 1;  // nosense variable search
int att = 0;    // used to find and format attractors
signed char *plwanted;	// for reachability: 1=positive, -1=negative, 0=dontcare

clist_t *p_reach, *n_reach; // positive and negative reachability targets

char opt_comments = 0;	// whether to put comments into the sat file
char opt_reach = 0;	// perform reachability (=1) or deadlock (=0)
char opt_final = 0;	// find a final configuration

void causal (int c, int father, list_t *post)
{
  int i, *iptr = &(post->contents);
  if (!post->size) return;

  if (opt_comments) P("\nc causal chains around c%d\n",c);

  /* father is direct predecessor event of events in post */
  // \/ post -> father
  for (i = 0; i < post->size; i++)
  {
    P("-%d %d 0\n",evtrl[iptr[i]],evtrl[father]);
    clauses++;
  }
}

void condition_marked (int c, int place, int gen, list_t *post)
{
  int i, *iptr = &(post->contents);

  // do not consider conditions generating by cutoff events
  if (gen && cutoff[gen]) return;
  if (opt_reach && !plwanted[place]) return;

  cotrl[c] = ++vars;
  if (opt_reach) l_add(plcond+place,c);	// collect conds with that label

  if (opt_comments) P("\nc condition c%d (%s) ~ var %d\n",c,
    plname[place],vars);
  if (opt_comments) P("c whether c%d is marked\n",c);

  // gen /\ !post -> cond
  if (gen) P("-%d ",evtrl[gen]);
  P("%d ",cotrl[c]);
  for (i = 0; i < post->size; i++) P("%d ",evtrl[iptr[i]]);
  P("0\n");
  clauses++;

  if (!opt_reach && !opt_final) return;
  // cond -> gen /\ !post
  if (gen) { P("-%d %d 0\n",cotrl[c],evtrl[gen]); clauses++; }
  for (i = 0; i < post->size; i++)
    { P("-%d -%d 0\n",cotrl[c],evtrl[iptr[i]]); clauses++; }
}

int intlog (int n)
{
  int l = 0;
  if (n & 0xffff0000) { l += 16; n >>= 16; }
  if (n & 0xff00)     { l += 8;  n >>= 8;  }
  if (n & 0xf0)       { l += 4;  n >>= 4;  }
  if (n & 0xc)        { l += 2;  n >>= 2;  }
  if (n & 0x2) l++;
  return l;
}

int confl_recur (list_t *post, int lower, int upper, int propag)
{
  int v1, v2;
  int *iptr = &(post->contents);
  if (lower == upper)
  {
    return evtrl[iptr[lower]];
  }
  else if (lower + 1 == upper)
  {
    v1 = evtrl[iptr[lower]];
    v2 = evtrl[iptr[upper]];
  }
  else
  {
    int middle = lower + (1 << intlog(upper-lower));
    v1 = confl_recur(post,lower,middle-1,1);
    v2 = confl_recur(post,middle,upper,1);
  }
  P("-%d -%d 0\n",v1,v2); clauses++;
  if (!propag) return 0;
  vars++;
  P("-%d %d 0\n",v1,vars); clauses++;
  P("-%d %d 0\n",v2,vars); clauses++;
  return vars;
}

void conflicts (int c, list_t *post, int gen)
{
  if (post->size < 2) return;
  if (opt_comments) P("\nc conflicts for c%d\n",c);
  confl_recur(post,0,post->size-1,0);
}

void disabled (int e, list_t *pre)
{
  int i, *iptr = &(pre->contents);
  if (opt_comments) P("\nc event e%d disabled\n",e);
  for (i = 0; i < pre->size; i++) P("-%d ",cotrl[iptr[i]]);
  P("0\n");
  clauses++;
}

void reachable ()
{
  int i;

  for (i = 1; i <= numpl; i++)
  {
    if (!plwanted[i]) continue;

    list_t *conds = plcond[i];
    int j, *jptr = &(conds->contents);

    if (plwanted[i] == 1)
    {
      if (opt_comments)
        P("\nc place %s is marked\n",plname[i]);
      for (j = 0; j < conds->size; j++)
        P("%d ",cotrl[jptr[j]]);
      P("0\n");
      clauses++;
    }
    else
    {
      if (opt_comments)
        P("\nc place %s is unmarked\n",plname[i]);
      for (j = 0; j < conds->size; j++)
      {
        P("-%d 0\n",cotrl[jptr[j]]);
        clauses++;
      }
    }
  }
}

int mci2sat (const char * infile, const char *outfile)
{
  int i, j, e, maxszname = 1;
  list_t *post;
  char *blob, *bconds, *plinunf;
  int *harmfuls;
  int nqure, nqure_, nquszcut, nquszevscut, szcuts, harmful, token, queried;
  cut_t **cuts;

  /* open input and output files */
  struct stat statbuf;
  stat(infile,&statbuf);
  blob = malloc(statbuf.st_size);
  int fd = open(infile,O_RDONLY);
  read(fd,blob,statbuf.st_size);
  close(fd);
  bpos = blob;

  fout = fopen(outfile,"w");
  if (!fout) error("'%s': %s", outfile, strerror (errno));
  if (fd == -1) error("'%s': %s", infile, strerror (errno));

  // leave space for number of vars and clauses
  if (opt_comments) P("p cnf                              \n");

  // start reading mci file
  conds = read_int();
  events = read_int();

  preset = malloc((events+1) * sizeof(list_t*));
  for (i = 1; i <= events; i++) l_init(preset+i);
  cutoff = calloc((events+1) * sizeof(char),1);
  evtrl = malloc((events+1) * sizeof(int));
  cotrl = malloc((conds+1) * sizeof(int));
  harmfuls = calloc(events+1, sizeof(int));

  nqure = read_int();
  nqure_ = abs(nqure);
  cuts = calloc((szcuts = nqure_+1), sizeof(cut_t*));
  while(nqure_)
  {
    nquszcut = read_int();
    nquszevscut = read_int();
    cuts[nqure_] = malloc(sizeof(cut_t));
    cuts[nqure_]->repeat = nqure;
    cuts[nqure_]->szcut = nquszcut;
    cuts[nqure_]->szevscut = nquszevscut;
    cuts[nqure_]->cut = calloc(nquszcut+1, sizeof(int));
    cuts[nqure_]->evscut = calloc(nquszevscut+1, sizeof(int));
    for (i = 1; i <= nquszcut; i++)
      cuts[nqure_]->cut[i] = read_int();
    for (i = 1; i <= nquszevscut; i++)
      cuts[nqure_]->evscut[i] = read_int();
    nqure = read_int();
    nqure_ = abs(nqure);
  }

  // skip events and conditions in mci file
  bconds = bpos += (events*2) * sizeof(unsigned int);
  for (i = 1; i <= conds; i++)
  {
    bpos += 4 * sizeof(unsigned int);
    while (read_int());
  }

  for (;;) {
    harmful = read_int();
    if (!harmful) break;
    harmfuls[harmful] = harmful;
  }

  // read cutoff information
  // reserve variable names only for non-cutoffs
  if (opt_comments) P("\n");
  while ((e = read_int()))
  {
    cutoff[e] = 1;
    if (opt_comments) P("c e%d is a cutoff\n",e);
    read_int();
  }
  for (i = 1; i <= events; i++)
  {
    if (cutoff[i]) continue;
    evtrl[i] = ++vars;
    if (opt_comments) P("c event e%d ~ var %d\n",i,vars);
  }
  // dummy ints followed by number of places and transitions
  while (read_int());
  numpl = read_int();
  //printf("p_reach->size: %d\n", p_reach->size);
  //printf("numco: %d\n", conds);
  //printf("1. nosense: %d\n", nosense);


  if(p_reach->size > conds-1 || n_reach->size > conds-1)
    return 0;

  numtr = read_int();
  read_int();
  
  // read place names
  plcond = malloc((numpl+1) * sizeof(list_t*));
  plwanted = calloc((numpl+1) * sizeof(signed char),1);
  plname = malloc((numpl+1) * sizeof(char*));
  for (i = 1; i <= numpl; i++) l_init(plcond+i);
  for (i = 1; i <= numpl; i++)
  {
    idpl = read_int();
    char **jptr;
    jptr = &(p_reach->contents);
    for (j = 0; j < p_reach->size; j++)
      if (!strcmp(bpos,jptr[j])) 
      {
        plwanted[idpl] = 1;
        nosense = 0;
        //printf("bpos: %s\n", bpos);
        //printf("jptr[j]: %s\n", jptr[j]);
        //printf("2. nosense: %d\n", nosense);

      }
      else
      {
        char* bpos_ = bltokstr(bpos, 0, '_');
        if (!strcmp(bpos_,jptr[j]))
        {
          plwanted[idpl] = 1;
          nosense = 0;
          //printf("3. nosense: %d\n", nosense);

        }
      }
    jptr = &(n_reach->contents);
    for (j = 0; j < n_reach->size; j++)
      if (!strcmp(bpos,jptr[j])) 
      {
        plwanted[idpl] = -1;
        nosense = 0;
      }
      else
      {
        char* bpos_ = bltokstr(bpos, 0, '_');
        if (!strcmp(bpos_,jptr[j]))
        {
          plwanted[idpl] = -1;
          nosense = 0;
        }
      }

    plname[idpl] = bpos;
    if (strlen(plname[idpl]) > maxszname)
      maxszname = strlen(plname[idpl]);
    read_str();
  }
  plinunf = calloc((maxszname*numpl)*10, sizeof(char));
  //printf("4. nosense: %d\n", nosense);
  if (nosense && !att)
    return 0;
  
  if (!opt_comments) P("p cnf                              \n");
  /* go back and read conditions */
  l_init(&post);
  bpos = bconds;

  //printf("num names: %d\n", numpl);
  for (i = 1; i <= conds; i++)
  {

    int consumers = 0;
    int place = read_int();
    //printf("name num: %d\n", place);
    if(i == 1 && place && !strstr(plinunf, plname[place]))
    { 
      //printf("plinunf: %s,", plinunf);
      strcat(plinunf, plname[place]);
    }
    else if(place && !strstr(plinunf, plname[place]))
    {
      strcat(plinunf, ",");
      strcat(plinunf, plname[place]);
    }
    token = read_int();
    queried = read_int();
    int gen = read_int();

    if(!token)
    {
      while (read_int());
      token = queried; // rm warning unused variables
      continue;
    }
    l_reset(&post);
    while (1) {
      e = read_int();
      if (!e) break;
      consumers++;
      l_add(preset+e,i);
      // only consider non-cutoffs for the configuration
      if (!cutoff[e]) l_add(&post,e);
    }

    if (gen) causal(i,gen,post);
    condition_marked(i,place,gen,post);
    conflicts(i,post,gen);
  }

  char **jptr, *needle;
  jptr = &(p_reach->contents);
  for (j = 0; j < p_reach->size; j++)
  {
    needle = strstr(plinunf, jptr[j]);
    if(!needle) {nosense = 1; break;}
  }

  jptr = &(n_reach->contents);
  for (j = 0; j < n_reach->size; j++)
  {
    needle = strstr(plinunf, jptr[j]);
    if(!needle) {nosense = 1; break;}
  }

  //printf("5. nosense: %d\n", nosense);
  if (nosense && !att) return 0;


  if (!opt_reach)
  {
    for (i = 1; i <= events; i++)
      if (!opt_final || !cutoff[i]) disabled(i,preset[i]);
  }
  else
  {
    reachable();
  }


  free(blob);

  // insert correct number of variables and clauses
  fseek(fout,6,SEEK_SET);
  P("%d %d",vars,clauses);
  fclose(fout);
  return 1;
}

void find_targets (clist_t **list, char *string)
{
  char *c, *tmp;
  tmp = strdup(string);
  opt_reach = 1;
  while ((c = strchr(tmp,',')))
  {
    *c = 0;
    c_add(list,tmp);
    tmp = c+1;
    //printf("tmp: %s\n", tmp);
  }
  c_add(list,tmp);
}

void usage ()
{
  fprintf(stderr,
  "\nusage: mci2sat [options] <mcifile>\n\n"
  "  options:\n"
  "\t-c: add comments to sat encoding\n"
  "\t-d: check for deadlock [default]\n"
  "\t-f: find final configurations\n"
  "\t-att: used to find and format attractors\n"
  "\t-r <list>, -n <list>: comma-separated list of place names\n"
  "\t-o <filename>: output file (default: mcifile with .sat)\n"
  "\nIf -r and/or -n are given, mci2sat will generate a formula for\n"
  "reachability checking (-r=positive targets, -n=negative targets).\n\n"
  );
  exit(1);
}

int main (int argc, char ** argv)
{
  int i, correct = 0;
        char outfile[1024];
  char *filename = NULL;

  *outfile = 0;
  c_init(&p_reach);
  c_init(&n_reach);

  for (i = 1; i < argc; i++)
  {
    if (!strcmp(argv[i],"-c"))
      opt_comments = 1;
    else if (!strcmp(argv[i],"-d"))
      opt_final = opt_reach = 0;
    else if (!strcmp(argv[i],"-f"))
      opt_final = 1;
    else if (!strcmp(argv[i],"-att"))
      att = 1;
    else if (!strcmp(argv[i],"-r"))
    {
      if (++i == argc) usage();
      find_targets(&p_reach,argv[i]);
      //print_clist(p_reach);
    }
    else if (!strcmp(argv[i],"-n"))
    {
      if (++i == argc) usage();
      find_targets(&n_reach,argv[i]);
    }
    else if (!strcmp(argv[i],"-o"))
    {
      if (*outfile || ++i == argc) usage();
      strcpy(outfile,argv[i]);
    }
    else
    {
      if (filename) usage();
      filename = argv[i];
    }
  }

  if (!filename) usage();
  if (opt_reach && opt_final) error("cannot use -f with -n or -p");

  if (!*outfile)
  {
    int l = strlen(filename);
    if (l < 5) error("'%s': filename too short!",filename);
    if (l > 1020) error("'%s': filename too long!",filename);
    strcpy(outfile,filename);
    strcpy(outfile + l - 3, "sat");
  }

  correct = mci2sat (filename,outfile);

  return correct;
}

