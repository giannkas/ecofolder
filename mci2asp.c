#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct cut_t
{
  int repeat;
  int szcut, szevscut;
  int *cut;
  int *evscut;
} cut_t;

void read_mci_file (char *mcifile, int m_repeat, const char* ns)
{
  #define read_int(x) fread(&(x),sizeof(int),1,mcif)

  FILE *mcif, *evcof;
  int nqure, nqure_, nquszcut, nquszevscut, szcuts, 
    numco, numev, numpl, numtr, sz, i, value;
  int pre_ev, post_ev, cutoff, harmful, dummy = 0;
  int *co2pl, *ev2tr, *tokens, *cut0, *queries_co,
   *queries_ev, *cutoffs, *harmfuls, *queries_coset;
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
  cut0 = calloc(numco+1, sizeof(int));
  queries_co = malloc((numco+1) * sizeof(int));
  queries_ev = malloc((numev+1) * sizeof(int));
  queries_coset = malloc((numco+1) * sizeof(int));
  ev2tr = malloc((numev+1) * sizeof(int));
  cutoffs = calloc(numev+1, sizeof(int));
  harmfuls = calloc(numev+1, sizeof(int));

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
    printf("event((%s,e%d)).\n", ns, i);
    read_int(ev2tr[i]);
    read_int(queries_ev[i]);
  }

  for (i = 1; i <= numco; i++)
  {
    read_int(co2pl[i]);
    read_int(tokens[i]);
    read_int(queries_co[i]);
    read_int(pre_ev);
    if (pre_ev) printf("edge((%s,e%d),(%s,c%d)).\n",ns,pre_ev,ns,i);
    do {
      read_int(post_ev);
      if (post_ev) printf("edge((%s,c%d),(%s,e%d)).\n",ns,i,ns,post_ev);
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
    printf("cutoff((%s,e%d)).\n", ns, cutoff);
    read_int(dummy);
  }

  do { read_int(dummy); } while(dummy);
  read_int(numpl);
  read_int(numtr);
  read_int(sz);

  plname = malloc((numpl+2) * sizeof(char*));
  trname = malloc((numtr+2) * sizeof(char*));
  for (i = 1; i <= numpl+1; i++) plname[i] = malloc(sz+1);
  for (i = 1; i <= numtr+1; i++) trname[i] = malloc(sz+1);

  for (c = plname[i=1]; i <= numpl; c = plname[++i])
    do { fread(c,1,1,mcif); } while (*c++);
  fread(c,1,1,mcif);

  for (c = trname[i=1]; c=trname[i], i <= numtr; c = trname[++i])
    do { fread(c,1,1,mcif); } while (*c++);
  fread(c,1,1,mcif);

  for (i = 1; i <= numco; i++)
    printf("h((%s,c%d),p%d).\n", ns,i,co2pl[i]);
  for (i = 1; i <= numev; i++)
    printf("h((%s,e%d),t%d).\n", ns,i,ev2tr[i]);

  for (i = 1; i <= numpl; i++) {
        printf("name(p%d,\"%s\").\n",i,plname[i]);
    }
  for (i = 1; i <= numtr; i++)
        printf("name(t%d,\"%s\").\n",i,trname[i]);

  fclose(mcif);
}

int main (int argc, char **argv)
{
  if (argc != 2 && argc != 3)
  {
    fprintf(stderr,"usage: mci2asp <mcifile> [<ns>]\n");
    exit(1);
  }
  char* ns = argc == 3 ? argv[2] : "pi0";
  read_mci_file(argv[1], 0, ns);
  exit(0);
}
