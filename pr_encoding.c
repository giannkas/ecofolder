#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "common.h"

#define MAX_LINE_SIZE 500
#define MAX_READ_PLACES 100
#define MAX_REPLICATED_PLACES_PER_PLACE 50

/*****************************************************************************/

void usage(char *myname)
{
	fprintf(stderr,
		"%s -- place replication encoding for contextual nets\n\n"
		"Usage: %s <LLnetfile> \n\n"

	"Unless specified otherwise, all filenames will default to\n"
	"the basename of <LLnetfile> plus appropriate extensions.\n\n"

	"Version 1.0.0 (22.03.2022)\n", myname, myname);

	exit(1);
}

/*****************************************************************************/


char* pr_encoding(char* in_file){
  // Declare the file pointer
  FILE *r_pointer ;
  FILE *w_pointer = NULL;
    
  // Variables that come along
  char d_read[MAX_READ_PLACES];
  char out_file[MAX_READ_PLACES] = {0};
  char *token, *tmp, *tmp1, *token2;    
  int header = 0, places = 0, trans = 0, new_places = 0, 
    counter_pl = 0, num_tmp, rd_arcs = 0, names = 1;

  // Open the existing file using fopen()
  // in read mode using "r" attribute
  r_pointer = fopen(in_file, "r") ;
    
  // Check if this r_pointer is null
  // which maybe if the file does not exist
  if ( r_pointer == NULL ){
    printf( "%s file failed to open.", in_file ) ;
  }
  else
  {
    char read_place_names[MAX_READ_PLACES][MAX_READ_PLACES] = {0};
    /* Read up to RD label and get number of transitions and places*/
    while(fgets(d_read, MAX_READ_PLACES, r_pointer) != NULL && !strstr(d_read, "RD")){
      if (d_read[0] == 'P' && d_read[1] == 'L');
      else if (places == header) header++;
      if (d_read[0] == 'T' && d_read[1] == 'R');
      else if (trans == places){
        if( d_read[0] == '\"'){
          strcpy(read_place_names[names], d_read);
          names++;
        }
        places++;
      }
      trans++;
    }
    /* Compute header lines, number of places and transitions  */
    header++;
    places = places - header;
    header++;
    trans = trans - places - header;
    printf("header: %d\n", header);
    printf("places: %d\n", places);
    printf("trans: %d\n", trans);

    if(strstr(d_read, "RD")){
      strcat(out_file, ftokstr(in_file, 0, '.'));
      strcat(out_file, "_pr.ll_net");			
      w_pointer = fopen(out_file, "w"); // if we have reset arcs then create a new file.      
      char buf_arcs[MAX_READ_PLACES];
      int read_place_arcs[places+1];
      int read_place_written[places+1];
      int replicated_places_per_place[places+1][MAX_REPLICATED_PLACES_PER_PLACE];
      
      for (size_t i = 0; i <= places; i++){
        read_place_arcs[i] = 0;
        read_place_written[i] = -1;
        for (size_t j = 0; j < MAX_REPLICATED_PLACES_PER_PLACE; j++)
          replicated_places_per_place[i][j] = -1;
      }
      
      
      char buffer_pl[MAX_LINE_SIZE] = {0};
      char buffer_rd[MAX_LINE_SIZE] = {0};

      /* Check for those places that have more than one read arc and make its respective
        replication in buffer_pl which will be pasted into the output file */
      while(fgets(d_read, MAX_READ_PLACES, r_pointer) != NULL && isdigit(d_read[0])){
        strcat(buffer_rd, d_read);
        token = strtok(d_read, ">");
        num_tmp = strtol(token, &token2, 10);
        read_place_arcs[num_tmp]++;
        if(read_place_arcs[num_tmp] > 1){          
          sprintf(buf_arcs, "\"%s_%d\"%s,", 
            ftokstr(read_place_names[num_tmp], 1, '\"'), read_place_arcs[num_tmp], 
              ltokstr(read_place_names[num_tmp], 1, '\"'));
          strcat(buffer_pl, buf_arcs);
          new_places++;
          replicated_places_per_place[num_tmp][read_place_arcs[num_tmp]] = places+new_places;
        }        
      }

      /* Print into the output file the places that come from the input file plus
      those that were replicated (print the content from buffer_pl) */
      new_places = 1;
      fseek( r_pointer, 0, SEEK_SET );
      while(fgets(d_read, MAX_READ_PLACES, r_pointer) != NULL && !strstr(d_read, "TR")){
        token = ftokstr(d_read, 1, '\"');
        if(token){
          if(read_place_arcs[new_places] > 1){
            sprintf(buf_arcs, "\"%s_%d\"%s", token, 1, ltokstr(d_read, 1, '\"'));
            fprintf(w_pointer, "%s", buf_arcs);
          }
          else
            fprintf(w_pointer, "%s", d_read);
          new_places++;
        }
        else
          fprintf(w_pointer, "%s", d_read);
      }
      token = strtok(buffer_pl, ",");
      while (token != NULL){
        fprintf(w_pointer, "%s", token);
        token = strtok(NULL, ",");
      }

      /* TP SECTION */
      /* Any transition t producing p in N produces p_i in N', i.e., p_i in postset(t)*/      
      if(strstr(d_read, "TR")){
        fprintf(w_pointer, "%s", d_read);        
        while(fgets(d_read, MAX_READ_PLACES, r_pointer) && !strstr(d_read, "PT")){          
          if( strlen(d_read) > 2 && isdigit(d_read[0]) ){
            tmp1 = ltokstr(d_read, 0, '<');            
            num_tmp = strtol(tmp1, &token2, 10);
            counter_pl = read_place_arcs[num_tmp]-1;
            for(int i = 1; i <= counter_pl && counter_pl > 0; i++){
              sprintf(buf_arcs, "%s<%d", ftokstr(d_read, 0, '<'), replicated_places_per_place[num_tmp][i+1]);
              fprintf(w_pointer, "%s\n", buf_arcs);
            }
          }
          fprintf(w_pointer, "%s", d_read);
        }
      }
      /* Transition t_i produces place p_i, i.e., p_i in postset(t)*/
      new_places = places;
      if(strstr(d_read, "PT")){
        rd_arcs = 0;
        token = ftokstr(buffer_rd, rd_arcs, '\n'); tmp = "";
        tmp1 = ftokstr(token, 0, '>');
        token2 = ltokstr(token, 0, '>');
        num_tmp = strtol(tmp1, &tmp, 10);
        if(token)
          read_place_written[num_tmp] = read_place_arcs[num_tmp];
        while (strlen(token) != 0){
          if(read_place_written[num_tmp] == read_place_arcs[num_tmp]){
            fprintf(w_pointer, "%s<%s\n", token2, tmp1);
            read_place_written[num_tmp]--;
          }else{
            if (read_place_written[num_tmp] > 0){
              new_places++;
              sprintf(buf_arcs, "%s<%d", token2, new_places);
              fprintf(w_pointer, "%s\n", buf_arcs);
              read_place_written[num_tmp]--;
            }
          }          
          rd_arcs++;
          token = ftokstr(buffer_rd, rd_arcs,'\n');
          tmp1 = ftokstr(token, 0, '>');
          token2 = ltokstr(token, 0, '>');
          num_tmp = strtol(tmp1, &tmp, 10);          
          if(token && read_place_written[num_tmp] == -1){
            read_place_written[num_tmp] = read_place_arcs[num_tmp];}
        }
      }

      /* PT SECTION */
      /* Any transition t consuming p in N consumes p_i in N', i.e., p_i in preset(t)*/      
      if(strstr(d_read, "PT")){
        fprintf(w_pointer, "%s", d_read);
        while(fgets(d_read, MAX_READ_PLACES, r_pointer) != NULL && !strstr(d_read, "RS")){
          if( strlen(d_read) > 2 ){
            tmp1 = ftokstr(d_read, 0, '>');
            num_tmp = strtol(tmp1, &token2, 10);
            counter_pl = read_place_arcs[num_tmp]-1;
            for(int i = 1; i <= counter_pl && counter_pl > 0; i++){
              sprintf(buf_arcs, "%d>%s", replicated_places_per_place[num_tmp][i+1], strtok(ltokstr(d_read, 0, '>'), "\n"));
              fprintf(w_pointer, "%s\n", buf_arcs);              
            }
          }
          fprintf(w_pointer, "%s", d_read);
        }
      }
      /* Transition t_i consumes place p_i, i.e., p_i in preset(t)*/
      new_places = places;
      for (size_t i = 1; i <= places; i++)
        read_place_written[i] = -1;
      if((strstr(d_read, "RS")) || fgets(d_read, MAX_READ_PLACES, r_pointer) == NULL ){        
        //if (fgets(d_read, MAX_READ_PLACES, r_pointer) == NULL) fprintf(w_pointer, "\n");
        rd_arcs = 0;
        token = ftokstr(buffer_rd, rd_arcs, '\n'); tmp = "";
        tmp1 = ftokstr(token, 0, '>');
        token2 = ltokstr(token, 0, '>');
        num_tmp = strtol(tmp1, &tmp, 10);
        if(token)
          read_place_written[num_tmp] = read_place_arcs[num_tmp];
        while (strlen(token) != 0){
          if(read_place_written[num_tmp] == read_place_arcs[num_tmp]){
            fprintf(w_pointer, "%s\n", token);
            read_place_written[num_tmp]--;
          }else{
            if (read_place_written[num_tmp] > 0){
              new_places++;              
              sprintf(buf_arcs, "%d>%s", new_places, token2);
              fprintf(w_pointer, "%s", buf_arcs);
              read_place_written[num_tmp]--;
            }
          }          
          rd_arcs++;
          token = ftokstr(buffer_rd, rd_arcs,'\n');
          tmp1 = ftokstr(token, 0, '>');
          token2 = ltokstr(token, 0, '>');
          num_tmp = strtol(tmp1, &tmp, 10);          
          if(token && read_place_written[num_tmp] == -1){
            read_place_written[num_tmp] = read_place_arcs[num_tmp];}
        }
      }
      /* RS SECTION */
      /* Any transition t resetting p in N resets p_i in N', i.e., p_i in reset(t)*/      
      if(strstr(d_read, "RS")){
        fprintf(w_pointer, "\n%s", d_read);
        while(fgets(d_read, MAX_READ_PLACES, r_pointer) != NULL){
          if( strlen(d_read) > 2 ){
            tmp1 = ftokstr(d_read, 0, '>');
            num_tmp = strtol(tmp1, &token2, 10);
            counter_pl = read_place_arcs[num_tmp]-1;
            for(int i = 1; i <= counter_pl && counter_pl > 0; i++){
              sprintf(buf_arcs, "%d>%s", replicated_places_per_place[num_tmp][i+1], strtok(ltokstr(d_read, 0, '>'), "\n"));
              fprintf(w_pointer, "%s\n", buf_arcs);              
            }
          }
          fprintf(w_pointer, "%s", d_read);
        }
      }
    }
    fclose(r_pointer) ;
    fclose(w_pointer) ;
  }
  return out_file[0] != '\0' ? out_file : in_file;
}

int main (int argc, char **argv)
{
	if (argc != 2){
		fprintf(stderr,"usage: pr_encoding <mcifile>\n");
		exit(1);
	}
	pr_encoding(argv[1]);
	exit(0);
}