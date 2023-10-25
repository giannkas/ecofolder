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

int numplaces = 0;

void read_mci_file (char *mcifile, int m_repeat)
{
  #define read_int(x) fread(&(x),sizeof(int),1,mcif)

  FILE *mcif;
  int nqure, nqure_, nquszcut, nquszevscut, szcuts, 
    numco, numev, i, j, gsize;
  int pre_ev, post_ev, dummy = 0;
  int *co2pl, *ev2tr, *tokens, *queries_co,
   *queries_ev;
  //int **evstumps;
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
  gsize = numev+1 >= numco+1 ? numev+1 : numco+1;
  int (*evstumps)[gsize] = calloc(numev+1, sizeof *evstumps);
  // numev+1 rows and gsize cols. matrix to record events' stumps.
  // (events right after the initial cut save it as their stumps).
  // another way to build the same matrix
  /* evstumps = calloc(numev+1, sizeof(int*));
  for (i = 0; i <= numev; i++) {
    evstumps[i] = calloc(gsize, sizeof(int));
  }  */

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
    read_int(ev2tr[i]);
    read_int(queries_ev[i]);
  }

  for (i = 1; i <= numco; i++)
  {
    read_int(co2pl[i]);
    read_int(tokens[i]);
    read_int(queries_co[i]);
    read_int(pre_ev);
    do {
      read_int(post_ev);
      if (!pre_ev && !post_ev && tokens[i]) evstumps[0][i] = co2pl[i]*-1;
      else if (!pre_ev && post_ev && tokens[i])
        evstumps[post_ev][i] = co2pl[i]*-1;
        /* identify between id places (co2pl[i]) and events (pre_ev) */
      else if (post_ev && pre_ev)
        evstumps[post_ev][pre_ev] = pre_ev;
    } while (post_ev);
  }

  dummy = 0;
  for (i = 0; i <= numev; i++)
  {
    if (i == 0) printf("c%d,",i);
    else printf("e%d,", i);
    for (j = 1; j <= gsize; j++)
    {
      if (evstumps[i][j] && !dummy) 
        {printf("%d", evstumps[i][j]); dummy = 1;}
      else if (evstumps[i][j] && dummy)
        printf(",%d", evstumps[i][j]);
    }
    printf("\n");
    dummy = 0;
  }

  fclose(mcif);
}

void usage ()
{
  fprintf(stderr,"usage: mci2evstump <mcifile> \n");
  exit(1);
}

int main (int argc, char **argv)
{
  char* mcifile = NULL;

  if (argc != 2)  usage();
  else if (sizeof(argv[1]) != sizeof(char*)) usage();

  mcifile = argv[1];

  read_mci_file(mcifile, 0);
  exit(0);
}
