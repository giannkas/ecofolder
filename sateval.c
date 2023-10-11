#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <err.h>
#include <stdarg.h>
#include <fcntl.h>
#include <assert.h>

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

int numpl, numtr, events, conds;
int evars;    // highest variable index used for events
int *co2pl;   // maps conditions to place indices
int *ev2tr;   // maps non-cutoff events to transition indices
int *marking; // records the current marking when fetching a reachable marking.
int *cut; // records the current cut when fetching a reachable marking.
int **coxev;  // records the prefix

char **plname, **trname;    // names of conditions/transitions



char opt_reach = 0; // perform reachability (=1) or deadlock (=0)

void readmci (const char * infile)
{
  int i, cutoffs = 0;
  char *blob;
  int *queries_ev, *harmfuls;
  int nqure, nqure_, nquszcut, nquszevscut, szcuts, harmful, token, queried, pre_ev, post_ev;
  cut_t **cuts;

  /* open input and output files */
  struct stat statbuf;
  stat(infile,&statbuf);
  blob = malloc(statbuf.st_size);
  int fd = open(infile,O_RDONLY);
  read(fd,blob,statbuf.st_size);
  close(fd);
  bpos = blob;

  // start reading mci file
  conds = read_int();
  events = read_int();
  /* printf("conds: %d\n", conds);
  printf("events: %d\n", events); */

  // read mapping of events to transitions
  co2pl = malloc((conds+1) * sizeof(int));
  ev2tr = malloc((events+1) * sizeof(int));
  queries_ev = malloc((events+1) * sizeof(int));
  harmfuls = calloc(events+1, sizeof(int));
  coxev = calloc(conds+1, sizeof(int*));
  for (i = 0; i <= conds; i++) {
    coxev[i] = calloc(events+1, sizeof(int));
  }

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

  // read events
  for (i = 1; i <= events; i++)
  {
    ev2tr[i] = read_int();
    queries_ev[i] = read_int();
  }

  // read conditions
  for (i = 1; i <= conds; i++)
  {
    co2pl[i] = read_int();
    token = read_int();
    queried =  read_int();
    pre_ev =  read_int();

    if(!token)
    {
      while (read_int());
      token = queried; // rm warning unused variables
      continue;
    }

    post_ev = read_int();
    if(pre_ev) coxev[i][pre_ev] = co2pl[i];
    else if (!pre_ev && !post_ev) coxev[i][0] = co2pl[i];
    while(token && post_ev)
    {
      if (!pre_ev){ 
        coxev[i][0] = co2pl[i];
        coxev[i][post_ev] = co2pl[i]*-1;
      }
      else coxev[i][post_ev] = co2pl[i]*-1;
      post_ev = read_int();
    }
  }

  /* for (i = 1; i <= conds; i++){
    if (i<10) printf("%d:  ", i);
    else printf("%d: ", i);
    for (int j = 0; j <= events; j++)
      printf("%d ", coxev[i][j]);
    printf("\n");
  } */

  for (;;) {
    harmful = read_int();
    if (!harmful) break;
    harmfuls[harmful] = harmful;
  }

  // skip cutoff information
  while ((i = read_int())) { cutoffs++; read_int(); }
  evars = events - cutoffs;
  // dummy ints followed by number of places and transitions
  while (read_int());
  numpl = read_int();
  numtr = read_int();
  read_int();

  // read place names
  plname = malloc((numpl+1) * sizeof(char*));
  for (i = 1; i <= numpl; i++) { plname[i] = bpos; read_str(); }
  bpos++;
  // read transition names
  trname = malloc((numtr+1) * sizeof(char*));
  for (i = 1; i <= numtr; i++) { trname[i] = bpos; read_str(); }
}

int sateval (char *satfile, int data)
{
  char *tmpname, *idx, *evcofile;

  FILE *f;

  if (!(f = fopen(satfile,"r")))
  {
    fprintf(stderr,"cannot read file %s\n",satfile);
    exit(1);
  }

  tmpname = malloc(strlen(satfile)+5);
  strcpy(tmpname, satfile);
  idx = strrchr(tmpname,'.');
  if (!idx) strcat(tmpname,".");
  idx = strrchr(tmpname,'.');
  strcpy(idx, ".evco");
  evcofile = strdup(tmpname);
  FILE *fo = fopen(evcofile,"w");

  int i, place;
  marking = calloc(numpl+1, sizeof(int));
  cut = calloc(numpl+1, sizeof(int));
  char c = getc(f);
  int v;

  if (c == 'U')
  {
    if (opt_reach && data)
      printf("Marking unreachable.\n");
    else if (data)
      printf("The net is alive.\n");
    fclose(f);
    return 0;
  }
  else
  {
    while (getc(f) != '\n');
    c = getc(f);
    fseek(f, -1, SEEK_CUR);

    if (c == ' ')
    {
      if (opt_reach && data) printf("Marking unreachable.\n");
      fclose(f);
      return 0;
    }
    else if (opt_reach && data)
      printf("Marking reachable:\n");
    else if (data)
      printf("Deadlock sequence:");
  }

  for (i = 1; i <= conds; i++)
    if (coxev[i][0]){
      marking[coxev[i][0]] = coxev[i][0];
      cut[coxev[i][0]] = i;
    }

  while (fscanf(f," %d",&v) != EOF)
  {
    int vnr = abs(v);
    if (!vnr) break;
    if (vnr > evars) break;

    if (v < 0) continue;
    if (data) printf(" %s (e%d)",trname[ev2tr[v]],v);
    fprintf(fo,"%d ",v);
    for (i = 1; i <= conds; i++)
    {
      if (coxev[i][v])
      {
        place = abs(coxev[i][v]);
        if (coxev[i][v] < 0 && marking[place] == place)
          {marking[place] = 0; cut[place] = 0;}
        else
          {marking[place] = place; cut[place] = i;}
      }
    }
  }
  if (data) printf("\n\t");
  fprintf(fo,"0\n");
  for (i = 1; i <= numpl; i++)
  {
    if (marking[i] > 0 ){
      if (data) printf("%s (c%d) ", plname[i], cut[i]);
      fprintf(fo,"%d ", cut[i]);
    }
  }
  if (data) printf("\n");
  fprintf(fo,"0\n");

  fclose(f);
  fclose(fo);
  return 1;
}

void usage ()
{
  fprintf(stderr,
  "\nusage: sateval {-d|-r} <mcifile> <satfile>\n\n"
  "  options:\n"
  "\t-d or -r: output for deadlock or reachability checking\n"
  "\t-data: no data printed but evco file is still created.\n\n"
  );
  exit(1);
}

int main (int argc, char ** argv)
{
  char outfile[1024];
  int reachable = 0;
  int data = 1;
  char *mcifile = NULL;
  char *resfile = NULL;

  *outfile = 0;

  for (int i = 1; i < argc; i++)
    if (argc != 4 && argc != 5) usage();
    else if (!strcmp(argv[i],"-r")) opt_reach = 1;
    else if (!strcmp(argv[i],"-data")) data = 0;
    else if (i == 3) mcifile = argv[i];
    else resfile = argv[i];

  /* if (strcmp(argv[1],"-d") && strcmp(argv[1],"-r")) usage();
  if (!strcmp(argv[1],"-r")) opt_reach = 1; */
  
  readmci(mcifile);
  reachable = sateval(resfile, data);
  //printf("sateval is: %d\n", reachable);

  return reachable;
}
