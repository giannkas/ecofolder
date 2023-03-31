#include <stdio.h>
#include <stdlib.h>

void read_mci_file (char *filename)
{
  //printf("hola\n");
  #define read_int(x) fread(&(x),sizeof(int),1,file)

  FILE *file;
  int numco, numev, numpl, numtr, sz, i, j;
  int pre_ev, post_ev, cutoff, dummy = 1;
  int *co2pl, *co2coo, *ev2tr, *tokens, *queries_co,
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
  co2coo = calloc(numco+1, sizeof(int));
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
    do {
      read_int(post_ev);
      if (post_ev) printf("  c%d -> e%d;\n",j,post_ev);
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

  char color1[] = "black";
  char color2[] = "hotpink";
  char color3[] = "orange";
  char color4[] = "palegreen";
  char color5[] = "firebrick2";

  for (i = 1; i <= numco; i++)
  {
    printf("  c%d [fillcolor=lightblue label= <", i);
    dummy = 0;
    for (j = i+1; j <= numco && co2coo[i] == co2coo[j]; j++)
    {
      printf("<FONT COLOR =\"%s\">%s</FONT><FONT COLOR=\"red\"><SUP>%d</SUP></FONT>&nbsp;<FONT COLOR=\"%s\">(c%d)</FONT><BR/>",
        queries_co[j] ? color2 : color1, plname[co2pl[j]],tokens[j],queries_co[j] ? color2 : color1,j);
      dummy = 1;
    }
    printf("<FONT COLOR =\"%s\">%s</FONT><FONT COLOR=\"red\"><SUP>%d</SUP></FONT>&nbsp;<FONT COLOR=\"%s\">(c%d)</FONT>> shape=circle style=filled];\n",
      queries_co[i] ? color2 : color1, plname[co2pl[i]],tokens[i],queries_co[i] ? color2 : color1,i);
    if (dummy) i = j-1;
  }
  
  for (i = 1; i <= numev; i++)
    if (i != cutoffs[i])
      printf("  e%d [fillcolor=%s label=\"%s (e%d)\" shape=box style=filled];\n",
          i,queries_ev[i] ? color3 : color4,trname[ev2tr[i]],i);
    else
      printf("  e%d [fillcolor=%s label=\"%s (e%d)\" shape=box style=filled];\n",
          i,queries_ev[i] ? color3 : color5,trname[ev2tr[i]],i);
  printf("}\n");

  fclose(file);
}

int main (int argc, char **argv)
{
  if (argc != 2)
  {
    fprintf(stderr,"usage: mci2dot <mcifile>\n");
    exit(1);
  }
  read_mci_file(argv[1]);
  exit(0);
}
