#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct cut_t
{
  int repeat;
  int szcut, szevscut;
  int *cut;
  int *evscut;
} cut_t;

void read_mci_file (char *filename, int m_repeat)
{
  #define read_int(x) fread(&(x),sizeof(int),1,file)

  FILE *file;
  int nqure, nqure_, nquszcut, nquszevscut, szcuts, 
    numco, numev, numpl, numtr, idpl, idtr, sz, i, j;
  int pre_ev, post_ev, cutoff, harmful, dummy = 0, dummy_ = 0;
  int *co2pl, *co2coo, *ev2tr, *tokens, *cut0, *queries_co,
    *queries_ev, *cutoffs, *harmfuls;
  char **plname, **trname, *c;
  cut_t **cuts;

  if (!(file = fopen(filename,"rb")))
  {
    fprintf(stderr,"cannot read file %s\n",filename);
    exit(1);
  }

  printf("digraph test {\n");

  read_int(numco);
  read_int(numev);

  co2pl = malloc((numco+1) * sizeof(int));
  co2coo = calloc(numco+1, sizeof(int));
  tokens = malloc((numco+1) * sizeof(int));
  cut0 = calloc(numco+1, sizeof(int));
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
    read_int(pre_ev);
    if (pre_ev)
      printf("  e%d -> c%d;\n",pre_ev,j);
    if (!pre_ev) cut0[i] = 1;
    do {
      read_int(post_ev);
      if (post_ev) printf("  c%d -> e%d;\n",j,post_ev);
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

  for (i=1; i <= numpl; i++)
  {
    read_int(idpl);
    c = plname[idpl];
    do { fread(c,1,1,file); } while (*c++);
  }
  fread(c,1,1,file);

  for (i=1; i <= numtr; i++)
  {
    read_int(idtr);
    c = trname[idtr];
    do { fread(c,1,1,file); } while (*c++);
  }
  fread(c,1,1,file);

  char color0[] = "transparent";
  char color1[] = "black";
  char color2[] = "orangered";
  char color3[] = "orange";
  char color4[] = "#cce6cc"; // or "palegreen";
  char color5[] = "cornflowerblue";
  char color6[] = "firebrick2";
  char color7[] = "#ccccff"; // or "lightblue";
  char color8[] = "#4040ff";
  char color9[] = "#409f40";


  for (i = 1; i <= numco; i++)
  {
    printf("  c%d [color= \"%s\"", i, color8);
    dummy = 0;
    for (j = i+1; j <= numco && co2coo[i] == co2coo[j]; j++)
    {
      if (j+1 <= numco && co2coo[i] == co2coo[j] && co2coo[i] != co2coo[j+1])
      {
        printf("fillcolor=\"%s\" label= <", cut0[j] ? color0 : color7);
        dummy = 1;
      }
    }
    if (!dummy) printf("fillcolor=\"%s\" label= <", cut0[i] ? color0 : color7);
    dummy = 0;
    for (j = i+1; j <= numco && co2coo[i] == co2coo[j]; j++)
    {
      printf("<FONT COLOR =\"%s\">%s</FONT><FONT COLOR=\"red\"><SUP>%d</SUP></FONT><FONT COLOR=\"%s\"> (c%d)</FONT><BR/>",
        queries_co[j] ? color2 : color1, plname[co2pl[j]],tokens[j],queries_co[j] ? color2 : color1,j);
      dummy = 1;
    }
    printf("<FONT COLOR =\"%s\">%s</FONT><FONT COLOR=\"red\"><SUP>%d</SUP></FONT><FONT COLOR=\"%s\"> (c%d)</FONT>> shape=circle style=filled];\n",
      queries_co[i] ? color2 : color1, plname[co2pl[i]],tokens[i],queries_co[i] ? color2 : color1,i);
    if (dummy) i = j-1;
  }
  
  for (i = 1; i <= numev; i++)
    if (i == harmfuls[i])
      printf("  e%d [color=%s fillcolor=%s label=\"%s (e%d)\" shape=box style=filled];\n",
          i,queries_ev[i] ? color3 : color1,color6,trname[ev2tr[i]],i);
    else if (i == cutoffs[i])
      printf("  e%d [color=%s fillcolor=%s label=\"%s (e%d)\" shape=box style=filled];\n",
          i,queries_ev[i] ? color3 : color1,color5,trname[ev2tr[i]],i);
    else
      printf("  e%d [color=\"%s\" fillcolor=\"%s\" label=\"%s (e%d)\" shape=box style=filled];\n",
          i,queries_ev[i] ? color1 : color9,queries_ev[i] ? color3 : color4,trname[ev2tr[i]],i);
  printf("}\n");

  fclose(file);
}

int main (int argc, char **argv)
{
  int i, m_repeat = 0;
  char *filename;

  for (i = 1; i < argc; i++)
    if (!strcmp(argv[i],"-r"))
      m_repeat = atoi(argv[++i]);
    else
      filename = argv[i];

  if (!filename)
  {
    fprintf(stderr,"usage: mci2dot_cpr <mcifile>\n");
    exit(1);
  }
  read_mci_file(filename, m_repeat);
  exit(0);
}
