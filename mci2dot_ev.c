#include <stdio.h>
#include <stdlib.h>

#include "keyevent.h"

int find_match_matrix(int rows, int cols, int (*matrix)[cols], int pre_ev, int post_ev ){
  size_t i, j;
  if(matrix[pre_ev][post_ev] != post_ev){
    for (i = pre_ev; i < rows; i++){
      for (j = 1; j < cols; j++){
        if( matrix[i][j] > 0)
          return find_match_matrix(rows, cols, matrix, matrix[i][j], post_ev);
      }
    }
  }
  if(i > rows && j > cols) post_ev = 0;
  return post_ev;
}

void read_mci_file_ev (char *filename)
{
	//printf("hola\n");
	#define read_int(x) fread(&(x),sizeof(int),1,file)

	FILE *file;
	int numco, numev, numpl, numtr, sz, i, ev1, ev2;
	int pre_ev, post_ev, cutoff, dummy;
	int *co2pl, *ev2tr, *tokens;
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
	ev2tr = malloc((numev+1) * sizeof(int));
  int (*co_presets)[numev+1] = calloc(numco+1, sizeof *co_presets);
  int (*ev_succs)[numev+1] = calloc(numev+1, sizeof *ev_succs);
  int (*ev_confl)[numev+1] = calloc(numev+1, sizeof *ev_confl);

	for (i = 1; i <= numev; i++)
		read_int(ev2tr[i]);

	for (i = 1; i <= numco; i++)
	{
		read_int(co2pl[i]);
		read_int(tokens[i]);
		read_int(pre_ev);
		do {
      read_int(post_ev);      
      if(pre_ev && post_ev && ev_succs[pre_ev][post_ev] == 0){
        ev_succs[pre_ev][post_ev] = post_ev;
        printf("  e%d -> e%d;\n",pre_ev,post_ev);
      }
      if (post_ev) co_presets[i][post_ev] = post_ev;
		} while (post_ev);
	}

  printf("\n//conflicts\n");  

  for (size_t i = 1; i <= numco; i++){
    for (size_t j = 1; j <= numev-1; j++){
      for (size_t k = j+1; k <= numev; k++){
        ev1 = co_presets[i][j]; ev2 = co_presets[i][k];
        if (ev1 != 0 && ev2 != 0 && ev_confl[ev1][ev2] == 0){
          ev_confl[ev1][ev2] = ev2;
          //printf("  e%d -> e%d [arrowhead=none color=gray60 style=dashed constraint=false];\n",ev1,ev2);          
        }
      }
    }
  }

  for (size_t i = 1; i <= numev; i++){
    for (size_t j = 1; j <= numev; j++){
      if (ev_confl[i][j] != 0){
        for (size_t k = 1; k <= numev; k++){
          if(ev_confl[j][k] != 0){
            int found = find_match_matrix(numev+1, numev+1, ev_succs, i, ev_confl[j][k]);            
            if(found > 0){
              ev_confl[j][k] = 0;
              ev_succs[i][k] = ev_succs[i][k] == 0 ? k : 0;
            }
          }
        } 
      }
    }
  }
  
  for (int i = 1; i <= numev; i++){
    for (int j = 1; j <= numev; j++){
      if (ev_confl[i][j] > 0)
        printf("  e%d -> e%d [arrowhead=none color=gray60 style=dashed constraint=false];\n",i,ev_confl[i][j]);          
    }
  }
  

  printf("\n");
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

	for (c = plname[i=1]; i <= numpl; c = plname[++i])
		do { fread(c,1,1,file); } while (*c++);
	fread(c,1,1,file);

	for (c = trname[i=1]; c = trname[i], i <= numtr; c = trname[++i])
		do { fread(c,1,1,file); } while (*c++);
	fread(c,1,1,file);

	/* for (i = 1; i <= numco; i++)
		printf("  c%d [fillcolor=lightblue label= <%s<FONT COLOR=\"red\"><SUP>%d</SUP></FONT>&nbsp;(c%d)> shape=circle style=filled];\n",
				i,plname[co2pl[i]],tokens[i],i); */
	for (i = 1; i <= numev; i++)
		printf("  e%d [fillcolor=palegreen label=\"%s (e%d)\" shape=box style=filled];\n",
				i,trname[ev2tr[i]],i);
	printf("}\n");

	fclose(file);
}

int main (int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr,"usage: mci2dot_ev <mcifile>\n");
		exit(1);
	}
	read_mci_file_ev(argv[1]);
	exit(0);
}
