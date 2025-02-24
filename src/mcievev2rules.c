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

/**
 * @brief function that reads sequentially an .mci file format
 * 
 * @param mcifile string that corresponds to the needed mcifile.
 */
void read_mci_file_ev (char *mcifile, char* evevfile)
{
  #define read_int(x) fread(&(x),sizeof(int),1,mcif)
  /* define a micro substitution to read_int.
    &(x) - is the pointer to a block of memory with a minimum
            size of "sizeof(int)*1" bytes.
    sizeof(int) - is the size in bytes of each element to be read.
    1 - is the number of elements, each one with a size of 
        "sizeof(int)" bytes.
    file - is the pointer to a FILE object that specifies an input stream.
  */

  FILE *mcif, *evevf;
  int nqure, nqure_, nquszcut, nquszevscut, szcuts,
    numco, numev, numpl, numtr, idpl, idtr, sz, i, j, prevalue, value, nconfs = 1;
  int pre_ev, post_ev, cutoff, harmful, dummy = 0, m_repeat = 0;
  int *co2pl, *ev2tr, *tokens, *queries_co, *harmfuls;
  char **plname, **trname, *c;
  char valuech[20] = "";
  cut_t **cuts;


  if (!(mcif = fopen(mcifile,"rb")) || !strstr(mcifile, ".mci"))
  {
    fprintf(stderr,"cannot read file %s\n",mcifile);
    exit(1);
  }

  if (evevfile)
  {
    if (!(evevf = fopen(evevfile, "r")) || !strstr(evevfile, ".evev"))
    {
      fprintf(stderr,"cannot read file %s\n",evevfile);
      exit(1);
    }
  }

  read_int(numco); // read from input file the total number of conditions.
  read_int(numev); // read from input file the total number of events.

  co2pl = malloc((numco+1) * sizeof(int)); // reserve empty memory for the total number 
                                           // conditions.
  tokens = malloc((numco+1) * sizeof(int)); // reserve the same amount of empty memory
                                            // to save the particular tokens' conditions.
  queries_co = malloc((numco+1) * sizeof(int)); // reserve the same amount of empty memory
                                            // to save the particular queries' conditions.
  int (*queries_ev)[numev+1] = calloc(numev+1, sizeof *queries_ev); // reserve the same amount of empty memory
                                            // to save the particular queries' events.
  ev2tr = malloc((numev+1) * sizeof(int)); // reserve empty memory for the total number 
                                           // events.

  harmfuls = calloc(numev+1, sizeof(int));
  int (*co_postsets)[numev+1] = calloc(numco+1, sizeof *co_postsets);
                                           // conditions' postsets to detect conflicts in events.
  if (co_postsets == NULL) {
    fprintf(stderr, "Memory allocation failed.\n");
  }
  
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

  if(evevfile)
  {
    int result;
    while ((result = fscanf(evevf," %s",valuech)) != EOF)
      if (!strstr(valuech, "+")) // in case of reading redundant events
      {
        value = strtoint(valuech);
        if (value > 0 && !queries_ev[nconfs][value])
        {
          queries_ev[nconfs][value] = value;
          prevalue = value;
        }
        else if(value == 0)
        {
          queries_ev[nconfs][prevalue+1] = -1;
          nconfs++;
        }
      }
    nconfs--;
  }
  
  for (i = 1; i <= numev; i++){
    read_int(ev2tr[i]);
    read_int(dummy);
  }

  for (i = 1; i <= numco; i++)
  {
    read_int(co2pl[i]);
    read_int(tokens[i]);
    read_int(queries_co[i]);
    read_int(pre_ev);
    do {
      read_int(post_ev);
    } while (post_ev); // if post_ev is not null.
  }
  
  for (;;) {
    read_int(harmful);
    if (!harmful) break;
    harmfuls[harmful] = harmful;
  }
  
  for (;;) {
    read_int(cutoff);
    if (!cutoff) break;
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

  /* Applying the labelling function to each event */
  for (i = 1; i <= nconfs; i++)
  {
    dummy = 0;
    for ( j = 1; j <= numev && queries_ev[i][j] >= 0; j++)
    {
      if (queries_ev[i][j] > 0 && !dummy)
      {
        printf("%s", trname[ev2tr[j]]);
        dummy = 1;
      }
      else if (queries_ev[i][j] > 0)
        printf(" %s", trname[ev2tr[j]]);
    }
    printf("\n");
  }
  
  fclose(mcif);
  fclose(evevf);
}

void usage (char *myname)
{
  fprintf(stderr,
    "%s -- conversion of evevfile to rulesfile"
    "usage: %s -mci <mcifile> -evev <evevfile>\n\n"

    "     Files:\n"
    "      -mci <mcifile>  is a file that contains the model's prefix \n produced by ecofolder. \n"
    "      -evev <evevfile>  is a file that contains a list of configurations (each per line) \n, the numbers correspond to ids of events in the prefix.\n\n"
    
    "Output is sent to standard output.\n\n", myname, myname);
    exit(1);
}

int main (int argc, char **argv)
{
  char *mcifile = NULL, *evevfile = NULL;

  for (int i = 1; i < argc; i++)
    if (!strcmp(argv[i],"-mci"))
    {
      if (++i == argc) usage(argv[0]);
      mcifile = argv[i];
    }
    else if (!strcmp(argv[i], "-evev"))
    {
      if (++i == argc) usage(argv[0]);
      evevfile = argv[i];
    }
  
  if (!mcifile || !evevfile)
    usage(argv[0]);
    
  read_mci_file_ev(mcifile, evevfile);
  exit(0);
}
