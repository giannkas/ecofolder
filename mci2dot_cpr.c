#include <stdio.h>
#include <stdlib.h>

void read_mci_file (char *filename)
{
  //printf("hola\n");
  #define read_int(x) fread(&(x),sizeof(int),1,file)

  FILE *file;
  int numco, numev, numpl, numtr, sz, i, j;
  int pre_ev, post_ev, cutoff, dummy = 1;
  int *co2pl, *co2coo, *ev2tr, *tokens, *cutoffs;
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
  ev2tr = malloc((numev+1) * sizeof(int));
  cutoffs = calloc(numev+1, sizeof(int));

  for (i = 1; i <= numev; i++)
    read_int(ev2tr[i]);

  for (i = 1; i <= numco; i++)
  {
    read_int(co2pl[i]); dummy = 1; j = i;
    if (!co2pl[i]){
      read_int(co2pl[i]);
      //printf("no similarities\n");
      //printf("co2pl[i]: %d\n", co2pl[i]);
      read_int(tokens[i]);
      //printf("tokens[i]: %d\n", tokens[i]);
      read_int(dummy); /* reading null in this case */ 
    }
    else{
      while (co2pl[i] && dummy){
        //printf("similarities\n");
        //printf("co2pl[i]: %d\n", co2pl[i]);
        read_int(tokens[i]);
        //printf("tokens[i]: %d\n", tokens[i]);
        read_int(co2coo[i]);
        dummy = co2coo[i];
        if (co2coo[i]){
          //printf("co2coo[i]: %d\n", co2coo[i]);
          i++;
          read_int(co2pl[i]);
        }
      }
    }
    read_int(pre_ev);
    if (pre_ev)
    { 
      //printf("pre_ev: %d\n", pre_ev);
      printf("  e%d -> c%d;\n",pre_ev,j);
    }
    //else printf("pre_ev: null\n");
    do {
      read_int(post_ev);
      //if (post_ev) printf("post_ev: %d\n", post_ev);
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

  for (i = 1; i <= numco; i++)
  {
    if( co2coo[i] == 0){
      //printf("hola\n");
      printf("  c%d [fillcolor=lightblue label= <%s<FONT COLOR=\"red\"><SUP>%d</SUP></FONT>&nbsp;(c%d)> shape=circle style=filled];\n",
        i,plname[co2pl[i]],tokens[i],i);
    }
    else
    {
      printf("  c%d [fillcolor=lightblue label= <", co2coo[i]);
      for (j = i+1; j <= numco && co2coo[i] == co2coo[j]; j++)
      {
        printf("%s<FONT COLOR=\"red\"><SUP>%d</SUP></FONT>&nbsp;(c%d)<BR/>",
          plname[co2pl[j]],tokens[j], j);
        i = j;
      }
      printf("%s<FONT COLOR=\"red\"><SUP>%d</SUP></FONT>&nbsp;(c%d)> shape=circle style=filled];\n",
        plname[co2pl[co2coo[i]]],tokens[co2coo[i]],co2coo[i]);
    }
  }
  for (i = 1; i <= numev; i++)
    if (i != cutoffs[i])
      printf("  e%d [fillcolor=palegreen label=\"%s (e%d)\" shape=box style=filled];\n",
          i,trname[ev2tr[i]],i);
    else
      printf("  e%d [fillcolor=firebrick2 label=\"%s (e%d)\" shape=box style=filled];\n",
          i,trname[ev2tr[i]],i);
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
