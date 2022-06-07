#include <stdio.h>
#include <stdlib.h>

#include "keyevent.h"

//void find_conflict(int rows, int cols, int (*matrix)[cols],  )


/**
 * @brief auxiliary function to find event successors of a particular event.
 * 
 * @param rows is the number of rows that the *matrix has.
 * @param cols is the number of cols that the *matrix has.
 * @param matrix matrix variable to look in for the event.
 * @param pre_ev the event source whose successor (post_ev) is needed.
 * @param post_ev the event target which it is a successor of (pre_ev).
 * @return integer we the event's index number if found else 0.
 */

int find_successor(int rows, int cols, int (*ev_succs)[cols], int pre_ev, int post_ev ){
  size_t j, found = 0;
  if(ev_succs[pre_ev][post_ev] != post_ev){ // if post_ev is not in the 
                                          //immediate successors of pre_ev.
    // we do loops to search in breadth.
    for ( j = pre_ev+1; j < cols && found == 0; j++){
      if( ev_succs[pre_ev][j] > 0)
        found = find_successor(rows, cols, ev_succs, ev_succs[pre_ev][j], post_ev);
      //if (found > 0) ev_succs[pre_ev][post_ev] = found;    
    }
  }
  else
    found = post_ev;
  return found;
}

int find_conflict(int rows, int cols, int (*ev_confl)[cols],
  int (*ev_confl_copy)[cols], int(*ev_succs)[cols], int ev_cfl, int ev_src ){
  size_t k;
  int found = 0;
  for (k = ev_cfl+1; k < cols && found == 0; k++){
    if(ev_confl[ev_cfl][k] != 0){
      found = find_successor(rows, cols, ev_succs, ev_src, ev_confl[ev_cfl][k]);
      if(found > 0){
        ev_confl_copy[ev_cfl][k] = 0;
      }
      else 
        ev_confl_copy[ev_cfl][k] = find_conflict(rows, cols, ev_confl, ev_confl_copy, ev_succs, 
            ev_confl[ev_cfl][k], ev_src) > 0 ? 0 : k;
      //if (k > ev_src) ev_succs[ev_src][k] = ev_succs[ev_src][k] == 0 ? k : 0;
    }
  }
  return found;
}

void display_matrix(int rows, int cols, int (*matrix)[cols]){
  int i, j;
  for (i=1; i<rows; i++)
  {
      for(j=1; j<cols; j++)
      {
        // upper triangular matrix
        /* if(j >= i){
          printf("%d   ", matrix[i][j]);
        }else{
          printf("    ");
        } */

        // lower triangular matrix
        // if(i >= j) printf("%d   ", matrix[i][j]);
        //printf("%*c", j, ' ');
        // complete matrix
        int tmp = matrix[i][j];
        if (tmp <= 9) printf("%d   ", tmp);
        else printf("%d  ", tmp);
      }
      printf("\n");
  }
}

/**
 * @brief function that reads sequentially an .mci file format
 * 
 * @param filename string that corresponds to the needed filename.
 */
void read_mci_file_ev (char *filename)
{
	#define read_int(x) fread(&(x),sizeof(int),1,file)
  /* define a micro substitution to read_int.
    &(x) - is the pointer to a block of memory with a minimum
            size of "sizeof(int)*1" bytes.
    sizeof(int) - is the size in bytes of each element to be read.
    1 - is the number of elements, each one with a size of 
        "sizeof(int)" bytes.
    file - is the pointer to a FILE object that specifies an input stream.
  */

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

	printf("digraph test {\n"); // start to creating the output 
                              //  file in dot format.

	read_int(numco); // read from input file the total number of conditions.
	read_int(numev); // read from input file the total number of events.

	co2pl = malloc((numco+1) * sizeof(int)); // reserve empty memory for the total number 
                                           // conditions.
	tokens = malloc((numco+1) * sizeof(int)); // reserve the same amount of empty memory
                                            // to save the particular tokens' conditions.
	ev2tr = malloc((numev+1) * sizeof(int)); // reserve empty memory for the total number 
                                           // events.
  int (*co_postsets)[numev+1] = calloc(numco+1, sizeof *co_postsets); 
                                           // conditions' postsets to detect conflicts in events.
  int (*ev_succs)[numev+1] = calloc(numev+1, sizeof *ev_succs); // matrix to record events' successors.
  int (*ev_confl)[numev+1] = calloc(numev+1, sizeof *ev_confl); // matrix to record events' conflicts.
  int (*ev_confl_copy)[numev+1] = calloc(numev+1, sizeof *ev_confl_copy); // a copy of the previous variable.

  for (i = 1; i <= numev; i++){
    read_int(ev2tr[i]); // assign a value to an entry in the array ev2tr for every event
                        // in the unfolding in order to map its respective 
                        // transition, eg., ev1 -> tr3 (ev2tr[1] -> 3)
  }

	for (i = 1; i <= numco; i++)
	{
		read_int(co2pl[i]); // assign a value to the ith entry co2pl array for
                        // every condition in the unfolding in order to
                        // map its respective place, eg., c2 -> pl4 
                        // (co2pl[2] -> 4)
		read_int(tokens[i]); // assign a value to the ith entry tokens array
                         // for every condition in the unfolding in order to
                         // keep track of the conditions that are empty or full
                         // with tokens due to reset arcs.
		read_int(pre_ev); // read the respective event number (mark attribute
                      // in event_t structure definition), if any, which is the only
                      // event in the preset of a condition in the unfolding.
		do {
      read_int(post_ev);  // read the respective event number (mark attribute
                          // in event_t structure definition), if any, which 
                          // are the events in the postset of a condition in
                          // the unfolding.
      //printf("post_ev: %d\n", post_ev);
      if(pre_ev && post_ev && ev_succs[pre_ev][post_ev] == 0){ // check if a 
                                                               // value hasn't
                                                               // been assigned yet
                                                               // and if pre_ev and
                                                               // post_ev are not null
        ev_succs[pre_ev][post_ev] = post_ev; // assign in the entry pre_ev of matrix 
                                             // ev_succs the connection 
                                             // between pre_ev and post_ev
                                             // with the value of post_ev itself.
                                             // The idea is to have a record
                                             // of pre_ev's successors.
        printf("  e%d -> e%d;\n",pre_ev,post_ev); // write the connection.
      }
      if (post_ev) co_postsets[i][post_ev] = post_ev; // assign in the ith
                                                      // (which corresponds
                                                      // to the ith condition)
                                                      // entry of matrix
                                                      // co_postsets in the
                                                      // position post_ev
                                                      // the value post_ev itself.
                                                      // this keep track of the postset
                                                      // of a particular conditions
                                                      // to detect conflicts.
		} while (post_ev); // if post_ev is not null.
	}
  printf("\n//conflicts\n");  
  
  // A loop over co_postsets matrix to fill ev_confl matrix with conflicts
  // among events part of the same condition's postset. We make a copy 
  // conflicts to find immediate conflicts correctly in the function right after.
  for (size_t i = 1; i <= numco; i++){
    for (size_t j = 1; j <= numev-1; j++){
      for (size_t k = j+1; k <= numev; k++){
        ev1 = co_postsets[i][j]; ev2 = co_postsets[i][k];
        if (ev1 != 0 && ev2 != 0 && ev_confl[ev1][ev2] == 0){
          ev_confl[ev1][ev2] = ev2;
          //ev_confl[ev2][ev1] = ev2;
          ev_confl_copy[ev1][ev2] = ev2;
          //ev_confl_copy[ev2][ev1] = ev2;
          //printf("  e%d -> e%d [arrowhead=none color=gray60 style=dashed constraint=false];\n",ev1,ev2);          
        }
      }
    }
  }

  // A loop over ev_confl matrix to find two pairwise conflict between events, eg.,
  // e2->e3 and e3->e4, thereafter with find_successor we use ev_confl_succs to check 
  // if e4 is a successor of e2, if yes we remove the conflict between e4 and e3
  // because the conflict is inherited from e2 and e3, so we leave only immediate
  // conflicts. Finally, we use the matrix ev_confl_copy to remove those "redundant
  // conflicts" while ev_confl remains unchangeable to keep the trace of conflicts
  // for posterior relations.
  //display_matrix(numev+1, numev+1, ev_confl_copy);
  for (size_t i = 1; i <= numev; i++)
    for (size_t j = i+1; j <= numev; j++)
      if(ev_succs[i][j] == 0)
        ev_succs[i][j] = find_successor(numev+1, numev+1, ev_succs, i, j);
  for (size_t i = 1; i <= numev; i++){
    for (size_t j = i+1; j <= numev; j++){
      if(ev_confl[i][j] > 0){
        for(size_t k = i+1; k <= numev; k++)
          if(ev_succs[i][k] > 0){
            if(k > j) ev_confl_copy[j][k] = 0;
            else ev_confl_copy[k][j] = 0;
          }
            //{ev_confl_copy[j][k] = k > j ? ev_confl[j][k] > 0;}
        for(size_t k = j+1; k <= numev; k++)
          if(ev_succs[j][k] > 0){
            if(k > i) ev_confl_copy[i][k] = 0;
            else ev_confl_copy[k][i] = 0;
          }

        for (size_t k = i+1; k <= numev; k++)
          for (size_t m = j+1; m <= numev; m++)
            if(ev_succs[i][k] > 0 && ev_succs[j][m] > 0){
              if(k > m) ev_confl_copy[m][k] = 0;
              else ev_confl_copy[k][m] = 0;
            }
      }
    }
  }
  
  
  //display_matrix(numev+1, numev+1, ev_succs);
  //printf("####################################################\n");
  //display_matrix(numev+1, numev+1, ev_confl_copy);
  // After leaving only immediate conflicts we do a loop over ev_confl
  // to write in the output file those conflict relations.
  for (int i = 1; i <= numev; i++){
    for (int j = i+1; j <= numev; j++){
      if (ev_confl_copy[i][j] > 0)
        printf("  e%d -> e%d [arrowhead=none color=gray60 style=dashed constraint=false];\n",i,ev_confl_copy[i][j]);          
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
