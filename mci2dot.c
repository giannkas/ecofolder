#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "netconv.h"
#include "unfold.h"

int m_repeat;

void read_mci_file (char *filename)
{
  #define read_int(x) fread(&(x),sizeof(int),1,file)

  FILE *file;
  int numco, numev, numpl, numtr, sz, i;
  int pre_ev, post_ev, cutoff, dummy;
  int *co2pl, *ev2tr, *tokens, *queries_co,
   *queries_ev, *cutoffs;
  char **plname, **trname, *c;

  if (!(file = fopen(filename,"rb")))
  {
    fprintf(stderr,"cannot read file %s\n",filename);
    exit(1);
  }

  printf("digraph test {\n");

  read_int(numco);
  read_int(numev);

  co2pl = malloc((numco+1) * sizeof(int));
  tokens = malloc((numco+1) * sizeof(int));
  queries_co = malloc((numco+1) * sizeof(int));
  queries_ev = malloc((numev+1) * sizeof(int));
  ev2tr = malloc((numev+1) * sizeof(int));
  cutoffs = calloc(numev+1, sizeof(int));

  for (i = 1; i <= numev; i++){
    read_int(ev2tr[i]);
    read_int(queries_ev[i]);
  }

  for (i = 1; i <= numco; i++)
  {
    read_int(co2pl[i]);
    read_int(tokens[i]);
    read_int(queries_co[i]);
    //printf("queries_co i: %d\n", queries_co[i]);
    //printf("token i: %d\n", tokens[i]);
    read_int(pre_ev);
    if (pre_ev) printf("  e%d -> c%d;\n",pre_ev,i);
    do {
      read_int(post_ev);
      if (post_ev) printf("  c%d -> e%d;\n",i,post_ev);
    } while (post_ev);
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

  for (c = plname[i=1]; i <= numpl; c = plname[++i])
    do { fread(c,1,1,file); } while (*c++);
  fread(c,1,1,file);

  for (c = trname[i=1]; c = trname[i], i <= numtr; c = trname[++i])
    do { fread(c,1,1,file); } while (*c++);
  fread(c,1,1,file);

  char color1[] = "lightblue";
  char color2[] = "gold";
  char color3[] = "orange";
  char color4[] = "palegreen";
  char color5[] = "firebrick2";
  char color6[] = "black";

  for (i = 1; i <= numco; i++)
    printf("  c%d [fillcolor=%s label= <%s<FONT COLOR=\"red\"><SUP>%d</SUP></FONT>&nbsp;(c%d)> shape=circle style=filled];\n",
        i,queries_co[i] ? color2 : color1,plname[co2pl[i]],tokens[i],i);
  for (i = 1; i <= numev; i++)
    if (i != cutoffs[i])
      printf("  e%d [fillcolor=%s label=\"%s (e%d)\" shape=box style=filled];\n",
          i,queries_ev[i] ? color3 : color4,trname[ev2tr[i]],i);
    else
      printf("  e%d [color=%s fillcolor=%s label=\"%s (e%d)\" shape=box style=filled];\n",
          i,queries_ev[i] ? color3 : color6,color5,trname[ev2tr[i]],i);
  printf("}\n");

  fclose(file);
}

int main (int argc, char **argv)
{
  int i;
  char *filename;

  //printf("m_repeat: %d\n", m_repeat);
  for (i = 1; i < argc; i++)
    if (!strcmp(argv[i],"-r"))
      m_repeat = atoi(argv[++i]);
    else
      filename = argv[i];

  if (!filename)
  {
    fprintf(stderr,"usage: mci2dot <mcifile>\n");
    exit(1);
  }
  read_mci_file(filename);
  exit(0);
}
