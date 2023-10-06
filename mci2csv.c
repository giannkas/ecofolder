#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define LINE_SIZE 100

typedef struct cut_t
{
  int repeat;
  int szcut, szevscut;
  int *cut;
  int *evscut;
} cut_t;

void read_mci_file (char *filename, int m_repeat, int compressed)
{
  #define read_int(x) fread(&(x),sizeof(int),1,file_in)

  FILE *file_in, *file_nodes, *file_edges;
  char fullfilename[LINE_SIZE], *fname;
  int nqure, nqure_, nquszcut, nquszevscut, szcuts, 
    numco, numev, numpl, numtr, sz, i, j;
  int pre_ev, post_ev, cutoff, harmful, dummy = 0, dummy_ = 0;
  int *co2pl, *co2coo, *ev2tr, *tokens, *queries_co,
   *queries_ev, *cutoffs, *harmfuls;
  char **plname, **trname, *c;
  cut_t **cuts;

  if (!(file_in = fopen(filename,"rb")))
  {
    fprintf(stderr,"cannot read file_in %s\n",filename);
    exit(1);
  }

  fname = strtok(filename, ".");
  sprintf(fullfilename,"%s_nodes.csv",fname);
  if (!(file_nodes = fopen(fullfilename,"w")))
  {
    fprintf(stderr,
      "cannot write file_in %s\n",fullfilename);
    exit(1);
  }
  fprintf(file_nodes, "id,type,name,tokens,cutoff\n");

  sprintf(fullfilename,"%s_edges.csv",fname);
  if (!(file_edges = fopen(fullfilename,"w")))
  {
    fprintf(stderr,
      "cannot write file_in %s\n",fullfilename);
    exit(1);
  }
  fprintf(file_edges,"src,dst\n");

  read_int(numco);
  read_int(numev);

  co2pl = malloc((numco+1) * sizeof(int));
  co2coo = calloc(numco+1, sizeof(int));
  tokens = malloc((numco+1) * sizeof(int));
  queries_co = malloc((numco+1) * sizeof(int));
  queries_ev = malloc((numev+1) * sizeof(int));
  ev2tr = malloc((numev+1) * sizeof(int));
  cutoffs = calloc(numev+1, sizeof(int));
  harmfuls = calloc(numev+1, sizeof(int));

  read_int(nqure);
  nqure_ = abs(nqure);
  cuts = calloc((szcuts = nqure_+1), sizeof(cut_t*));
  if(nqure_ && m_repeat > 0 && m_repeat <= nqure_) 
    dummy_ = 1;
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

  for (i = 1; i <= numev; i++){
    read_int(ev2tr[i]);
    read_int(queries_ev[i]);
  }

  for (i = 1; i <= numco; i++)
  {
    read_int(co2pl[i]);
    if(compressed){
      dummy = 1; j = i;
      while (co2pl[i] && dummy){
        read_int(tokens[i]);
        read_int(queries_co[i]);
        read_int(co2coo[i]);
        read_int(dummy);
        if (dummy && i <= numco){
          i++;
          co2pl[i] = dummy;
        }
      }
    }
    else{
      read_int(tokens[i]);
      read_int(queries_co[i]);
    }
    read_int(pre_ev);
    if (pre_ev)
        fprintf(file_edges,"\"e%d\",\"c%d\"\n",pre_ev, i);
    do {
      read_int(post_ev);
      if (post_ev)
        fprintf(file_edges,"\"c%d\",\"e%d\"\n",i,post_ev);
    } while (post_ev);
  }

  if(dummy_)
  {
    if (cuts[m_repeat] && cuts[m_repeat]->repeat < 0)
    {
      memset(queries_ev,0,(numev)*sizeof(int));
      memset(queries_co,0,(numco)*sizeof(int));
      for (i = 1; i <= cuts[m_repeat]->szcut; i++)
        queries_co[cuts[m_repeat]->cut[i]] = 1;
      for (i = 1; i <= cuts[m_repeat]->szevscut; i++)
        queries_ev[cuts[m_repeat]->evscut[i]] = 1;
    }
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

  for (c = plname[i=1]; i <= numpl; c = plname[++i])
    do { fread(c,1,1,file_in); } while (*c++);
  fread(c,1,1,file_in);

  for (c = trname[i=1]; c = trname[i], i <= numtr; c = trname[++i])
    do { fread(c,1,1,file_in); } while (*c++);
  fread(c,1,1,file_in);

  for (i = 1; i <= numco; i++)
  {
    if (compressed)
    {  
      fprintf(file_nodes,"\"c%d",i);
      dummy = 0;
      for (j = i+1; j <= numco && co2coo[i] == co2coo[j]; j++)
      {
        fprintf(file_nodes, ",c%d",j);
        dummy = 1;
      }
      fprintf(file_nodes,"\",\"condition\",\"%s",plname[co2pl[i]]);

      for (j = i+1; j <= numco && co2coo[i] == co2coo[j]; j++)
        fprintf(file_nodes,",%s",plname[co2pl[j]]);
      fprintf(file_nodes,"\",\"%d",tokens[i]);

      for (j = i+1; j <= numco && co2coo[i] == co2coo[j]; j++)
        fprintf(file_nodes,",%d",tokens[j]);
        
      fprintf(file_nodes,"\",\n");
      
      if (dummy) i = j-1;
    }
    else
    {
      fprintf(file_nodes,"\"c%d\",\"condition\",\"%s\",\"%d\",\n",i,plname[co2pl[i]],tokens[i]);
    }
  }

  for (i = 1; i <= numev; i++)
    if (i == cutoffs[i]){
      fprintf(file_nodes,"\"e%d\",\"event\",\"%s\",,\"%d\"\n",i,trname[ev2tr[i]],1);
    }
    else{
      fprintf(file_nodes,"\"e%d\",\"event\",\"%s\",,\"%d\"\n",i,trname[ev2tr[i]],0);
    }

  fclose(file_in);
  fclose(file_nodes);
  fclose(file_edges);
}

int main (int argc, char **argv)
{
  int i, m_repeat = 0, compressed = 0;
  char *filename;

  for (i = 1; i < argc; i++)
    if (!strcmp(argv[i],"-r"))
    {
      if (++i == argc) filename = NULL;
      m_repeat = atoi(argv[i]);
    }
    else if (!strcmp(argv[i],"-c"))
      compressed = 1;
    else
      filename = argv[i];

  if (!filename)
  {
    fprintf(stderr,"usage: mci2csv [options] <mcifile>\n");
    exit(1);
  }
  read_mci_file(filename, m_repeat, compressed);
  exit(0);
}
